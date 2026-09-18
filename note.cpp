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
        x - style.spatium , y - style.spatium * 2,
        x - style.spatium, y + style.spatium * 2
    );
}

struct BeamGroup
{
    int startIndex;
    int count;
    bool stemUp;
};

std::vector<int> computeNoteLines(const std::vector<std::pair<int, double>> &notes, const StaveLayout &style)
{
    std::vector<int> noteLines;
    double cumulativeNoteX = style.margin;
    int line = 0;

    for (size_t i = 0; i < notes.size(); ++i)
    {
        std::vector<double> noteLengthArray = findNoteLength(static_cast<int>(i), notes);
        if (!noteLengthArray.empty() && noteLengthArray[0] == -1)
            continue;

        int notePosition = notes[i].first;
        bool isRest = (notePosition == 0);
        int flatSharp = isRest ? 0 : findAccidental(notePosition).second;

        bool firstTime = true;
        for (double noteLength : noteLengthArray)
        {
            noteLines.push_back(line);
            if (!isRest && firstTime && flatSharp != 0){
                cumulativeNoteX += style.fontSize * 0.3;
                line = static_cast<int>(std::floor(cumulativeNoteX / style.screenBeatThreshold));
            }
            firstTime = false;

            cumulativeNoteX += findNoteSpacingDistance(isRest, noteLength, flatSharp, style.fontSize);
            line = static_cast<int>(std::floor(cumulativeNoteX / style.screenBeatThreshold));
        }
    }
    return noteLines;
}

std::vector<BeamGroup> computeBeamGroups(const std::vector<std::pair<int, double>> &notes, const std::vector<int> &noteLines)
{
    std::vector<BeamGroup> groups;
    constexpr double epsilon = 1e-6;

    int flatIndex = 0;
    int groupStart = -1;
    int groupCount = 0;
    bool groupStemUp = false;
    int groupHalfBar = -1;
    int groupLine = -1; 

    auto closeGroup = [&]()
    {
        if (groupCount >= 2){
            groups.push_back({groupStart, groupCount, groupStemUp});
        }
        groupStart = -1;
        groupCount = 0;
        groupHalfBar = -1;
        groupLine = -1;
    };

    for (size_t i = 0; i < notes.size(); ++i)
    {
        std::vector<double> noteLengthArray = findNoteLength(static_cast<int>(i), notes);
        if (!noteLengthArray.empty() && noteLengthArray[0] == -1){
            continue;
        }

        int notePosition = notes[i].first;
        bool isRest = (notePosition == 0);
        bool stemUp = (notePosition > 59);
        double beat = notes[i].second;

        for (double noteLength : noteLengthArray)
        {
            bool beamable = !isRest && noteLength < 0.75 - epsilon;
            int halfBar = static_cast<int>(std::floor(beat / 2.0 + epsilon));
            int currentLine = (flatIndex < static_cast<int>(noteLines.size()))
                                  ? noteLines[flatIndex] : 0;

            if (beamable && groupCount > 0 && halfBar == groupHalfBar && groupCount < 4){
                groupCount++;
            } else {
                closeGroup();
                if (beamable){
                    groupStart = flatIndex;
                    groupCount = 1;
                    groupHalfBar = halfBar;
                    groupStemUp = stemUp;
                    groupLine = currentLine;
                }
            }

            beat += noteLength;
            flatIndex++;
        }
    }
    closeGroup();
    return groups;
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


void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(lelandFont);
    double cumulativeNoteX = style.margin;
    int line = 0;
    double spacingNoteX = style.margin;

    std::vector<int> noteLines = computeNoteLines(notes, style);
    std::vector<BeamGroup> beamGroups = computeBeamGroups(notes, noteLines);
    size_t nextGroupIdx = 0;
    int flatIndex = 0;
    bool inGroup = false;
    int groupRemaining = 0;
    bool groupStemUpCached = false;
    std::vector<QPointF> beamStemPoints;

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
        /*if (!notes.empty()){
            int maxNoteX = style.margin + style.fontSize * 1.5 * notes[notes.size() - 1].second;
            if (maxNoteX > style.screenBeatThreshold){
                notePanning = maxNoteX - style.screenBeatThreshold;
            }
        }*/

        bool firstTime = true;
        double previousSpacingNoteX = -1;
        double previousNoteY = -1;
        for (double noteLength : noteLengthArray){

            if (!isRest && firstTime && flatSharp != 0){
                cumulativeNoteX += style.fontSize * 0.3;
                line = std::floor(cumulativeNoteX / style.screenBeatThreshold);
                spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;
            }

            bool startsGroup = (nextGroupIdx < beamGroups.size() && beamGroups[nextGroupIdx].startIndex == flatIndex);
            if (startsGroup){
                inGroup = true;
                groupRemaining = beamGroups[nextGroupIdx].count;
                groupStemUpCached = beamGroups[nextGroupIdx].stemUp;
                beamStemPoints.clear();
                nextGroupIdx++;
            }

            QString crochet;
            isRest = (notePosition == 0);

            if (inGroup){
                crochet = QString(SMuFL::noteheadBlack);
            } else {
                crochet = findNoteGlyph(noteLength, notePosition, isRest);
            }

            double noteSpacingDistance = 0;
            noteSpacingDistance = findNoteSpacingDistance(isRest, noteLength, flatSharp, style.fontSize);
            int noteY = style.staffY - style.spatium * distanceFromBase / 2 + line * style.systemSpacing;

            double noteHeadX = spacingNoteX;
            painter.drawText(spacingNoteX - notePanning, noteY, crochet);

            if (inGroup){
                beamStemPoints.push_back(QPointF(spacingNoteX - notePanning, noteY));
                groupRemaining--;
                if (groupRemaining == 0){
                    drawBeamGroup(painter, beamStemPoints, groupStemUpCached, style);
                    inGroup = false;
                }
            }

            currentBeat += noteLength;

            double beatMod = std::fmod(std::fmod(currentBeat, 4.0) + 4.0, 4.0);
            bool isBarLine = (beatMod < 0.001 || beatMod > 3.999);

            double newCumulativeNoteX = cumulativeNoteX + noteSpacingDistance;
            int newLine = std::floor(newCumulativeNoteX / style.screenBeatThreshold);

            if (isBarLine){
                if (newLine > line){
                    double edgeX = (line + 1) * style.screenBeatThreshold;
                    drawBarLine(painter, edgeX - notePanning, line, style);
                } else {
                    double barLineX = newCumulativeNoteX - line * style.screenBeatThreshold;
                    drawBarLine(painter, barLineX - notePanning, line, style);
                }
            }

            cumulativeNoteX = newCumulativeNoteX;
            line = newLine;
            spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;

            for (int j = 0; j < std::abs(distanceFromBase) / 2 - 2; j++){
            painter.drawLine(
                spacingNoteX - style.fontSize / 4 - notePanning,
                style.staffY + style.spatium * (j + 3) * ledgerDirection,
                spacingNoteX + style.fontSize * 3 / 4  - notePanning,
                style.staffY + style.spatium * (j + 3) * ledgerDirection);
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

            flatIndex++;
        }
    }
}