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



std::pair<int, int> findAccidental(int notePosition){
    int semiTones = notePosition % 12;
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
    return {0, 0};
}



QChar findNoteGlyph(double noteLength, int notePosition, bool isRest){
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

    bool stemUp;
    if(notePosition > 59){stemUp = true;}
    else{stemUp = false;}
        
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

std::vector<double> findNoteLengthArray(double noteLength, double beat){
    std::vector<double> noteLengthArray;
    if (std::floor(beat) != beat){
        double fractionalNoteLength = std::ceil(beat) - beat;
        noteLengthArray.emplace_back(fractionalNoteLength);

        noteLength -= fractionalNoteLength;
    }

    double validNoteLengths[] = {4, 3, 2, 1.5, 1, 0.75, 0.5, 0.25};

    while (noteLength > 0){
        for (double validNoteLength : validNoteLengths){
            if (noteLength >= validNoteLength){
                noteLengthArray.emplace_back(noteLength);
                noteLength -= validNoteLength;
            }
        }
    }
    return noteLengthArray;
}


std::vector<double> findNoteLength(int i, std::vector<std::pair<int, double>> notes)
{
    int notePosition = notes[i].first;
    std::vector<double> noteLengthArray;
    double noteLength = -1;

    if (i > 0 && i < notes.size() - 1)
    {
        if (notePosition == notes[i-1].first){return {-1};}

        for (int j = i + 1; j < notes.size(); j++)
        {
            if (notePosition != notes[j].first){
                noteLength = notes[j].second - notes[i].second;
                break;
            }
            if (j == notes.size() - 1){
                noteLength = std::ceil(notes[j].second) - notes[i].second;
            }
        }
    }
    if (noteLength == -1){
        noteLength = 1.0 - (notes[i].first - std::floor(notes[i].first));
    }
    return findNoteLengthArray(noteLength, notes[i].second);
}



std::pair<int, int> findLedgerLines(int notePosition, int note){
    int ledgerDirection;
    int distanceFromBase;
    int octaves = notePosition / 12;
    if (notePosition >= 48){
    distanceFromBase = (note - 6) + (octaves - 4) * 7;
    ledgerDirection = -1;
    }
    else if (notePosition != 0){
        distanceFromBase = (note - 1) + (octaves - 3) * 7;
        ledgerDirection = 1;
    }
    return {distanceFromBase, ledgerDirection};
}



double findNoteSpacingDistance(bool isRest, double noteLength, int flatSharp, int fontSize){
    double noteSpacingDistance;
    if (noteLength >= 1){noteSpacingDistance = noteLength;}
        else{
            noteSpacingDistance = sqrt(noteLength);
            if (!isRest && flatSharp != 0){noteSpacingDistance += 0.3;}
        }
        noteSpacingDistance *= fontSize * 1.5;

    return noteSpacingDistance;
}

void drawTie(QPainter &painter, double startX, double endX, double startY, double endY, double notePanning, bool above, const StaveLayout &style)
{
    double x1 = startX - notePanning;
    double x2 = endX   - notePanning;

    const double endInset     = style.endInset;
    const double minShoulderH = style.minShoulderH;
    const double maxShoulderH = style.maxShoulderH;
    const double heightRatio  = style.heightRatio;           
    const double midThickness = style.midThickness;
    const double baseGap      = style.baseGap;

    x1 += endInset;
    x2 -= endInset;
    double len = x2 - x1;
    if (len <= 0.0)
        return;

    double shoulderH = qBound(minShoulderH, len * heightRatio, maxShoulderH);
    double dir = above ? -1.0 : 1.0;

    double y1 = startY + dir * baseGap;
    double y2 = endY   + dir * baseGap;
    double yShoulder = ((y1 + y2) * 0.5) + dir * shoulderH;
    double half = midThickness * 0.5;

    double cx1 = x1 + len * 0.25;
    double cx2 = x1 + len * 0.75;

    QPainterPath tie;
    tie.moveTo(x1, y1);
    
    tie.cubicTo(cx1, yShoulder - dir * half,
                cx2, yShoulder - dir * half,
                x2, y2);
    
    tie.cubicTo(cx2, yShoulder + dir * half,
                cx1, yShoulder + dir * half,
                x1, y1);
    tie.closeSubpath();

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawPath(tie);
    painter.restore();
}


void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(lelandFont);
    double cumulativeNoteX = style.margin;

    for (size_t i = 0; i < notes.size(); ++i)
    {
        QString accidental;
        int notePosition = notes[i].first;
        int note, distanceFromBase = 0, ledgerDirection, flatSharp = 0;
        bool isRest = (notePosition == 0);

        if (!isRest) {
            std::tie(note, flatSharp) = findAccidental(notePosition);
            std::tie(distanceFromBase, ledgerDirection) = findLedgerLines(notePosition, note);
        }

        bool stemUp = (notePosition > 59);

        std::vector<double> noteLengthArray = findNoteLength(i, notes);
        if (!noteLengthArray.empty() && noteLengthArray[0] == -1){continue;}

        double notePanning = 0.0;
        if (!notes.empty()){
            int maxNoteX = style.margin + style.fontSize * 1.5 * notes[notes.size() - 1].second;
            if (maxNoteX > style.screenBeatThreshold){
                notePanning = maxNoteX - style.screenBeatThreshold;
            }
        }

        bool firstTime = true;
        double previousCumulativeNoteX = -1;
        double previousNoteY = -1;
        for (double noteLength : noteLengthArray){

            QString crochet;
            isRest = (notePosition == 0);
            crochet = findNoteGlyph(noteLength, notePosition, isRest);

            double noteSpacingDistance = 0;
            noteSpacingDistance = findNoteSpacingDistance(isRest, noteLength, flatSharp, style.fontSize);
            int noteY = style.staffY - style.staffSpacing * distanceFromBase / 2;

            double noteHeadX = cumulativeNoteX;
            painter.drawText(cumulativeNoteX - notePanning, noteY, crochet);
            cumulativeNoteX += noteSpacingDistance;

            if (previousCumulativeNoteX != -1 && !isRest){
                drawTie(painter, previousCumulativeNoteX, cumulativeNoteX, previousNoteY, noteY, notePanning, !stemUp, style);
            }

            for (int i = 0; i < std::abs(distanceFromBase) / 2 - 2; i++){
            painter.drawLine(
                cumulativeNoteX - style.fontSize / 4 - notePanning,
                style.staffY + style.staffSpacing * (i + 3) * ledgerDirection,
                cumulativeNoteX + style.fontSize * 3 / 4  - notePanning,
                style.staffY + style.staffSpacing * (i + 3) * ledgerDirection);
            }

            if (!isRest && firstTime){
                if (flatSharp == 1){
                    accidental = QString(SMuFL::sharp);
                }
                else if (flatSharp == -1){
                    accidental = QString(SMuFL::flat);
                }

                if (!accidental.isEmpty()){
                    double accidentalWidth = painter.fontMetrics().horizontalAdvance(accidental);
                    painter.drawText(noteHeadX - accidentalWidth - notePanning - style.fontSize * 0.15, noteY, accidental);
                }
            }
            firstTime = false;
            previousCumulativeNoteX = cumulativeNoteX;
            previousNoteY = noteY;
        }
    }
}