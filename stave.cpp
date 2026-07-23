#include "stave.h"
#include "smufl.h"
#include "layout.h"

#include <QPainter>
#include <QFont>

StaveWidget::StaveWidget(QWidget *parent)
    : QWidget(parent),
    lelandFont("Leland"),
    lelandMetrics(lelandFont)
    {  
        lelandFont.setPointSize(100);
        noteWidget = new NoteWidget(this);
        noteWidget->raise();
    }

void StaveWidget::setNote(int position)
{
    noteWidget->setNote(position);
    update();
}

void StaveWidget::resizeEvent(QResizeEvent *)
{
    noteWidget->setGeometry(rect());
    style = DefineLayoutConstants(height(), width(), 100);
    noteWidget->setStaveLayout(style);
}

void StaveWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(lelandFont);

    for (int i = -2; i < 3; i++)
    {
        painter.drawLine(
            style.margin,
            style.staffY + i * style.staffSpacing,
            width() - style.margin,
            style.staffY + i * style.staffSpacing
        );
    }

    painter.drawText(
        style.margin + style.preClefSpacing, style.staffY + style.staffSpacing, QString(SMuFL::trebleClef)
    );
}