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
 
Buttons::Buttons(QWidget *parent)
    : QWidget(parent)
{

    setAutoFillBackground(true);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor("#ff0000"));
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

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 3, 0, 5);
    layout->setSpacing(12);
    layout->addStretch();
    layout->addWidget(playPauseButton);
    layout->addWidget(clearButton);
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
}