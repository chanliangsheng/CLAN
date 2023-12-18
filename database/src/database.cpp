#include "database.h"

using namespace std;
Database::Database(QObject* parent)
    : QObject(parent)
{
}

void Database::LoadDatebase(QString file_name)
{
    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        database = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(file_name);
    }

    if (!database.open()) {
        qDebug() << "Error: Failed to connect database." << database.lastError();
    } else {
        this->m_database = database;
    }
}

void Database::LoadAllTable()
{
    // 加载数据库
    this->LoadDatebase();

    // 清除上次读取的所有数据
    Clear();

    // 加载所有数据库，并交换数据
    this->m_CL_M_H = this->LoadSingelTable("cl_m_h");
    this->m_CL_M_2H = this->LoadSingelTable("cl_m_2h");

    this->m_MLCL_M_H = this->LoadSingelTable("mlcl_m_h");
    this->m_MLCL_M_2H = this->LoadSingelTable("mlcl_m_2h");

    this->m_DLCL_M_H = this->LoadSingelTable("dlcl_m_h");
    this->m_DLCL_M_2H = this->LoadSingelTable("dlcl_m_2h");

    this->m_PA = this->LoadSingelTable("pa");
    this->m_FA = this->LoadSingelTable("fa");

    // 关闭数据库
    this->m_database.close();

    emit SendMessage("Loading database finish!");
}

std::pair<db_ptr_v, db_ptr_v> Database::GetLocalClPair()
{
    return { m_CL_M_H, m_CL_M_2H };
}

std::pair<db_ptr_v, db_ptr_v> Database::GetLocalMlclPair()
{
    return { m_MLCL_M_H, m_MLCL_M_2H };
}

std::pair<db_ptr_v, db_ptr_v> Database::GetLocalDlclPair()
{
    return { m_DLCL_M_H, m_DLCL_M_2H };
}

db_ptr_v Database::GetLocalPA()
{
    return m_PA;
}

db_ptr_v Database::GetLocalFA()
{
    return m_FA;
}

void Database::Clear()
{
    vector<db_ptr>().swap(m_CL_M_H);
    vector<db_ptr>().swap(m_CL_M_2H);
    vector<db_ptr>().swap(m_MLCL_M_H);
    vector<db_ptr>().swap(m_MLCL_M_2H);
    vector<db_ptr>().swap(m_DLCL_M_H);
    vector<db_ptr>().swap(m_DLCL_M_2H);
    vector<db_ptr>().swap(m_PA);
    vector<db_ptr>().swap(m_FA);
}

db_ptr_v Database::LoadSingelTable(QString table)
{
    // 结果初始化，智能指针
    db_ptr_v res = vector<db_ptr>();

    // 绑定某个数据库
    QSqlQuery sql_query(this->m_database);
    QString select_all_sql = "select * from " + table + ";";
    sql_query.prepare(select_all_sql);

    if (!sql_query.exec()) {
        qDebug() << sql_query.lastError();
    } else {
        while (sql_query.next()) {
            db_ptr dabaseRecord = make_shared<DatabaseRecord>(sql_query.value(0).toFloat(),
                sql_query.value(1).toString().toStdString(),
                sql_query.value(2).toUInt(),
                sql_query.value(3).toUInt(),
                sql_query.value(4).toString().toStdString(),
                sql_query.value(5).toUInt());

            res.emplace_back(dabaseRecord);
        }
    }

    return res;
}
