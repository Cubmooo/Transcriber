#include "layout.h"

StaveLayout DefineLayoutConstants(int height, int width, int fontSize)
{
    StaveLayout  style;

    style.spatium = 35;
    int spatium = style.spatium;

    style.fontSize = spatium * 3;
    style.margin = spatium * 3;
    style.staffY = height * 3/5;
    style.staffSpacing = spatium;
    style.preClefSpacing = spatium * 3/5;
    style.systemSpacing = spatium * 3;
    style.screenBeatThreshold = width - spatium * 5;


    // tie layout constants
    style.endInset     = 0.20 * spatium; // pull endpoints in from the notehead edge
    style.minShoulderH = 0.9  * spatium; // minimum arch height
    style.maxShoulderH = 2.0  * spatium; // cap arch height for long ties
    style.heightRatio  = 0.20;           // arch height as a fraction of tie length
    style.midThickness = 0.18 * spatium; // thickness at the fattest point
    style.baseGap      = 0.35 * spatium; // gap between notehead and tie
    return style;
}