#include <RtFilter.h>

using namespace std;

RtFilter::RtFilter(FragmentCombiner fragment_combiner)
{
    // 把合并拼接的结果赋值给这个对象
    m_merge_result = fragment_combiner.GetMergeResult();
}

RtFilter::RtFilter()
{
}

void RtFilter::Set(FragmentCombiner fragment_combiner)
{
    // 清空上次的结果
    map<shared_ptr<cardiolipin_ptr_pair>, list<spec_stru_ptr>>().swap(m_merge_result);
    // 把合并拼接的结果赋值给这个对象
    m_merge_result = fragment_combiner.GetMergeResult();
}

void RtFilter::Set(FragmentCombiner* fragment_combiner_ptr)
{
    // 清空上次的结果
    map<shared_ptr<cardiolipin_ptr_pair>, list<spec_stru_ptr>>().swap(m_merge_result);

    // 把合并拼接的结果赋值给这个对象
    m_merge_result = fragment_combiner_ptr->GetMergeResult();
}

void RtFilter::Filter()
{
    // 去除离群值
    RemoveOutlier();

    // 合并mz和rt相近的心磷脂
    MergeSameCardiolipin();

    // 去除不符合碳数规律的心磷脂
    CarbonNumberLaw();

    // 进一步去除离群值
    RemoveOutlier();

    // 发送信号
    FragmentCombiner::PrintTypesCount();

    // 发送已经完成的信号
    emit Done();
}

void RtFilter::SetParameter(Parameter& par)
{
    m_rt_tor = par.m_same_cardiolipin_rt_tor;
}

void RtFilter::RemoveOutlier()
{
    // 通过四分位数去除某种类型心磷脂的离群值
    auto RemoveSingleType = [this](Type type = cl) {
        // 存储所有心磷脂的rt
        vector<float> rt_vector;

        // 遍历对应类型心磷脂，加入rt
        for (auto itr : m_merge_result) {
            if (GetType(itr.first) == type) {
                rt_vector.push_back(GetRt(itr.first));
            }
        }

        // 如果这个类型的心磷脂的个数小于等于1个，直接返回
        if (rt_vector.size() <= 1) {
            return;
        }

        // 计算数据的q1和q3，就是计算四分位数
        int size = rt_vector.size();

        // 整数集从小到大排序
        sort(rt_vector.begin(), rt_vector.end());

        // 删除最大值和最小值
        rt_vector.pop_back();
        rt_vector.erase(rt_vector.begin());

        // 中位数和相应的索引
        int mid, mid1, mid3;
        float median1, median3;

        mid = size / 2;

        // mid1 = size % 2 == 0 ? (mid-1)/2 : mid/2;
        mid1 = mid / 2;
        mid3 = size % 2 == 0 ? (mid + mid1) : (mid + mid1 + 1);

        median1 = mid % 2 == 0 ? (rt_vector[mid1] + rt_vector[mid1 - 1]) / 2.0
                               : rt_vector[mid1];
        median3 = mid % 2 == 0 ? (rt_vector[mid3] + rt_vector[mid3 - 1]) / 2.0
                               : rt_vector[mid3];

        // 计算iqr，和上下限
        float iqr = median3 - median1;
        float min_rt_tor = median1 - 1.5 * iqr;
        float max_rt_tor = median3 + 1.5 * iqr;

        // 去除不在上下限之内的心磷脂
        for (auto itr = m_merge_result.begin(); itr != m_merge_result.end();) {
            if (GetType(itr->first) == type) {
                // 提取rt
                float rt = GetRt(itr->first);
                // 如果不在上下限之内，则去除
                if (rt<min_rt_tor | rt> max_rt_tor) {
                    itr = m_merge_result.erase(itr);
                } else {
                    itr++;
                }
            } else {
                itr++;
            }
        }
    };

    // 去除这三种类型的心磷脂的离群值
    RemoveSingleType(cl);
    RemoveSingleType(mlcl);
    RemoveSingleType(dlcl);
}

