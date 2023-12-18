#include <HeadgroupFinder.h>

using namespace std;

HeadgroupFinder::HeadgroupFinder()
{
}

HeadgroupFinder::HeadgroupFinder(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector)
    : Workflow(cardiolipin_pair_vector)
{
}

void HeadgroupFinder::FindMS2Headgroup()
{
    // 头基的mz
    float headgroup_mz = 152.9963485799;

    // 所容许的误差
    float tolerable_diff = headgroup_mz * m_ppm / 1000000;

    // 计算头基分数所需要的常量
    float constant = -((1.17741 / pow((m_ppm_with_half_score) / static_cast<double>(1000000), 2)) / 2);

    // lamba:对心磷脂的二级进行头基的查找
    auto FindHeadgroup = [&headgroup_mz, &tolerable_diff, &constant, this](cardiolipin_ptr cardiolipin) {
        // 如果这个心磷脂是无效的，则直接返回
        if (!cardiolipin->CheckValid()) {
            return;
        }
        // 获取二级向量
        vector<ms2_ptr> ms2_ptr_v = cardiolipin->GetMS2PtrVector();

        // 遍历所有的二级
        for (auto ms2 = ms2_ptr_v.begin(); ms2 != ms2_ptr_v.end();) {
            // 清空上次搜索的头基的结果
            (*ms2)->ClearHeadgroup();
            bool find_head_group = 0; // 判断是否找到了头基
            float max_intensity = (*ms2)->GetMaxFragmentIntensity(); // 这个二级中最大的intensity

            auto fragment_mz = (*ms2)->GetFragmentIonMz(); // 二级碎片的mz，是经过排序的
            auto fragment_intensity = (*ms2)->GetFragmentIonIntensity(); // 二级碎片的intensity

            // 二分查找头基
            int left = 0;
            int right = fragment_mz->size() - 1;
            while (left <= right) {
                int mid = (left + right) / 2;
                // 最小值和最大值
                float min_mz = fragment_mz->at(mid) - tolerable_diff;
                float max_mz = fragment_mz->at(mid) + tolerable_diff;
                // 如果符合要求
                if (headgroup_mz >= min_mz && headgroup_mz <= max_mz) {
                    find_head_group = 1; // 设定这个二级具有头基
                    float real_ppm = (fragment_mz->at(mid) - headgroup_mz) / headgroup_mz; // 计算真实的ppm
                    float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                    float intensity_score = fragment_intensity->at(mid) / max_intensity; // intensity分数
                    float total_score = this->m_mz_score_weight * mz_score + (1 - this->m_mz_score_weight) * intensity_score; // 最终分数
                    headgroup_ptr headgroup = make_shared<Headgroup>(fragment_mz->at(mid), fragment_intensity->at(mid), total_score); // 新建头基
                    (*ms2)->EmplaceBackHeadgroup(headgroup); // 加入这个二级中

                    // 向左和向右继续寻找
                    int left_t = mid - 1;
                    int right_t = mid + 1;
                    // 向左寻找
                    while (left_t >= 0) {
                        float left_t_min_mz = fragment_mz->at(left_t) - tolerable_diff;
                        float left_t_max_mz = fragment_mz->at(left_t) + tolerable_diff;
                        if (headgroup_mz >= left_t_min_mz && headgroup_mz <= left_t_max_mz) {
                            float real_ppm = (fragment_mz->at(left_t) - headgroup_mz) / headgroup_mz; // 计算真实的ppm
                            float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                            float intensity_score = fragment_intensity->at(left_t) / max_intensity; // intensity分数
                            float total_score = this->m_mz_score_weight * mz_score + (1 - this->m_mz_score_weight) * intensity_score; // 最终分数
                            headgroup_ptr headgroup = make_shared<Headgroup>(fragment_mz->at(left_t), fragment_intensity->at(left_t), total_score); // 新建头基
                            (*ms2)->EmplaceBackHeadgroup(headgroup); // 加入这个二级中
                            left_t--; // 向左继续找
                        } else {
                            break;
                        }
                    }

                    // 向右寻找
                    while (right_t <= int(fragment_mz->size() - 1)) {
                        float right_t_min_mz = fragment_mz->at(right_t) - tolerable_diff;
                        float right_t_max_mz = fragment_mz->at(right_t) + tolerable_diff;
                        if (headgroup_mz >= right_t_min_mz && headgroup_mz <= right_t_max_mz) {
                            float real_ppm = (fragment_mz->at(right_t) - headgroup_mz) / headgroup_mz; // 计算真实的ppm
                            float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                            float intensity_score = fragment_intensity->at(right_t) / max_intensity; // intensity分数
                            float total_score = this->m_mz_score_weight * mz_score + (1 - this->m_mz_score_weight) * intensity_score; // 最终分数
                            headgroup_ptr headgroup = make_shared<Headgroup>(fragment_mz->at(right_t), fragment_intensity->at(right_t), total_score); // 新建头基
                            (*ms2)->EmplaceBackHeadgroup(headgroup); // 加入这个二级中
                            right_t++; // 向右边继续找
                        } else {
                            break;
                        }
                    }

                    break;
                }
                // 如果头基mz比最大值大，说明配对值在右侧
                else if (headgroup_mz >= max_mz) {
                    left = mid + 1;
                }
                // 如果头基mz比最小值小，说明配对值在左侧
                else if (headgroup_mz <= min_mz) {
                    right = mid - 1;
                }
            }
            // 二分查找头基

            // 如果这个二级没有找到头基，则删除这个二级
            if (find_head_group) {
                ++ms2;
            } else {
                ms2 = ms2_ptr_v.erase(ms2);
            }
        }

        // 重新把二级赋值，不然不是原来删除掉部分二级的二级向量
        cardiolipin->SetMS2PtrVector(ms2_ptr_v);

        // 如果这个心磷脂的所有二级都没有找到头基，则清空这个心磷脂的信息
        if (ms2_ptr_v.size() == 0) {
            cardiolipin->Clear();
        }
    };

    // 进行查找头基
    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end();) {

        FindHeadgroup(itr->first);
        FindHeadgroup(itr->second);
        // 如果M-H和M-2H都没有找到头基，则删除这对心磷脂
        if (!itr->first->CheckValid() && !itr->second->CheckValid()) {
            itr = m_cardiolipin.erase(itr);
        } else {
            ++itr;
        }
    }

    // 删除由于查找头基所带来的冗余心磷脂
    this->DeleteRedundantCardiolipinPair();

    // 发送信号
    this->PrintTypesCount();
}

void HeadgroupFinder::SetParameter(Parameter& par)
{
    m_ppm = par.m_headgroup_ppm;
    m_ppm_with_half_score = par.m_headgroup_ppm_with_half_score;
    m_mz_score_weight = par.m_headgroup_mz_score_weight;
}
