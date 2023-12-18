#ifndef MLCLSPECIFICSTRUCTURE_H
#define MLCLSPECIFICSTRUCTURE_H
#include <SpecificStructure.h>
#include <cardiolipin.h>
#include <mzml.h>
#include <set>

class MlclSpecificStructure final: public SpecificStructure {
public:
    using mlcl_spec_stru_ptr = std::shared_ptr<MlclSpecificStructure>;

public:
    MlclSpecificStructure();
    ~MlclSpecificStructure();
    MlclSpecificStructure(fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, fa_ptr fa_3_ptr, ms2_ptr ms2_ptr);
    MlclSpecificStructure(pa_ptr pa_1_ptr, fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, fa_ptr fa_3_ptr, ms2_ptr ms2_ptr);
    MlclSpecificStructure(pa_ptr pa_1_ptr, fa_ptr fa_3_ptr, ms2_ptr ms2_ptr);

    // score
public:
    void Score() override;

    // merge
public:
    bool CopyFrom(spec_stru_ptr other) override;
    bool CopyFrom(spec_stru_ptr other, mode mode) override;
    std::vector<fa_ptr> GetAllFa() override;
    void Update() override;

    // other
public:
    QString ShowSimpleInfomation() override;

private:
    fa_ptr m_right_fa_ptr;
};
#endif // MLCLSPECIFICSTRUCTURE_H
