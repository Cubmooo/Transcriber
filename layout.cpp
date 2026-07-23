#include "layout.h"

StaveLayout DefineLayoutConstants(int height, int width, int fontSize)
{
    StaveLayout  style;

    style.fontSize = 100;

    style.margin = 100;
    style.staffY = height * 3/5;
    style.staffSpacing = fontSize/3;
    style.preClefSpacing = fontSize/5;
    return style;
}