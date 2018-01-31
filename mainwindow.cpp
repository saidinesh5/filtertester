#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mInput(0),
    mNoise(0),
    mEMANoiseFilter(0,
                 1024,
                 true),
    mSimpleNoiseFilter(10,
                       5),
    mNoiseTimer(new QTimer(this))
{
    ui->setupUi(this);

    connect(ui->emaFilterLowerBoundSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onEMAFilterLowerBoundChanged(int)));

    connect(ui->emaFilterUpperBoundSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onEMAFilterUpperBoundChanged(int)));

    connect(ui->emaActivityThresholdSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onEMAActivityThresholdChanged(int)));

    connect(ui->emaFilterSnapMultiplierSpinBox,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(onEMAFilterSnapMultiplierChanged(double)));

    connect(ui->simpleFilterThresholdSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onSimpleFilterThresholdChanged(int)));

    connect(ui->simpleFilterSuppressionCountSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onSimpleFilterSuppressionCountChanged(int)));


    connect(ui->inputSlider,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onInputValueChanged(int)));

    connect(ui->enableFiltersCheckbox,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(onFiltersEnabledChanged(int)));
    connect(ui->enableSleepCheckBox,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(onSleepEnabledChanged(int)));
    connect(ui->enableSnaptoEdgesCheckBox,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(onSnapToEdgesChanged(int)));
    connect(ui->enableNoiseCheckBox,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(onNoiseEnabledChanged(int)));


    connect(mNoiseTimer,
            SIGNAL(timeout()),
            this,
            SLOT(onNoiseTimerTriggered()));

    connect(this,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onValueChanged(int)));

    ui->enableFiltersCheckbox->setCheckState(Qt::Checked);

    ui->emaFilterLowerBoundSpinBox->setMinimum(0);
    ui->emaFilterLowerBoundSpinBox->setMaximum(10240);
    ui->emaFilterLowerBoundSpinBox->setValue(0);
    ui->emaFilterUpperBoundSpinBox->setMinimum(0);
    ui->emaFilterUpperBoundSpinBox->setMaximum(10240);
    ui->emaFilterUpperBoundSpinBox->setValue(1024);
    ui->emaFilterSnapMultiplierSpinBox->setValue(0.01);
    ui->emaActivityThresholdSpinBox->setValue(mEMANoiseFilter.activityThreshold());
    ui->enableSleepCheckBox->setCheckState(Qt::Checked);
    ui->enableSnaptoEdgesCheckBox->setCheckState(Qt::Checked);

    ui->simpleFilterThresholdSpinBox->setMinimum(0);
    ui->simpleFilterThresholdSpinBox->setMaximum(10240);
    ui->simpleFilterSuppressionCountSpinBox->setMinimum(0);
    ui->simpleFilterSuppressionCountSpinBox->setMaximum(10240);
    ui->simpleFilterThresholdSpinBox->setValue(mSimpleNoiseFilter.activityThreshold());
    ui->simpleFilterSuppressionCountSpinBox->setValue(mSimpleNoiseFilter.suppressionCount());

    ui->inputSlider->setMinimum(0);
    ui->emaOutputSlider->setMinimum(0);
    ui->simpleOutputSlider->setMinimum(0);
    ui->inputSlider->setMaximum(1024);
    ui->emaOutputSlider->setMaximum(1024);
    ui->simpleOutputSlider->setMaximum(1024);
    ui->effectiveInputSlider->setMinimum(0);
    ui->effectiveInputSlider->setMaximum(1024);

    ui->inputSpinBox->setMinimum(0);
    ui->inputSpinBox->setMaximum(1024);
    ui->emaOutputSpinBox->setMinimum(0);
    ui->simpleOutputSpinBox->setMinimum(0);
    ui->emaOutputSpinBox->setMaximum(1024);
    ui->simpleOutputSpinBox->setMaximum(1024);
    ui->noiseSpinBox->setMinimum(-1024);
    ui->noiseSpinBox->setMaximum(1024);
    ui->effectiveInputSpinBox->setMinimum(-10240);
    ui->effectiveInputSpinBox->setMaximum(10240);

    qsrand(QDateTime::currentMSecsSinceEpoch());
    ui->enableNoiseCheckBox->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onEMAActivityThresholdChanged(int threshold)
{
    mEMANoiseFilter.setActivityThreshold(threshold);
}

void MainWindow::onEMAFilterLowerBoundChanged(int lowerBound)
{
    mEMANoiseFilter.setLowerBound(lowerBound);
    ui->inputSlider->setMinimum(lowerBound);
    ui->emaOutputSlider->setMinimum(lowerBound);
    ui->simpleOutputSlider->setMinimum(lowerBound);
}

void MainWindow::onEMAFilterUpperBoundChanged(int upperBound)
{
    mEMANoiseFilter.setUpperBound(upperBound);
    ui->inputSlider->setMaximum(upperBound);
    ui->emaOutputSlider->setMaximum(upperBound);
    ui->simpleOutputSlider->setMaximum(upperBound);
}

void MainWindow::onInputValueChanged(int value)
{
    mInput = value;
    ui->inputSpinBox->setValue(value);
    emit valueChanged(mInput + mNoise);
}

void MainWindow::onNoiseTimerTriggered()
{
    int noiseRange = (mEMANoiseFilter.upperBound() - mEMANoiseFilter.lowerBound())/40;
    mNoise = noiseRange*(0.5 - static_cast<double>(qrand())/RAND_MAX);
    ui->noiseSpinBox->setValue(mNoise);
    emit valueChanged(mInput + mNoise);
}

void MainWindow::onValueChanged(int value)
{
    auto outputValue = mEMANoiseFilter.update(value);
    auto simpleOutputValue = mSimpleNoiseFilter.update(value);

    ui->effectiveInputSpinBox->setValue(value);
    ui->effectiveInputSlider->setValue(value);

    ui->emaOutputSlider->setValue(outputValue);
    ui->emaOutputSpinBox->setValue(outputValue);

    ui->simpleOutputSlider->setValue(simpleOutputValue);
    ui->simpleOutputSpinBox->setValue(simpleOutputValue);
}

void MainWindow::onEMAFilterSnapMultiplierChanged(double snapMultiplier)
{
    mEMANoiseFilter.setSnapMultiplier(snapMultiplier);
}

void MainWindow::onSimpleFilterThresholdChanged(int threshold)
{
    mSimpleNoiseFilter.setActivityThreshold(threshold);
}

void MainWindow::onSimpleFilterSuppressionCountChanged(int count)
{
    mSimpleNoiseFilter.setSuppressionCount(count);
}

void MainWindow::onFiltersEnabledChanged(int enabled)
{
    mEMANoiseFilter.setEnabled(enabled != Qt::Unchecked);
    mSimpleNoiseFilter.setEnabled(enabled != Qt::Unchecked);
}

void MainWindow::onSleepEnabledChanged(int enabled)
{
    mEMANoiseFilter.setSleepEnabled(enabled != Qt::Unchecked);
}

void MainWindow::onSnapToEdgesChanged(int enabled)
{
    mEMANoiseFilter.setEdgeSnapEnabled(enabled != Qt::Unchecked);
}

void MainWindow::onNoiseEnabledChanged(int enabled)
{
    if (enabled != Qt::Unchecked)
    {
        mNoiseTimer->start(20);
    }
    else
    {
        mNoiseTimer->stop();
    }

    mNoise = 0;
    ui->noiseSpinBox->setValue(0);
    emit valueChanged(mNoise + mInput);
}
