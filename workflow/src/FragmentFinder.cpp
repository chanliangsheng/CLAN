#include <FragmentFinder.h>

using namespace std;

FragmentFinder::FragmentFinder()
{
}

FragmentFinder::FragmentFinder(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector)
    : Workflow(cardiolipin_pair_vector)
{
}

void FragmentFinder::FindMS2PAAndFA(Database& database)
{
    // 定义数据库指针
    using db_ptr = shared_ptr<db_ptr_v>;

    // FA与PA数据库
    auto pa_db = database.GetLocalPA();
    auto fa_db = database.GetLocalFA();

    // FA与PA数据库的指针
    auto pa_db_v = make_shared<db_ptr_v>(pa_db);
    auto fa_db_v = make_shared<db_ptr_v>(fa_db);

    // 计算头基分数所需要的常量
    float constant = -((1.17741 / pow((m_ppm_with_half_score) / static_cast<double>(1000000), 2)) / 2);

    // 定义寻找的类型
    enum find_type { PA,
        FA };

    // 在某个二级中寻找PA或者FA
    auto FindInDB = [this, &constant](db_ptr db, ms2_ptr ms2, find_type type) {
        // 二级的碎片信息
        auto fragment_mz = ms2->GetFragmentIonMz();
        auto fragment_intensity = ms2->GetFragmentIonIntensity();

        // 获取最大的intensity
        float max_intensity = ms2->GetMaxFragmentIntensity();
        // 索引
        int library_left = 0;
        int library_right = db->size() - 1;
        int sample_left = 0;
        int sample_right = fragment_mz->size() - 1;

        // 遍历sample
        while (sample_left <= sample_right) {
            int left = library_left;
            int right = library_right;
            bool find_sample_left = 0;
            bool find_sample_right = 0;

            float sample_mz = fragment_mz->at(sample_left);
            float sample_intensity = fragment_intensity->at(sample_left);
            // 用sample_left遍历数据库
            while (left <= right) {
                int mid = (left + right) / 2;
                float min_mz = db->at(mid)->GetMz() - db->at(mid)->GetMz() * this->m_ppm / 1000000;
                float max_mz = db->at(mid)->GetMz() + db->at(mid)->GetMz() * this->m_ppm / 1000000;
                // 如果满足条件
                if (sample_mz >= min_mz && sample_mz <= max_mz) {
                    // 说明找到了
                    find_sample_left = 1;
                    int oxygen_limit = ceil((db->at(mid)->GetChainLength() - (db->at(mid)->GetUnsaturation() * 2 - 2)) / 2); // 允许的氧的最大个数
                    float intensity_score = sample_intensity / max_intensity; // intensity分数

                    // 如果这个氧个数在允许的最大个数之内，则说明是正确的PA
                    if (oxygen_limit >= db->at(mid)->GetOxygen()) {
                        float real_ppm = (sample_mz - db->at(mid)->GetMz()) / db->at(mid)->GetMz(); // 计算真实ppm
                        float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                        float total_score = m_mz_score_weight * mz_score + (1 - m_mz_score_weight) * intensity_score; // 最终分数
                        // 如果是PA，则增加到ms2的PA向量中；否则，加入到FA向量中
                        if (type == PA) {
                            pa_ptr pa = make_shared<Pa>(sample_mz, sample_intensity, total_score, db->at(mid));
                            ms2->EmplaceBackPa(pa);
                        } else {
                            fa_ptr fa = make_shared<Fa>(sample_mz, sample_intensity, total_score, db->at(mid));
                            ms2->EmplaceBackFa(fa);
                        }
                    }

                    // 向左和右继续寻找
                    int left_t = mid - 1;
                    int right_t = mid + 1;

                    // 向左寻找
                    while (left_t >= 0) {
                        // 更新区间
                        float left_t_min_mz = db->at(left_t)->GetMz() - db->at(left_t)->GetMz() * m_ppm / 1000000;
                        float left_t_max_mz = db->at(left_t)->GetMz() + db->at(left_t)->GetMz() * m_ppm / 1000000;
                        if (sample_mz >= left_t_min_mz && sample_mz <= left_t_max_mz) {
                            int oxygen_limit = ceil((db->at(left_t)->GetChainLength() - (db->at(left_t)->GetUnsaturation() * 2 - 2)) / 2); // 允许的氧的最大个数
                            // 如果这个氧个数在允许的最大个数之内，则说明是正确的PA
                            if (oxygen_limit >= db->at(left_t)->GetOxygen()) {
                                float real_ppm = (sample_mz - db->at(left_t)->GetMz()) / db->at(left_t)->GetMz(); // 计算真实ppm
                                float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                                float total_score = m_mz_score_weight * mz_score + (1 - m_mz_score_weight) * intensity_score; // 最终分数
                                // 如果是PA，则增加到ms2的PA向量中；否则，加入到FA向量中
                                if (type == PA) {
                                    pa_ptr pa = make_shared<Pa>(sample_mz, sample_intensity, total_score, db->at(left_t));
                                    ms2->EmplaceBackPa(pa);
                                } else {
                                    fa_ptr fa = make_shared<Fa>(sample_mz, sample_intensity, total_score, db->at(left_t));
                                    ms2->EmplaceBackFa(fa);
                                }
                            }
                            left_t--;
                        } else {
                            break;
                        }
                    }

                    // 向右寻找
                    while (right_t <= int(db->size() - 1)) {
                        // 更新区间
                        float right_t_min_mz = db->at(right_t)->GetMz() - db->at(right_t)->GetMz() * m_ppm / 1000000;
                        float right_t_max_mz = db->at(right_t)->GetMz() + db->at(right_t)->GetMz() * m_ppm / 1000000;
                        if (sample_mz >= right_t_min_mz && sample_mz <= right_t_max_mz) {
                            int oxygen_limit = ceil((db->at(right_t)->GetChainLength() - (db->at(right_t)->GetUnsaturation() * 2 - 2)) / 2); // 允许的氧的最大个数
                            // 如果这个氧个数在允许的最大个数之内，则说明是正确的PA
                            if (oxygen_limit >= db->at(right_t)->GetOxygen()) {
                                float real_ppm = (sample_mz - db->at(right_t)->GetMz()) / db->at(right_t)->GetMz(); // 计算真实ppm
                                float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                                float total_score = m_mz_score_weight * mz_score + (1 - m_mz_score_weight) * intensity_score; // 最终分数
                                // 如果是PA，则增加到ms2的PA向量中；否则，加入到FA向量中
                                if (type == PA) {
                                    pa_ptr pa = make_shared<Pa>(sample_mz, sample_intensity, total_score, db->at(right_t));
                                    ms2->EmplaceBackPa(pa);
                                } else {
                                    fa_ptr fa = make_shared<Fa>(sample_mz, sample_intensity, total_score, db->at(right_t));
                                    ms2->EmplaceBackFa(fa);
                                }
                            }
                            right_t++;
                        } else {
                            break;
                        }
                    }
                    // 更新left和library_left ++++++++++++++++++++++++++++++++++++++ // 重要！！！
                    left = left_t + 1;
                    library_left = left_t + 1;
                    // 向左和右都找完了，退出
                    break;
                } else if (sample_mz <= min_mz) {
                    right = mid - 1;
                } else if (sample_mz >= max_mz) {
                    left = mid + 1;
                }
            }

            // 如果两个sample是一样的，那么只需要进行一次比较就好了
            if (sample_left == sample_right) {
                break;
            }
            // 如果left已经超过了库的索引，说明这个sample的mz已经大于库最大的mz了，则退出
            else if (left > int(db->size() - 1)) {
                break;
            }

            // 如果sample_left在库中无法找到，则更新library_left为left
            if (!find_sample_left) {
                library_left = left;
            }

            // 复原right，left不用复原
            right = library_right;

            // 重新赋值
            sample_mz = fragment_mz->at(sample_right);
            sample_intensity = fragment_intensity->at(sample_right);

            // 进行和sample_left一样的遍历，用sample_right对应的mz和intensity进行遍历
            while (left <= right) {
                int mid = (left + right) / 2;
                float min_mz = db->at(mid)->GetMz() - db->at(mid)->GetMz() * this->m_ppm / 1000000;
                float max_mz = db->at(mid)->GetMz() + db->at(mid)->GetMz() * this->m_ppm / 1000000;
                // 如果满足条件
                if (sample_mz >= min_mz && sample_mz <= max_mz) {
                    // 说明找到了
                    find_sample_left = 1;
                    int oxygen_limit = ceil((db->at(mid)->GetChainLength() - (db->at(mid)->GetUnsaturation() * 2 - 2)) / 2); // 允许的氧的最大个数
                    float intensity_score = sample_intensity / max_intensity; // intensity分数

                    // 如果这个氧个数在允许的最大个数之内，则说明是正确的PA
                    if (oxygen_limit >= db->at(mid)->GetOxygen()) {
                        float real_ppm = (sample_mz - db->at(mid)->GetMz()) / db->at(mid)->GetMz(); // 计算真实ppm
                        float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                        float total_score = m_mz_score_weight * mz_score + (1 - m_mz_score_weight) * intensity_score; // 最终分数
                        // 如果是PA，则增加到ms2的PA向量中；否则，加入到FA向量中
                        if (type == PA) {
                            pa_ptr pa = make_shared<Pa>(sample_mz, sample_intensity, total_score, db->at(mid));
                            ms2->EmplaceBackPa(pa);
                        } else {
                            fa_ptr fa = make_shared<Fa>(sample_mz, sample_intensity, total_score, db->at(mid));
                            ms2->EmplaceBackFa(fa);
                        }
                    }

                    // 向左和右继续寻找
                    int left_t = mid - 1;
                    int right_t = mid + 1;

                    // 向左寻找
                    while (left_t >= 0) {
                        // 更新区间
                        float left_t_min_mz = db->at(left_t)->GetMz() - db->at(left_t)->GetMz() * m_ppm / 1000000;
                        float left_t_max_mz = db->at(left_t)->GetMz() + db->at(left_t)->GetMz() * m_ppm / 1000000;
                        if (sample_mz >= left_t_min_mz && sample_mz <= left_t_max_mz) {
                            int oxygen_limit = ceil((db->at(left_t)->GetChainLength() - (db->at(left_t)->GetUnsaturation() * 2 - 2)) / 2); // 允许的氧的最大个数
                            // 如果这个氧个数在允许的最大个数之内，则说明是正确的PA
                            if (oxygen_limit >= db->at(left_t)->GetOxygen()) {
                                float real_ppm = (sample_mz - db->at(left_t)->GetMz()) / db->at(left_t)->GetMz(); // 计算真实ppm
                                float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                                float total_score = m_mz_score_weight * mz_score + (1 - m_mz_score_weight) * intensity_score; // 最终分数
                                // 如果是PA，则增加到ms2的PA向量中；否则，加入到FA向量中
                                if (type == PA) {
                                    pa_ptr pa = make_shared<Pa>(sample_mz, sample_intensity, total_score, db->at(left_t));
                                    ms2->EmplaceBackPa(pa);
                                } else {
                                    fa_ptr fa = make_shared<Fa>(sample_mz, sample_intensity, total_score, db->at(left_t));
                                    ms2->EmplaceBackFa(fa);
                                }
                            }
                            left_t--;
                        } else {
                            break;
                        }
                    }

                    // 向右寻找
                    while (right_t <= int(db->size() - 1)) {
                        // 更新区间
                        float right_t_min_mz = db->at(right_t)->GetMz() - db->at(right_t)->GetMz() * m_ppm / 1000000;
                        float right_t_max_mz = db->at(right_t)->GetMz() + db->at(right_t)->GetMz() * m_ppm / 1000000;
                        if (sample_mz >= right_t_min_mz && sample_mz <= right_t_max_mz) {
                            int oxygen_limit = ceil((db->at(right_t)->GetChainLength() - (db->at(right_t)->GetUnsaturation() * 2 - 2)) / 2); // 允许的氧的最大个数
                            // 如果这个氧个数在允许的最大个数之内，则说明是正确的PA
                            if (oxygen_limit >= db->at(right_t)->GetOxygen()) {
                                float real_ppm = (sample_mz - db->at(right_t)->GetMz()) / db->at(right_t)->GetMz(); // 计算真实ppm
                                float mz_score = exp(constant * pow(real_ppm, 2)); // mz分数
                                float total_score = m_mz_score_weight * mz_score + (1 - m_mz_score_weight) * intensity_score; // 最终分数
                                // 如果是PA，则增加到ms2的PA向量中；否则，加入到FA向量中
                                if (type == PA) {
                                    pa_ptr pa = make_shared<Pa>(sample_mz, sample_intensity, total_score, db->at(right_t));
                                    ms2->EmplaceBackPa(pa);
                                } else {
                                    fa_ptr fa = make_shared<Fa>(sample_mz, sample_intensity, total_score, db->at(right_t));
                                    ms2->EmplaceBackFa(fa);
                                }
                            }
                            right_t++;
                        } else {
                            break;
                        }
                    }
                    // 更新library_right ++++++++++++++++++++++++++++++++++++++ // 重要！！！
                    library_right = right_t - 1;
                    // 向左和右都找完了，退出
                    break;
                } else if (sample_mz <= min_mz) {
                    right = mid - 1;
                } else if (sample_mz >= max_mz) {
                    left = mid + 1;
                }
            }

            // 如果sample_right在库中无法找到，则更新library_right为right
            if (!find_sample_right) {
                library_right = right;
            }

            // 移动指针
            sample_left++;
            sample_right--;
        }
    };

    // 在某个心磷脂中寻找PA或者FA
    auto FindInCardiolipin = [&FindInDB, &pa_db_v, &fa_db_v](cardiolipin_ptr cardiolipin) {
        auto ms2_ptr_v = cardiolipin->GetMS2PtrVector();

        // 遍历所有二级
        for (auto ms2_itr = ms2_ptr_v.begin(); ms2_itr != ms2_ptr_v.end(); ms2_itr++) {
            // 清空上次配对的结果
            (*ms2_itr)->ClearPaInfo();
            (*ms2_itr)->ClearFaInfo();
            FindInDB(pa_db_v, *ms2_itr, PA); // 搜索PA库
            FindInDB(fa_db_v, *ms2_itr, FA); // 搜索FA库
        }

        // 删除既没有PA又没有FA的二级
        for (auto ms2_itr = ms2_ptr_v.begin(); ms2_itr != ms2_ptr_v.end();) {
            // 如果既没有PA又没有FA，则把对二级的引用去除
            if (((*ms2_itr)->GetPaCount() == 0) && ((*ms2_itr)->GetFaCount() == 0)) {
                ms2_itr = ms2_ptr_v.erase(ms2_itr);
            } else {
                ms2_itr++;
            }
        }

        // 重新把二级赋值，不然不是原来删除掉部分二级的二级向量
        cardiolipin->SetMS2PtrVector(ms2_ptr_v);

        // 如果这个心磷脂的所有二级都没有PA和FA，则将这个心磷脂的信息清空
        if (ms2_ptr_v.size() == 0) {
            cardiolipin->Clear();
        }
    };

    // 进行PA和FA的查找
    for (auto itr = m_cardiolipin.begin(); itr != m_cardiolipin.end();) {
        FindInCardiolipin(itr->first);
        FindInCardiolipin(itr->second);
        // 如果M-H和M-2H都没有找到PA和FA，则删除这对心磷脂
        if (!itr->first->CheckValid() && !itr->second->CheckValid()) {
            itr = m_cardiolipin.erase(itr);
        } else {
            ++itr;
        }
    }

    // 删除由于查找PA和FA所带来的冗余心磷脂
    this->DeleteRedundantCardiolipinPair();

    // 发送信号
    this->PrintTypesCount();
}

void FragmentFinder::SetParameter(Parameter& par)
{
    m_ppm = par.m_fragmentfinder_ppm;
    m_ppm_with_half_score = par.m_fragmentfinder_ppm_with_half_score;
    m_mz_score_weight = par.m_fragmentfinder_mz_score_weight;
}
