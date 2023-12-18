#ifndef WORKFLOW_H
#define WORKFLOW_H
#include <CL.h>
#include <DLCL.h>
#include <MLCL.h>
#include <Parameter.h>
#include <QObject>
#include <cardiolipin.h>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>

using cardiolipin_ptr = std::shared_ptr<Cardiolipin>;
class Workflow : public QObject {
    Q_OBJECT
public:
    Workflow(QObject* parent = nullptr);
    Workflow(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector);

    // set
public:
    void Set(std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> cardiolipin_pair_vector); // 设置心磷脂数据

    // output
public:
    virtual void PrintTypesCount(); // 打印CL，MLCL，DLCL数量有多少
    virtual void OutputCsv(QString file_name); // 输出csv文件

    // Get
public:
    std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> GetCardiolipinPairVector();
    std::array<int, 3> GetCompound(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    std::array<int, 3> GetCompound(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);
    int GetChainLength(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    int GetChainLength(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);
    int GetUnsaturation(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    int GetUnsaturation(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);
    int GetOxygen(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    int GetOxygen(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);
    float GetM_HArea(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    float GetM_HArea(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);
    float GetM_2HArea(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    float GetM_2HArea(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);

    float GetRt(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    float GetRt(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);
    float GetIntensity(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    Type GetType(std::pair<cardiolipin_ptr, cardiolipin_ptr> pair);
    Type GetType(std::shared_ptr<std::pair<cardiolipin_ptr, cardiolipin_ptr>> pair_ptr);

    // set parameter
public:
    virtual void SetParameter(Parameter& par);

    // delete
protected:
    void DeleteRedundantCardiolipinPair(); // 去除由于二级匹配/头基/PAFA产生的冗余心磷脂对
    void DeleteCardiolipinWithOxygen(); // 去除含氧的心磷脂

protected:
    std::vector<std::pair<cardiolipin_ptr, cardiolipin_ptr>> m_cardiolipin;

    //    发送信息
signals:
    void SendMessage(QString message);
    void Done();
    void DoneWithOutputCsv();
    // 用于子线程的结束标记和主线程通信
public:
    bool m_received_done_signal = false;
};
#endif // WORKFLOW_H
