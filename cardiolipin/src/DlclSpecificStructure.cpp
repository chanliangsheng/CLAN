#include <DlclSpecificStructure.h>

using namespace std;

DlclSpecificStructure::DlclSpecificStructure()
    : SpecificStructure()
{
}

DlclSpecificStructure::DlclSpecificStructure(fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, ms2_ptr ms2_ptr)
{
    m_left_pa = PaNode(fa_1_ptr, fa_2_ptr);
    m_ms2_ptr = ms2_ptr;

    AddFaInfo({ fa_1_ptr, fa_2_ptr });

    this->m_pa_exist = 0;
    this->m_fa_exist = 1;

    // 进行打分
    this->Score();
}

DlclSpecificStructure::DlclSpecificStructure(pa_ptr pa_1_ptr, fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, ms2_ptr ms2_ptr)
{
    m_left_pa = PaNode(pa_1_ptr, fa_1_ptr, fa_2_ptr);
    m_ms2_ptr = ms2_ptr;

    AddPaInfo({ pa_1_ptr });
    AddFaInfo({ fa_1_ptr, fa_2_ptr });

    this->m_pa_exist = 1;
    this->m_fa_exist = 1;

    // 进行打分
    this->Score();
}

DlclSpecificStructure::DlclSpecificStructure(pa_ptr pa_1_ptr, ms2_ptr ms2_ptr)
{
    m_left_pa = PaNode(pa_1_ptr);
    m_ms2_ptr = ms2_ptr;

    AddPaInfo({ pa_1_ptr });

    this->m_pa_exist = 1;
    this->m_fa_exist = 0;

    // 进行打分
    this->Score();
}

void DlclSpecificStructure::Score()
{
    SpecificStructure::Score({ m_left_pa.GetLeftFaPtr(), m_left_pa.GetRightFaPtr() });
}

QString DlclSpecificStructure::ShowSimpleInfomation()
{
    QString message;
    // 如果PA和FA都存在
    if (this->m_fa_exist && this->m_pa_exist) {
        auto left_pa = m_left_pa.GetPaPtr()->GetCompound();
        auto left_left_fa = m_left_pa.GetLeftFaPtr()->GetCompound();
        auto left_right_fa = m_left_pa.GetRightFaPtr()->GetCompound();

        QString pa_1_message = QString::number(left_pa[0]) + ":" + QString::number(left_pa[1]);

        QString fa_1_message = QString::number(left_left_fa[0]) + ":" + QString::number(left_left_fa[1]);
        QString fa_2_message = QString::number(left_right_fa[0]) + ":" + QString::number(left_right_fa[1]);

        message = message + "PA:(" + pa_1_message + ")" + ";";
        message = message + "FA:(" + fa_1_message + "/" + fa_2_message + ")";

    }
    // 如果只有FA存在
    else if (this->m_fa_exist) {
        auto left_left_fa = m_left_pa.GetLeftFaPtr()->GetCompound();
        auto left_right_fa = m_left_pa.GetRightFaPtr()->GetCompound();

        QString fa_1_message = QString::number(left_left_fa[0]) + ":" + QString::number(left_left_fa[1]);
        QString fa_2_message = QString::number(left_right_fa[0]) + ":" + QString::number(left_right_fa[1]);

        message = message + "PA:()" + ";";
        message = message + "FA:(" + fa_1_message + "/" + fa_2_message + ")";
    }
    // 如果只有PA存在
    else if (this->m_pa_exist) {
        auto left_pa = m_left_pa.GetPaPtr()->GetCompound();
        QString pa_1_message = QString::number(left_pa[0]) + ":" + QString::number(left_pa[1]);

        message = message + "PA:(" + pa_1_message + ")" + ";";
        message = message + "FA:()";
    }

    return message;
}

