#ifndef DLCLSPECIFICSTRUCTURE_H
#define DLCLSPECIFICSTRUCTURE_H
#include <SpecificStructure.h>
#include <cardiolipin.h>
#include <mzml.h>
#include <set>

class DlclSpecificStructure final : public SpecificStructure {
public:
    using dlcl_spec_stru_ptr = std::shared_ptr<DlclSpecificStructure>;

public:
    DlclSpecificStructure();
    DlclSpecificStructure(fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, ms2_ptr ms2_ptr);
    DlclSpecificStructure(pa_ptr pa_1_ptr, fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, ms2_ptr ms2_ptr);
    DlclSpecificStructure(pa_ptr pa_1_ptr, ms2_ptr ms2_ptr);

    // score
public:
    void Score() override;

    // other
public:
    QString ShowSimpleInfomation() override;

    // merge
public:
    bool CopyFrom(spec_stru_ptr other) override;
    bool CopyFrom(spec_stru_ptr other, mode mode) override;
    std::vector<fa_ptr> GetAllFa() override;
    void Update() override;
};

#endif // DLCLSPECIFICSTRUCTURE_H
