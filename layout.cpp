#include "layout.h"

StaveLayout DefineLayoutConstants(int height, int width)
{
    StaveLayout  style;

    style.spatium = 25;
    int spatium = style.spatium;

    style.fontSize = spatium * 3;
    style.margin = spatium * 3;
    style.staffY = height * 1/5;
    style.preClefSpacing = spatium * 3/5;
    style.systemSpacing = spatium * 8;
    style.screenBeatThreshold = width - spatium * 5;
    style.staveWidgetHeight = style.systemSpacing * 3;
    style.numberOfLines = 5;

    style.lelandFontSize = spatium * 3;

    style.stemLength = spatium * 3.5;
    style.beamThickness = spatium * 0.5;
    style.stemThickness = spatium * 0.12;


    // tie layout constants
    style.endInset     = 0.20 * spatium; // pull endpoints in from the notehead edge
    style.minShoulderH = 0.9  * spatium; // minimum arch height
    style.maxShoulderH = 2.0  * spatium; // cap arch height for long ties
    style.heightRatio  = 0.20;           // arch height as a fraction of tie length
    style.midThickness = 0.18 * spatium; // thickness at the fattest point
    style.baseGap      = 0.35 * spatium; // gap between notehead and tie
    return style;
}