#pragma once

struct StaveLayout
{
    int spatium;

    int fontSize;
    int margin;
    int staffY;
    int preClefSpacing;
    int systemSpacing;
    int screenBeatThreshold;
    int staveWidgetHeight;
    int numberOfLines;

    int lelandFontSize;

    double stemLength;
    double beamThickness;
    double stemThickness;

    double noteGap;
    double barGap;
    double barLineInset;
    double dotRadius;
    double stubLength;
    double afterBarLineGap;

    double endInset;
    double minShoulderH;
    double maxShoulderH;
    double heightRatio;
    double midThickness;
    double baseGap;
};

StaveLayout DefineLayoutConstants(int width, int height);