bool DlclSpecificStructure::CopyFrom(SpecificStructure::spec_stru_ptr other)
{
    // 转换为子类的指针
    dlcl_spec_stru_ptr other_dlcl = std::dynamic_pointer_cast<DlclSpecificStructure>(other);

    // 如果两者的FA都不存在，指的是组成PA的2个FA不存在，则比较PA是否相同
    if (!this->m_fa_exist && !other_dlcl->m_fa_exist) {
        // 如果两者的PA信息相同，则把this的分数更新为最大值
        if (this->m_pa_info == other_dlcl->m_pa_info) {
            this->m_score = UpdateScore(this->m_score, other_dlcl->m_score);
            return true;
        } else {
            return false;
        }
    }
    // 如果两者的PA都不存在，则比较FA是否相同
    else if (!this->m_pa_exist && !other_dlcl->m_pa_exist) {
        // 如果两者的FA信息相同，则把this的分数更新为最大值
        if (this->m_fa_info == other_dlcl->m_fa_info) {
            this->m_score = UpdateScore(this->m_score, other_dlcl->m_score);

            return true;
        } else {
            return false;
        }
    }
    // 如果两者的PA，FA都存在
    else if (this->m_fa_exist && other_dlcl->m_fa_exist && this->m_pa_exist && other_dlcl->m_pa_exist) {
        // 如果两者的PA和FA信息相同，则把this的分数更新为最大值
        if ((this->m_pa_info == other_dlcl->m_pa_info) && (this->m_fa_info == other_dlcl->m_fa_info)) {
            this->m_score = UpdateScore(this->m_score, other_dlcl->m_score);
            return true;
        } else {
            return false;
        }
    }
    // 如果一个的PA没有FA，另一个的PA有FA
    else if ((!this->m_fa_exist && other_dlcl->m_fa_exist && other_dlcl->m_pa_exist) || (!other_dlcl->m_fa_exist && this->m_fa_exist && this->m_pa_exist)) {
        // 如果两者的PA信息相同，把this的内容换成新的内容；如果PA相同，则右侧的FA也一定相同
        if (this->m_pa_info == other_dlcl->m_pa_info) {
            if (!this->m_fa_exist) {
                *this = *other_dlcl;
                return true;
            } else if (!other_dlcl->m_fa_exist) {
                return true;
            }
        } else {
            return false;
        }
    }
    // 如果一个只有FA，另一个既有PA，PA有对应的2个FA
    else if ((!this->m_pa_exist && other_dlcl->m_fa_exist && other_dlcl->m_pa_exist) || (!other_dlcl->m_pa_exist && this->m_fa_exist && this->m_pa_exist)) {
        // 如果两者的FA信息相同，把this的内容换成新的内容，并且把分数换为最大值
        if (this->m_fa_info == other_dlcl->m_fa_info) {
            float max_score = UpdateScore(this->m_score, other_dlcl->m_score);
            if (!this->m_pa_exist) {
                *this = *other_dlcl;
                this->m_score = max_score;
                return true;
            } else if (!other_dlcl->m_pa_exist) {
                this->m_score = max_score;
                return true;
            }
        }
    }
    // 如果一个只有FA，另一个只有PA
    else if ((!this->m_pa_exist && !other_dlcl->m_fa_exist) || (!other_dlcl->m_pa_exist && !this->m_pa_exist)) {
        // 抽象出两个指针，一个指向只有FA的，另一个指向只有PA的；交换指针
        DlclSpecificStructure* object_only_fa = this;
        DlclSpecificStructure* object_only_pa = new DlclSpecificStructure(*other_dlcl);
        // 如果this为只有PA的那一个，则交换指针
        if (this->m_pa_exist) {
            DlclSpecificStructure* temp;
            temp = object_only_fa;
            object_only_fa = object_only_pa;
            object_only_pa = temp;
        }

        // 把FA取出
        auto fa_ptr_vector = object_only_fa->GetAllFa();

        Cardiolipin cal;

        // 用fa拼接左边的pa
        auto fa_splice_left_pa = cal.TwoSum_Not_Repeat(fa_ptr_vector, object_only_pa->m_left_pa.GetPaPtr()->GetCompound());

        // 如果无法拼接成左边的PA，则退出
        if (fa_splice_left_pa.size() == 0) {
            return false;
        }

        // 取出第一个数组
        auto arr = fa_splice_left_pa[0];

        // 存储将要被删除的fa
        auto fa_1 = fa_ptr_vector[arr[0]];
        auto fa_2 = fa_ptr_vector[arr[1]];

        // 赋值
        object_only_pa->m_left_pa.Set(fa_1, fa_2);

        // 如果this仅有FA，说明only_pa是新建的，需要删除
        if (this == object_only_fa) {
            *this = *object_only_pa;
            delete object_only_pa;
        }
        // 如果this仅有PA，说明only_fa是新建的，需要删除
        else{
            delete object_only_fa;
        }

        // 更新信息
        this->Update();

        return true;
    }
    return false;
}

bool DlclSpecificStructure::CopyFrom(SpecificStructure::spec_stru_ptr other, mode mode)
{
    // 如果是严格模式，则直接调用原本的copy函数
    if (mode == strict) {
        return this->CopyFrom(other);
    }
    // 如果是宽松模式，则需要改写，只要FA一致即可
    else if (mode == flexible) {
        // 转换为子类的指针
        dlcl_spec_stru_ptr other_dlcl = std::dynamic_pointer_cast<DlclSpecificStructure>(other);
        // 如果两者的PA，FA都存在
        if (this->m_fa_exist && other_dlcl->m_fa_exist && this->m_pa_exist && other_dlcl->m_pa_exist) {
            // 如果两者的FA信息一致
            if (this->m_fa_info == other_dlcl->m_fa_info) {
                float new_score = UpdateScore(this->m_score, other_dlcl->m_score);
                // 把新分数赋值给原本分数最大者
                if (this->m_score >= other_dlcl->m_score) {
                    this->m_score = new_score;
                } else {
                    *this = *other_dlcl;
                    this->m_score = new_score;
                }
                return true;
            }
            // 如果FA的信息不一致，返回false
            else {
                return false;
            }
        }
        // 其他情况下，都使用严格模式的copy函数
        else {
            return this->CopyFrom(other);
        }
    }

    return false;
}

std::vector<fa_ptr> DlclSpecificStructure::GetAllFa()
{
    return SpecificStructure::GetAllFa();
}

void DlclSpecificStructure::Update()
{
    SpecificStructure::Update();
}
