#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_parameter = new Parameter(); // 参数，不设置父对象
    m_thread = new QThread(this); // 新建一个线程

    // 载入文件的信号槽
    LoadFileConnectFunctions();

    // 设置参数的信号槽
    SetParameterConnectFunctions();

    // 开始过程的信号槽
    StartProcessConnectFunctions();

    // 输出文本的信号槽，需要排在开始的后面
    OutputTextConnectFunctions();

    // 测试的信号槽
    Test();

    // 线程启动
    m_thread->start();
}

void MainWindow::LoadFileConnectFunctions()
{
    // 载入一级
    connect(this->ui->LoadMS1, &QPushButton::clicked, this, [=]() {
        QFileDialog dialog(this);
        dialog.setDirectory("G:/CL/DDMS");
        QString file_name = dialog.getOpenFileName(); // 获取文件路径
        m_ms1_path = file_name;

        QString text;

        text += "MS1 From:" + file_name;

        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText(text);
    });

    // 载入二级
    connect(this->ui->LoadMS2, &QPushButton::clicked, this, [=]() {
        QFileDialog dialog(this);
        dialog.setDirectory("G:/CL/DDMS");
        QStringList file_names = dialog.getOpenFileNames();
        m_ms2_path = file_names;

        QString text;

        text += "MS2 From:";

        for (auto i : file_names) {
            text += i;
            text += ";";
        }
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText(text);
    });
}

void MainWindow::OutputTextConnectFunctions()
{
    // 读取完一级和二级要输出
    connect(m_mzml, &Mzml::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText(text);
    });

    // 加载完数据库的输出
    connect(m_database, &Database::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText(text);
    });

    // 一级搜库的输出
    connect(m_ms1_library_matcher, &Workflow::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText("Cardilipin in MS1:");
        this->ui->plainTextEdit->appendPlainText(text);
    });

    // 一级匹配二级的输出
    connect(m_ms_level_matcher, &Workflow::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText("Cardilipin with MS2:");
        this->ui->plainTextEdit->appendPlainText(text);
    });

    // 寻找头基
    connect(m_headgroup_finder, &Workflow::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText("Cardilipin with Headgroup:");
        this->ui->plainTextEdit->appendPlainText(text);
    });

    // 寻找PA和FA
    connect(m_fragment_finder, &Workflow::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText("Cardilipin with PA || FA:");
        this->ui->plainTextEdit->appendPlainText(text);
    });

    // 拼接
    connect(m_fragment_combiner, &Workflow::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText("Cardilipin can be splied:");
        this->ui->plainTextEdit->appendPlainText(text);
    });

    // 保留时间过滤
    connect(m_rt_filter, &Workflow::SendMessage, this, [=](QString text) {
        this->ui->plainTextEdit->appendPlainText("> ");
        this->ui->plainTextEdit->insertPlainText("After rt-filtering:");
        this->ui->plainTextEdit->appendPlainText(text);
    });

    // 输出csv
    connect(this->ui->Output, &QPushButton::clicked, this, [=]() {
        QFileDialog dialog(this);
        dialog.setDirectory("G:/CL/DDMS");
        QString dir_name = dialog.getExistingDirectory();
        m_ms1_library_matcher->OutputCsv(dir_name + "/计算机所有一级(不含氧).csv");
        m_ms_level_matcher->OutputCsv(dir_name + "/计算机中有二级部分(不含氧).csv", dir_name + "/计算机中没有二级部分(不含氧).csv");
        m_headgroup_finder->OutputCsv(dir_name + "/计算机中二级有头基的心磷脂(不含氧).csv");
        m_fragment_finder->OutputCsv(dir_name + "/计算机中二级有PA和FA的心磷脂(不含氧).csv");
        m_fragment_combiner->OutputCsv(dir_name + "/计算机所有定性结果(不含氧).csv");
        m_rt_filter->OutputCsv(dir_name + "/计算机所有定性结果(保留时间过滤后).csv");
    });
}

