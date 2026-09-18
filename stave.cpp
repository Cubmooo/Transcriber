#include "pch/pch.h"
#include "stave.h"
#include "smufl.h"
#include "layout.h"

StaveWidget::StaveWidget(QWidget *parent)
    : QWidget(parent),
      lelandFont("Leland"),
      lelandMetrics(lelandFont)
{
    style = DefineLayoutConstants(height(), width());
    lelandFont.setPointSize(style.lelandFontSize);
    setMinimumHeight(style.staveWidgetHeight);
    noteWidget = new NoteWidget(this);
    noteWidget->raise();
}

void StaveWidget::setNote(std::vector<std::pair<int, double>> BPMTimeList)
{
    if (BPMTimeList.empty())
        return;
    noteWidget->setNote(BPMTimeList);
    notePosition = BPMTimeList.back().first;
    update();
}

void StaveWidget::resizeEvent(QResizeEvent *)
{
    noteWidget->setGeometry(rect());
    style = DefineLayoutConstants(height(), width());
    noteWidget->setStaveLayout(style);
}

void StaveWidget::paintEvent(QPaintEvent *)
{
    int clefYOffset;
    QString clef;
    QPainter painter(this);
    painter.setFont(lelandFont);

    /*
    for (int j = -2; j < 3; j++)
    {
        painter.drawLine(
            style.margin,
            style.staffY + j * style.staffSpacing,
            width() - style.margin,
            style.staffY + j * style.staffSpacing);
    }
    */

    
    for (int i = 0; i <= style.numberOfLines - 1; i++){
        for (int j = -2; j < 3; j++)
        {
            painter.drawLine(
                style.margin,
                style.staffY + j * style.spatium + style.systemSpacing * i,
                width() - style.margin,
                style.staffY + j * style.spatium + style.systemSpacing * i);
        }
    }
    



    if (notePosition >= 48)
    {
        clef = QString(SMuFL::trebleClef);
        clefYOffset = style.spatium;
    }
    else
    {
        clef = QString(SMuFL::bassClef);
        clefYOffset = -style.spatium;
    }

    painter.drawText(
        style.margin + style.preClefSpacing, style.staffY + clefYOffset, clef);
}