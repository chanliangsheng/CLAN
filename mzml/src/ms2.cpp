#include "ms2.h"
using namespace std;
Ms2::Ms2()
{
    this->m_rt = 0;
    this->m_precursor_ion_mz = 0;
    this->m_precursor_ion_intensity = 0;
    this->m_fragment_ion_mz = fg_v_ptr();
    this->m_fragment_ion_intensity = fg_v_ptr();
}

Ms2::Ms2(float precursor_ion_mz, float precursor_ion_intensity, float rt, fg_v_ptr fragment_ion_mz, fg_v_ptr fragment_ion_intensity)
{
    // 有参构造函数
    this->m_rt = rt;
    this->m_fragment_ion_mz = fragment_ion_mz;
    this->m_precursor_ion_mz = precursor_ion_mz;
    this->m_fragment_ion_intensity = fragment_ion_intensity;
    this->m_precursor_ion_intensity = precursor_ion_intensity;
}

Ms2::Ms2(float precursor_ion_mz, float precursor_ion_intensity, float rt, std::shared_ptr<std::vector<double>> fragment_ion_mz, std::shared_ptr<std::vector<double>> fragment_ion_intensity)
{
    this->m_rt = rt;
    this->m_precursor_ion_mz = precursor_ion_mz;
    this->m_precursor_ion_intensity = precursor_ion_intensity;

    // 声明固定长度的vector
    m_fragment_ion_mz = std::make_shared<std::vector<float>>(fragment_ion_mz->size());
    m_fragment_ion_intensity = std::make_shared<std::vector<float>>(fragment_ion_intensity->size());

    // 转换为float数组
    std::transform(fragment_ion_mz->begin(), fragment_ion_mz->end(), this->m_fragment_ion_mz->begin(), [=](double x) { return (float)x; });
    std::transform(fragment_ion_intensity->begin(), fragment_ion_intensity->end(), this->m_fragment_ion_intensity->begin(), [=](double x) { return (float)x; });
}

void Ms2::SetPrecuisorIonMz(float precuisor_ion_mz)
{
    this->m_precursor_ion_mz = precuisor_ion_mz;
}

void Ms2::SetPrecuisorIonIntensity(float precursor_ion_intensity)
{
    this->m_precursor_ion_intensity = precursor_ion_intensity;
}

void Ms2::SetRt(float rt)
{
    this->m_rt = rt;
}

void Ms2::SetFragmentIonMz(fg_v_ptr fragment_ion_mz)
{
    this->m_fragment_ion_mz = fragment_ion_mz;
}

void Ms2::SetFragmentIonIntensity(fg_v_ptr fragment_ion_intensity)
{
    this->m_fragment_ion_intensity = fragment_ion_intensity;
}

void Ms2::SetHeadgroup(std::vector<headgroup_ptr> headgroup)
{
    this->m_headgroup = headgroup;
}

float Ms2::GetPrecuisorIonMz()
{
    return this->m_precursor_ion_mz;
}

float Ms2::GetPrecuisorIonIntensity()
{
    return this->m_precursor_ion_intensity;
}

float Ms2::GetRt()
{
    return this->m_rt;
}

fg_v_ptr Ms2::GetFragmentIonMz()
{
    return this->m_fragment_ion_mz;
}

fg_v_ptr Ms2::GetFragmentIonIntensity()
{
    return this->m_fragment_ion_intensity;
}

float Ms2::GetTotalIntensity()
{
    return this->m_total_intensity;
}

void Ms2::CalculateTotalIntensity()
{
    for (auto itr = this->m_fragment_ion_intensity->begin(); itr != this->m_fragment_ion_intensity->end(); itr++) {
        this->m_total_intensity = this->m_total_intensity + (*itr);
    }
}

std::vector<headgroup_ptr> Ms2::GetHeadgroup()
{
    return this->m_headgroup;
}

void Ms2::ClearHeadgroup()
{
    vector<headgroup_ptr>().swap(this->m_headgroup);
}

void Ms2::ClearPaInfo()
{
    vector<pa_ptr>().swap(this->m_pa);
}

void Ms2::ClearFaInfo()
{
    vector<fa_ptr>().swap(this->m_fa);
}

