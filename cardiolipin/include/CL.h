#ifndef CL_H
#define CL_H

#include <ClSpecificStructure.h>
#include <cardiolipin.h>

class CL : public Cardiolipin {
public:
    CL();
    CL(ms1_ptr ms1_ptr, db_ptr db_ptr);

    // splice
public:
    void splice() override; // 覆写父类的splice函数
    void FourFaSpliceCl(ms2_ptr ms2_ptr); // 用4个FA来拼接这个Cl
    void TwoPaSpliceCl(ms2_ptr ms2_ptr); // 用2个PA来拼接这个Cl
    void TwoPaFourFaSpliceCl(ms2_ptr ms2_ptr); // 2个PA和4个FA拼接这个Cl

    // copy
public:
    std::shared_ptr<Cardiolipin> Clone() override;

    // other
public:
    Type GetType() override;
    std::list<spec_stru_ptr> GetNewSpecificStructureList() override;
};

#endif // CL_H
