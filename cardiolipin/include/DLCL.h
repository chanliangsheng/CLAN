#ifndef DLCL_H
#define DLCL_H
#include <DlclSpecificStructure.h>
#include <cardiolipin.h>

class DLCL : public Cardiolipin {
public:
    DLCL();
    DLCL(ms1_ptr ms1_ptr, db_ptr db_ptr);

    // splice
public:
    void splice() override;
    void TwoFaSpliceDlcl(ms2_ptr ms2_ptr); // 用2个FA来拼接成这个DLCl
    void OnePaTwoFaSpliceDlcl(ms2_ptr ms2_ptr); // 1个PA和2个FA拼接这个Dlcl

    // copy
public:
    std::shared_ptr<Cardiolipin> Clone() override;

public:
    Type GetType() override;
    std::list<spec_stru_ptr> GetNewSpecificStructureList() override;
};

#endif // DLCL_H
