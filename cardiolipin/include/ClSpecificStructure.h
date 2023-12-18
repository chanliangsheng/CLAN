#ifndef CLSPECIFICSTRUCTURE_H
#define CLSPECIFICSTRUCTURE_H

#include <SpecificStructure.h>
#include <cardiolipin.h>
#include <mzml.h>
#include <set>


class ClSpecificStructure final : public SpecificStructure
{
public:
    using cl_spec_stru_ptr = std::shared_ptr<ClSpecificStructure>;

public:
    ClSpecificStructure();
    ~ClSpecificStructure();
    ClSpecificStructure(fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, fa_ptr fa_3_ptr, fa_ptr fa_4_ptr, ms2_ptr ms2_ptr);
    ClSpecificStructure(pa_ptr pa_1_ptr, pa_ptr pa_2_ptr, ms2_ptr ms2_ptr);
    ClSpecificStructure(pa_ptr pa_1_ptr, fa_ptr fa_1_ptr, fa_ptr fa_2_ptr, pa_ptr pa_2_ptr, fa_ptr fa_3_ptr, fa_ptr fa_4_ptr, ms2_ptr ms2_ptr);

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
    PaNode m_right_pa;
};

#endif // CLSPECIFICSTRUCTURE_H
