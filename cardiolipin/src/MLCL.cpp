#include "MLCL.h"

using namespace std;
MLCL::MLCL()
    : Cardiolipin()
{
}

MLCL::MLCL(ms1_ptr ms1_ptr, db_ptr db_ptr)
    : Cardiolipin(ms1_ptr, db_ptr)
{
}

void MLCL::splice()
{
    // 如果是个无效的对象，返回
    if (!CheckValid()) {
        return;
    }

    for (auto ms2 : m_ms2_ptr_v) {
        // 如果这个心磷脂没有PA，用3个FA来拼接
        if (ms2->GetPaCount() == 0) {
            this->ThreeFaSpliceMlcl(ms2);
        }
        // 如果这个二级即有FA，又有PA
        else {
            this->OnePaThreeFaSpliceMlcl(ms2);
        }
    }

    // 如果没有拼接成功，则清空对象
    if (m_specific_structure_vector.size() == 0) {
        this->Clear();
        return;
    }

    // 合并拼接结果
    this->MergeSplice();
}

void MLCL::ThreeFaSpliceMlcl(ms2_ptr ms2_ptr)
{
    ms2_ptr->SortFaByChainLength();

    auto fa_ptr_vector = ms2_ptr->GetFaPtrVector();

    // 三数之和结果
    auto res = ThreeSum(fa_ptr_vector, this->GetCompound());

    // 加入到结果中
    for (auto itr : res) {
        spec_stru_ptr cl = make_shared<MlclSpecificStructure>(fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], fa_ptr_vector[itr[2]], ms2_ptr);
        m_specific_structure_vector.push_back(cl);
    }
}

void MLCL::OnePaThreeFaSpliceMlcl(ms2_ptr ms2_ptr)
{

    // 记录哪2个FA可以组成1个PA，和另外的一个FA可以组成MLCL
    set<set<fa_ptr>> which_two_fa_splice_one_pa;

    // 对FA和PA的向量进行排序
    ms2_ptr->SortPaByChainLength();
    ms2_ptr->SortFaByChainLength();

    // 获取PA和FA向量
    auto fa_ptr_vector = ms2_ptr->GetFaPtrVector();
    auto pa_ptr_vector = ms2_ptr->GetPaPtrVector();

    // 以链长为键，对应的fa为值构建多重哈希表
    multimap<unsigned int, fa_ptr> fa_hash_map;

    // 把FA的信息加入到表中
    for (auto fa_ptr : fa_ptr_vector) {
        fa_hash_map.insert({ fa_ptr->GetChainLength(), fa_ptr });
    }

    // 寻找哪些PA和FA可以组成MLCL，把可以组成PA的两个FA和那个FA加入到set中
    for (auto pa_ptr : pa_ptr_vector) {
        // PA和某个FA可以拼接成这个MLCL
        auto search_pair_itr = fa_hash_map.equal_range(this->GetChainLength() - pa_ptr->GetChainLength());
        // 如果能找到
        if (search_pair_itr.first != search_pair_itr.second) {
            // 遍历搜索到的范围，match_itr是fa_hash_map的迭代器
            for (auto match_itr = search_pair_itr.first; match_itr != search_pair_itr.second; match_itr++) {
                // 获取PA和FA的组成
                auto fa_compound = match_itr->second->GetCompound();
                auto pa_compound = pa_ptr->GetCompound();
                array<int, 3> merge = array<int, 3> { fa_compound[0] + pa_compound[0], fa_compound[1] + pa_compound[1], fa_compound[2] + pa_compound[2] };
                // 需要这个PA能和这个FA能组成MLCL才可以
                if (merge != this->GetCompound()) {
                    continue;
                }

                // 用FA去拼接这个PA
                auto fa_splice_pa = TwoSum(fa_ptr_vector, pa_ptr->GetCompound());
                // 如果没有FA能拼接成这个PA
                if (fa_splice_pa.size() == 0) {
                    spec_stru_ptr mlcl = make_shared<MlclSpecificStructure>(pa_ptr, match_itr->second, ms2_ptr);
                    m_specific_structure_vector.push_back(mlcl);
                    break;

                }
                // 如果有FA能拼接成这个PA
                else {
                    for (auto arr : fa_splice_pa) {
                        spec_stru_ptr mlcl = make_shared<MlclSpecificStructure>(pa_ptr, fa_ptr_vector.at(arr[0]), fa_ptr_vector.at(arr[1]), match_itr->second, ms2_ptr);
                        m_specific_structure_vector.push_back(mlcl);
                        which_two_fa_splice_one_pa.insert({ fa_ptr_vector.at(arr[0]), fa_ptr_vector.at(arr[1]), match_itr->second });
                    }
                }
            }
        }
    }

    // 用3个FA拼接这个mlcl
    auto res = ThreeSum(fa_ptr_vector, this->GetCompound());

    for (auto itr : res) {
        // 如果这组FA无法组成1个PA，则加入到结果中
        if (which_two_fa_splice_one_pa.find({ fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], fa_ptr_vector[itr[2]] }) == which_two_fa_splice_one_pa.end()) {
            spec_stru_ptr mlcl = make_shared<MlclSpecificStructure>(fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], fa_ptr_vector[itr[2]], ms2_ptr);
            m_specific_structure_vector.push_back(mlcl);
        }
    }
}

std::shared_ptr<Cardiolipin> MLCL::Clone()
{
    // 复制新对象
    auto temp = make_shared<MLCL>(*this);

    // 精细结构也是新的
    temp->m_specific_structure_vector = this->GetNewSpecificStructureList();

    return temp;
}

Type MLCL::GetType()
{
    return mlcl;
}

std::list<spec_stru_ptr> MLCL::GetNewSpecificStructureList()
{
    list<spec_stru_ptr> res;

    for (auto itr : m_specific_structure_vector) {
        // 转换为子类的指针
        shared_ptr<MlclSpecificStructure> origin = std::dynamic_pointer_cast<MlclSpecificStructure>(itr);
        shared_ptr<SpecificStructure> mlcl = make_shared<MlclSpecificStructure>(*origin);
        res.push_back(mlcl);
    }

    return res;
}
