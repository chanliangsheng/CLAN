#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <CL.h>
#include <FragmentCombiner.h>
#include <FragmentFinder.h>
#include <HeadgroupFinder.h>
#include <MS1LibraryMatcher.h>
#include <MSLevelMatcher.h>
#include <Parameter.h>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QMainWindow>
#include <QPushButton>
#include <QThread>
#include <RtFilter.h>
#include <cardiolipin.h>
#include <mzml.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // 参数的信号槽
private:
    void SetParameterConnectFunctions();

    // 读取一级和二级的信号槽
private:
    void LoadFileConnectFunctions();

    // 输出文本的信号槽
private:
    void OutputTextConnectFunctions();

    // 开始
private:
    void StartProcessConnectFunctions();

    // 用于测试的信号槽
private:
    void Test();

private:
    Ui::MainWindow* ui;
    Parameter* m_parameter; // 参数
    QThread* m_thread;
    QString m_ms1_path;
    QStringList m_ms2_path;

private:
    // 流程变量
    Mzml* m_mzml;
    Database* m_database;
    MS1LibraryMatcher* m_ms1_library_matcher;
    MSLevelMatcher* m_ms_level_matcher;
    HeadgroupFinder* m_headgroup_finder;
    FragmentFinder* m_fragment_finder;
    FragmentCombiner* m_fragment_combiner;
    RtFilter* m_rt_filter;

    // 信号
signals:
};
#endif // MAINWINDOW_H
