#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "emanoisefilter.h"
#include "simplenoisefilter.h"

namespace Ui {
class MainWindow;
}

class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void valueChanged(int value);

public slots:
    void onEMAActivityThresholdChanged(int threshold);
    void onEMAFilterLowerBoundChanged(int lowerBound);
    void onEMAFilterUpperBoundChanged(int upperBound);
    void onEMAFilterSnapMultiplierChanged(double snapMultiplier);

    void onSimpleFilterThresholdChanged(int threshold);
    void onSimpleFilterSuppressionCountChanged(int count);

    void onInputValueChanged(int value);
    void onNoiseTimerTriggered();
    void onValueChanged(int value);


    void onFiltersEnabledChanged(int enabled);
    void onSleepEnabledChanged(int enabled);
    void onSnapToEdgesChanged(int enabled);
    void onNoiseEnabledChanged(int enabled);

private:
    Ui::MainWindow *ui;
    int mInput;
    int mNoise;
    EMANoiseFilter<int> mEMANoiseFilter;
    SimpleNoiseFilter<int> mSimpleNoiseFilter;
    QTimer *mNoiseTimer;
};

#endif // MAINWINDOW_H
