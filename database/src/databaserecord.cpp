#include "databaserecord.h"
#include <QDebug>

using namespace std;
DatabaseRecord::DatabaseRecord()
{
    this->m_mz = 0;
    this->m_additive_form = "0";
    this->m_formula = "0";
    this->m_chain_length = 0;
    this->m_unsaturation = 0;
    this->m_oxygen = 0;
}

DatabaseRecord::DatabaseRecord(float mz, std::string additive_form, int chain_length, int unsaturation, std::string formula, int oxygen)
{
    // 有参构造函数
    this->m_mz = mz;
    this->m_additive_form = additive_form;
    this->m_formula = formula;
    this->m_chain_length = chain_length;
    this->m_unsaturation = unsaturation;
    this->m_oxygen = oxygen;
}

void DatabaseRecord::Setmz(float mz)
{
    this->m_mz = mz;
}

void DatabaseRecord::SetAdditiveForm(std::string additive_form)
{
    this->m_additive_form = additive_form;
}

void DatabaseRecord::SetFormula(string formula)
{
    this->m_formula = formula;
}

void DatabaseRecord::SetChainLength(unsigned int chain_length)
{
    this->m_chain_length = chain_length;
}

void DatabaseRecord::SetUnsaturation(unsigned int unsaturation)
{
    this->m_unsaturation = unsaturation;
}

void DatabaseRecord::SetOxygen(unsigned oxygen)
{
    this->m_oxygen = oxygen;
}

float DatabaseRecord::GetMz()
{
    return this->m_mz;
}

string DatabaseRecord::GetAdditiveForm()
{
    return this->m_additive_form;
}

string DatabaseRecord::GetFormula()
{
    return this->m_formula;
}

int DatabaseRecord::GetChainLength()
{
    return this->m_chain_length;
}

int DatabaseRecord::GetUnsaturation()
{
    return this->m_unsaturation;
}

int DatabaseRecord::GetOxygen()
{
    return this->m_oxygen;
}
