#pragma once

#include "pch/pch.h"
#include "stave.h"

class Buttons; 

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void updateStave(std::vector<std::pair<int, double>> BPMTimeList);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLabel *label;
    StaveWidget *stave;
    StaveLayout style;
    Buttons *buttons;
    QHBoxLayout *pageFrameLayout;
    bool pageView = true;
    double tempoScale = 1.0;
};