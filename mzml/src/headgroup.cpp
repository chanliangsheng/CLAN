#include <headgroup.h>

Headgroup::Headgroup()
{
    this->m_mz = 0;
    this->m_intensity = 0;
    this->m_score = 0;
}

Headgroup::Headgroup(float mz, float intensity, float score)
{
    this->m_mz = mz;
    this->m_intensity = intensity;
    this->m_score = score;
}

void Headgroup::SetMz(float mz)
{
    this->m_mz = mz;
}

void Headgroup::SetIntensity(float intensity)
{
    this->m_intensity = intensity;
}

void Headgroup::SetScore(float score)
{
    this->m_score = score;
}

float Headgroup::GetMz()
{
    return this->m_mz;
}

float Headgroup::GetIntensity()
{
    return this->m_intensity;
}

float Headgroup::GetScore()
{
    return this->m_score;
}
