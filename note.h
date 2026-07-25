#pragma once

#include <QWidget>
#include <QFont>
#include <QFontMetrics>
#include "layout.h"
#include <vector>

class NoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteWidget(QWidget *parent = nullptr);

    void setNotes(std::vector<std::pair<int, int>> BPMTimeList);
    void setStaveLayout(const StaveLayout& layout);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    StaveLayout style;
    QFont lelandFont;
    QFontMetrics lelandMetrics;
    std::vector<std::pair<int, int>> BPMTimeList;
};