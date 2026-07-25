#pragma once

#include <QWidget>
#include <QFont>
#include <QFontMetrics>
#include "note.h"
#include <vector>
#include <utility>

class StaveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StaveWidget(QWidget *parent = nullptr);
    void setNotes(std::vector<std::pair<int, int>> BPMTimeList);

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