#include <FragmentCombiner.h>
using namespace std;

FragmentCombiner::FragmentCombiner()
{
}

FragmentCombiner::FragmentCombiner(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector)
    : Workflow(cardiolipin_pair_vector)
{
}

void FragmentCombiner::Splice()
{
    std::map<std::shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>>().swap(m_merge_result);

    // 遍历所有心磷脂，进行拼接
    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end();) {
        // [M-H]和[M-2H]进行拼接
        itr->first->splice();
        itr->second->splice();
        // 如果两个都没有拼接成功；或者一个原本是没有的，另一个没有拼接成功，则将其删除
        if (!itr->first->CheckValid() && !itr->second->CheckValid()) {
            itr = m_cardiolipin.erase(itr);
        } else {
            ++itr;
        }
    }

    // 删除冗余心磷脂
    this->DeleteRedundantCardiolipinPair();

    // 进行[M-H]和[M-2H]的合并
    for (auto itr : m_cardiolipin) {
        MergePair(itr);
    }

    // 对哈希表中的每个拼接结果的分数从大到小进行排序
    for (auto itr = this->m_merge_result.begin(); itr != this->m_merge_result.end(); itr++) {
        itr->second.sort([](const spec_stru_ptr& lhs, const spec_stru_ptr& rhs) {
            return lhs->GetScore() > rhs->GetScore();
        });
    }

    // 发送信号
    this->PrintTypesCount();
}

std::map<std::shared_ptr<FragmentCombiner::cardiolipin_ptr_pair>, std::list<spec_stru_ptr>> FragmentCombiner::GetMergeResult()
{
    return m_merge_result;
}

void FragmentCombiner::MergePair(FragmentCombiner::cardiolipin_ptr_pair pair)
{
    // 如果M-H或者M-2H某个是空的，则把结果加入到m_cl_merge_hash_table中，退出函数
    if (!pair.first->CheckValid()) {
        m_merge_result.insert({ make_shared<cardiolipin_ptr_pair>(pair), pair.second->GetNewSpecificStructureList() });
        return;
    } else if (!pair.second->CheckValid()) {
        m_merge_result.insert({ make_shared<cardiolipin_ptr_pair>(pair), pair.first->GetNewSpecificStructureList() });
        return;
    }

    // 取出[M-H]和[M-2H]的精细结构，需要复制，因为下面需要直接在此内存上修改
    auto m_h_cl_specific_structure_list_copy = pair.first->GetNewSpecificStructureList();
    auto m_2h_cl_specific_structure_list_copy = pair.second->GetNewSpecificStructureList();

    // 结果
    list<spec_stru_ptr> res;

    for (auto first_itr = m_h_cl_specific_structure_list_copy.begin(); first_itr != m_h_cl_specific_structure_list_copy.end(); first_itr++) {
        for (auto second_itr = m_2h_cl_specific_structure_list_copy.begin(); second_itr != m_2h_cl_specific_structure_list_copy.end();) {
            // 不同的模式，进行不同的合并
            // [M-2H]中被合并的需要去除
            if ((*first_itr)->CopyFrom(*second_itr, m_mode)) {
                second_itr = m_2h_cl_specific_structure_list_copy.erase(second_itr);
            } else {
                second_itr++;
            }
        }
        res.push_back(*first_itr);
    }

    // 加入结果中
    m_merge_result.insert({ make_shared<cardiolipin_ptr_pair>(pair), res });
}

void FragmentCombiner::SetParameter(Parameter& par)
{
    m_mode = par.m_merge_mode;
    SpecificStructure::m_fragment_score_weight = par.m_fragment_score_weight;
    SpecificStructure::m_pa_exist_score_weight = par.m_pa_exist_score_weight;
    SpecificStructure::m_fa_intensity_variance_score_weight = par.m_fa_intensity_variance_score_weight;
}

void FragmentCombiner::PrintTypesCount()
{
    int CL_count = 0;
    int MLCL_count = 0;
    int DLCL_count = 0;

    for (auto itr = m_merge_result.begin(); itr != m_merge_result.end(); itr++) {
        Type type = itr->first->first->GetType();
        if (type == cl) {
            CL_count++;
        } else if (type == mlcl) {
            MLCL_count++;
        } else if (type == dlcl) {
            DLCL_count++;
        }
    }

    // 发送信息
    QString text;
    text += "CL:" + QString::number(CL_count) + " ";
    text += "MLCL:" + QString::number(MLCL_count) + " ";
    text += "DLCL:" + QString::number(DLCL_count) + " ";

    SendMessage(text);
}

void FragmentCombiner::OutputCsv(QString file_name)
{
    // 求所有拼接结果中最多有多少个
    int max_top = 0;
    for (auto itr : m_merge_result) {
        max_top = std::max(int(itr.second.size()), max_top);
    }

    QFile file(file_name);

    // 打开文件
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    // 文本流
    QTextStream stream(&file);
    stream << "Compound"
           << ","
           << "M.H.m.z"
           << ","
           << "M.2H.m.z"
           << ","
           << "Rt"
           << ","
           << "type"
           << ","
           << "intensity"
           << ","
           << "m_h_area"
           << ","
           << "m_2h_area";

    // 加入top的列名
    for (int i = 1; i <= max_top; i++) {
        QString top = QString("top") + QString("-") + QString::number(i);
        stream << "," << top;
    }

    stream << endl;

    // 遍历所有拼接结果
    for (auto itr : m_merge_result) {
        //
        auto pair = *(itr.first);
        auto compound = GetCompound(pair);
        // 化合物
        QString compound_message = QString::number(compound[0]) + ":" + QString::number(compound[1]);
        float rt = GetRt(pair);
        float intensity = GetIntensity(pair);
        QString type;
        if (pair.first->GetType() == cl) {
            type = "CL";
        } else if (pair.first->GetType() == mlcl) {
            type = "MLCL";
        } else {
            type = "DLCL";
        }
        // 追加一级的信息
        stream << compound_message << "," << pair.first->GetMS1Mz() << "," << pair.second->GetMS1Mz() << "," << rt << "," << type << "," << intensity << "," << pair.first->GetMS1Area() << "," << pair.second->GetMS1Area();

        // 追加拼接结果
        for (auto itr_splice = itr.second.begin(); itr_splice != itr.second.end(); itr_splice++) {
            auto splice_info = (*itr_splice)->ShowSimpleInfomation();
            stream << "," << splice_info;
        }

        stream << endl;
    }

    file.close(); // 关闭文件

    QString text;
    text += "输出->" + file_name;
    SendMessage(text);
}
