#include <workflow.h>

using namespace std;
Workflow::Workflow(QObject* parent)
    : QObject(parent)
{
}

Workflow::Workflow(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector)
{
    this->Set(cardiolipin_pair_vector);
}

void Workflow::Set(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector)
{
    // 清空本来的心磷脂
    vector<pair<cardiolipin_ptr, cardiolipin_ptr>>().swap(m_cardiolipin);

    // 创建新的内存
    for (auto itr : cardiolipin_pair_vector) {
        cardiolipin_ptr m_h;
        cardiolipin_ptr m_2h;

        m_h = itr.first->Clone();
        m_2h = itr.second->Clone();
        m_cardiolipin.push_back({ m_h, m_2h });
    }
}

void Workflow::PrintTypesCount()
{
    int CL_count = 0;
    int MLCL_count = 0;
    int DLCL_count = 0;

    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end(); itr++) {
        Type type = itr->first->GetType();
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

void Workflow::OutputCsv(QString file_name)
{
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
           << "m_2h_area"
           << endl;

    // 遍历所有心磷脂
    for (auto itr : m_cardiolipin) {
        auto compound = GetCompound(itr);
        // 化合物
        QString compound_message = QString::number(compound[0]) + ":" + QString::number(compound[1]);
        float rt = GetRt(itr);
        float intensity = GetIntensity(itr);
        QString type;
        if (itr.first->GetType() == cl) {
            type = "CL";
        } else if (itr.first->GetType() == mlcl) {
            type = "MLCL";
        } else {
            type = "DLCL";
        }
        stream << compound_message << "," << itr.first->GetMS1Mz() << "," << itr.second->GetMS1Mz() << "," << rt << "," << type << "," << intensity << "," << itr.first->GetMS1Area() << "," << itr.second->GetMS1Area() << endl;
    }

    file.close(); // 关闭文件

    QString text;
    text += "输出->" + file_name;
    SendMessage(text);
}

std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> Workflow::GetCardiolipinPairVector()
{
    return m_cardiolipin;
}

std::array<int, 3> Workflow::GetCompound(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    auto m_h = pair.first->GetCompound();
    auto m_2h = pair.second->GetCompound();

    if (m_h == array<int, 3> { 0, 0, 0 }) {
        return m_2h;
    }

    return m_h;
}

std::array<int, 3> Workflow::GetCompound(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetCompound(*pair_ptr);
}

int Workflow::GetChainLength(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    return GetCompound(pair)[0];
}

int Workflow::GetChainLength(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetChainLength(*pair_ptr);
}

int Workflow::GetUnsaturation(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    return GetCompound(pair)[1];
}

int Workflow::GetUnsaturation(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetUnsaturation(*pair_ptr);
}

int Workflow::GetOxygen(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    return GetCompound(pair)[2];
}

int Workflow::GetOxygen(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetOxygen(*pair_ptr);
}

float Workflow::GetM_HArea(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    return pair.first->GetMS1Area();
}

float Workflow::GetM_HArea(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetM_HArea(*pair_ptr);
}

float Workflow::GetM_2HArea(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    return pair.second->GetMS1Area();
}

float Workflow::GetM_2HArea(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetM_2HArea(*pair_ptr);
}

float Workflow::GetRt(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    float first_rt = pair.first->GetMS1Rt();
    float second_rt = pair.second->GetMS1Rt();

    if (first_rt == 0) {
        return second_rt;
    } else if (second_rt == 0) {
        return first_rt;
    } else if (first_rt != 0 && second_rt != 0) {
        return (first_rt + second_rt) / 2;
    }

    return 0;
}

float Workflow::GetRt(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetRt(*pair_ptr);
}

float Workflow::GetIntensity(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    float first_intensity = pair.first->GetMS1Intensity();
    float second_intensity = pair.second->GetMS1Intensity();

    if (first_intensity == 0) {
        return second_intensity;
    } else if (second_intensity == 0) {
        return first_intensity;
    } else if (first_intensity != 0 && second_intensity != 0) {
        return std::max(first_intensity, second_intensity);
    }

    return 0;
}

Type Workflow::GetType(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair)
{
    return pair.first->GetType();
}

Type Workflow::GetType(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr)
{
    return GetType(*pair_ptr);
}

void Workflow::SetParameter(Parameter& par)
{
    Q_UNUSED(par);
    qDebug() << "Class Workflow call this 'SetParameter' function!!!";
}

void Workflow::DeleteRedundantCardiolipinPair()
{
    // 两个心磷脂都存在的pair
    set<pair<ms1_ptr, db_ptr>> cardiolipin_set;
    for (auto itr : m_cardiolipin) {
        if (itr.first->CheckValid() && itr.second->CheckValid()) {
            cardiolipin_set.insert({ itr.first->GetMS1Ptr(), itr.first->GetMS1DbPtr() });
            cardiolipin_set.insert({ itr.second->GetMS1Ptr(), itr.second->GetMS1DbPtr() });
        }
    }

    // 删除重复项
    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end();) {

        // 如果仅有M-H
        if (itr->first->CheckValid() && !itr->second->CheckValid()) {
            // 如果找到了重复项，则删除
            if (cardiolipin_set.find({ itr->first->GetMS1Ptr(), itr->first->GetMS1DbPtr() }) != cardiolipin_set.end()) {
                itr = m_cardiolipin.erase(itr);
            } else {
                cardiolipin_set.insert({ itr->first->GetMS1Ptr(), itr->first->GetMS1DbPtr() });
                itr++;
            }
        }

        // 如果仅有M-2H
        else if (!itr->first->CheckValid() && itr->second->CheckValid()) {
            // 如果找到了重复项，则删除
            if (cardiolipin_set.find({ itr->second->GetMS1Ptr(), itr->second->GetMS1DbPtr() }) != cardiolipin_set.end()) {
                itr = m_cardiolipin.erase(itr);
            } else {
                cardiolipin_set.insert({ itr->second->GetMS1Ptr(), itr->second->GetMS1DbPtr() });
                itr++;
            }
        }

        // 如果两个都有，跳过
        else {
            itr++;
        }
    }
}

void Workflow::DeleteCardiolipinWithOxygen()
{
    // 去除含氧的心磷脂
    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end();) {
        // 氧的个数
        int oxygen = itr->first->GetCompound()[2] + itr->second->GetCompound()[2];
        if (oxygen > 0) {
            itr = m_cardiolipin.erase(itr);
        } else {
            itr++;
        }
    }
}
