#pragma once

#include <QWidget>
#include <QFont>
#include <QFontMetrics>
#include "layout.h"

class NoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteWidget(QWidget *parent = nullptr);

    void setNote(int staffPosition);
    void setStaveLayout(const StaveLayout& layout);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    StaveLayout style;
    QFont lelandFont;
    QFontMetrics lelandMetrics;
    int notePosition = 0;
};