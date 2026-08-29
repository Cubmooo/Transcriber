#include "pch/pch.h"
#include "note.h"
#include "smufl.h"
#include "layout.h"

NoteWidget::NoteWidget(QWidget *parent)
    : QWidget(parent),
      lelandFont("Leland"),
      lelandMetrics(lelandFont)
{
    lelandFont.setPointSize(100);
}

void NoteWidget::setStaveLayout(const StaveLayout &layout)
{
    style = layout;
    update();
}

void NoteWidget::setNote(std::vector<std::pair<int, double>> BPMTimeList)
{
    notes = BPMTimeList;
    update();
}

void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(lelandFont);

    for (const auto &noteData : notes)
    {

        int notePosition = noteData.first;
        double beat = noteData.second;

        QString crochet;
        QString accidental;
        if (notePosition == 0)
        {
            painter.drawText(
                style.margin + 150, style.staffY, QString(SMuFL::crochetRest));
            continue;
        }
        else if (notePosition > 59)
        {
            crochet = QString(SMuFL::upStemCrochet);
        }
        else
        {
            crochet = QString(SMuFL::downStemCrochet);
        }

        int octaves = notePosition / 12;
        int semiTones = notePosition % 12;
        int flatSharp = 0;
        int note;
        int distanceFromBase;
        int ledgerDirection;
        int noteY;
        int noteX;

        static const std::unordered_map<int, std::pair<int, int>> noteTable = {
            {0, {0, 0}},
            {1, {0, 1}},
            {2, {1, 0}},
            {3, {2, -1}},
            {4, {2, 0}},
            {5, {3, 0}},
            {6, {3, 1}},
            {7, {4, 0}},
            {8, {5, -1}},
            {9, {5, 0}},
            {10, {6, -1}},
            {11, {6, 0}},
        };

        auto it = noteTable.find(semiTones);
        if (it != noteTable.end())
        {
            note = it->second.first;
            flatSharp = it->second.second;
        }

        if (notePosition >= 48)
        {
            distanceFromBase = (note - 6) + (octaves - 4) * 7;
            ledgerDirection = 1;
        }
        else
        {
            distanceFromBase = (note - 1) + (octaves - 3) * 7;
            ledgerDirection = -1;
        }

        noteX = style.margin + style.fontSize * 1.5 * beat;
        noteY = style.staffY - style.staffSpacing * distanceFromBase / 2;

        std::cout << distanceFromBase << " distanceFromBase    " << beat << " beat" << std::endl;
        painter.drawText(
            noteX, noteY, crochet);

        for (int i = 0; i < std::abs(distanceFromBase) / 2 - 2; i++)
        {
            painter.drawLine(
                noteX - style.fontSize / 4,
                style.staffY + style.staffSpacing * (i + 3) * ledgerDirection,
                noteX + style.fontSize * 3 / 4,
                style.staffY + style.staffSpacing * (i + 3) * ledgerDirection);
        }

        if (flatSharp == 1)
        {
            accidental = QString(SMuFL::sharp);
        }
        else if (flatSharp == -1)
        {
            accidental = QString(SMuFL::flat);
        }
        else
        {
            continue;
        }

        if (!accidental.isEmpty())
        {
            painter.drawText(
                noteX - style.fontSize / 2, noteY, accidental);
        }
    }
}