void MainWindow::StartProcessConnectFunctions()
{

    // 构造
    m_mzml = new Mzml();
    m_database = new Database(); // 不设置父对象
    m_ms1_library_matcher = new MS1LibraryMatcher();
    m_ms_level_matcher = new MSLevelMatcher();
    m_headgroup_finder = new HeadgroupFinder();
    m_fragment_finder = new FragmentFinder();
    m_fragment_combiner = new FragmentCombiner();
    m_rt_filter = new RtFilter();

    // 移动到新线程
    m_mzml->moveToThread(m_thread);
    m_database->moveToThread(m_thread);
    m_ms1_library_matcher->moveToThread(m_thread);
    m_ms_level_matcher->moveToThread(m_thread);
    m_headgroup_finder->moveToThread(m_thread);
    m_fragment_finder->moveToThread(m_thread);
    m_fragment_combiner->moveToThread(m_thread);
    m_rt_filter->moveToThread(m_thread);

    // 开始运行
    connect(this->ui->start, &QPushButton::clicked, m_mzml, [=]() {
        // 获取当前的日期和时间
        this->ui->plainTextEdit->appendPlainText("+++++++++++++++++++++++++++++++");
        QString currentDateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        this->ui->plainTextEdit->appendPlainText(currentDateTimeString);
        // 重置数据
        m_mzml->ReSet();

        // 读取一级和二级文件
        m_mzml->ReadMs1FromCsv(this->m_ms1_path);
        m_mzml->ReadMs2FromMzmls(this->m_ms2_path);

        // 设置参数
        m_ms1_library_matcher->SetParameter(*m_parameter);
        m_ms_level_matcher->SetParameter(*m_parameter);
        m_headgroup_finder->SetParameter(*m_parameter);
        m_fragment_finder->SetParameter(*m_parameter);
        m_fragment_combiner->SetParameter(*m_parameter);
        m_rt_filter->SetParameter(*m_parameter);

        // 加载数据库
        m_database->LoadAllTable();

        // 一级配对
        m_ms1_library_matcher->MatchMs1WithAllTables(*m_mzml, *m_database);

        // 一级配对二级
        m_ms_level_matcher->Set(m_ms1_library_matcher->GetCardiolipinPairVector());

        m_ms_level_matcher->MatchCardiolipinWithMS2(*m_mzml);

        // 寻找头基
        m_headgroup_finder->Set(m_ms_level_matcher->GetCardiolipinPairVector());
        m_headgroup_finder->FindMS2Headgroup();

        // 寻找PA和FA
        m_fragment_finder->Set(m_headgroup_finder->GetCardiolipinPairVector());
        m_fragment_finder->FindMS2PAAndFA(*m_database);

        // 拼接心磷脂
        m_fragment_combiner->Set(m_fragment_finder->GetCardiolipinPairVector());
        m_fragment_combiner->Splice();

        // 去除离群值
        m_rt_filter->Set(m_fragment_combiner);
        m_rt_filter->Filter();
    });
}

