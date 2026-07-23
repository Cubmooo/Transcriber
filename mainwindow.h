#pragma once

#include <QMainWindow>
#include <QLabel>

#include "stave.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);

public slots:
    void updateFrequency(int note);
    void updateStave(int note);

private:
    QLabel *label;
    StaveWidget *stave;
};