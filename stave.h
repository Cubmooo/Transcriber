#pragma once

#include "pch/pch.h"
#include "note.h"

class StaveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StaveWidget(QWidget *parent = nullptr);
    void setNote(std::vector<std::pair<int, double>> BPMTimeList);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    NoteWidget *noteWidget;
    QFont lelandFont;
    QFontMetrics lelandMetrics;
    StaveLayout style;
    int notePosition = 0;
};