void MainWindow::Test()
{
    // DDA普通测试
    connect(this->ui->Test_DDA, &QRadioButton::clicked, this, [=]() {
        m_ms1_path = "D:/qt_project/R_analyze/MS1/mzmine/50X_NEG_001.raw_chromatograms_resolved.csv";
        m_ms2_path = QStringList("D:/qt_project/R_analyze/mzml/MSnbase/50X_NEG_001(after smooth).mzml");

        // 开始进行
        this->ui->start->click();

        // 输出csv
        connect(m_rt_filter, &Workflow::Done, this, [=]() {
            QString dir_name = "D:/qt_project/R_analyze/计算机定性结果/findmyCL使用mzmine一级峰表(dda)/一级：5ppm；加和形式rt：12s";
            m_ms1_library_matcher->OutputCsv(dir_name + "/计算机所有一级(不含氧).csv");
            m_ms_level_matcher->OutputCsv(dir_name + "/计算机中有二级部分(不含氧).csv", dir_name + "/计算机中没有二级部分(不含氧).csv");
            m_headgroup_finder->OutputCsv(dir_name + "/计算机中二级有头基的心磷脂(不含氧).csv");
            m_fragment_finder->OutputCsv(dir_name + "/计算机中二级有PA和FA的心磷脂(不含氧).csv");
            m_fragment_combiner->OutputCsv(dir_name + "/计算机所有定性结果(不含氧).csv");
            m_rt_filter->OutputCsv(dir_name + "/计算机所有定性结果(保留时间过滤后).csv");
        });
    });

    // RBL
    connect(this->ui->Test_RBL, &QRadioButton::clicked, this, [=]() {
        // 设置三个RQC的路径
        std::vector<QString> ms1_path_vector = { "D:/qt_project/R_analyze/MS1/mzmine/RQC-4-Negative-ddms.raw_chromatograms_resolved.csv",
            "D:/qt_project/R_analyze/MS1/mzmine/RQC-5-Negative-ddms.raw_chromatograms_resolved.csv",
            "D:/qt_project/R_analyze/MS1/mzmine/RQC-6-ddms.raw_chromatograms_resolved.csv" };

        std::vector<QString> ms2_path_vector = { "D:/qt_project/R_analyze/mzml/MSnbase/RQC-4-Negative-ddms(centroid).mzML",
            "D:/qt_project/R_analyze/mzml/MSnbase/RQC-5-Negative-ddms(centroid).mzML",
            "D:/qt_project/R_analyze/mzml/MSnbase/RQC-6-ddms(centroid).mzML" };

        std::vector<QString> out_path_vector = { "D:/qt_project/R_analyze/计算机定性结果/findmyCL使用mzmine一级峰表(RBL)/一级：5ppm；加和形式rt：12s/RQC4",
            "D:/qt_project/R_analyze/计算机定性结果/findmyCL使用mzmine一级峰表(RBL)/一级：5ppm；加和形式rt：12s/RQC5",
            "D:/qt_project/R_analyze/计算机定性结果/findmyCL使用mzmine一级峰表(RBL)/一级：5ppm；加和形式rt：12s/RQC6" };

        QString dir = out_path_vector[0];

        connect(m_rt_filter, &Workflow::Done, this, [=, &dir]() {
            qDebug() << dir;
            QString dir_name = dir;
            m_ms1_library_matcher->OutputCsv(dir_name + "/计算机所有一级(不含氧).csv");
            m_ms_level_matcher->OutputCsv(dir_name + "/计算机中有二级部分(不含氧).csv", dir_name + "/计算机中没有二级部分(不含氧).csv");
            m_headgroup_finder->OutputCsv(dir_name + "/计算机中二级有头基的心磷脂(不含氧).csv");
            m_fragment_finder->OutputCsv(dir_name + "/计算机中二级有PA和FA的心磷脂(不含氧).csv");
            m_fragment_combiner->OutputCsv(dir_name + "/计算机所有定性结果(不含氧).csv");
            m_rt_filter->OutputCsv(dir_name + "/计算机所有定性结果(保留时间过滤后).csv");

            // 设置子线程已经完成工作
            m_rt_filter->m_received_done_signal = 1;
        });

        int size = ms1_path_vector.size();

        for (int i = 0; i < size; i++) {
            m_ms1_path = ms1_path_vector[i];
            m_ms2_path = QStringList(ms2_path_vector[i]);
            dir = out_path_vector[i];

            // 子线程开始进行
            this->ui->start->click();

            // 等待信号，等待子线程完成后，才继续进行下一个任务
            while (!m_rt_filter->m_received_done_signal) {
                QApplication::processEvents();
            }

            // 重置状态
            m_rt_filter->m_received_done_signal = 0;
        }
    });

    // 多器官大QC
    connect(this->ui->Test_organs_Big_QC, &QRadioButton::clicked, this, [=]() {
        m_ms1_path = "D:/qt_project/R_analyze/MS1/mzmine/QCL_NEG_ID_1.raw_chromatograms_resolved.csv";
        m_ms2_path = QStringList({ "D:/qt_project/R_analyze/mzml/MSnbase/QCL_NEG_ID_1(centroid).mzML",
            "D:/qt_project/R_analyze/mzml/MSnbase/QCL_NEG_ID_2(centroid).mzML",
            "D:/qt_project/R_analyze/mzml/MSnbase/QCL_NEG_ID_3(centroid).mzML" });

        // 开始进行
        this->ui->start->click();

        // 输出csv
        connect(m_rt_filter, &Workflow::Done, this, [=]() {
            QString dir_name = "D:/qt_project/R_analyze/计算机定性结果/findmyCL使用mzmine一级峰表(多器官)";
            m_ms1_library_matcher->OutputCsv(dir_name + "/计算机所有一级(不含氧).csv");
            m_ms_level_matcher->OutputCsv(dir_name + "/计算机中有二级部分(不含氧).csv", dir_name + "/计算机中没有二级部分(不含氧).csv");
            m_headgroup_finder->OutputCsv(dir_name + "/计算机中二级有头基的心磷脂(不含氧).csv");
            m_fragment_finder->OutputCsv(dir_name + "/计算机中二级有PA和FA的心磷脂(不含氧).csv");
            m_fragment_combiner->OutputCsv(dir_name + "/计算机所有定性结果(不含氧).csv");
            m_rt_filter->OutputCsv(dir_name + "/计算机所有定性结果(保留时间过滤后).csv");
        });
    });

    // 单器官qc定性
    connect(this->ui->Test_Single_organ, &QRadioButton::clicked, this, [=]() {
        // 设置12个器官的路径
        std::vector<QString> ms1_path_vector = { "G:/CL/DDMS/BAT/BAT-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Bra/Bra-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Col/Col-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Heart/Hrt-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Kid/Kid-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Liv/Liv-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Lung/Lug-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Pan/Pan-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Sole/Sole-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Spl/Spl-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/Tes/Tes-QC-ddms.raw_chromatograms_resolved.csv",
            "G:/CL/DDMS/WAT/eWAT-QC-ddms.raw_chromatograms_resolved.csv" };

        std::vector<QString> ms2_path_vector = { "G:/CL/DDMS/BAT/BAT-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Bra/Bra-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Col/Col-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Heart/Hrt-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Kid/Kid-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Liv/Liv-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Lung/Lug-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Pan/Pan-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Sole/Sole-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Spl/Spl-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/Tes/Tes-QC-ddms(centroid).mzML",
            "G:/CL/DDMS/WAT/eWAT-QC-ddms(centroid).mzML" };

        std::vector<QString> out_path_vector_branch_chain = { "D:/R project/心磷脂酰基链组成分析/data/BAT",
            "D:/R project/心磷脂酰基链组成分析/data/Brain",
            "G:/CL/DDMS/Col/res",
            "D:/R project/心磷脂酰基链组成分析/data/Heart",
            "D:/R project/心磷脂酰基链组成分析/data/Kidney",
            "D:/R project/心磷脂酰基链组成分析/data/Liver",
            "D:/R project/心磷脂酰基链组成分析/data/Lung",
            "D:/R project/心磷脂酰基链组成分析/data/Pancreas",
            "D:/R project/心磷脂酰基链组成分析/data/Sole",
            "D:/R project/心磷脂酰基链组成分析/data/Spleen",
            "D:/R project/心磷脂酰基链组成分析/data/Testis",
            "G:/CL/DDMS/WAT/res" };

        std::vector<QString> out_path_vector_EODF = { "D:/R project/心磷脂EODF分析/data/BAT",
            "D:/R project/心磷脂EODF分析/data/Brain",
            "G:/CL/DDMS/Col/res",
            "D:/R project/心磷脂EODF分析/data/Heart",
            "D:/R project/心磷脂EODF分析/data/Kidney",
            "D:/R project/心磷脂EODF分析/data/Liver",
            "D:/R project/心磷脂EODF分析/data/Lung",
            "D:/R project/心磷脂EODF分析/data/Pancreas",
            "D:/R project/心磷脂EODF分析/data/Sole",
            "D:/R project/心磷脂EODF分析/data/Spleen",
            "D:/R project/心磷脂EODF分析/data/Testis",
            "G:/CL/DDMS/WAT/res" };

        QString dir_branch_chain = out_path_vector_branch_chain[0];
        QString dir_EODF = out_path_vector_EODF[0];

        connect(m_rt_filter, &Workflow::Done, this, [=, &dir_branch_chain, &dir_EODF]() {
            qDebug() << dir_branch_chain;
            qDebug() << dir_EODF;

            // 输出到分析分链的文件夹中
            m_ms1_library_matcher->OutputCsv(dir_branch_chain + "/计算机所有一级(不含氧).csv");
            m_ms_level_matcher->OutputCsv(dir_branch_chain + "/计算机中有二级部分(不含氧).csv", dir_branch_chain + "/计算机中没有二级部分(不含氧).csv");
            m_headgroup_finder->OutputCsv(dir_branch_chain + "/计算机中二级有头基的心磷脂(不含氧).csv");
            m_fragment_finder->OutputCsv(dir_branch_chain + "/计算机中二级有PA和FA的心磷脂(不含氧).csv");
            m_fragment_combiner->OutputCsv(dir_branch_chain + "/计算机所有定性结果(不含氧).csv");
            m_rt_filter->OutputCsv(dir_branch_chain + "/计算机所有定性结果(保留时间过滤后).csv");

            // 输出到分析EODF的文件夹中
            m_ms1_library_matcher->OutputCsv(dir_EODF + "/计算机所有一级(不含氧).csv");
            m_ms_level_matcher->OutputCsv(dir_EODF + "/计算机中有二级部分(不含氧).csv", dir_EODF + "/计算机中没有二级部分(不含氧).csv");
            m_headgroup_finder->OutputCsv(dir_EODF + "/计算机中二级有头基的心磷脂(不含氧).csv");
            m_fragment_finder->OutputCsv(dir_EODF + "/计算机中二级有PA和FA的心磷脂(不含氧).csv");
            m_fragment_combiner->OutputCsv(dir_EODF + "/计算机所有定性结果(不含氧).csv");
            m_rt_filter->OutputCsv(dir_EODF + "/计算机所有定性结果(保留时间过滤后).csv");

            // 发出已经完成输出csv的信号
            emit m_rt_filter->DoneWithOutputCsv();
        });

        int size = ms1_path_vector.size();
        QEventLoop loop;
        connect(m_rt_filter, &Workflow::DoneWithOutputCsv, &loop, &QEventLoop::quit);

        for (int i = 0; i < size; i++) {
            m_ms1_path = ms1_path_vector[i];
            m_ms2_path = QStringList(ms2_path_vector[i]);
            dir_branch_chain = out_path_vector_branch_chain[i];
            dir_EODF = out_path_vector_EODF[i];

            // 子线程开始进行
            this->ui->start->click();

            // 进入事件循环等待子线程完成
            loop.exec();
        }
    });
}

MainWindow::~MainWindow()
{
    this->m_thread->quit();
    delete ui;
}

void MainWindow::SetParameterConnectFunctions()
{
    // 打开参数设置
    connect(this->ui->Parameter, &QPushButton::clicked, [=]() {
        m_parameter->show();
    });
}
