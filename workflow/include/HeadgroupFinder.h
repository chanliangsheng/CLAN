#ifndef HEADGROUPFINDER_H
#define HEADGROUPFINDER_H
#include <workflow.h>
class HeadgroupFinder : public Workflow {
public:
    HeadgroupFinder();
    HeadgroupFinder(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector);

public:
    void FindMS2Headgroup(); // 检查心磷脂的二级是否有头基

    // 设置参数
public:
    void SetParameter(Parameter& par) override;

private:
    float m_ppm = 30;
    float m_ppm_with_half_score = 30; // 设定ppm为多少的时候分数为0.5，初始值为30
    float m_mz_score_weight = 0.5;
};
#endif // HEADGROUPFINDER_H
