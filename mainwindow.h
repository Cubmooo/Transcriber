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

private:
    QLabel *label;
    StaveWidget *stave;
    StaveLayout style;
    Buttons *buttons;
    double tempoScale = 1.0;
};