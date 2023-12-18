#include <cardiolipin.h>

using namespace std;

Cardiolipin::Cardiolipin()
{
    m_ms1_ptr = nullptr;
    m_ms1_db_ptr = nullptr;
    m_ms2_ptr_v = vector<ms2_ptr>();
}

Cardiolipin::Cardiolipin(ms1_ptr ms1_ptr, db_ptr db_ptr)
{
    m_ms1_ptr = ms1_ptr;
    m_ms1_db_ptr = db_ptr;
}

void Cardiolipin::ScoreMS1(float constant)
{
    if (CheckValid()) {
        float real_ppm = (this->m_ms1_ptr->GetMz() - this->m_ms1_db_ptr->GetMz()) / (this->m_ms1_db_ptr->GetMz()); // 计算真实的ppm
        this->m_ms1_matching_score = exp(constant * pow(real_ppm, 2)); // 返回分数
    }
}

float Cardiolipin::GetMS1Mz()
{
    if (!CheckValid()) {
        return 0;
    }

    return m_ms1_ptr->GetMz();
}

float Cardiolipin::GetMS1Rt()
{
    if (!CheckValid()) {
        return 0;
    }

    return m_ms1_ptr->GetRt();
}

float Cardiolipin::GetMS1Intensity()
{
    if (!CheckValid()) {
        return 0;
    }

    return m_ms1_ptr->GetIntensity();
}

float Cardiolipin::GetMS1Area()
{
    if (!CheckValid()) {
        return 0;
    }

    return m_ms1_ptr->GetArea();
}

bool Cardiolipin::CheckValid()
{
    if (m_ms1_ptr == nullptr && m_ms1_db_ptr == nullptr) {
        return 0;
    } else {
        return 1;
    }
}

void Cardiolipin::Clear()
{
    m_ms1_ptr = nullptr;
    m_ms1_db_ptr = nullptr;
    m_ms1_matching_score = 0;
    vector<ms2_ptr>().swap(m_ms2_ptr_v);
    list<spec_stru_ptr>().swap(m_specific_structure_vector);
}

Type Cardiolipin::GetType()
{
    return cardiolipin;
}

std::list<spec_stru_ptr> Cardiolipin::GetSpecificStructureList()
{
    return m_specific_structure_vector;
}

std::shared_ptr<Cardiolipin> Cardiolipin::Clone()
{
    qDebug() << "Class Cardiolipin call this 'Clone' function!!!";
    return make_shared<Cardiolipin>();
}

std::list<spec_stru_ptr> Cardiolipin::GetNewSpecificStructureList()
{
    list<spec_stru_ptr> res;

    for (auto itr : m_specific_structure_vector) {
        res.push_back(make_shared<SpecificStructure>(*itr));
    }

    return res;
}

void Cardiolipin::AddMS2(ms2_ptr ms2_ptr)
{
    m_ms2_ptr_v.push_back(ms2_ptr);
}

bool Cardiolipin::HasMS2()
{
    return (m_ms2_ptr_v.size() != 0);
}

const ms1_ptr Cardiolipin::GetMS1Ptr()
{
    return m_ms1_ptr;
}

const db_ptr Cardiolipin::GetMS1DbPtr()
{
    return m_ms1_db_ptr;
}

int Cardiolipin::GetChainLength()
{
    return m_ms1_db_ptr->GetChainLength();
}

int Cardiolipin::GetUnsaturation()
{
    return m_ms1_db_ptr->GetUnsaturation();
}

int Cardiolipin::GetOxygen()
{
    return m_ms1_db_ptr->GetOxygen();
}

std::array<int, 3> Cardiolipin::GetCompound()
{
    if (CheckValid()) {
        return { GetChainLength(), GetUnsaturation(), GetOxygen() };
    }

    return { 0, 0, 0 };
}

std::vector<ms2_ptr> Cardiolipin::GetMS2PtrVector()
{
    return m_ms2_ptr_v;
}

void Cardiolipin::SetMS2PtrVector(std::vector<ms2_ptr> ms2_ptr_v)
{
    m_ms2_ptr_v = ms2_ptr_v;
}

void Cardiolipin::splice()
{
    qDebug() << "Class Cardiolipin call this 'splice' function!!!";
}

