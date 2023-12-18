#ifndef CARDIOLIPIN_H
#define CARDIOLIPIN_H

#include <QDebug>
#include <QString>
#include <SpecificStructure.h>
#include <database.h>
#include <list>
#include <mzml.h>

enum Type { cl,
    mlcl,
    dlcl,
    cardiolipin };

// 计算链长，不饱和度，氧个数的函数
int calculate_total_chain(std::initializer_list<pa_ptr> pa_ptr_list);
int calculate_total_unsaturation(std::initializer_list<pa_ptr> pa_ptr_list);
int calculate_total_oxygen(std::initializer_list<pa_ptr> pa_ptr_list);

class Cardiolipin {
public:
    Cardiolipin();
    Cardiolipin(ms1_ptr ms1_ptr, db_ptr db_ptr);

    // MS1
public:
    void ScoreMS1(float constant);
    float GetMS1Mz();
    float GetMS1Rt();
    float GetMS1Intensity();
    float GetMS1Area();
    const ms1_ptr GetMS1Ptr();
    const db_ptr GetMS1DbPtr();
    int GetChainLength();
    int GetUnsaturation();
    int GetOxygen();
    std::array<int, 3> GetCompound();

    // MS2
public:
    void AddMS2(ms2_ptr ms2_ptr);
    bool HasMS2();
    std::vector<ms2_ptr> GetMS2PtrVector();
    void SetMS2PtrVector(std::vector<ms2_ptr> ms2_ptr_v);

    // splice
public:
    virtual void splice(); // 拼接函数
    std::vector<std::array<int, 4>> FourSum(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target); // 四数之和
    std::vector<std::array<int, 2>> TwoSum(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target); // 两数之和
    std::vector<std::array<int, 2>> TwoSum(std::vector<pa_ptr>& pa_ptr_vector, std::array<int, 3> target); // 两数之和
    std::vector<std::array<int, 3>> ThreeSum(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target); // 三数之和
    std::vector<std::array<int, 2>> TwoSum_Not_Repeat(std::vector<fa_ptr>& fa_ptr_vector, std::array<int, 3> target); // 两数之和

    // merge
public:
    void MergeSplice(); // 合并[M-H]和[M-2H]

    // other
public:
    bool CheckValid();
    void Clear(); // 清空这个对象的信息
    virtual Type GetType(); // 得到这个心磷脂的类型
    std::list<spec_stru_ptr> GetSpecificStructureList();

    // copy
public:
    virtual std::shared_ptr<Cardiolipin> Clone(); // 复制一个新的心磷脂
    virtual std::list<spec_stru_ptr> GetNewSpecificStructureList(); // 复制一组新的精细结构

protected:
    ms1_ptr m_ms1_ptr;
    db_ptr m_ms1_db_ptr;
    std::vector<ms2_ptr> m_ms2_ptr_v;
    float m_ms1_matching_score;
    std::list<spec_stru_ptr> m_specific_structure_vector; // 精细结构
};

#endif // SINGLEMS1MATCH_H
