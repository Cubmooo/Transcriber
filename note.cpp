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

std::pair<int, int> findLedgerLines(int notePosition, int note); 

int staffStepsFromMiddleLine(int notePosition)
{
    int note = findAccidental(notePosition).first;
    return findLedgerLines(notePosition, note).first;
}

bool noteStemsUp(int notePosition)
{
    return notePosition != 0 && staffStepsFromMiddleLine(notePosition) < 0;
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
        { 4.0,   SMuFL::semibreve,        SMuFL::semibreve,         SMuFL::semibreveRest },
        { 2.0,   SMuFL::upStemMinim,      SMuFL::downStemMinim,     SMuFL::minimRest },
        { 1.0,   SMuFL::upStemCrochet,    SMuFL::downStemCrochet,   SMuFL::crochetRest },
        { 0.5,   SMuFL::upStemQuaver,     SMuFL::downStemQuaver,    SMuFL::quaverRest },
        { 0.0,   SMuFL::upStemSemiquaver, SMuFL::downStemSemiquaver, SMuFL::semiquaverRest }
    };

    const bool stemUp = !isRest && noteStemsUp(notePosition);
        
    for (const auto& type : noteTypes)
    {
        if (noteLength >= type.minLength - 1e-6)
        {
            if (isRest)
                return type.rest;

            return stemUp ? type.noteUp : type.noteDown;
        }
    }

    return QChar();
}



bool isShortNote(double noteLength){
    return noteLength < 0.5 - 1e-6;
}



bool isDottedLength(double noteLength){
    static const double dottedLengths[] = {3.0, 1.5, 0.75, 0.375};
    for (double d : dottedLengths){
        if (std::abs(noteLength - d) < 1e-6) return true;
    }
    return false;
}



std::vector<double> findNoteLengthArray(double noteLength, double beat, bool isRest = false)
{
    std::vector<double> noteLengthArray;
    double currentBeat = beat;

    if (std::floor(currentBeat) != currentBeat){
        double fractionalNoteLength = std::min(std::ceil(currentBeat) - currentBeat, noteLength);


        if (isRest && isDottedLength(fractionalNoteLength)){
            double firstPart = fractionalNoteLength / 3.0;
            noteLengthArray.emplace_back(firstPart);
            noteLength -= firstPart;
            currentBeat += firstPart;
            fractionalNoteLength -= firstPart;
        }

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
            if (isRest && isDottedLength(validNoteLength)) continue;
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


    return findNoteLengthArray(noteLength, notes[i].second, notePosition == 0);
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



void drawAugmentationDot(QPainter &painter, double x, double y, bool onLine, const StaveLayout &style)
{
    const double radius = style.dotRadius;
    if (onLine) y -= style.spatium * 0.5;
 
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QPointF(x, y), radius, radius);
    painter.restore();
}



void drawTie(QPainter &painter, double startX, double endX, double startY, double endY, double notePanning, bool above, const StaveLayout &style)
{
    startX += (style.endInset - notePanning);
    endX -= (style.endInset + notePanning);

    double length = endX - startX;
    if (length <= 0.0)
        return;

    double shoulderH = qBound(style.minShoulderH, length * style.heightRatio, style.maxShoulderH);
    double direction = above ? -1.0 : 1.0;

    startY += direction * style.baseGap;
    endY += direction * style.baseGap;
    double yShoulder = ((startY + endY) * 0.5) + direction * shoulderH;

    double cx1 = startX + length * 0.25;
    double cx2 = startX + length * 0.75;

    QPainterPath tie;
    tie.moveTo(startX, startY);
    
    tie.cubicTo(cx1, yShoulder - direction * style.midThickness * 0.5,
                cx2, yShoulder - direction * style.midThickness * 0.5,
                endX, endY);
    
    tie.cubicTo(cx2, yShoulder + direction * style.midThickness * 0.5,
                cx1, yShoulder + direction * style.midThickness * 0.5,
                startX, startY);
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
        x - style.barLineInset, y - style.spatium * 2,
        x - style.barLineInset, y + style.spatium * 2
    );
}



