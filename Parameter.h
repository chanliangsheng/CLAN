#ifndef PARAMETER_H
#define PARAMETER_H

#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include <SpecificStructure.h>

namespace Ui {
class Parameter;
}

class Parameter : public QMainWindow {
    Q_OBJECT

public:
    explicit Parameter(QMainWindow* parent = nullptr);
    ~Parameter();

private:
    Ui::Parameter* ui;

    // MS1LibraryMatcher
public:
    float m_ms1_ppm = 5; // ppm，初始化为5
    float m_ms1_torlerance_rt = 0.2; // 可容忍的时间，初始化为0.2min
    float m_ms1_ppm_with_half_score = 5; // 设定ppm为多少的时候分数为0.5，初始值为5

    // MSLevelMatcher
public:
    float m_ms2_dalton = 0.5; // 初始化为0.5道尔顿
    float m_ms2_torlerance_rt = 0.25; // 初始化为15s

    // HeadgroupFinder
public:
    float m_headgroup_ppm = 30;
    float m_headgroup_ppm_with_half_score = 30; // 设定ppm为多少的时候分数为0.5，初始值为30
    float m_headgroup_mz_score_weight = 0.5;

    // FragmentFinder
public:
    float m_fragmentfinder_ppm = 30;
    float m_fragmentfinder_ppm_with_half_score = 5;
    float m_fragmentfinder_mz_score_weight = 0;

    // FragmentCombiner
public:
    mode m_merge_mode = flexible;
    float m_fragment_score_weight = 1;
    float m_pa_exist_score_weight = 0.1;
    float m_fa_intensity_variance_score_weight = 0.6;

    // Rt filter
public:
    float m_same_cardiolipin_rt_tor = 0.4;
};

#endif // PARAMETER_H