void RtFilter::MergeSameCardiolipin()
{
    // 存储top最多的结果
    pair<shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>> max_top_fix_store;
    // 存储合并相同心磷脂后的结果
    map<std::shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>> new_merge_result;

    // 两个循环遍历合并的结果
    for (auto first_itr = m_merge_result.begin(); first_itr != m_merge_result.end(); first_itr++) {
        // 初始化
        max_top_fix_store = (*first_itr);

        // 第二个迭代器从first_itr的下一个开始
        for (auto second_itr = next(first_itr); second_itr != m_merge_result.end();) {
            // 需要是同一类心磷脂才做比较
            if (GetType(max_top_fix_store.first) == GetType(second_itr->first)) {
                bool check_compound = GetCompound(max_top_fix_store.first) == GetCompound(second_itr->first); // 检查化合物的链长和不饱和度是否一致
                bool check_rt = fabs(GetRt(max_top_fix_store.first) - GetRt(second_itr->first)) <= m_rt_tor; // 检查时间是否一致
                bool check_fa = GetTop1UniqueFaSet(first_itr->second) == GetTop1UniqueFaSet(second_itr->second); // 检查FA组成是否一致
                // 如果都符合，则更新max_top_fix_store为M_H面积更高的那一个，然后把second_itr去除
                if (check_compound && check_rt && check_fa) {
                    // 更新max_top_fix_store，保存M_H面积更高的那一个
                    if (GetM_HArea(second_itr->first) > GetM_HArea(first_itr->first)) {
                        max_top_fix_store.first = second_itr->first;
                        max_top_fix_store.second = second_itr->second;
                    }
                    second_itr = m_merge_result.erase(second_itr);
                }
                // 如果都不符合，则跳过
                else {
                    second_itr++;
                }
            }
            // 如果不是同一类，则跳过
            else {
                second_itr++;
            }
        }

        // 插入到新的结果中
        new_merge_result[max_top_fix_store.first] = max_top_fix_store.second;
    }

    // 更新结果
    m_merge_result = new_merge_result;
}

void RtFilter::CarbonNumberLaw()
{
    // 碳数规律的结果
    vector<pair<std::shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>>> cl_res, mlcl_res, dlcl_res;

    // 对单一类别的心磷脂进行碳数规律过滤
    auto CarbonNumberLawSingleType = [this, &cl_res, &mlcl_res, &dlcl_res](Type type = cl) {
        // 需要处理的数据
        vector<SimpleCardiolipin> cardiolipin_vector;

        // 把对应类比欸的心磷脂加入cardiolipin_vector中
        for (auto itr : m_merge_result) {
            if (GetType(itr.first) == type) {
                cardiolipin_vector.push_back(SimpleCardiolipin(itr.first, itr.second));
            }
        }

        // 进行筛查
        while (1) {
            // 计算每个心磷脂的分数
            for (unsigned int i = 0; i < cardiolipin_vector.size(); i++) {
                for (unsigned int j = i + 1; j < cardiolipin_vector.size(); j++) {
                    cardiolipin_vector[i].FollowCarbonNumberLaw(cardiolipin_vector[j]); // 判断两者是否符合碳数规律
                }
            }
            // 计算每个心磷脂的最终分数
            for (auto itr = cardiolipin_vector.begin(); itr != cardiolipin_vector.end(); itr++) {
                itr->CalculateFinalScore();
            }

            if (AllFallowCarbonNumberLaw(cardiolipin_vector)) {
                break;
            } else {
                // 找出cardiolipin_vector中最小分数者
                auto min_itr = std::min_element(cardiolipin_vector.begin(), cardiolipin_vector.end(), [](SimpleCardiolipin& a, SimpleCardiolipin& b) { return a.GetScore() < b.GetScore(); });
                // 去除这个最小分数者
                cardiolipin_vector.erase(min_itr);

                // 重置score和ComparisonTimes
                for (auto itr = cardiolipin_vector.begin(); itr != cardiolipin_vector.end(); itr++) {
                    itr->RetSetScore();
                    itr->RetSetComparisonTimes();
                }
            }
        }

        // 根据不同的情况加入不同的容器中
        if (type == cl) {
            for (auto& itr : cardiolipin_vector) {
                cl_res.push_back({ itr.m_cardiolinpin_pair_ptr, itr.m_spec_stru_ptr_list });
            }
        } else if (type == mlcl) {
            for (auto& itr : cardiolipin_vector) {
                mlcl_res.push_back({ itr.m_cardiolinpin_pair_ptr, itr.m_spec_stru_ptr_list });
            }

        } else if (dlcl) {
            for (auto& itr : cardiolipin_vector) {
                dlcl_res.push_back({ itr.m_cardiolinpin_pair_ptr, itr.m_spec_stru_ptr_list });
            }
        }
    };

    // 对三种类别的心磷脂进行碳数规律
    CarbonNumberLawSingleType(cl);
    CarbonNumberLawSingleType(mlcl);
    CarbonNumberLawSingleType(dlcl);

    // 存储碳数规律后所有心磷脂后的结果
    map<std::shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>> new_merge_result;

    // 加入到新结果中
    for (auto itr : cl_res) {
        new_merge_result[itr.first] = itr.second;
    }
    for (auto itr : mlcl_res) {
        new_merge_result[itr.first] = itr.second;
    }
    for (auto itr : dlcl_res) {
        new_merge_result[itr.first] = itr.second;
    }

    // 覆盖原来的结果
    m_merge_result = new_merge_result;
}

