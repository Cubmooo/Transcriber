#include "mainwindow.h"
#include <QString>

void MainWindow::updateFrequency(double freq)
{
    label->setText(QString("Frequency: %1 Hz").arg(freq));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Hello Qt");

    label = new QLabel("Hello World", this);
    label->setGeometry(100, 100, 200, 50);
}