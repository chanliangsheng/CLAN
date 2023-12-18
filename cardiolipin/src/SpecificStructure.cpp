#include <SpecificStructure.h>

using namespace std;

float SpecificStructure::m_fragment_score_weight = 1;
float SpecificStructure::m_pa_exist_score_weight = 0.1;
float SpecificStructure::m_fa_intensity_variance_score_weight = 0.6;

SpecificStructure::SpecificStructure()
{
    m_ms2_ptr = nullptr;
    m_score = 0;
    m_left_pa = PaNode();
    m_pa_exist = 0;
    m_fa_exist = 0;
}

SpecificStructure::~SpecificStructure()
{
}

void SpecificStructure::AddFaInfo(std::initializer_list<pa_ptr> pa_ptr_list)
{
    for (auto fa_ptr : pa_ptr_list) {
        m_fa_info.insert({ fa_ptr->GetChainLength(), fa_ptr->GetUnsaturation(), fa_ptr->GetOxygen() });
    }
}

void SpecificStructure::AddPaInfo(std::initializer_list<pa_ptr> pa_ptr_list)
{
    for (auto pa_ptr : pa_ptr_list) {
        m_pa_info.insert({ pa_ptr->GetChainLength(), pa_ptr->GetUnsaturation(), pa_ptr->GetOxygen() });
    }
}

void SpecificStructure::Score()
{
    qDebug() << "Class SpecificStructure call this 'score' function!!!";
}

void SpecificStructure::Score(std::initializer_list<pa_ptr> fa_ptr_list)
{
    float fragment_score = 0;
    int fa_list_size = fa_ptr_list.size();

    // 如果这个组合的FA不存在，但是PA存在
    if (!this->m_fa_exist && this->m_pa_exist) {
        // 最后总分就是权重*1，因为pa存在就是1
        this->m_score = this->m_pa_exist_score_weight;
    }
    // 如果FA存在，PA不存在
    else if (this->m_fa_exist && !this->m_pa_exist) {
        unordered_map<pa_ptr, int> fa_hashmap = CountUniqueFa(fa_ptr_list); // 计算唯一的FA
        for (auto itr : fa_hashmap) {
            fragment_score += itr.first->GetScore();
        }
        // 碎片分数是总分的平均值
        fragment_score = fragment_score / fa_list_size;

        // fa强度标准差分数
        // 计算每个fa的分数
        vector<float> fa_score_vector;
        for (auto itr : fa_hashmap) {
            for (float i = 0; i < itr.second; i++) {
                fa_score_vector.push_back(itr.first->GetScore() / itr.second);
            }
        }
        // 求平方和
        float sum = std::accumulate(fa_score_vector.begin(), fa_score_vector.end(), 0.0, [fragment_score](float total, float value) {
            return total + std::pow(value - fragment_score, 2);
        });
        float fa_intensity_variance_score = 1 - sqrt(sum / fa_list_size); // 分数为1-标准差，方差越大，分数越小

        // 总分是碎片分数加上一致性分数
        this->m_score = this->m_fragment_score_weight * fragment_score + this->m_fa_intensity_variance_score_weight * fa_intensity_variance_score;
    }
    // 如果FA和PA都存在
    else if (this->m_fa_exist && this->m_pa_exist) {
        unordered_map<pa_ptr, int> fa_hashmap = CountUniqueFa(fa_ptr_list); // 计算唯一的FA
        for (auto itr : fa_hashmap) {
            fragment_score += itr.first->GetScore();
        }
        // 碎片分数是总分的平均值
        fragment_score = fragment_score / fa_list_size;

        // fa强度标准差分数
        // 计算每个fa的分数
        vector<float> fa_score_vector;
        for (auto itr : fa_hashmap) {
            for (float i = 0; i < itr.second; i++) {
                fa_score_vector.push_back(itr.first->GetScore() / itr.second);
            }
        }
        // 求平方和
        float sum = std::accumulate(fa_score_vector.begin(), fa_score_vector.end(), 0.0, [fragment_score](float total, float value) {
            return total + std::pow(value - fragment_score, 2);
        });
        float fa_intensity_variance_score = 1 - sqrt(sum / fa_list_size); // 分数为1-标准差，方差越大，分数越小
        // 总分是碎片分数加上一致性分数
        this->m_score = this->m_pa_exist_score_weight + this->m_fragment_score_weight * fragment_score + this->m_fa_intensity_variance_score_weight * fa_intensity_variance_score;
    }
}

