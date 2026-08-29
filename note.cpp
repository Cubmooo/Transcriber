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

std::pair<int, int> findAccidental(int semiTones){
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
    if (it != noteTable.end()){
        return {it->second.first, it->second.second};
    }
}

QChar findNoteGlyph(double noteLength, bool stemUp, bool isRest){
    struct NoteType
    {
        double minLength;
        QChar noteUp;
        QChar noteDown;
        QChar rest;
    };

    static const NoteType noteTypes[] =
    {
        { 3.0,   SMuFL::semibreve,        SMuFL::semibreve,         SMuFL::semibreveRest },
        { 1.5,   SMuFL::upStemMinim,      SMuFL::downStemMinim,     SMuFL::minimRest },
        { 0.75,  SMuFL::upStemCrochet,    SMuFL::downStemCrochet,   SMuFL::crochetRest },
        { 0.375, SMuFL::upStemQuaver,     SMuFL::downStemQuaver,    SMuFL::quaverRest },
        { 0.0,   SMuFL::upStemSemiquaver, SMuFL::downStemSemiquaver, SMuFL::semiquaverRest }
    };

    for (const auto& type : noteTypes)
    {
        if (noteLength >= type.minLength)
        {
            if (isRest)
                return type.rest;

            return stemUp ? type.noteUp : type.noteDown;
        }
    }

    return QChar();
}

void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setFont(lelandFont);

    for (size_t i = 0; i < notes.size(); ++i)
    {
        const auto &noteData = notes[i];
        int notePosition = noteData.first;
        double beat = noteData.second;
        double noteLength = 1;

        int octaves = notePosition / 12;
        int semiTones = notePosition % 12;
        int distanceFromBase;
        int ledgerDirection;
        int noteY;
        int noteX;

        if (i >= 1){
            if (notePosition == notes[i-1].first){continue;}

            if (i != notes.size() - 1){
                for(int j = i + 1; j < notes.size(); j++){
                    if (notePosition == notes[j].first){
                        noteLength = notes[j].second - beat;
                    }
                    else{break;}
                }
            }
        } 
        std::cout << "note length, note position, beat  " << noteLength << "  " << notePosition <<  "  " << beat << std::endl;




        /*if (i >= 1) {
            if (notePosition == notes[i-1].first){
                int j = 1;
                while (true){
                    if (j <= i){
                        if (notePosition != notes[i - j].first){
                            noteLength = beat - notes[i - j].second;
                            break;
                        }
                    }
                    else{
                        noteLength = beat;
                    }
                    j++;
                }    
            }

            else{
            noteLength = beat - notes[i - 1].second;
            }
        }*/

        
        QString crochet;
        bool stemUp = false;
        bool isRest = false;
        if(notePosition > 59){stemUp = true;}
        if (notePosition == 0){isRest = true;}

        crochet = findNoteGlyph(noteLength, stemUp, isRest);
        if (isRest){continue;}

        QString accidental;
        auto [note, flatSharp] = findAccidental(semiTones);

        if (notePosition >= 48){
            distanceFromBase = (note - 6) + (octaves - 4) * 7;
            ledgerDirection = 1;
        }
        else if (notePosition != 0){
            distanceFromBase = (note - 1) + (octaves - 3) * 7;
            ledgerDirection = -1;
        }

        double notePanning = 0.0;
        if (!notes.empty()){
            notePanning = std::max(0.0,notes.back().second - style.screenBeatThreshold);
        }
        noteX = style.margin + style.fontSize * 1.5 * (beat - notePanning);
        noteY = style.staffY - style.staffSpacing * distanceFromBase / 2;

        /*
        noteX = style.margin + style.fontSize * 1.5 * (beat - style.screenBeatThreshold);
        noteY = style.staffY - style.staffSpacing * distanceFromBase / 2 + style.systemSpacing;
        */

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

        if (flatSharp == 1){
            accidental = QString(SMuFL::sharp);
        }
        else if (flatSharp == -1){
            accidental = QString(SMuFL::flat);
        }

        if (!accidental.isEmpty()){
            painter.drawText(noteX - style.fontSize / 2, noteY, accidental);
        }
    }
}