std::set<std::array<int, 3>> RtFilter::GetTop1UniqueFaSet(std::list<spec_stru_ptr> list)
{
    return (*list.begin())->GetUniqueFaSet();
}

SimpleCardiolipin::SimpleCardiolipin()
{
}

SimpleCardiolipin::~SimpleCardiolipin()
{
}

SimpleCardiolipin::SimpleCardiolipin(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolinpin_pair_ptr, std::list<spec_stru_ptr> spec_stru_ptr_list)
{
    Workflow w_help;
    m_chain_length = w_help.GetChainLength(cardiolinpin_pair_ptr);
    m_unsaturation = w_help.GetUnsaturation(cardiolinpin_pair_ptr);
    m_oxygen = w_help.GetOxygen(cardiolinpin_pair_ptr);
    m_rt = w_help.GetRt(cardiolinpin_pair_ptr);

    m_cardiolinpin_pair_ptr = cardiolinpin_pair_ptr;
    m_spec_stru_ptr_list = spec_stru_ptr_list;
}

int SimpleCardiolipin::GetChainLength()
{
    return m_chain_length;
}

int SimpleCardiolipin::GetUnsaturation()
{
    return m_unsaturation;
}

int SimpleCardiolipin::GetOxygen()
{
    return m_oxygen;
}

float SimpleCardiolipin::GetRt()
{
    return m_rt;
}

void SimpleCardiolipin::SetRt(float rt)
{
    m_rt = rt;
}

float SimpleCardiolipin::GetScore()
{
    return m_score;
}

void SimpleCardiolipin::SetScore(float score)
{
    m_score = score;
}

int SimpleCardiolipin::GetComparisonTimes()
{
    return m_comparison_times;
}

void SimpleCardiolipin::SetComparisonTimes(int comparison_time)
{
    m_comparison_times = comparison_time;
}

void SimpleCardiolipin::CalculateFinalScore()
{
    this->m_score = this->m_score / (this->m_comparison_times + 1);
}

void SimpleCardiolipin::RetSetScore()
{
    this->m_score = 1;
}

void SimpleCardiolipin::RetSetComparisonTimes()
{
    this->m_comparison_times = 0;
}

