#ifndef MLCL_H
#define MLCL_H
#include <MlclSpecificStructure.h>
#include <cardiolipin.h>

class MLCL : public Cardiolipin {
public:
    MLCL();
    MLCL(ms1_ptr ms1_ptr, db_ptr db_ptr);

    // splice
public:
    void splice() override;
    void ThreeFaSpliceMlcl(ms2_ptr ms2_ptr); // 用3个FA来拼接成这个MLCl
    void OnePaThreeFaSpliceMlcl(ms2_ptr ms2_ptr); // 1个PA和3个FA拼接这个Mlcl

    // copy
public:
    std::shared_ptr<Cardiolipin> Clone() override;

public:
    Type GetType() override;
    std::list<spec_stru_ptr> GetNewSpecificStructureList() override;
};

#endif // MLCL_H
