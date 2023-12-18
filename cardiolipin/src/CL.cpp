#include "CL.h"

using namespace std;
CL::CL()
    : Cardiolipin()
{
}

CL::CL(ms1_ptr ms1_ptr, db_ptr db_ptr)
    : Cardiolipin(ms1_ptr, db_ptr)
{
}

Type CL::GetType()
{
    return cl;
}

std::list<spec_stru_ptr> CL::GetNewSpecificStructureList()
{
    list<spec_stru_ptr> res;

    for (auto itr : m_specific_structure_vector) {
        // 转换为子类的指针
        shared_ptr<ClSpecificStructure> origin = std::dynamic_pointer_cast<ClSpecificStructure>(itr);
        shared_ptr<SpecificStructure> cl = make_shared<ClSpecificStructure>(*origin);
        res.push_back(cl);
    }

    return res;
}

void CL::splice()
{
    // 如果是个无效的对象，返回
    if (!CheckValid()) {
        return;
    }

    for (auto ms2 : m_ms2_ptr_v) {
        // 如果这个心磷脂没有PA，用4个FA来拼接
        if (ms2->GetPaCount() == 0) {
            this->FourFaSpliceCl(ms2);
        }
        // 如果这个二级没有FA，则尝试用2个PA拼接
        else if (ms2->GetFaCount() == 0) {
            this->TwoPaSpliceCl(ms2);
        }
        // 如果这个二级即有FA，又有PA
        else {
            this->TwoPaFourFaSpliceCl(ms2);
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

void CL::FourFaSpliceCl(ms2_ptr ms2_ptr)
{
    // 对FA的链长对这个二级的FA向量进行排序
    ms2_ptr->SortFaByChainLength();

    // 获取这组FA
    vector<fa_ptr> fa_ptr_v = ms2_ptr->GetFaPtrVector();

    // 四数之和计算结果
    vector<array<int, 4>> res = FourSum(fa_ptr_v, this->GetCompound());

    // 加入到结果中
    for (auto itr : res) {
        spec_stru_ptr cl = make_shared<ClSpecificStructure>(fa_ptr_v[itr[0]], fa_ptr_v[itr[1]], fa_ptr_v[itr[2]], fa_ptr_v[itr[3]], ms2_ptr);
        m_specific_structure_vector.push_back(cl);
    }
}

void CL::TwoPaSpliceCl(ms2_ptr ms2_ptr)
{
    // 对PA的链长对这个二级的FA向量进行排序
    ms2_ptr->SortPaByChainLength();

    // 获取这组PA
    auto pa_ptr_vector = ms2_ptr->GetPaPtrVector();

    // 两数之和计算结果
    vector<array<int, 2>> res = TwoSum(pa_ptr_vector, this->GetCompound());

    // 加入到结果中
    for (auto itr : res) {
        spec_stru_ptr cl = make_shared<ClSpecificStructure>(pa_ptr_vector[itr[0]], pa_ptr_vector[itr[1]], ms2_ptr);
        m_specific_structure_vector.push_back(cl);
    }
}

void CL::TwoPaFourFaSpliceCl(ms2_ptr ms2_ptr)
{
    // 记录哪些FA可以组成2个PA
    set<set<fa_ptr>> which_four_fa_splice_two_pa;

    // 对FA和PA的向量进行排序
    ms2_ptr->SortPaByChainLength();
    ms2_ptr->SortFaByChainLength();

    // 获取PA和FA向量
    auto fa_ptr_vector = ms2_ptr->GetFaPtrVector();
    auto pa_ptr_vector = ms2_ptr->GetPaPtrVector();

    // 4个FA拼接2个PA
    auto FourFaSpliceTwoPa = [&which_four_fa_splice_two_pa, &fa_ptr_vector, this, &ms2_ptr](pa_ptr pa_ptr_1, pa_ptr pa_ptr_2) {
        auto two_fa_splice_first_pa = TwoSum(fa_ptr_vector, pa_ptr_1->GetCompound());

        // 如果无法拼接成第一个PA，则直接返回结果
        if (two_fa_splice_first_pa.size() == 0) {
            spec_stru_ptr cl = make_shared<ClSpecificStructure>(pa_ptr_1, pa_ptr_2, ms2_ptr);
            m_specific_structure_vector.push_back(cl);
            return;
        }

        // 寻找能拼接成第二个PA的2个FA
        auto two_fa_splice_second_pa = TwoSum(fa_ptr_vector, pa_ptr_2->GetCompound());

        // 如果无法拼接成第二个PA，则直接返回结果
        if (two_fa_splice_second_pa.size() == 0) {
            spec_stru_ptr cl = make_shared<ClSpecificStructure>(pa_ptr_1, pa_ptr_2, ms2_ptr);
            m_specific_structure_vector.push_back(cl);
            return;
        }

        // 有可能pa_ptr_1和pa_ptr_2是同样的PA，所以需要区分
        bool same_pa;
        if ((pa_ptr_1->GetChainLength() == pa_ptr_2->GetChainLength()) && (pa_ptr_1->GetUnsaturation() == pa_ptr_2->GetUnsaturation()) && (pa_ptr_1->GetOxygen() == pa_ptr_2->GetOxygen())) {
            same_pa = 1;
        } else {
            same_pa = 0;
        }

        // 遍历拼接第一个PA和第二个PA的结果，加入结果中
        for (unsigned int i = 0; i < two_fa_splice_first_pa.size(); i++) {
            // 如果是一样的PA，则j需要在i的时候开始
            unsigned int start = 0;
            if (same_pa) {
                start = i;
            } else {
                start = 0;
            }
            for (unsigned int j = start; j < two_fa_splice_second_pa.size(); j++) {
                // 加入到结果中
                spec_stru_ptr cl = make_shared<ClSpecificStructure>(pa_ptr_1, fa_ptr_vector.at(two_fa_splice_first_pa[i][0]), fa_ptr_vector.at(two_fa_splice_first_pa[i][1]), pa_ptr_2, fa_ptr_vector.at(two_fa_splice_second_pa[j][0]), fa_ptr_vector.at(two_fa_splice_second_pa[j][1]), ms2_ptr);
                m_specific_structure_vector.push_back(cl);
                // 把这些可以拼接成PA的FA的信息加入到which_four_fa_splice_two_pa中
                which_four_fa_splice_two_pa.insert({ fa_ptr_vector.at(two_fa_splice_first_pa[i][0]), fa_ptr_vector.at(two_fa_splice_first_pa[i][1]), fa_ptr_vector.at(two_fa_splice_second_pa[j][0]), fa_ptr_vector.at(two_fa_splice_second_pa[j][1]) });
            }
        }
    };

    // 找哪两个PA可以组成CL
    // 两数之和计算结果
    vector<array<int, 2>> two_sum_res = TwoSum(pa_ptr_vector, this->GetCompound());
    // 寻找哪些FA可以组成这两个PA
    for (auto itr : two_sum_res) {
        FourFaSpliceTwoPa(pa_ptr_vector[itr[0]], pa_ptr_vector[itr[1]]);
    }

    // 四数之和计算结果
    vector<array<int, 4>> res = FourSum(fa_ptr_vector, this->GetCompound());

    // 加入到结果中
    for (auto itr : res) {
        // 如果这组FA无法组成2个PA，则加入到结果中
        if (which_four_fa_splice_two_pa.find({ fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], fa_ptr_vector[itr[2]], fa_ptr_vector[itr[3]] }) == which_four_fa_splice_two_pa.end()) {
            spec_stru_ptr cl = make_shared<ClSpecificStructure>(fa_ptr_vector[itr[0]], fa_ptr_vector[itr[1]], fa_ptr_vector[itr[2]], fa_ptr_vector[itr[3]], ms2_ptr);
            m_specific_structure_vector.push_back(cl);
        }
    }
}

std::shared_ptr<Cardiolipin> CL::Clone()
{
    // 复制新对象
    auto temp = make_shared<CL>(*this);

    // 精细结构也是新的
    temp->m_specific_structure_vector = this->GetNewSpecificStructureList();

    return temp;
}