void SimpleCardiolipin::FollowCarbonNumberLaw(SimpleCardiolipin& other_cardiolipin)
{
    // 如果链长相同
    if (this->m_chain_length == other_cardiolipin.GetChainLength()) {
        // 如果不饱和度和氧个数都相同，则不比较
        if ((this->m_unsaturation == other_cardiolipin.GetUnsaturation()) && (this->m_oxygen == other_cardiolipin.GetOxygen())) {
            return;
        }
        // 如果氧个数相同，不饱和度不同，则需要不饱和度和rt的方向相反，说明服从碳数规律；反之，不服从
        else if ((this->m_oxygen == other_cardiolipin.GetOxygen()) && (this->m_unsaturation != other_cardiolipin.GetUnsaturation())) {
            if ((this->m_unsaturation > other_cardiolipin.GetUnsaturation() && this->m_rt < other_cardiolipin.GetRt()) || (this->m_unsaturation < other_cardiolipin.GetUnsaturation() && this->m_rt > other_cardiolipin.GetRt())) {
                this->m_score++;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
            } else {
                this->m_score--;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
            }
            // 比较的次数加1
            this->m_comparison_times++;
            other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        }
        // 如果氧个数不同，不饱和度相同，则需要氧个数和rt的方向相反，说明服从碳数规律；反之，不服从
        else if ((this->m_oxygen != other_cardiolipin.GetOxygen()) && (this->m_unsaturation == other_cardiolipin.GetUnsaturation())) {
            if ((this->m_oxygen > other_cardiolipin.GetOxygen() && this->m_rt < other_cardiolipin.GetRt()) || (this->m_oxygen < other_cardiolipin.GetOxygen() && this->m_rt > other_cardiolipin.GetRt())) {
                this->m_score++;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
            } else {
                this->m_score--;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
            }
            // 比较的次数加1
            this->m_comparison_times++;
            other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        }
        // //如果不饱和度和氧个数都不相同，则需要不饱和度和氧个数都方向一致，rt跟它们反向相反，则说明服从碳数规律；如果不饱和度和氧个数方向不一致，无法比较
        // else if((this->m_unsaturation != other_cardiolipin.GetUnsaturation())&&(this->m_oxygen != other_cardiolipin.GetOxygen())){
        //     //服从碳数规律的情况
        //     if((this->m_unsaturation > other_cardiolipin.GetUnsaturation() && this->m_oxygen > other_cardiolipin.GetOxygen() && this->m_rt < other_cardiolipin.GetRt()) || (this->m_unsaturation < other_cardiolipin.GetUnsaturation() && this->m_oxygen < other_cardiolipin.GetOxygen() && this->m_rt > other_cardiolipin.GetRt())){
        //         this->m_score++;
        //         other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
        //         //比较的次数加1
        //         this->m_comparison_times++;
        //         other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        //     }
        //     //不服从碳数规律的情况
        //     else if((this->m_unsaturation > other_cardiolipin.GetUnsaturation() && this->m_oxygen > other_cardiolipin.GetOxygen() && this->m_rt >= other_cardiolipin.GetRt()) || (this->m_unsaturation < other_cardiolipin.GetUnsaturation() && this->m_oxygen < other_cardiolipin.GetOxygen() && this->m_rt <= other_cardiolipin.GetRt())){
        //         this->m_score--;
        //         other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
        //         //比较的次数加1
        //         this->m_comparison_times++;
        //         other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        //     }
        // }
    }

    // 如果不饱和度相同
    else if (this->m_unsaturation == other_cardiolipin.GetUnsaturation()) {
        // 如果链长和氧个数都相同，则不比较
        if ((this->m_chain_length == other_cardiolipin.GetChainLength()) && (this->m_oxygen == other_cardiolipin.GetOxygen())) {
            return;
        }
        // 如果氧个数相同，链长不同，则需要链长和rt的方向相同，说明服从碳数规律；反之，不服从
        else if ((this->m_oxygen == other_cardiolipin.GetOxygen()) && (this->m_chain_length != other_cardiolipin.GetChainLength())) {
            if ((this->m_chain_length > other_cardiolipin.GetChainLength() && this->m_rt > other_cardiolipin.GetRt()) || (this->m_chain_length < other_cardiolipin.GetChainLength() && this->m_rt < other_cardiolipin.GetRt())) {
                this->m_score++;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
            } else {
                this->m_score--;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
            }
            // 比较的次数加1
            this->m_comparison_times++;
            other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        }
        // 如果氧个数不同，链长相同，则需要氧个数和rt的方向相反，说明服从碳数规律；反之，不服从
        else if ((this->m_oxygen != other_cardiolipin.GetOxygen()) && (this->m_chain_length == other_cardiolipin.GetChainLength())) {
            if ((this->m_oxygen > other_cardiolipin.GetOxygen() && this->m_rt < other_cardiolipin.GetRt()) || (this->m_oxygen < other_cardiolipin.GetOxygen() && this->m_rt > other_cardiolipin.GetRt())) {
                this->m_score++;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
            } else {
                this->m_score--;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
            }
            // 比较的次数加1
            this->m_comparison_times++;
            other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        }
        // //如果链长和氧个数都不相同，则需要链长和氧个数都方向不一致，rt跟氧个数反向相反，和链长方向相同，则说明服从碳数规律；如果链长和氧个数方向不一致，无法比较
        // else if((this->m_chain != other_cardiolipin.GetChain())&&(this->m_oxygen != other_cardiolipin.GetOxygen())){
        //     //服从碳数规律的情况
        //     if((this->m_chain < other_cardiolipin.GetChain() && this->m_oxygen > other_cardiolipin.GetOxygen() && this->m_rt < other_cardiolipin.GetRt()) || (this->m_chain > other_cardiolipin.GetChain() && this->m_oxygen < other_cardiolipin.GetOxygen() && this->m_rt > other_cardiolipin.GetRt())){
        //         this->m_score++;
        //         other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
        //         //比较的次数加1
        //         this->m_comparison_times++;
        //         other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        //     }
        //     //不服从碳数规律的情况
        //     else if((this->m_chain < other_cardiolipin.GetChain() && this->m_oxygen > other_cardiolipin.GetOxygen() && this->m_rt >= other_cardiolipin.GetRt()) || (this->m_chain > other_cardiolipin.GetChain() && this->m_oxygen < other_cardiolipin.GetOxygen() && this->m_rt <= other_cardiolipin.GetRt())){
        //         this->m_score--;
        //         other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
        //         //比较的次数加1
        //         this->m_comparison_times++;
        //         other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        //     }
        // }
    }

    // 如果氧个数相同
    else if (this->m_oxygen == other_cardiolipin.GetOxygen()) {
        // 如果链长和不饱和度都相同，则不比较
        if ((this->m_chain_length == other_cardiolipin.GetChainLength()) && (this->m_unsaturation == other_cardiolipin.GetUnsaturation())) {
            return;
        }
        // 如果不饱和度相同，链长不同，则需要链长和rt的方向相同，说明服从碳数规律；反之，不服从
        else if ((this->m_unsaturation == other_cardiolipin.GetUnsaturation()) && (this->m_chain_length != other_cardiolipin.GetChainLength())) {
            if ((this->m_chain_length > other_cardiolipin.GetChainLength() && this->m_rt > other_cardiolipin.GetRt()) || (this->m_chain_length < other_cardiolipin.GetChainLength() && this->m_rt < other_cardiolipin.GetRt())) {
                this->m_score++;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
            } else {
                this->m_score--;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
            }
            // 比较的次数加1
            this->m_comparison_times++;
            other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        }
        // 如果不饱和度不同，链长相同，则需要氧个数和rt的方向相反，说明服从碳数规律；反之，不服从
        else if ((this->m_unsaturation != other_cardiolipin.GetUnsaturation()) && (this->m_chain_length == other_cardiolipin.GetChainLength())) {
            if ((this->m_unsaturation > other_cardiolipin.GetUnsaturation() && this->m_rt < other_cardiolipin.GetRt()) || (this->m_unsaturation < other_cardiolipin.GetUnsaturation() && this->m_rt > other_cardiolipin.GetRt())) {
                this->m_score++;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() + 1);
            } else {
                this->m_score--;
                other_cardiolipin.SetScore(other_cardiolipin.GetScore() - 1);
            }
            // 比较的次数加1
            this->m_comparison_times++;
            other_cardiolipin.SetComparisonTimes(other_cardiolipin.GetComparisonTimes() + 1);
        }
    }
}

bool AllFallowCarbonNumberLaw(std::vector<SimpleCardiolipin>& cardiolipin_vector)
{
    // 如果其中有分数小于1的，则返回false；否则，返回true
    for (auto itr = cardiolipin_vector.begin(); itr != cardiolipin_vector.end(); itr++) {
        if (itr->GetScore() < 1) {
            return false;
        }
    }
    return true;
}
