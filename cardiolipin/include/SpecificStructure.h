#ifndef SPECIFICSTRUCTURE_H
#define SPECIFICSTRUCTURE_H
#include <memory>
#include <mzml.h>
#include <set>
#include <tuple>
#include <unordered_map>

enum mode { strict,
    flexible }; // 两种合并的模式

class PaNode {
public:
    PaNode();
    PaNode(pa_ptr pa_ptr, fa_ptr left_fa_ptr, fa_ptr right_fa_ptr);
    PaNode(pa_ptr pa_ptr);
    PaNode(fa_ptr left_fa_ptr, fa_ptr right_fa_ptr);

    // Get
public:
    pa_ptr GetPaPtr();
    fa_ptr GetLeftFaPtr();
    fa_ptr GetRightFaPtr();
    void Set(fa_ptr left_fa_ptr, fa_ptr right_fa_ptr);

private:
    pa_ptr m_pa_ptr;
    fa_ptr m_left_fa_ptr;
    fa_ptr m_right_fa_ptr;
};

class SpecificStructure {
public:
    using spec_stru_ptr = std::shared_ptr<SpecificStructure>;

public:
    SpecificStructure();
    ~SpecificStructure();

    // 增加信息
public:
    void AddFaInfo(std::initializer_list<pa_ptr> pa_ptr_list); // 把PA信息加入pa_info
    void AddPaInfo(std::initializer_list<pa_ptr> pa_ptr_list); // 把FA信息加入fa_info

    // score
public:
    virtual void Score(); // 不同的子类打分的方法不同
    void Score(std::initializer_list<pa_ptr> fa_ptr_list); // 打分函数
    std::unordered_map<pa_ptr, int> CountUniqueFa(std::initializer_list<pa_ptr> pa_ptr_list); // 计算不重复的FA出现的次数
    static float m_fragment_score_weight; // 碎片分数的权重
    static float m_pa_exist_score_weight; // PA存在的权重
    static float m_fa_intensity_variance_score_weight; // FA的强度的方差分数的权重

    // merge
public:
    virtual bool CopyFrom(spec_stru_ptr other);
    virtual bool CopyFrom(spec_stru_ptr other, mode mode);
    float UpdateScore(float score_1, float score_2);
    virtual void Update();
    virtual std::vector<fa_ptr> GetAllFa();

    // other
public:
    bool CheckPaExist();
    bool CheckFaExist();
    float GetScore();
    std::set<std::array<int, 3>> GetUniqueFaSet(); // 获取这个拼接结果的FA的不重复集合
    std::set<std::array<int, 3>> GetUniquePaSet(); // 获取这个拼接结果的PA的不重复集合
    virtual QString ShowSimpleInfomation();

protected:
    ms2_ptr m_ms2_ptr;
    float m_score = 0;
    PaNode m_left_pa;
    bool m_pa_exist = 0;
    bool m_fa_exist = 0;
    std::set<std::array<int, 3>> m_pa_info; //{chain,unsaturation,oxygen}
    std::set<std::array<int, 3>> m_fa_info; //{chain,unsaturation,oxygen}
};
using spec_stru_ptr = std::shared_ptr<SpecificStructure>;
#endif // SPECIFICSTRUCTURE_H
