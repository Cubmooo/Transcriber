#include "pch/pch.h"
#include "note.h"
#include "smufl.h"
#include "layout.h"

NoteWidget::NoteWidget(QWidget *parent)
    : QWidget(parent),
      lelandFont("Leland"),
      lelandMetrics(lelandFont)
{
}



void NoteWidget::setStaveLayout(const StaveLayout &layout)
{
    style = layout;
    lelandFont.setPointSize(style.lelandFontSize);
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

std::vector<double> findNoteLengthArray(double noteLength, double beat)
{
    std::vector<double> noteLengthArray;
    double currentBeat = beat;

    if (std::floor(currentBeat) != currentBeat){
        double fractionalNoteLength = std::min(std::ceil(currentBeat) - currentBeat, noteLength);
        noteLengthArray.emplace_back(fractionalNoteLength);
        noteLength -= fractionalNoteLength;
        currentBeat += fractionalNoteLength;
    }

    static const double validNoteLengths[] = {4, 3, 2, 1.5, 1, 0.75, 0.5, 0.25};
    constexpr double barLength = 4.0;
    constexpr double epsilon = 1e-6;

    while (noteLength > epsilon){
        double beatWithinBar = std::fmod(currentBeat, barLength);
        double distanceToBar = barLength - beatWithinBar;
        if (distanceToBar <= epsilon) distanceToBar = barLength;

        double maxFragment = std::min(noteLength, distanceToBar);

        double fragment = maxFragment;
        for (double validNoteLength : validNoteLengths){
            if (maxFragment >= validNoteLength - epsilon){
                fragment = validNoteLength;
                break;
            }
        }

        noteLengthArray.emplace_back(fragment);
        noteLength -= fragment;
        currentBeat += fragment;
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
        noteLength = 1.0 - (notes[i].second - std::floor(notes[i].second));
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
        }
        noteSpacingDistance *= fontSize * 1;

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

void drawBarLine(QPainter &painter, double x, int line, const StaveLayout &style)
{
    double y = style.staffY + line * style.systemSpacing;

    painter.drawLine(
        x - style.spatium * 0.4 , y - style.spatium * 2,
        x - style.spatium * 0.4, y + style.spatium * 2
    );
}


void drawBeamGroup(QPainter &painter, const std::vector<QPointF> &noteHeads, bool stemUp, const StaveLayout &style)
{
    if (noteHeads.size() < 2) return;
    const double dir = stemUp ? -1.0 : 1.0;

    double beamY;
    if (stemUp){
        double minY = noteHeads.front().y();
        for (const auto &p : noteHeads) minY = std::min(minY, p.y());
        beamY = minY + dir * style.stemLength;
    } else {
        double maxY = noteHeads.front().y();
        for (const auto &p : noteHeads) maxY = std::max(maxY, p.y());
        beamY = maxY + dir * style.stemLength;
    }

    painter.save();
    QPen stemPen(Qt::black);
    stemPen.setWidthF(style.stemThickness);
    painter.setPen(stemPen);
    for (const auto &p : noteHeads){
        painter.drawLine(QPointF(p.x(), p.y()), QPointF(p.x(), beamY));
    }
    painter.restore();

    double x1 = noteHeads.front().x();
    double x2 = noteHeads.back().x();

    QRectF beamRect(x1, beamY - style.beamThickness / 2.0, x2 - x1, style.beamThickness);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRect(beamRect);
    painter.restore();
}

int computeLineOffset(double lastFragmentX, int lastFragmentLine, const StaveLayout &style)
{
    if (lastFragmentLine < style.numberOfLines){return 0;}
    else{
        if (lastFragmentX > style.screenBeatThreshold * 0.8){
            return lastFragmentLine - style.numberOfLines + 2;
        }
        else{return lastFragmentLine - style.numberOfLines + 1;}
    }
}


void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(lelandFont);

    static double lastFragmentX = 0;
    static int lastFragmentLine = 1;
    int lineOffset = computeLineOffset(lastFragmentX, lastFragmentLine, style);

    double cumulativeNoteX = style.margin;
    int line = 0;
    double spacingNoteX = style.margin;

    struct PendingBeamNote { double x, y, noteLength; int notePosition; bool isRest; };
    std::vector<PendingBeamNote> pendingBeam;
    int pendingHalfBar = -1, pendingLine = -1;
    bool pendingStemUp = false;

    auto flushBeam = [&]()
    {
        if (pendingBeam.size() >= 2){
            std::vector<QPointF> pts;
            for (auto &pn : pendingBeam){
                painter.drawText(pn.x, pn.y, QString(SMuFL::noteheadBlack));
                pts.emplace_back(pn.x, pn.y);
            }
            drawBeamGroup(painter, pts, pendingStemUp, style);
        } else if (pendingBeam.size() == 1){
            auto &pn = pendingBeam[0];
            painter.drawText(pn.x, pn.y, QString(findNoteGlyph(pn.noteLength, pn.notePosition, pn.isRest)));
        }
        pendingBeam.clear();
    };

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

        double currentBeat = notes[i].second;

        double notePanning = 0.0;

        bool firstTime = true;
        double previousSpacingNoteX = -1;
        double previousNoteY = -1;
        for (double noteLength : noteLengthArray){

            bool isFirstInBar = std::fmod(std::fmod(currentBeat, 4.0) + 4.0, 4.0) < 1e-6;

            if (isFirstInBar){
                cumulativeNoteX += style.fontSize * 0.3;
                line = std::floor(cumulativeNoteX / style.screenBeatThreshold);
                spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;
            }

            if (!isRest && firstTime && flatSharp != 0){
                cumulativeNoteX += style.fontSize * 0.3;
                line = std::floor(cumulativeNoteX / style.screenBeatThreshold);
                spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;
            }

            int displayedLine = line - lineOffset;

            lastFragmentX = spacingNoteX;
            lastFragmentLine = line;

            isRest = (notePosition == 0);
            double noteSpacingDistance = findNoteSpacingDistance(isRest, noteLength, flatSharp, style.fontSize);
            int noteY = style.staffY - style.spatium * distanceFromBase / 2 + displayedLine * style.systemSpacing;
            double noteHeadX = spacingNoteX;

            bool beamable = !isRest && noteLength < 0.75 - 1e-6;
            int halfBar = static_cast<int>(std::floor(currentBeat / 2.0 + 1e-6));

            if (beamable && !pendingBeam.empty() && halfBar == pendingHalfBar
                && line == pendingLine && pendingBeam.size() < 4){
                pendingBeam.push_back({spacingNoteX - notePanning, (double)noteY, noteLength, notePosition, isRest});
            } else {
                flushBeam();
                if (beamable){
                    pendingBeam.push_back({spacingNoteX - notePanning, (double)noteY, noteLength, notePosition, isRest});
                    pendingHalfBar = halfBar;
                    pendingLine = line;
                    pendingStemUp = stemUp;
                } else {
                    painter.drawText(spacingNoteX - notePanning, noteY, QString(findNoteGlyph(noteLength, notePosition, isRest)));
                }
            }

            currentBeat += noteLength;

            double beatMod = std::fmod(std::fmod(currentBeat, 4.0) + 4.0, 4.0);
            bool isBarLine = (beatMod < 0.001 || beatMod > 3.999);

            double newCumulativeNoteX = cumulativeNoteX + noteSpacingDistance;
            int newLine = std::floor(newCumulativeNoteX / style.screenBeatThreshold);
            double barLineX = newCumulativeNoteX - line * style.screenBeatThreshold;
            if (isBarLine){drawBarLine(painter, barLineX - notePanning, displayedLine, style);}

            cumulativeNoteX = newCumulativeNoteX;
            line = newLine;
            spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;

            for (int j = 0; j < std::abs(distanceFromBase) / 2 - 2; j++){
            painter.drawLine(
                spacingNoteX - style.fontSize / 4 - notePanning,
                style.staffY + style.spatium * (j + 3) * ledgerDirection + displayedLine * style.systemSpacing,
                spacingNoteX + style.fontSize * 3 / 4  - notePanning,
                style.staffY + style.spatium * (j + 3) * ledgerDirection + displayedLine * style.systemSpacing);
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
            previousSpacingNoteX = spacingNoteX;
            previousNoteY = noteY;
        }
    }
    flushBeam();
}