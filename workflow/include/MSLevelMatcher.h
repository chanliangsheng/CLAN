#ifndef MSLEVELMATCHER_H
#define MSLEVELMATCHER_H
#include <workflow.h>

class MSLevelMatcher : public Workflow {
public:
    MSLevelMatcher();
    MSLevelMatcher(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector);

public:
    void MatchCardiolipinWithMS2(Mzml& mzml);

    // 设置参数
public:
    void SetParameter(Parameter& par) override;

    // 重写
public:
    void OutputCsv(QString got_ms2_filename, QString without_ms2_filename);

private:
    float m_dalton = 0.5; // 初始化为0.5道尔顿
    float m_torlerance_rt = 0.25; // 初始化为15s
    std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> m_cardiolipin_without_ms2; // 无二级的心磷脂
};

#endif // MS1MATCH_H
