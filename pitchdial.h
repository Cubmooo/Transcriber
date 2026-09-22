#pragma once

#include <QWidget>
#include <QColor>

class PitchDial : public QWidget
{
    Q_OBJECT

public:
    explicit PitchDial(QWidget *parent = nullptr);

    void setCents(double cents);
    void setColor(const QColor &color);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_cents = 0.0;
    QColor m_color = QColor("#FFFFFF");
};