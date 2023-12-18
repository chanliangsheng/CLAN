#ifndef MS1LIBRARYMATCH_H
#define MS1LIBRARYMATCH_H

#include <CL.h>
#include <DLCL.h>
#include <MLCL.h>
#include <database.h>
#include <functional>
#include <map>
#include <workflow.h>

// 根据不同的type来生成不同的子类
cardiolipin_ptr CreateCardiolipin(ms1_ptr ms1_ptr, db_ptr db_ptr, Type type);

class MS1LibraryMatcher : public Workflow {
public:
    MS1LibraryMatcher();

public:
    // 对CL，MLCL，DLCL进行搜索
    void MatchMs1WithAllTables(Mzml& mzml, Database& database);

private:
    // 对某些库进行搜索
    void MatchMS1With2Tables(std::vector<ms1_ptr>& ms1_ptr_v, std::pair<db_ptr_v, db_ptr_v> db_ptr_v_pair, Type type);

    // 设置参数
public:
    void SetParameter(Parameter& par) override;

private:
    float m_ppm = 5; // ppm，初始化为5
    float m_torlerance_rt = 0.2; // 可容忍的时间，初始化为0.2min
    float m_ppm_with_half_score = 5; // 设定ppm为多少的时候分数为0.5，初始值为5
};

#endif // MS1MATCH_H
