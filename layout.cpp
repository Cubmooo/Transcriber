#include "layout.h"

StaveLayout DefineLayoutConstants(int height, int width, double zoom = 1.0)
{
    StaveLayout  style;

    style.spatium = 25 * zoom;
    int spatium = style.spatium;

    style.fontSize = spatium * 3;
    style.margin = spatium * 3;
    style.staffY = height * 1/5;
    style.preClefSpacing = spatium * 3/5;
    style.systemSpacing = spatium * 8;
    style.screenBeatThreshold = width - spatium * 5;
    style.numberOfLines = 3;
    style.staveWidgetHeight = style.systemSpacing * style.numberOfLines + spatium * 3;

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


    // tie layout constants
    style.endInset = spatium * 0;  // pull endpoints in from the notehead edge
    style.minShoulderH = spatium * 0.9; // minimum arch height
    style.maxShoulderH = spatium * 2.0; // cap arch height for long ties
    style.heightRatio = 0.20;           // arch height as a fraction of tie length
    style.midThickness = spatium * 0.18; // thickness at the fattest point
    style.baseGap = spatium * 0.35; // gap between notehead and tie
    return style;
}