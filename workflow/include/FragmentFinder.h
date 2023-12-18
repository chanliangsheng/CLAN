#ifndef FRAGMENTFINDER_H
#define FRAGMENTFINDER_H
#include <workflow.h>

class FragmentFinder : public Workflow {
public:
    FragmentFinder();
    FragmentFinder(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector);

public:
    void FindMS2PAAndFA(Database& database); // 寻找二级中的PA和FA

    // 设置参数
public:
    void SetParameter(Parameter& par) override;

private:
    float m_ppm = 30;
    float m_ppm_with_half_score = 5;
    float m_mz_score_weight = 0;
};

#endif // FRAGMENTFINDER_H