std::vector<std::array<int, 4>> Cardiolipin::FourSum(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target)
{
    int target_chain_length = target[0];
    int target_unsaturation = target[1];
    int target_oxygen = target[2];

    int fa_vector_size = fa_ptr_vector.size();
    vector<array<int, 4>> res;

    // 寻找哪4个FA可以组成这个CL
    for (int i = 0; i < fa_vector_size; i++) {
        for (int j = i; j < fa_vector_size; j++) {
            int left = j;
            int right = fa_vector_size - 1;
            while (left <= right) {
                // 如果符合要求
                if ((calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_chain_length) && (calculate_total_unsaturation({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_oxygen)) {
                    // 则加入结果中
                    res.push_back({ i, j, left, right });

                    // 因为有可能有相同的FA，所以需要left继续推进寻找
                    int left_t = left;
                    while ((left_t < fa_vector_size - 1) && (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_chain_length) && (calculate_total_unsaturation({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_oxygen)) {
                        res.push_back({ i, j, left_t + 1, right });
                        left_t++;
                    }
                    // right需要移动
                    right--;
                }
                // 如果所有链长加起来大于目标链长，说明大了，right需要向左移
                else if (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) > target_chain_length) {
                    right--;
                }
                // 如果所有链长加起来小于目标链长，说明小了，right需要向右移
                else if (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) < target_chain_length) {
                    left++;
                }
                // 如果链长加起来等于目标链长，但是其他属性加起来不等于目标属性，则向left向右寻找是否有符合要求的，寻找完right--
                else if (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_chain_length) {
                    int left_t = left;
                    while (left_t + 1 < right) {
                        if ((calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_chain_length) && (calculate_total_unsaturation({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector.at(i), fa_ptr_vector.at(j), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_oxygen)) {
                            res.push_back({ i, j, left_t + 1, right });
                        }
                        left_t++;
                    }
                    right--;
                }
            }
        }
    }

    return res;
}

std::vector<std::array<int, 2>> Cardiolipin::TwoSum(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target)
{
    int target_chain_length = target[0];
    int target_unsaturation = target[1];
    int target_oxygen = target[2];

    vector<array<int, 2>> res;
    // 哈系法解两数之和

    // 以链长为键，对应的fa为位置构建多重哈希表
    multimap<unsigned int, int> hash_map;

    for (unsigned int i = 0; i < fa_ptr_vector.size(); i++) {
        // 插入数据
        hash_map.insert({ fa_ptr_vector[i]->GetChainLength(), i });
        // 寻找剩余链长的键的范围
        auto search_pair_itr = hash_map.equal_range(target_chain_length - fa_ptr_vector[i]->GetChainLength());
        // 如果能够找到
        if (search_pair_itr.first != search_pair_itr.second) {
            // 遍历搜索到的范围
            for (auto match_itr = search_pair_itr.first; match_itr != search_pair_itr.second; match_itr++) {
                // 如果链长相加起来等于目标链长的情况下，如果不饱和度和氧个数加起来都等于目标的这些属性，加入到结果中
                if ((calculate_total_unsaturation({ fa_ptr_vector[i], fa_ptr_vector.at(match_itr->second) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector[i], fa_ptr_vector.at(match_itr->second) }) == target_oxygen)) {
                    // 加入到结果中
                    res.push_back({ int(i), match_itr->second });
                }
            }
        }
    }

    return res;
}

std::vector<std::array<int, 2>> Cardiolipin::TwoSum(std::vector<pa_ptr>& pa_ptr_vector, std::array<int, 3> target)
{
    int target_chain_length = target[0];
    int target_unsaturation = target[1];
    int target_oxygen = target[2];

    vector<array<int, 2>> res;
    // 哈系法解两数之和

    // 以链长为键，对应的fa为位置构建多重哈希表
    multimap<unsigned int, int> hash_map;

    for (unsigned int i = 0; i < pa_ptr_vector.size(); i++) {
        // 插入数据
        hash_map.insert({ pa_ptr_vector[i]->GetChainLength(), i });
        // 寻找剩余链长的键的范围
        auto search_pair_itr = hash_map.equal_range(target_chain_length - pa_ptr_vector[i]->GetChainLength());
        // 如果能够找到
        if (search_pair_itr.first != search_pair_itr.second) {
            // 遍历搜索到的范围
            for (auto match_itr = search_pair_itr.first; match_itr != search_pair_itr.second; match_itr++) {
                // 如果链长相加起来等于目标链长的情况下，如果不饱和度和氧个数加起来都等于目标的这些属性，加入到结果中
                if ((calculate_total_unsaturation({ pa_ptr_vector[i], pa_ptr_vector.at(match_itr->second) }) == target_unsaturation) && (calculate_total_oxygen({ pa_ptr_vector[i], pa_ptr_vector.at(match_itr->second) }) == target_oxygen)) {
                    // 加入到结果中
                    res.push_back({ int(i), match_itr->second });
                }
            }
        }
    }

    return res;
}

std::vector<std::array<int, 3>> Cardiolipin::ThreeSum(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target)
{
    int target_chain_length = target[0];
    int target_unsaturation = target[1];
    int target_oxygen = target[2];

    vector<std::array<int, 3>> res;
    int fa_vector_size = fa_ptr_vector.size();
    for (int i = 0; i < fa_vector_size; i++) {
        int left = i;
        int right = fa_vector_size - 1;
        while (left <= right) {

            if ((calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_chain_length) && (calculate_total_unsaturation({ fa_ptr_vector.at(i), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector.at(i), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_oxygen)) {
                res.push_back({ i, left, right });
                int left_t = left;

                while ((left_t < fa_vector_size - 1) && (fa_ptr_vector.at(left_t)->GetCompound() == fa_ptr_vector.at(left_t + 1)->GetCompound())) {
                    res.push_back({ i, left_t + 1, right });
                    left_t++;
                }
                right--;
            }
            // 如果所有链长加起来大于目标链长，说明大了，right需要向左移
            else if (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) > target_chain_length) {
                right--;
            }
            // 如果所有链长加起来小于目标链长，说明小了，right需要向右移
            else if (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) < target_chain_length) {
                left++;
            }
            // 如果链长加起来等于目标链长，但是其他属性加起来不等于目标属性，则向left向右寻找是否有符合要求的，寻找完right--
            else if (calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(left), fa_ptr_vector.at(right) }) == target_chain_length) {
                int left_t = left;
                int right_t = right;
                while (left_t + 1 < right_t) {

                    if ((calculate_total_chain({ fa_ptr_vector.at(i), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_chain_length) && (calculate_total_unsaturation({ fa_ptr_vector.at(i), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector.at(i), fa_ptr_vector.at(left_t + 1), fa_ptr_vector.at(right) }) == target_oxygen)) {
                        res.push_back({ i, left_t + 1, right });
                    }
                    left_t++;
                }
                right--;
            }
        }
    }

    return res;
}

std::vector<std::array<int, 2>> Cardiolipin::TwoSum_Not_Repeat(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target)
{
    int target_chain_length = target[0];
    int target_unsaturation = target[1];
    int target_oxygen = target[2];

    vector<array<int, 2>> res;
    // 哈系法解两数之和

    // 以链长为键，对应的fa为位置构建多重哈希表
    multimap<unsigned int, int> hash_map;

    for (unsigned int i = 0; i < fa_ptr_vector.size(); i++) {
        // 寻找剩余链长的键的范围
        auto search_pair_itr = hash_map.equal_range(target_chain_length - fa_ptr_vector[i]->GetChainLength());
        // 插入数据
        hash_map.insert({ fa_ptr_vector[i]->GetChainLength(), i });
        // 如果能够找到
        if (search_pair_itr.first != search_pair_itr.second) {
            // 遍历搜索到的范围
            for (auto match_itr = search_pair_itr.first; match_itr != search_pair_itr.second; match_itr++) {
                // 如果链长相加起来等于目标链长的情况下，如果不饱和度和氧个数加起来都等于目标的这些属性，加入到结果中
                if ((calculate_total_unsaturation({ fa_ptr_vector[i], fa_ptr_vector.at(match_itr->second) }) == target_unsaturation) && (calculate_total_oxygen({ fa_ptr_vector[i], fa_ptr_vector.at(match_itr->second) }) == target_oxygen)) {
                    // 加入到结果中
                    res.push_back({ int(i), match_itr->second });
                }
            }
        }
    }

    return res;
}

void Cardiolipin::MergeSplice()
{
    list<spec_stru_ptr> non_repeating_only_pa_list; // 存储只有PA和FA的对象，用list来存储
    list<spec_stru_ptr> non_repeating_only_fa_list; // 存储只有PA和FA的对象，用list来存储
    list<spec_stru_ptr> non_repeating_have_both_pa_fa_list; // 存储既有PA和又有FA的对象，用list来存储
    list<spec_stru_ptr> compare_to_have_both_pa_fa_list; // 存储合并后仅有PA和仅有FA的对象

    // 把只有PA的合并，只有FA的合并，两者都有的合并
    for (auto first_itr = m_specific_structure_vector.begin(); first_itr != m_specific_structure_vector.end(); first_itr++) {
        // 只有PA
        if (!(*first_itr)->CheckFaExist()) {
            for (auto second_itr = next(first_itr); second_itr != this->m_specific_structure_vector.end();) {
                if (!(*second_itr)->CheckFaExist()) {
                    // 如果这两个Cl相同，进行这个操作的时候，first_itr对应的Cl内部会发生改变
                    if ((*first_itr)->CopyFrom(*second_itr)) {
                        // 如果这两个Cl相同，那么从m_specific_structure_vector中去除第二个Cl
                        second_itr = this->m_specific_structure_vector.erase(second_itr);
                    } else {
                        second_itr++;
                    }
                } else {
                    second_itr++;
                }
            }
            non_repeating_only_pa_list.push_back(*first_itr); // 加入到object_only_pa_only_fa_list中
        }
        // 只有PA
        else if (!(*first_itr)->CheckPaExist()) {
            for (auto second_itr = next(first_itr); second_itr != this->m_specific_structure_vector.end();) {
                if (!(*second_itr)->CheckPaExist()) {
                    // 如果这两个Cl相同，进行这个操作的时候，first_itr对应的Cl内部会发生改变
                    if ((*first_itr)->CopyFrom(*second_itr)) {
                        second_itr = this->m_specific_structure_vector.erase(second_itr);
                    } else {
                        second_itr++;
                    }
                } else {
                    second_itr++;
                }
            }
            non_repeating_only_fa_list.push_back(*first_itr); // 加入到object_only_pa_only_fa_list中
        }
        // 既有PA又有FA
        else if ((*first_itr)->CheckFaExist() && (*first_itr)->CheckPaExist()) {
            for (auto second_itr = next(first_itr); second_itr != this->m_specific_structure_vector.end();) {
                if ((*second_itr)->CheckFaExist() && (*second_itr)->CheckPaExist()) {
                    // 如果这两个Cl相同，进行这个操作的时候，first_itr对应的Cl内部会发生改变
                    if ((*first_itr)->CopyFrom(*second_itr)) {
                        second_itr = this->m_specific_structure_vector.erase(second_itr);
                    } else {
                        second_itr++;
                    }
                } else {
                    second_itr++;
                }
            }
            non_repeating_have_both_pa_fa_list.push_back(*first_itr); // 加入到object_only_pa_only_fa_list中
        }
    }

    // 把只有PA和只有FA的合并，non_repeating_only_fa_list去除了可被拼接的部分
    for (auto first_itr : non_repeating_only_pa_list) {
        // 判定这个Cl是否能在后面找到成功拼接上的
        bool first_itr_success = 0;
        for (auto second_itr = non_repeating_only_fa_list.begin(); second_itr != non_repeating_only_fa_list.end();) {
            if (first_itr->CopyFrom(*second_itr)) {
                first_itr_success = 1;
                compare_to_have_both_pa_fa_list.push_back(first_itr); // 追加到结果中
                second_itr = non_repeating_only_fa_list.erase(second_itr);
            } else {
                second_itr++;
            }
        }
        // 如果没有找到成功拼接的情况，则把这个仅有PA或者仅有FA的Cl加入到结果中
        if (!first_itr_success) {
            compare_to_have_both_pa_fa_list.push_back(first_itr); // 追加到结果中
        }
    }

    // PA和FA合并产生了两个新结果，compare_to_have_both_pa_fa_list和non_repeating_only_fa_list的减少，把两者连起来，里面的元素都是互相之间无法拼接的
    compare_to_have_both_pa_fa_list.splice(compare_to_have_both_pa_fa_list.end(), non_repeating_only_fa_list);

    // 去除本身的精细结构
    list<spec_stru_ptr>().swap(m_specific_structure_vector);

    // 最终的合并
    for (auto first_itr : non_repeating_have_both_pa_fa_list) {
        for (auto second_itr = compare_to_have_both_pa_fa_list.begin(); second_itr != compare_to_have_both_pa_fa_list.end();) {
            // 如果这两个Cl相同，进行这个操作的时候，first_itr对应的Cl内部会发生改变
            if (first_itr->CopyFrom(*second_itr)) {
                second_itr = compare_to_have_both_pa_fa_list.erase(second_itr);
            } else {
                second_itr++;
            }
        }
        this->m_specific_structure_vector.push_back(first_itr);
    }

    // 把合并后的结果和不能被合并的结合进行list的拼接
    this->m_specific_structure_vector.splice(this->m_specific_structure_vector.end(), compare_to_have_both_pa_fa_list);
}

int calculate_total_chain(std::initializer_list<pa_ptr> pa_ptr_list)
{
    int sum = 0;
    for (auto pa_ptr : pa_ptr_list) {
        sum += pa_ptr->GetChainLength();
    }
    return sum;
}

int calculate_total_unsaturation(std::initializer_list<pa_ptr> pa_ptr_list)
{
    int sum = 0;
    for (auto pa_ptr : pa_ptr_list) {
        sum += pa_ptr->GetUnsaturation();
    }
    return sum;
}

int calculate_total_oxygen(std::initializer_list<pa_ptr> pa_ptr_list)
{
    int sum = 0;
    for (auto pa_ptr : pa_ptr_list) {
        sum += pa_ptr->GetOxygen();
    }
    return sum;
}
