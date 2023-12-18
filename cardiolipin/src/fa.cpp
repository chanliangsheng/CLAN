#include <fa.h>

Fa::Fa()
{
    this->m_mz = 0;
    this->m_intensity = 0;
    this->m_score = 0;
    this->m_database_record = nullptr;
}

Fa::Fa(float mz, float intensity, float score, std::shared_ptr<DatabaseRecord> database_record)
{
    this->m_mz = mz;
    this->m_intensity = intensity;
    this->m_score = score;
    this->m_database_record = database_record;
}
