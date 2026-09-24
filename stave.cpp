#include "pch/pch.h"
#include "stave.h"
#include "smufl.h"
#include "layout.h"

StaveWidget::StaveWidget(QWidget *parent)
    : QWidget(parent),
      lelandFont("Leland"),
      lelandMetrics(lelandFont)
{
    style = DefineLayoutConstants(height(), width(), zoom);
    lelandFont.setPointSize(style.lelandFontSize);
    setMinimumHeight(style.staveWidgetHeight);
    noteWidget = new NoteWidget(this);
    noteWidget->raise();
}

void StaveWidget::setNote(std::vector<std::pair<int, double>> BPMTimeList)
{
    noteWidget->setNote(BPMTimeList);
    update();
}

void StaveWidget::zoomBy(double factor)
{
    zoom = qBound(0.4, zoom * factor, 3.0);
    style = DefineLayoutConstants(height(), width(), zoom);
    lelandFont.setPointSize(style.lelandFontSize);
    setMinimumHeight(style.staveWidgetHeight);
    noteWidget->setStaveLayout(style);
    update();
}

void StaveWidget::setZoom(double value)
{
    zoom = qBound(0.4, value, 3.0);
    style = DefineLayoutConstants(height(), width(), zoom);
    lelandFont.setPointSize(style.lelandFontSize);
    setMinimumHeight(style.staveWidgetHeight);
    noteWidget->setStaveLayout(style);
    update();
}

void StaveWidget::scrollBy(int lines)
{
    scrollOffset = std::max(0, scrollOffset + lines);
    noteWidget->setScrollOffset(scrollOffset);
}

void StaveWidget::resetScroll()
{
    scrollOffset = 0;
    noteWidget->setScrollOffset(0);
}

void StaveWidget::resizeEvent(QResizeEvent *)
{
    noteWidget->setGeometry(rect());
    style = DefineLayoutConstants(height(), width(), zoom);
    lelandFont.setPointSize(style.lelandFontSize);
    noteWidget->setStaveLayout(style);
}

void StaveWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(lelandFont);

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
}