#include "pch/pch.h"
#include "buttons.h"
 
#include <QHBoxLayout>
#include <QPushButton>

QPushButton *Buttons::makeButton(const QString &text, int width)
{
    auto *button = new QPushButton(text, this);
    button->setMinimumWidth(width);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void Buttons::setBPM(double bpm)
{
    bpmButton->setText(QString("%1 BPM").arg(qRound(bpm)));
}
 
Buttons::Buttons(QWidget *parent)
    : QWidget(parent)
{

    setAutoFillBackground(true);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor("#2596BE"));
    setPalette(palette);

    setStyleSheet(
        "QPushButton {"
        "    background-color: #A0A0A0;"
        "    color: black;"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 8px 28px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
    );

    playPauseButton = makeButton("Pause", 120);
    clearButton     = makeButton("Clear", 120);
    bpmButton     = makeButton("? BPM", 150);
    bpmUpButton   = makeButton(QString(QChar(0x25B2)), 60);
    bpmDownButton = makeButton(QString(QChar(0x25BC)), 60);

    bpmButton->setFocusPolicy(Qt::NoFocus);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 3, 0, 5);
    layout->setSpacing(12);
    layout->addStretch();
    layout->addWidget(playPauseButton);
    layout->addWidget(clearButton);
    layout->addWidget(bpmButton);
    layout->addWidget(bpmUpButton);
    layout->addWidget(bpmDownButton);
    layout->addStretch();

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(80);
 
    connect(playPauseButton, &QPushButton::clicked, this, [this]()
    {
        paused = !paused;
        playPauseButton->setText(paused ? "Play" : "Pause");
        emit pauseToggled(paused);
    });

    connect(clearButton, &QPushButton::clicked, this, &Buttons::clearRequested);

    connect(bpmUpButton,   &QPushButton::clicked, this, [this]() { emit bpmScaleRequested(2.0); });
    connect(bpmDownButton, &QPushButton::clicked, this, [this]() { emit bpmScaleRequested(0.5); });
}
