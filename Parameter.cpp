#include "Parameter.h"
#include "ui_Parameter.h"

Parameter::Parameter(QMainWindow* parent)
    : QMainWindow(parent)
    , ui(new Ui::Parameter)
{
    ui->setupUi(this);

    // ms1
    this->ui->ms1_ppm->setValue(m_ms1_ppm);
    this->ui->ms1_tor_rt->setValue(m_ms1_torlerance_rt);
    this->ui->ms1_ppm_with_half_score->setValue(m_ms1_ppm_with_half_score);

    // mslevel
    this->ui->ms2_dalton->setValue(m_ms2_dalton);
    this->ui->ms2_tor_rt->setValue(m_ms2_torlerance_rt);

    // headgroup
    this->ui->headgroup_ppm->setValue(m_headgroup_ppm);
    this->ui->headgroup_mz_score_weight->setValue(m_headgroup_mz_score_weight);
    this->ui->headgroup_ppm_with_half_score->setValue(m_headgroup_ppm_with_half_score);

    // fa and pa
    this->ui->find_pafa_ppm->setValue(m_fragmentfinder_ppm);
    this->ui->find_pafa_mz_score_weight->setValue(m_fragmentfinder_mz_score_weight);
    this->ui->find_pafa_ppm_with_half_score->setValue(m_fragmentfinder_ppm_with_half_score);

    // splice
    this->ui->splice_fragment_score_weight->setValue(m_fragment_score_weight);
    this->ui->splice_m_pa_exist_score_weight->setValue(m_pa_exist_score_weight);
    this->ui->splice_fa_intensity_variance_score_weight->setValue(m_fa_intensity_variance_score_weight);
    this->ui->splice_mode->setCurrentText("flexible");

    // rt filter
    this->ui->rt_filter_same_cardiolipin_rt_tor->setValue(m_same_cardiolipin_rt_tor);

    // 改写参数
    connect(this->ui->pushButton, &QPushButton::clicked, [this]() {
        // ms1
        m_ms1_ppm = this->ui->ms1_ppm->value();
        m_ms1_torlerance_rt = this->ui->ms1_tor_rt->value();
        m_ms1_ppm_with_half_score = this->ui->ms1_ppm_with_half_score->value();

        // mslevel
        m_ms2_dalton = this->ui->ms2_dalton->value();
        m_ms2_torlerance_rt = this->ui->ms2_tor_rt->value();

        // headgroup
        m_headgroup_ppm = this->ui->headgroup_ppm->value();
        m_headgroup_mz_score_weight = this->ui->headgroup_mz_score_weight->value();
        m_headgroup_ppm_with_half_score = this->ui->headgroup_ppm_with_half_score->value();

        // fa and pa
        m_fragmentfinder_ppm = this->ui->find_pafa_ppm->value();
        m_fragmentfinder_mz_score_weight = this->ui->find_pafa_mz_score_weight->value();
        m_fragmentfinder_ppm_with_half_score = this->ui->find_pafa_ppm_with_half_score->value();

        // splice
        m_fragment_score_weight = this->ui->splice_fragment_score_weight->value();
        m_pa_exist_score_weight = this->ui->splice_m_pa_exist_score_weight->value();
        m_fa_intensity_variance_score_weight = this->ui->splice_fa_intensity_variance_score_weight->value();

        // rt filter
        m_same_cardiolipin_rt_tor = this->ui->rt_filter_same_cardiolipin_rt_tor->value();

        if (this->ui->splice_mode->currentText() == "flexible") {
            m_merge_mode = flexible;
        } else {
            m_merge_mode = strict;
        }

        this->close();
    });
}

Parameter::~Parameter()
{
    delete ui;
}
