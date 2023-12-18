#ifndef MS2_H
#define MS2_H

#include <QDebug>
#include <algorithm>
#include <cmath>
#include <fa.h>
#include <headgroup.h>
#include <pa.h>
#include <queue>
#include <vector>

using pa_ptr = std::shared_ptr<Pa>;
using fa_ptr = std::shared_ptr<Fa>;
using headgroup_ptr = std::shared_ptr<Headgroup>;
using fg_v_ptr = std::shared_ptr<std::vector<float>>;

class Ms2 {
public:
    Ms2();
    Ms2(float precursor_ion_mz, float precursor_ion_intensity, float rt, fg_v_ptr fragment_ion_mz, fg_v_ptr fragment_ion_intensity);
    Ms2(float precursor_ion_mz, float precursor_ion_intensity, float rt, std::shared_ptr<std::vector<double>> fragment_ion_mz, std::shared_ptr<std::vector<double>> fragment_ion_intensity);

public:
    void SetPrecuisorIonMz(float precuisor_ion_mz);
    void SetPrecuisorIonIntensity(float precursor_ion_intensity);
    void SetRt(float rt);
    void SetFragmentIonMz(fg_v_ptr fragment_ion_mz);
    void SetFragmentIonIntensity(fg_v_ptr fragment_ion_intensity);
    void SetHeadgroup(std::vector<headgroup_ptr> headgroup);

    float GetPrecuisorIonMz();
    float GetPrecuisorIonIntensity();
    float GetRt();
    fg_v_ptr GetFragmentIonMz();
    fg_v_ptr GetFragmentIonIntensity();
    float GetTotalIntensity();
    void CalculateTotalIntensity();
    std::vector<headgroup_ptr> GetHeadgroup();

    void ClearHeadgroup();
    void ClearPaInfo();
    void ClearFaInfo();

    float GetMaxFragmentIntensity();
    float GetMinFragmentIntensity();
    void EmplaceBackHeadgroup(headgroup_ptr headgroup);
    void EmplaceBackPa(pa_ptr pa);
    void EmplaceBackFa(fa_ptr fa);
    int GetPaCount();
    int GetFaCount();
    std::vector<fa_ptr> GetFaPtrVector();
    std::vector<pa_ptr> GetPaPtrVector();
    void SortPaByChainLength();
    bool m_pa_sort_by_chain_length = 0;
    void SortFaByChainLength();
    bool m_fa_sort_by_chain_length = 0;

    void SortFragmentIonMz(); // 对碎片的mz进行排序，并且intensity的位置也应该随着mz进行改变

public:
    void DeleteLowIntensityFragment(float radio); // 删除intensity最低的radio那么多个
private:
    // 前体离子的信息
    float m_precursor_ion_mz = 0;
    float m_precursor_ion_intensity = 0;
    float m_total_intensity = 0;
    float m_rt = 0;
    // 碎片离子的信息
    fg_v_ptr m_fragment_ion_mz;
    fg_v_ptr m_fragment_ion_intensity;
    // 头基信息
    std::vector<headgroup_ptr> m_headgroup;
    // Fa和Pa的信息
    std::vector<pa_ptr> m_pa;
    std::vector<fa_ptr> m_fa;
};

#endif // MS2_H
