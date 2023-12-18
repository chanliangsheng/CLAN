#include "DLCL.h"

using namespace std;
DLCL::DLCL()
    : Cardiolipin()
{
}

DLCL::DLCL(ms1_ptr ms1_ptr, db_ptr db_ptr)
    : Cardiolipin(ms1_ptr, db_ptr)
{
}

void DLCL::splice()
{
    // 如果是个无效的对象，返回
    if (!CheckValid()) {
        return;
    }

    for (auto ms2 : m_ms2_ptr_v) {
        // 如果这个心磷脂没有PA，用3个FA来拼接
        if (ms2->GetPaCount() == 0) {
            this->TwoFaSpliceDlcl(ms2);
        }
        // 如果这个二级即有FA，又有PA
        else {
            this->OnePaTwoFaSpliceDlcl(ms2);
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

void DLCL::TwoFaSpliceDlcl(ms2_ptr ms2_ptr)
{
    // 对PA的链长对这个二级的FA向量进行排序
    ms2_ptr->SortPaByChainLength();

    // 获取这组PA
    auto pa_ptr_vector = ms2_ptr->GetPaPtrVector();
    auto fa_ptr_vector = ms2_ptr->GetFaPtrVector();

    // 两数之和计算结果
    vector<array<int, 2>> res = TwoSum(fa_ptr_vector, this->GetCompound());

    // 加入到结果中
    for (auto itr : res) {
        spec_stru_ptr dlcl = make_shared<DlclSpecificStructure>(fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], ms2_ptr);
        m_specific_structure_vector.push_back(dlcl);
    }
}

void DLCL::OnePaTwoFaSpliceDlcl(ms2_ptr ms2_ptr)
{
    // 记录哪些FA可以组成1个PA
    set<set<fa_ptr>> which_two_fa_splice_one_pa;

    // 对FA和PA的向量进行排序
    ms2_ptr->SortPaByChainLength();
    ms2_ptr->SortFaByChainLength();

    // 获取PA和FA向量
    auto fa_ptr_vector = ms2_ptr->GetFaPtrVector();
    auto pa_ptr_vector = ms2_ptr->GetPaPtrVector();

    // 遍历pa向量，寻找哪些PA可以组成DLCL，把可以组成PA的两个FA加入到set中
    for (auto pa_ptr : pa_ptr_vector) {
        // 如果有PA可以组成DLCL
        if (pa_ptr->GetCompound() == this->GetCompound()) {
            // 用FA拼接这个DLCL
            auto two_fa_splice_pa = TwoSum(fa_ptr_vector, pa_ptr->GetCompound());
            // 如果没有FA能拼接成这个PA
            if (two_fa_splice_pa.size() == 0) {
                spec_stru_ptr dlcl = make_shared<DlclSpecificStructure>(pa_ptr, ms2_ptr);
                m_specific_structure_vector.push_back(dlcl);
            }
            // 如果有FA能拼接成这个PA
            else {
                for (auto arr : two_fa_splice_pa) {
                    spec_stru_ptr dlcl = make_shared<DlclSpecificStructure>(pa_ptr, fa_ptr_vector.at(arr[0]), fa_ptr_vector.at(arr[1]), ms2_ptr);
                    m_specific_structure_vector.push_back(dlcl);
                    which_two_fa_splice_one_pa.insert({ fa_ptr_vector.at(arr[0]), fa_ptr_vector.at(arr[1]) });
                }
            }
        }
    }

    // 用2个FA拼接这个mlcl
    auto res = TwoSum(fa_ptr_vector, this->GetCompound());
    for (auto itr : res) {
        // 如果这组FA无法组成1个PA，则加入到结果中
        if (which_two_fa_splice_one_pa.find({ fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]] }) == which_two_fa_splice_one_pa.end()) {
            spec_stru_ptr dlcl = make_shared<DlclSpecificStructure>(fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], ms2_ptr);
            m_specific_structure_vector.push_back(dlcl);
        }
    }
}

std::shared_ptr<Cardiolipin> DLCL::Clone()
{
    // 复制新对象
    auto temp = make_shared<DLCL>(*this);

    // 精细结构也是新的
    temp->m_specific_structure_vector = this->GetNewSpecificStructureList();

    return temp;
}

Type DLCL::GetType()
{
    return dlcl;
}

std::list<spec_stru_ptr> DLCL::GetNewSpecificStructureList()
{
    list<spec_stru_ptr> res;

    for (auto itr : m_specific_structure_vector) {
        // 转换为子类的指针
        shared_ptr<DlclSpecificStructure> origin = std::dynamic_pointer_cast<DlclSpecificStructure>(itr);
        shared_ptr<SpecificStructure> dlcl = make_shared<DlclSpecificStructure>(*origin);
        res.push_back(dlcl);
    }

    return res;
}