std::unordered_map<pa_ptr, int> SpecificStructure::CountUniqueFa(std::initializer_list<pa_ptr> pa_ptr_list)
{
    unordered_map<pa_ptr, int> fa_hashmap;
    // 如果不存在，则创建，初始化分数为1；如果存在，则次数+1
    for (auto fa_ptr : pa_ptr_list) {
        fa_hashmap[fa_ptr]++;
    }

    return fa_hashmap;
}

bool SpecificStructure::CopyFrom(SpecificStructure::spec_stru_ptr other)
{
    Q_UNUSED(other);
    qDebug() << "Class SpecificStructure call this 'CopyFrom' function!!!";

    return false;
}

bool SpecificStructure::CopyFrom(SpecificStructure::spec_stru_ptr other, mode mode)
{
    Q_UNUSED(other);
    Q_UNUSED(mode);
    qDebug() << "Class SpecificStructure call this 'CopyFrom has mode' function!!!";
    return false;
}

float SpecificStructure::UpdateScore(float score_1, float score_2)
{
    return max(score_1, score_2);
}

void SpecificStructure::Update()
{
    // 清空FA和PA信息
    set<array<int, 3>>().swap(this->m_pa_info);
    set<array<int, 3>>().swap(this->m_fa_info);

    // 更新PA信息
    if (!this->m_left_pa.GetPaPtr()) {
        this->m_pa_exist = 1;
        this->m_pa_info.insert(this->m_left_pa.GetPaPtr()->GetCompound());
    }

    // 更新FA信息
    if (!this->m_left_pa.GetLeftFaPtr()) {
        this->m_fa_info.insert(this->m_left_pa.GetLeftFaPtr()->GetCompound());
    }

    if (!this->m_left_pa.GetRightFaPtr()) {
        this->m_fa_info.insert(this->m_left_pa.GetRightFaPtr()->GetCompound());
    }

    this->Score();
}

std::vector<fa_ptr> SpecificStructure::GetAllFa()
{
    vector<fa_ptr> res;
    res.reserve(2);

    auto left_fa = m_left_pa.GetLeftFaPtr();
    auto right_fa = m_left_pa.GetRightFaPtr();

    if (!left_fa) {
        res.push_back(left_fa);
    }

    if (!right_fa) {
        res.push_back(right_fa);
    }

    return res;
}

bool SpecificStructure::CheckPaExist()
{
    return m_pa_exist;
}

bool SpecificStructure::CheckFaExist()
{
    return m_fa_exist;
}

float SpecificStructure::GetScore()
{
    return m_score;
}

std::set<std::array<int, 3>> SpecificStructure::GetUniqueFaSet()
{
    return m_fa_info;
}

std::set<std::array<int, 3>> SpecificStructure::GetUniquePaSet()
{
    return m_pa_info;
}

QString SpecificStructure::ShowSimpleInfomation()
{
    qDebug() << "Class SpecificStructure call this 'ShowSimpleInfomation' function!!!";

    return "";
}

PaNode::PaNode()
{
    m_pa_ptr = nullptr;
    m_left_fa_ptr = nullptr;
    m_right_fa_ptr = nullptr;
}

PaNode::PaNode(pa_ptr pa_ptr, fa_ptr left_fa_ptr, fa_ptr right_fa_ptr)
{
    m_pa_ptr = pa_ptr;
    m_left_fa_ptr = left_fa_ptr;
    m_right_fa_ptr = right_fa_ptr;
}

PaNode::PaNode(pa_ptr pa_ptr)
{
    m_pa_ptr = pa_ptr;
}

PaNode::PaNode(fa_ptr left_fa_ptr, fa_ptr right_fa_ptr)
{
    m_pa_ptr = nullptr;
    m_left_fa_ptr = left_fa_ptr;
    m_right_fa_ptr = right_fa_ptr;
}

pa_ptr PaNode::GetPaPtr()
{
    return m_pa_ptr;
}

fa_ptr PaNode::GetLeftFaPtr()
{

    return m_left_fa_ptr;
}

fa_ptr PaNode::GetRightFaPtr()
{

    return m_right_fa_ptr;
}

void PaNode::Set(fa_ptr left_fa_ptr, fa_ptr right_fa_ptr)
{
    m_left_fa_ptr = left_fa_ptr;
    m_right_fa_ptr = right_fa_ptr;
}
