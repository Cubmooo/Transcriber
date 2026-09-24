#pragma once

#include "pch/pch.h"
#include "note.h"

class StaveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StaveWidget(QWidget *parent = nullptr);
    void setNote(std::vector<std::pair<int, double>> BPMTimeList);
    void zoomBy(double factor);
    void setZoom(double value);
    void scrollBy(int lines);
    void resetScroll();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    NoteWidget *noteWidget;
    QFont lelandFont;
    QFontMetrics lelandMetrics;
    StaveLayout style;
    double zoom = 0.5;
    int scrollOffset = 0;
};