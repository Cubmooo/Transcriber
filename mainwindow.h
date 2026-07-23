#pragma once

#include <QMainWindow>
#include <QLabel>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);

public slots:
    void updateFrequency(double freq);

private:
    QLabel *label;
};