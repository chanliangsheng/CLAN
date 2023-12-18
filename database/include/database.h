#ifndef DATABASE_H
#define DATABASE_H

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <databaserecord.h>
#include <memory>
#include <string>
#include <vector>

using db_ptr = std::shared_ptr<DatabaseRecord>;
using db_ptr_v = std::vector<db_ptr>;

class Database : public QObject {
    Q_OBJECT
public:
    Database(QObject* parent = nullptr);

public:
    void LoadAllTable(); // 加载数据库中的所有数据表
    std::pair<db_ptr_v, db_ptr_v> GetLocalClPair(); // 返回CL的M-H和M-2H的向量对
    std::pair<db_ptr_v, db_ptr_v> GetLocalMlclPair(); // 返回MLCL的M-H和M-2H的向量对
    std::pair<db_ptr_v, db_ptr_v> GetLocalDlclPair(); // 返回DLCL的M-H和M-2H的向量对
    db_ptr_v GetLocalPA();
    db_ptr_v GetLocalFA();
    void Clear(); // 清除所有上次读取的数据

private:
    // 数据库
    QSqlDatabase m_database;
    db_ptr_v m_CL_M_H;
    db_ptr_v m_CL_M_2H;
    db_ptr_v m_MLCL_M_H;
    db_ptr_v m_MLCL_M_2H;
    db_ptr_v m_DLCL_M_H;
    db_ptr_v m_DLCL_M_2H;
    db_ptr_v m_PA;
    db_ptr_v m_FA;

private:
    void LoadDatebase(QString file_name = "database.db"); // 加载sql数据库
    db_ptr_v LoadSingelTable(QString table); // 加载数据库中的单个数据表

signals:
    void SendMessage(QString message);
};

#endif // DATABASE_H
