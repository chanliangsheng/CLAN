#include <MS1LibraryMatcher.h>

using namespace std;

MS1LibraryMatcher::MS1LibraryMatcher()
{
    m_ppm = 5;
    m_torlerance_rt = 0.2;
    m_ppm_with_half_score = 5;
}

void MS1LibraryMatcher::MatchMs1WithAllTables(Mzml& mzml, Database& database)
{
    auto ms1_ptr_v = mzml.GetLocalMs1Vector();

    // 清空上次配对的结果
    vector<pair<cardiolipin_ptr, cardiolipin_ptr>>().swap(this->m_cardiolipin);

    // 进行搜索
    MatchMS1With2Tables(ms1_ptr_v, database.GetLocalClPair(), cl);
    MatchMS1With2Tables(ms1_ptr_v, database.GetLocalMlclPair(), mlcl);
    MatchMS1With2Tables(ms1_ptr_v, database.GetLocalDlclPair(), dlcl);

    // 去除含氧的心磷脂
    this->DeleteCardiolipinWithOxygen();

    // 发送信号
    this->PrintTypesCount();
}

void MS1LibraryMatcher::MatchMS1With2Tables(std::vector<ms1_ptr>& ms1_ptr_v, std::pair<db_ptr_v, db_ptr_v> db_ptr_v_pair, Type type)
{
    // 提取[M-H]-和[M-2H]2-的数据库
    db_ptr_v m_h_db = db_ptr_v_pair.first;
    db_ptr_v m_2h_db = db_ptr_v_pair.second;

    // 两个multimap，用于匹配[M-H]-和[M-2H]2-
    multimap<int, ms1_ptr> m_h_find_map;
    multimap<int, ms1_ptr> m_h_left_map;

    // 遍历一级峰表，二分查找搜索[M-H]-库
    for (auto ms1_itr : ms1_ptr_v) {
        int left = 0;
        int right = m_h_db.size() - 1;
        while (left <= right) {
            int mid = (left + right) / 2;
            float m_h_min_mz = m_h_db.at(mid)->GetMz() - (m_h_db.at(mid)->GetMz() * m_ppm / 1000000);
            float m_h_max_mz = m_h_db.at(mid)->GetMz() + (m_h_db.at(mid)->GetMz() * m_ppm / 1000000);
            if ((ms1_itr->GetMz() >= m_h_min_mz) && (ms1_itr->GetMz() <= m_h_max_mz)) {
                // 加入到哈希表中
                m_h_find_map.insert({ mid, ms1_itr });

                // mid的左边一位和右边一位的各自的符合区间
                int left_t = mid - 1;
                int right_t = mid + 1;

                float m_h_left_t_min_mz;
                float m_h_left_t_max_mz;
                float m_h_right_t_min_mz;
                float m_h_right_t_max_mz;
                while (left_t >= 0) {
                    // 更新区间
                    m_h_left_t_min_mz = m_h_db.at(left_t)->GetMz() - (m_h_db.at(left_t)->GetMz() * m_ppm / 1000000);
                    m_h_left_t_max_mz = m_h_db.at(left_t)->GetMz() + (m_h_db.at(left_t)->GetMz() * m_ppm / 1000000);
                    if ((ms1_itr->GetMz() >= m_h_left_t_min_mz) && (ms1_itr->GetMz() <= m_h_left_t_max_mz)) {
                        m_h_find_map.insert({ left_t, ms1_itr }); // 加入到哈希表中
                        left_t--; // 向左继续找
                    } else {
                        break;
                    }
                }

                while (right_t <= int(m_h_db.size() - 1)) {
                    // 更新区间
                    m_h_right_t_min_mz = m_h_db.at(right_t)->GetMz() - (m_h_db.at(right_t)->GetMz() * m_ppm / 1000000);
                    m_h_right_t_max_mz = m_h_db.at(right_t)->GetMz() + (m_h_db.at(right_t)->GetMz() * m_ppm / 1000000);
                    if ((ms1_itr->GetMz() >= m_h_right_t_min_mz) && (ms1_itr->GetMz() <= m_h_right_t_max_mz)) {
                        m_h_find_map.insert({ right_t, ms1_itr });
                        right_t++; // 向右继续寻找
                    } else {
                        break;
                    }
                }

                break; // 找完左边和右边的，就说明ms1_itr的元素已经找完了，跳出这个while
            }
            // 如果mz大于mid对应的mz，说明在右侧可能有配对的
            else if (ms1_itr->GetMz() >= m_h_max_mz) {
                left = mid + 1;
            }
            // 如果mz小于mid对应的mz，说明在左侧可能有配对的
            else if (ms1_itr->GetMz() <= m_h_min_mz) {
                right = mid - 1;
            }
        }
    }

    // 赋值，m_h_left_hash_map用来保留那些仅有M-H的
    m_h_left_map = m_h_find_map;

    // lamba函数
    // 匹配M-H和M-2H的
    auto MatchM_hWithM_2H = [this, &m_h_find_map, type, &m_h_left_map, &m_2h_db, &m_h_db](int key, ms1_ptr ms1_ptr) {
        // 寻找M-H中是否匹配上同一个心磷脂，M-H和M-2H是同一个mid，多重哈希表返回第一个对应位置的迭代器
        // 1.如果无法找到对应的M-H
        auto m_h_find_map_pair_itr = m_h_find_map.equal_range(key);
        if (m_h_find_map_pair_itr.first == m_h_find_map_pair_itr.second) {
            cardiolipin_ptr m_h = CreateCardiolipin(nullptr, nullptr, type);
            cardiolipin_ptr m_2h = CreateCardiolipin(ms1_ptr, m_2h_db.at(key), type);
            // 加入结果中
            this->m_cardiolipin.push_back({ m_h, m_2h });
            // 退出
            return;
        }

        // 2.如果能找到对应的M-H
        bool merge_success = 0;
        for (auto itr = m_h_find_map_pair_itr.first; itr != m_h_find_map_pair_itr.second; itr++) {
            if (fabs(itr->second->GetRt() - ms1_ptr->GetRt()) <= this->m_torlerance_rt) {
                merge_success = 1;
                cardiolipin_ptr m_h = CreateCardiolipin(itr->second, m_h_db.at(key), type);
                cardiolipin_ptr m_2h = CreateCardiolipin(ms1_ptr, m_2h_db.at(key), type);
                // 加入结果中
                this->m_cardiolipin.push_back({ m_h, m_2h });
            }
        }
        // 如果M-H和M-2H没有合并成功，则把M-2H加入结果中
        if (!merge_success) {
            cardiolipin_ptr m_h = CreateCardiolipin(nullptr, nullptr, type);
            cardiolipin_ptr m_2h = CreateCardiolipin(ms1_ptr, m_2h_db.at(key), type);
            // 加入结果中
            this->m_cardiolipin.push_back({ m_h, m_2h });
            return;
        }

        // 3.从m_h_left_map去掉能跟M-2H匹配上的ms1_ptr，使得m_h_left_map剩下的都是没有对应M-2H的
        auto m_h_left_map_pair_itr = m_h_left_map.equal_range(key);
        if (m_h_left_map_pair_itr.first != m_h_left_map_pair_itr.second) {
            for (auto itr = m_h_left_map_pair_itr.first; itr != m_h_left_map_pair_itr.second;) {
                if (fabs(itr->second->GetRt() - ms1_ptr->GetRt()) <= this->m_torlerance_rt) {
                    itr = m_h_left_map.erase(itr);
                } else {
                    itr++;
                }
            }
        }
    };

    // 遍历一级峰表，二分查找搜索[M-2H]2-库
    for (auto ms1_itr : ms1_ptr_v) {
        int left = 0;
        int right = m_h_db.size() - 1;
        while (left <= right) {
            int mid = (left + right) / 2;
            float m_2h_min_mz = m_2h_db.at(mid)->GetMz() - (m_2h_db.at(mid)->GetMz() * m_ppm / 1000000);
            float m_2h_max_mz = m_2h_db.at(mid)->GetMz() + (m_2h_db.at(mid)->GetMz() * m_ppm / 1000000);
            if ((ms1_itr->GetMz() >= m_2h_min_mz) && (ms1_itr->GetMz() <= m_2h_max_mz)) {
                // 匹配M-H和M-2H，并加入结果中
                MatchM_hWithM_2H(mid, ms1_itr);
                int left_t = mid - 1;
                int right_t = mid + 1;
                // mid的左边一位和右边一位的各自的符合区间
                while (left_t >= 0) {
                    float m_2h_left_t_min_mz = m_2h_db.at(left_t)->GetMz() - (m_2h_db.at(left_t)->GetMz() * m_ppm / 1000000);
                    float m_2h_left_t_max_mz = m_2h_db.at(left_t)->GetMz() + (m_2h_db.at(left_t)->GetMz() * m_ppm / 1000000);
                    if ((ms1_itr->GetMz() >= m_2h_left_t_min_mz) && (ms1_itr->GetMz() <= m_2h_left_t_max_mz)) {
                        MatchM_hWithM_2H(left_t, ms1_itr);
                        left_t--; // 向左继续找
                    } else {
                        break;
                    }
                }

                while (right_t <= int(m_2h_db.size() - 1)) {
                    float m_2h_right_t_min_mz = m_2h_db.at(right_t)->GetMz() - (m_2h_db.at(right_t)->GetMz() * m_ppm / 1000000);
                    float m_2h_right_t_max_mz = m_2h_db.at(right_t)->GetMz() + (m_2h_db.at(right_t)->GetMz() * m_ppm / 1000000);
                    if ((ms1_itr->GetMz() >= m_2h_right_t_min_mz) && (ms1_itr->GetMz() <= m_2h_right_t_max_mz)) {
                        MatchM_hWithM_2H(right_t, ms1_itr);
                        right_t++;
                    } else {
                        break;
                    }
                }

                break;

            } else if (ms1_itr->GetMz() >= m_2h_min_mz) {
                left = mid + 1;
            } else if (ms1_itr->GetMz() <= m_2h_max_mz) {
                right = mid - 1;
            }
        }
    }

    // 遍历哈希表，m_h_left_map仅有M-H，其中既有M-H又有M-2H已经在MatchM_hWithM_2h被排除了
    for (auto map_itr = m_h_left_map.begin(); map_itr != m_h_left_map.end(); map_itr++) {
        cardiolipin_ptr m_h = CreateCardiolipin(map_itr->second, m_h_db.at(map_itr->first), type);
        cardiolipin_ptr m_2h = CreateCardiolipin(nullptr, nullptr, type);
        this->m_cardiolipin.push_back({ m_h, m_2h });
    }

    // 进行打分
    float constant = -((1.17741 / pow((m_ppm_with_half_score) / static_cast<double>(1000000), 2)) / 2);
    for (auto cardiolipin_itr_pair : m_cardiolipin) {
        cardiolipin_itr_pair.first->ScoreMS1(constant);
        cardiolipin_itr_pair.second->ScoreMS1(constant);
    }
}

