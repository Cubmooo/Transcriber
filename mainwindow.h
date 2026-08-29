#pragma once

#include "pch/pch.h"
#include "stave.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void updateFrequency(int note);
    void updateStave(std::vector<std::pair<int, double>> BPMTimeList);

private:
    QLabel *label;
    StaveWidget *stave;
};