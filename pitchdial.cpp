#include "pitchdial.h"

#include <QPainter>
#include <QtMath>

PitchDial::PitchDial(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(90, 55);
}

void PitchDial::setCents(double cents)
{
    m_cents = qBound(-50.0, cents, 50.0);
    update();
}

void PitchDial::setColor(const QColor &color)
{
    m_color = color;
    update();
}

QSize PitchDial::sizeHint() const
{
    return QSize(90, 55);
}

void PitchDial::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 6;
    const double radius = (width() - margin * 2) / 2.0;
    const QPointF centre(width() / 2.0, height() - margin);

    QColor trackColor = m_color;
    trackColor.setAlpha(80);
    painter.setPen(QPen(trackColor, 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(centre.x() - radius, centre.y() - radius, radius * 2, radius * 2),
                     0, 180 * 16);

    const double angleDeg = 90.0 + m_cents * (90.0 / 50.0);
    const double angleRad = qDegreesToRadians(angleDeg);
    const QPointF tip(centre.x() + radius * std::cos(angleRad),
                       centre.y() - radius * std::sin(angleRad));

    painter.setPen(QPen(m_color, 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(centre, tip);

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_color);
    painter.drawEllipse(centre, 3, 3);
}