void MS1LibraryMatcher::SetParameter(Parameter& par)
{
    m_ppm = par.m_ms1_ppm;
    m_torlerance_rt = par.m_ms1_torlerance_rt;
    m_ppm_with_half_score = par.m_ms1_ppm_with_half_score;
}

cardiolipin_ptr CreateCardiolipin(ms1_ptr ms1_ptr, db_ptr db_ptr, Type type)
{
    if (type == cl) {
        if (ms1_ptr == nullptr && db_ptr == nullptr) {
            return make_shared<CL>();
        } else {
            return make_shared<CL>(ms1_ptr, db_ptr);
        }
    } else if (type == mlcl) {
        if (ms1_ptr == nullptr && db_ptr == nullptr) {
            return make_shared<MLCL>();
        } else {
            return make_shared<MLCL>(ms1_ptr, db_ptr);
        }
    } else if (type == dlcl) {
        if (ms1_ptr == nullptr && db_ptr == nullptr) {
            return make_shared<DLCL>();
        } else {
            return make_shared<DLCL>(ms1_ptr, db_ptr);
        }
    } else {
        if (ms1_ptr == nullptr && db_ptr == nullptr) {
            return make_shared<Cardiolipin>();
        } else {
            return make_shared<Cardiolipin>(ms1_ptr, db_ptr);
        }
    }
}
