#include "pch/pch.h"
#include "mainwindow.h"
#include <QString>

extern std::mutex mtx;
extern std::vector<std::pair<double, int>> BPMTimeList;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showMaximized();
    setWindowTitle("Hello Qt");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    label = new QLabel("Hello World", central);
    label->move(100,100);
    label->resize(300,50);
}

void MainWindow::updateFrequency(double freq)
{
    std::cout << "thign" << std::flush;
    label->setText(QString("Frequency: %1 Hz").arg(freq));
    label->adjustSize();
    label->repaint();
    return;
}