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

void NoteWidget::setNotes(std::vector<std::pair<int, int>> newBPMTimeList)
{
    BPMTimeList = newBPMTimeList;
    if (!BPMTimeList.empty()) {
        int notePosition = BPMTimeList.back().first;
        int noteLength = BPMTimeList.back().second;
    }
    update();
}

void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(lelandFont);

    for (const auto& [notePosition, noteLength] : BPMTimeList){
        QString crochet;
        QString accidental;
        /*std::cout << notePosition << "note pos" << std::endl;*/       
        int octaves = notePosition / 12;
        int semiTones = notePosition % 12;
        int flatSharp = 0;
        int notePitch;
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
        if (it != noteTable.end()) {
            notePitch = it->second.first;
            flatSharp = it->second.second;
        }
        
        noteX = style.margin + style.fontSize * 1.5 * noteLength;

        if (notePosition == 0){
            painter.drawText(
            noteX, style.staffY, QString(SMuFL::crochetRest));
            continue;
        }
        else if (notePosition > 59){
            crochet = QString(SMuFL::upStemCrochet);
        }
        else{
            crochet = QString(SMuFL::downStemCrochet);
        }

        if (notePosition >= 48){
            distanceFromBase = (notePitch - 6) + (octaves - 4) * 7;
            ledgerDirection = 1;
        }
        else {
            distanceFromBase = (notePitch - 1) + (octaves - 3) * 7;
            ledgerDirection = -1;
        }

        noteY = style.staffY - style.staffSpacing * distanceFromBase/2;

        /*std::cout << distanceFromBase << "         " <<  notePitch <<"np  " << octaves << "oct" << std::endl;*/
        painter.drawText(
            noteX, noteY, crochet
        );

        for (int i = 0; i < std::abs(distanceFromBase)/2 - 2; i++){
            painter.drawLine(
                noteX - style.fontSize/4,
                style.staffY + style.staffSpacing * (i + 3) * ledgerDirection,
                noteX + style.fontSize * 3/4,
                style.staffY + style.staffSpacing * (i + 3) * ledgerDirection
            );
        }

        if (flatSharp == 1) {accidental = QString(SMuFL::sharp);}
        else if (flatSharp == -1) {accidental = QString(SMuFL::flat);}
        else {continue;}

        if (!accidental.isEmpty()) {
            painter.drawText(
                noteX - style.fontSize/2, noteY, accidental
            );
        }
    }
}