float Ms2::GetMaxFragmentIntensity()
{
    return *max_element(this->m_fragment_ion_intensity->begin(), this->m_fragment_ion_intensity->end());
}

float Ms2::GetMinFragmentIntensity()
{
    return *min_element(this->m_fragment_ion_intensity->begin(), this->m_fragment_ion_intensity->end());
}

void Ms2::EmplaceBackHeadgroup(headgroup_ptr headgroup)
{
    this->m_headgroup.emplace_back(headgroup);
}

void Ms2::EmplaceBackPa(pa_ptr pa)
{
    this->m_pa.emplace_back(pa);
}

void Ms2::EmplaceBackFa(fa_ptr fa)
{
    this->m_fa.emplace_back(fa);
}

int Ms2::GetPaCount()
{
    return this->m_pa.size();
}

int Ms2::GetFaCount()
{
    return this->m_fa.size();
}

vector<fa_ptr> Ms2::GetFaPtrVector()
{
    return m_fa;
}

vector<pa_ptr> Ms2::GetPaPtrVector()
{
    return m_pa;
}

void Ms2::SortPaByChainLength()
{
    if (this->m_pa_sort_by_chain_length == 0) {
        // 以FA的链长对fa_vector进行排序
        sort(this->m_pa.begin(), this->m_pa.end(), [](pa_ptr lhs, pa_ptr rhs) {
            return lhs->GetChainLength() < rhs->GetChainLength();
        });
        this->m_pa_sort_by_chain_length = 1;
    }
}

void Ms2::SortFaByChainLength()
{
    if (this->m_fa_sort_by_chain_length == 0) {
        // 以FA的链长对fa_vector进行排序
        sort(this->m_fa.begin(), this->m_fa.end(), [](fa_ptr lhs, fa_ptr rhs) {
            return lhs->GetChainLength() < rhs->GetChainLength();
        });
        this->m_fa_sort_by_chain_length = 1;
    }
}

void Ms2::SortFragmentIonMz()
{
    // 如果没有碎片mz的信息，则直接返回
    if (m_fragment_ion_mz->size() == 0) {
        return;
    }
    vector<pair<float, float>> pairs_v;
    pairs_v.reserve(m_fragment_ion_mz->size()); // 分配内存

    // 对pairs进行排序的方法
    auto compare = [](const pair<float, float>& p1, const pair<float, float>& p2) {
        return p1.first < p2.first;
    };

    // 排序
    sort(pairs_v.begin(), pairs_v.end(), compare);

    // 重新赋值
    for (int i = 0; i < m_fragment_ion_mz->size(); i++) {
        m_fragment_ion_mz->at(i) = pairs_v[i].first;
        m_fragment_ion_intensity->at(i) = pairs_v[i].second;
    }
}

void Ms2::DeleteLowIntensityFragment(float radio)
{
    if (radio < 0 || radio > 1) {
        qDebug() << "wrong radio!";
    }

    unsigned int delete_num = this->m_fragment_ion_mz->size() * radio; // 需要删除的数量

    // 如果不需要删除，则直接返回
    if (delete_num == 0) {
        return;
    }

    // 定义一个最小堆
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::less<std::pair<float, int>>> max_heap;

    // 遍历数组并将元素插入堆中，剩下的部分
    for (unsigned int i = 0; i < this->m_fragment_ion_intensity->size(); ++i) {
        max_heap.emplace(this->m_fragment_ion_intensity->at(i), i);
        if (max_heap.size() > delete_num) {
            max_heap.pop();
        }
    }

    vector<pair<float, int>> max_heap_vector;
    // 加入到vector中
    while (!max_heap.empty()) {
        max_heap_vector.push_back(max_heap.top());
        max_heap.pop();
    }

    // 对pairs中的第二个元素（即int）(位置)进行排序，结果为从大到小
    sort(max_heap_vector.begin(), max_heap_vector.end(), [](const pair<float, int>& a, const pair<float, int>& b) -> bool {
        return a.second > b.second;
    });

    // 删除元素
    for (auto itr = max_heap_vector.begin(); itr != max_heap_vector.end(); itr++) {
        this->m_fragment_ion_mz->erase(this->m_fragment_ion_mz->begin() + itr->second);
        this->m_fragment_ion_intensity->erase(this->m_fragment_ion_intensity->begin() + itr->second);
    }
}
