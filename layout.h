#pragma once

struct StaveLayout
{
    int fontSize;
    int margin;
    int staffY;
    int staffSpacing;
    int preClefSpacing;
    int systemSpacing;
    int screenBeatThreshold;
};

StaveLayout DefineLayoutConstants(int width, int height, int fontSize);