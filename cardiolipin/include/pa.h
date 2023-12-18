#ifndef PA_H
#define PA_H
#include <databaserecord.h>

class Pa {
public:
    Pa();
    Pa(float mz, float intensity, float score, std::shared_ptr<DatabaseRecord> database_record);

public:
    void SetMz(float mz);
    void SetIntensity(float intensity);
    void SetScore(float score);
    void SetDatabaseRecordPtr(std::shared_ptr<DatabaseRecord> database_record);
    float GetMz();
    float GetIntensity();
    float GetScore();
    int GetChainLength();
    int GetUnsaturation();
    int GetOxygen();
    std::array<int, 3> GetCompound();
    std::string GetAdditiveForm();
    std::string GetFormula();

protected:
    float m_mz;
    float m_intensity;
    float m_score;
    std::shared_ptr<DatabaseRecord> m_database_record;
};

#endif // PA_H
