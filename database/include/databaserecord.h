#ifndef DATABASERECORD_H
#define DATABASERECORD_H

#include <string>
#include <vector>
// #include <QSqlDatabase>
// #include <QSqlError>
// #include <QSqlQuery>
#include <memory>

class DatabaseRecord {
public:
    DatabaseRecord();
    DatabaseRecord(float mz, std::string additive_form, int chain_length, int unsaturation, std::string formula, int oxygen);

public:
    void Setmz(float mz);
    void SetAdditiveForm(std::string additive_form);
    void SetFormula(std::string formula);
    void SetChainLength(unsigned int chain_length);
    void SetUnsaturation(unsigned int unsaturation);
    void SetOxygen(unsigned int oxygen);

    float GetMz();
    std::string GetAdditiveForm();
    std::string GetFormula();
    int GetChainLength();
    int GetUnsaturation();
    int GetOxygen();

private:
    float m_mz;
    std::string m_additive_form;
    std::string m_formula;
    unsigned int m_chain_length;
    unsigned int m_unsaturation;
    unsigned int m_oxygen;
};

#endif // DATABASERECORD_H
