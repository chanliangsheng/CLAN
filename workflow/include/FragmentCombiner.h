#ifndef FRAGMENTCOMBINER_H
#define FRAGMENTCOMBINER_H
#include <cardiolipin.h>
#include <workflow.h>

class FragmentCombiner : public Workflow {
public:
    using cardiolipin_ptr_pair = std::pair<cardiolipin_ptr, cardiolipin_ptr>;

public:
    FragmentCombiner();
    FragmentCombiner(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector);

    // splice
public:
    void Splice();

    // Get
public:
    std::map<std::shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>> GetMergeResult(); // 获取合并的拼接结果

    // merge
public:
    void MergePair(cardiolipin_ptr_pair pair);

    // 设置参数
public:
    void SetParameter(Parameter& par) override;

    // output
public:
    void PrintTypesCount() override;
    void OutputCsv(QString file_name) override;

protected:
    std::map<std::shared_ptr<cardiolipin_ptr_pair>, std::list<spec_stru_ptr>> m_merge_result; // 合并[M-H]和[M-2H]后的结果
    mode m_mode = flexible; // M-H和M-2H合并的模式
};

#endif // FRAGMENTCOMBINER_H
