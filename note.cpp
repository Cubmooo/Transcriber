#include "note.h"
#include "smufl.h"
#include "layout.h"

#include <QPainter>
#include <QFont>

NoteWidget::NoteWidget(QWidget *parent)
    : QWidget(parent),
    lelandFont("Leland"),
    lelandMetrics(lelandFont)
{
    lelandFont.setPointSize(100);
}

void NoteWidget::setStaveLayout(const StaveLayout& layout)
{
    style = layout;
    update();
}

void NoteWidget::setNote(int position)
{
    notePosition = position;
    update();
}

void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(lelandFont);

    painter.drawText(
        style.margin + 150, style.staffY + style.staffSpacing * (notePosition - 59), QString(SMuFL::crochet)
    );
}