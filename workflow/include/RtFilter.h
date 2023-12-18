#ifndef RTFILTER_H
#define RTFILTER_H

#include <FragmentCombiner.h>

class RtFilter : public FragmentCombiner {

public:
    RtFilter(FragmentCombiner fragment_combiner);
    RtFilter();

    // Set
public:
    void Set(FragmentCombiner fragment_combiner); // 设置心磷脂数据，把合并的拼接的结果赋值给这个对象
    void Set(FragmentCombiner* fragment_combiner_ptr);

    // work
public:
    void Filter();

    // par
public:
    void SetParameter(Parameter& par) override;

    // Rt-base remove
public:
    void RemoveOutlier(); // 去除离群值
    void MergeSameCardiolipin(); // 合并相同的心磷脂
    void CarbonNumberLaw(); // 通过碳数规律去除心磷脂

    // Get
public:
    std::set<std::array<int, 3>> GetTop1UniqueFaSet(std::list<spec_stru_ptr> list); // 获取top-1的拼接结果的FA结构

private:
    float m_rt_tor = 0.1; // 合并相同的心磷脂的时候的容忍值
};

// 用于碳数规律比较的类
class SimpleCardiolipin {
public:
    SimpleCardiolipin();
    ~SimpleCardiolipin();
    SimpleCardiolipin(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolinpin_pair_ptr, std::list<spec_stru_ptr> spec_stru_ptr_list);

public:
    int GetChainLength();
    int GetUnsaturation();
    int GetOxygen();
    float GetRt();
    void SetRt(float rt);
    float GetScore();
    void SetScore(float score);
    int GetComparisonTimes();
    void SetComparisonTimes(int comparison_time);

    void CalculateFinalScore();
    void RetSetScore();
    void RetSetComparisonTimes();
    void FollowCarbonNumberLaw(SimpleCardiolipin& other_cardiolipin); // 两个心磷脂比较是否符合碳数规律

public:
    std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> m_cardiolinpin_pair_ptr;
    std::list<spec_stru_ptr> m_spec_stru_ptr_list;

private:
    int m_chain_length = 0;
    int m_unsaturation = 0;
    int m_oxygen = 0;
    float m_rt = 0;
    double m_score = 1; // 分数
    int m_comparison_times = 0; // 比较的次数
};

bool AllFallowCarbonNumberLaw(std::vector<SimpleCardiolipin>& cardiolipin_vector);
#endif // RTFILTER_H
