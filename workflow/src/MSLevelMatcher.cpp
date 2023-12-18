#include <MSLevelMatcher.h>

using namespace std;
MSLevelMatcher::MSLevelMatcher()
{
}

MSLevelMatcher::MSLevelMatcher(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector)
    : Workflow(cardiolipin_pair_vector)
{
}

void MSLevelMatcher::MatchCardiolipinWithMS2(Mzml& mzml)
{
    //    vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>>().swap(m_cardiolipin_without_ms2);

    // 对二级的前体mz进行排序
    mzml.SortMs2VectorByPrecursorIonMz();

    // 取出二级向量
    auto ms2_vector = mzml.GetLocalMs2Vector();

    // 二分搜索ms2_vector
    auto SearchMS2 = [&ms2_vector, this](cardiolipin_ptr cardiolipin) {
        // 如果这个心磷脂是无效的，则直接返回
        if (!cardiolipin->CheckValid()) {
            return;
        }

        int left = 0;
        int right = ms2_vector.size() - 1;
        while (left <= right) {
            int mid = (left + right) / 2;
            float min_mz = ms2_vector[mid]->GetPrecuisorIonMz() - this->m_dalton; // 最小mz匹配值
            float max_mz = ms2_vector[mid]->GetPrecuisorIonMz() + this->m_dalton; // 最大mz匹配值
            if ((cardiolipin->GetMS1Mz() >= min_mz) && (cardiolipin->GetMS1Mz() <= max_mz)) {

                float min_rt = ms2_vector[mid]->GetRt() - this->m_torlerance_rt; // 最小rt匹配值
                float max_rt = ms2_vector[mid]->GetRt() + this->m_torlerance_rt; // 最大rt匹配值
                if ((cardiolipin->GetMS1Rt() >= min_rt) && (cardiolipin->GetMS1Rt() <= max_rt)) {
                    // 增加结果
                    cardiolipin->AddMS2(ms2_vector[mid]);
                }
                // mid的左边一位和右边一位的各自的符合区间
                int left_t = mid - 1;
                int right_t = mid + 1;
                // 向左寻找
                while (left_t >= 0) {
                    float left_t_min_mz = ms2_vector[left_t]->GetPrecuisorIonMz() - this->m_dalton;
                    float left_t_max_mz = ms2_vector[left_t]->GetPrecuisorIonMz() + this->m_dalton;
                    float left_t_min_rt = ms2_vector[left_t]->GetRt() - this->m_torlerance_rt; // 最小rt匹配值
                    float left_t_max_rt = ms2_vector[left_t]->GetRt() + this->m_torlerance_rt; // 最大rt匹配值
                    if ((cardiolipin->GetMS1Mz() >= left_t_min_mz) && (cardiolipin->GetMS1Mz() <= left_t_max_mz)) {
                        // 如果mz是符合的，那么判断rt是否符合；如果mz符合，rt不符合，继续向左寻找，直到mz不符合；如果mz符合，rt符合，则加入ms2_vector，继续向左寻找
                        if ((cardiolipin->GetMS1Rt() >= left_t_min_rt) && (cardiolipin->GetMS1Rt() <= left_t_max_rt)) {
                            cardiolipin->AddMS2(ms2_vector[left_t]);
                        }
                        left_t--; // 向左继续找
                    } else {
                        break;
                    }
                }

                // 向右寻找
                while (right_t <= int(ms2_vector.size() - 1)) {
                    float right_t_min_mz = ms2_vector[right_t]->GetPrecuisorIonMz() - this->m_dalton;
                    float right_t_max_mz = ms2_vector[right_t]->GetPrecuisorIonMz() + this->m_dalton;
                    float right_t_min_rt = ms2_vector[right_t]->GetRt() - this->m_torlerance_rt; // 最小rt匹配值
                    float right_t_max_rt = ms2_vector[right_t]->GetRt() + this->m_torlerance_rt; // 最大rt匹配值
                    if ((cardiolipin->GetMS1Mz() >= right_t_min_mz) && (cardiolipin->GetMS1Mz() <= right_t_max_mz)) {
                        if ((cardiolipin->GetMS1Rt() >= right_t_min_rt) && (cardiolipin->GetMS1Rt() <= right_t_max_rt)) {
                            cardiolipin->AddMS2(ms2_vector[right_t]);
                        }
                        right_t++; // 向右继续寻找
                    } else {
                        break;
                    }
                }

                break;
            }

            else if (cardiolipin->GetMS1Mz() > min_mz) {
                left = mid + 1;
            } else if (cardiolipin->GetMS1Mz() < max_mz) {
                right = mid - 1;
            }
        }
    };

    // 对所有心磷脂进行二级的匹配
    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end();) {

        SearchMS2(itr->first);
        SearchMS2(itr->second);
        // 如果M-H和M-2H都没找到二级，加入无二级结果中，从有二级结果中去除
        if (!itr->first->HasMS2() && !itr->second->HasMS2()) {
            m_cardiolipin_without_ms2.push_back(*itr);
            itr = m_cardiolipin.erase(itr);
        } else {
            itr++;
        }
    }

    // 删除由于二级缺失带来的冗余
    this->DeleteRedundantCardiolipinPair();

    //    qDebug() << ms2_vector.size();
    // 发送信号
    this->PrintTypesCount();
}

void MSLevelMatcher::SetParameter(Parameter& par)
{
    m_dalton = par.m_ms2_dalton;
    m_torlerance_rt = par.m_ms2_torlerance_rt;
}

void MSLevelMatcher::OutputCsv(QString got_ms2_filename, QString without_ms2_filename)
{
    // 使用父类的输出csv函数输出有二级的内容
    Workflow::OutputCsv(got_ms2_filename);

    // 输出没有二级的内容
    QFile file(without_ms2_filename);

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

    // 遍历所有无二级的心磷脂
    for (auto itr : m_cardiolipin_without_ms2) {
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
    text += "输出->" + without_ms2_filename;
    SendMessage(text);
}
