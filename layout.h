#pragma once

struct StaveLayout {
    int fontSize;
    int margin;
    int staffY;
    int staffSpacing;
    int preClefSpacing;
};

StaveLayout DefineLayoutConstants(int width, int height, int fontSize);