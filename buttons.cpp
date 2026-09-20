#include "pch/pch.h"
#include "buttons.h"
 
#include <QHBoxLayout>
#include <QPushButton>
 
Buttons::Buttons(QWidget *parent)
    : QWidget(parent)
{

    setAutoFillBackground(true);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor("#FF0000"));
    setPalette(palette);

    playPauseButton = new QPushButton("Pause", this);
    playPauseButton->setMinimumWidth(120);
    playPauseButton->setCursor(Qt::PointingHandCursor);
    playPauseButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #A0A0A0;"
        "    color: black;"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 8px 28px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        /*"QPushButton:hover   { background-color: #B4B4B4; }"
        "QPushButton:pressed { background-color: #8C8C8C; }"&*/
    );
 
    // stretch on both sides keeps the button centred
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->addStretch();
    layout->addWidget(playPauseButton);
    layout->addStretch();
 
    // stay as tall as the button so the staves dont overlap
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(80);
 
    connect(playPauseButton, &QPushButton::clicked, this, [this]()
    {
        paused = !paused;
        playPauseButton->setText(paused ? "Play" : "Pause");
        emit pauseToggled(paused);
    });
}