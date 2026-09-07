#pragma once

struct StaveLayout
{
    int spatium;

    int fontSize;
    int margin;
    int staffY;
    int staffSpacing;
    int preClefSpacing;
    int systemSpacing;
    int screenBeatThreshold;

    double endInset;
    double minShoulderH;
    double maxShoulderH;
    double heightRatio;
    double midThickness;
    double baseGap;
};

StaveLayout DefineLayoutConstants(int width, int height, int fontSize);