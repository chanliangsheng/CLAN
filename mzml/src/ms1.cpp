#include <ms1.h>

using namespace std;
Ms1::Ms1()
{
    this->m_mz = 0;
    this->m_intensity = 0;
    this->m_rt = 0;
}

Ms1::Ms1(float mz, float intensity, float rt)
{
    // 有参构造函数
    this->m_mz = mz;
    this->m_intensity = intensity;
    this->m_rt = rt;
}

Ms1::Ms1(double mz, double intensity, double rt)
{
    // 有参构造函数
    this->m_mz = mz;
    this->m_intensity = intensity;
    this->m_rt = rt;
}

Ms1::Ms1(double mz, double intensity, float rt)
{
    // 有参构造函数
    this->m_mz = mz;
    this->m_intensity = intensity;
    this->m_rt = rt;
}

Ms1::Ms1(float mz, float intensity, float rt, float area)
{
    // 有参构造函数
    this->m_mz = mz;
    this->m_intensity = intensity;
    this->m_rt = rt;
    this->m_area = area;
}

void Ms1::SetMz(float mz)
{
    this->m_mz = mz;
}

void Ms1::SetIntensity(float intensity)
{
    this->m_intensity = intensity;
}

void Ms1::SetRt(float rt)
{
    this->m_rt = rt;
}

void Ms1::SetArea(float area)
{
    this->m_area = area;
}

float Ms1::GetMz()
{
    return this->m_mz;
}

float Ms1::GetIntensity()
{
    return this->m_intensity;
}

float Ms1::GetRt()
{
    return this->m_rt;
}

float Ms1::GetArea()
{
    return m_area;
}