void drawBeamGroup(QPainter &painter, const std::vector<QPointF> &heads, const std::vector<double> &lengths, bool stemUp, const StaveLayout &style)
{
    if (heads.size() < 2) return;
    const double dir = stemUp ? -1.0 : 1.0;

    const double headWidth = painter.fontMetrics().horizontalAdvance(QString(SMuFL::noteheadBlack));
    const double stemOffset = stemUp ? headWidth - style.stemThickness / 2.0 : style.stemThickness / 2.0;

    std::vector<QPointF> noteHeads = heads;
    for (auto &p : noteHeads) p.setX(p.x() + stemOffset);

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

    QRectF beamRect(noteHeads.front().x(), beamY - style.beamThickness / 2.0, noteHeads.back().x() - noteHeads.front().x(), style.beamThickness);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRect(beamRect);
    painter.restore();


    const double beam2Y = beamY - dir * style.beamThickness * 1.5;
    const double stubLength = style.spatium;
 
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    for (size_t k = 0; k < noteHeads.size(); ++k){
        if (!isShortNote(lengths[k])) continue;
 
        double xa = noteHeads[k].x();
        if (k + 1 < noteHeads.size() && isShortNote(lengths[k + 1])){
            painter.drawRect(QRectF(xa, beam2Y - style.beamThickness / 2.0, noteHeads[k + 1].x() - xa, style.beamThickness));
        }

        else if (k == 0 || !isShortNote(lengths[k - 1])){
            double xLeft = (k == 0) ? xa : xa - style.stubLength;
            painter.drawRect(QRectF(xLeft, beam2Y - style.beamThickness / 2.0, style.stubLength, style.beamThickness));
        }
    }
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

double findRestShift(bool isRest, double noteLength, const QPainter& painter, double noteSpacingDistance, double spacingNoteX, const StaveLayout &style){
    double restShift = 0.0;
    const QFontMetrics metrics = painter.fontMetrics();
    double restWidth;
    double restLeft;

    if (isRest && noteLength >= 4.0 - 1e-6){
        restWidth = metrics.horizontalAdvance(QString(SMuFL::semibreveRest));
        restLeft = metrics.boundingRect(QString(SMuFL::semibreveRest)).left();
    }
    else if (isRest && noteLength >= 2.0 - 1e-6) {
        restWidth = metrics.horizontalAdvance(QString(SMuFL::minimRest));
        restLeft = metrics.boundingRect(QString(SMuFL::minimRest)).left();
    }
    else{return restShift;}

    restShift = std::max(0.0, std::min((noteSpacingDistance - restWidth) / 2.0 - restLeft, style.screenBeatThreshold - spacingNoteX - restWidth / 2 )
    );
}



void NoteWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(lelandFont);

    const double headWidth = painter.fontMetrics().horizontalAdvance(QString(SMuFL::noteheadBlack));

    const double clefWidth = std::max(
        painter.fontMetrics().horizontalAdvance(QString(SMuFL::trebleClef)),
        painter.fontMetrics().horizontalAdvance(QString(SMuFL::bassClef))
    );

    const double clefEnd = style.margin + style.preClefSpacing + clefWidth + style.spatium;

    static double lastFragmentX = 0;
    static int lastFragmentLine = 1;
    if (notes.empty()){ lastFragmentX = 0; lastFragmentLine = 1; }
    int lineOffset = computeLineOffset(lastFragmentX, lastFragmentLine, style);

    auto isDisplayed = [&](int displayedLine){ return displayedLine >= 0 && displayedLine < style.numberOfLines; };

    double cumulativeNoteX = style.margin;
    int line = 0;
    double spacingNoteX = style.margin;
    bool afterBarLine = false;

    struct PendingBeamNote { double x, y, noteLength; int notePosition; bool isRest; int beat; };
    std::vector<PendingBeamNote> pendingBeam;
    int pendingHalfBar = -1, pendingLine = -1;

    auto flushBeam = [&]()
    {
        if (pendingBeam.size() >= 2){
            std::vector<QPointF> pts;
            std::vector<double> lengths;
            int furthestAbove = 0, furthestBelow = 0;  
            for (auto &pn : pendingBeam){
                painter.drawText(pn.x, pn.y, QString(SMuFL::noteheadBlack));
                pts.emplace_back(pn.x, pn.y);
                lengths.push_back(pn.noteLength);
                int steps = staffStepsFromMiddleLine(pn.notePosition);
                furthestAbove = std::max(furthestAbove, steps);
                furthestBelow = std::max(furthestBelow, -steps); 
            }
            drawBeamGroup(painter, pts, lengths, furthestBelow > furthestAbove, style);
        } 
        else if (pendingBeam.size() == 1){
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

        bool stemUp = !isRest && noteStemsUp(notePosition);

        std::vector<double> noteLengthArray = findNoteLength(i, notes);
        if (!noteLengthArray.empty() && noteLengthArray[0] == -1){continue;}

        double currentBeat = notes[i].second;

        double notePanning = 0.0;

        bool firstTime = true;
        double previousSpacingNoteX = -1; 
        double previousNoteY = -1;
        int previousDisplayedLine = 0;
        for (double noteLength : noteLengthArray){

            const double slotStart = cumulativeNoteX;

            bool isFirstInBar = std::fmod(std::fmod(currentBeat, 4.0) + 4.0, 4.0) < 1e-6;

            if (isFirstInBar){
                cumulativeNoteX += style.afterBarLineGap;
                line = std::floor(cumulativeNoteX / style.screenBeatThreshold);
                spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;
            }

            if (!isRest && firstTime && flatSharp != 0){
                cumulativeNoteX += style.afterBarLineGap;
                line = std::floor(cumulativeNoteX / style.screenBeatThreshold);
                spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;
            }

            {
                double leftReach = 0.0;
                if (!isRest && firstTime && flatSharp != 0){
                    QString accidentalGlyph = QString(flatSharp == 1 ? SMuFL::sharp : SMuFL::flat);
                    leftReach = painter.fontMetrics().horizontalAdvance(accidentalGlyph) + style.fontSize * 0.15;
                }
                double minHeadX = slotStart + leftReach + (afterBarLine ? style.barGap - style.spatium * 0.4 : 0.0);
                if (cumulativeNoteX < minHeadX){
                    cumulativeNoteX = minHeadX;
                    line = std::floor(cumulativeNoteX / style.screenBeatThreshold);
                    spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;
                }
                double minLineX = clefEnd + leftReach + (firstTime ? 0.0 : style.spatium * 2.0);
                if (spacingNoteX < minLineX){
                    cumulativeNoteX += minLineX - spacingNoteX;
                    spacingNoteX = minLineX;
                }
            }

            int displayedLine = line - lineOffset;
            const bool visible = isDisplayed(displayedLine);

            lastFragmentX = spacingNoteX;
            lastFragmentLine = line;

            isRest = (notePosition == 0);
            double noteSpacingDistance = findNoteSpacingDistance(isRest, noteLength, flatSharp, style.fontSize);
            int noteY = style.staffY - style.spatium * distanceFromBase / 2 + displayedLine * style.systemSpacing;
            double noteHeadX = spacingNoteX;
            double restShift = findRestShift(isRest, noteLength, painter, noteSpacingDistance, spacingNoteX, style);

            bool beamable = !isRest && noteLength < 1 - 1e-6;
            int halfBar = static_cast<int>(std::floor(currentBeat / 2.0 + 1e-6));
            int beat    = static_cast<int>(std::floor(currentBeat + 1e-6));
            PendingBeamNote current{spacingNoteX - notePanning, (double)noteY, noteLength, notePosition, isRest, beat}; 

            if (!visible){
                flushBeam();
            }
            else if (beamable && !pendingBeam.empty() && halfBar == pendingHalfBar
                && line == pendingLine && pendingBeam.size() < 4){
                
                bool hasShort = isShortNote(noteLength) ||
                    std::any_of(pendingBeam.begin(), pendingBeam.end(), [](const PendingBeamNote &p){ return isShortNote(p.noteLength); });
 
                if (hasShort && beat != pendingBeam.front().beat){
                    std::vector<PendingBeamNote> earlier, carry;
                    for (auto &pn : pendingBeam)
                        (pn.beat == beat ? carry : earlier).push_back(pn);
 
                    pendingBeam = earlier;
                    flushBeam();
                    pendingBeam = carry;
                }
                pendingBeam.push_back(current);
            }

            else {
                flushBeam();
                if (beamable){
                    pendingBeam.push_back(current);
                    pendingHalfBar = halfBar;
                    pendingLine = line;
                } else {
                    painter.drawText(spacingNoteX + restShift - notePanning, noteY, QString(findNoteGlyph(noteLength, notePosition, isRest)));
                }
            }

            if (visible && !isRest && isDottedLength(noteLength)){
                QString headGlyph = QString(SMuFL::noteheadBlack);
                double dotX = noteHeadX - notePanning + painter.fontMetrics().horizontalAdvance(headGlyph) + style.spatium * 0.4;
                drawAugmentationDot(painter, dotX, noteY, distanceFromBase % 2 == 0, style);
            }

            if (!isRest && !firstTime){
                bool above = !stemUp;
                if (displayedLine == previousDisplayedLine){
                    if (visible)
                        drawTie(painter, previousSpacingNoteX, noteHeadX, previousNoteY, noteY, notePanning, above, style);
                } else {
                    if (isDisplayed(previousDisplayedLine))
                        drawTie(painter, previousSpacingNoteX, style.screenBeatThreshold, previousNoteY, previousNoteY, notePanning, above, style);
                    if (visible)
                        drawTie(painter, 0.0, noteHeadX, noteY, noteY, notePanning, above, style);
                }
            }

            currentBeat += noteLength;

            double beatMod = std::fmod(std::fmod(currentBeat, 4.0) + 4.0, 4.0);
            bool isBarLine = (beatMod < 0.001 || beatMod > 3.999);

            QString extentGlyph = QString(findNoteGlyph(noteLength, notePosition, isRest));
            double noteExtent = std::max<double>(painter.fontMetrics().horizontalAdvance(extentGlyph),
                                                 painter.fontMetrics().boundingRect(extentGlyph).right());
            if (!isRest && isDottedLength(noteLength)) noteExtent = std::max(noteExtent, headWidth + style.spatium * 0.6);
            double minSlot = noteExtent + (isBarLine ? style.barGap + style.spatium * 0.4 : style.noteGap);

            double newCumulativeNoteX = cumulativeNoteX + std::max(noteSpacingDistance, minSlot);
            int newLine = std::floor(newCumulativeNoteX / style.screenBeatThreshold);
            double barLineX = newCumulativeNoteX - line * style.screenBeatThreshold;
            if (isBarLine && visible){drawBarLine(painter, barLineX - notePanning, displayedLine, style);}
            afterBarLine = isBarLine; 

            cumulativeNoteX = newCumulativeNoteX;
            line = newLine;
            spacingNoteX = cumulativeNoteX - line * style.screenBeatThreshold;

            for (int j = 0; visible && j < std::abs(distanceFromBase) / 2 - 2; j++){
            painter.drawLine(
                noteHeadX - style.fontSize / 4 - notePanning,
                style.staffY + style.spatium * (j + 3) * ledgerDirection + displayedLine * style.systemSpacing,
                noteHeadX + style.fontSize * 3 / 4  - notePanning,
                style.staffY + style.spatium * (j + 3) * ledgerDirection + displayedLine * style.systemSpacing);
            }

            if (visible && !isRest && firstTime){
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
            previousSpacingNoteX = noteHeadX + headWidth + (isDottedLength(noteLength) ? style.spatium * 0.6 : 0.0);
            previousNoteY = noteY;
            previousDisplayedLine = displayedLine;
        }
    }
    flushBeam();
}