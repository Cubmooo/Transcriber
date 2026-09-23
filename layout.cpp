#include "layout.h"

StaveLayout DefineLayoutConstants(int height, int width, double zoom)
{
    StaveLayout style;

    style.spatium = 25 * zoom;
    int spatium = style.spatium;

    style.fontSize = spatium * 3;
    style.margin = spatium * 3;
    style.staffY = spatium * 4;
    style.preClefSpacing = spatium * 3/5;
    style.systemSpacing = spatium * 8;
    style.screenBeatThreshold = width - spatium * 5;

    int availableHeight = height - style.staffY - spatium * 3;
    style.numberOfLines = std::max(1, static_cast<int>(availableHeight / style.systemSpacing) + 1);
    style.staveWidgetHeight = style.staffY + style.systemSpacing * (style.numberOfLines - 1) + spatium * 3;

    style.lelandFontSize = spatium * 3;

    style.noteGap = spatium * 0.5;
    style.barGap = spatium * 1;
    style.barLineInset = spatium * 0.4;
    style.dotRadius = spatium * 0.2;
    style.stubLength = spatium * 1;
    style.afterBarLineGap = spatium * 0.3;

    style.stemLength = spatium * 2.5;
    style.beamThickness = spatium * 0.5;
    style.stemThickness = spatium * 0.12;

    style.endInset = spatium * 0;
    style.minShoulderH = spatium * 0.9;
    style.maxShoulderH = spatium * 2.0;
    style.heightRatio = 0.20;
    style.midThickness = spatium * 0.18;
    style.baseGap = spatium * 0.35;
    return style;
}