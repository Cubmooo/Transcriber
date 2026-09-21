#include "pch/pch.h"
#include "buttons.h"
 
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QShortcut>

QPushButton *Buttons::makeButton(const QString &text, int width, const QString &shortcut)
{
    auto *button = new QPushButton(text, this);
    button->setMinimumWidth(width);
    button->setCursor(Qt::PointingHandCursor);

    if (!shortcut.isEmpty())
    {
        auto *key = new QShortcut(QKeySequence(shortcut, QKeySequence::PortableText), this);
        key->setAutoRepeat(false);
        connect(key, &QShortcut::activated, button, &QAbstractButton::click);
    }
    return button;
}

void Buttons::setBPM(double bpm)
{
    bpmButton->setText(QString("%1 BPM").arg(qRound(bpm)));
}

void Buttons::setInputDevices(const std::vector<std::pair<int, std::string>> &devices, int selected)
{
    if (inputBox->count() != static_cast<int>(devices.size()))
    {
        inputBox->clear();
        for (const auto &device : devices)
            inputBox->addItem(QString::fromUtf8(device.second.c_str()), device.first);
    }

    const int row = inputBox->findData(selected);
    if (row >= 0 && row != inputBox->currentIndex())
        inputBox->setCurrentIndex(row);
}

void Buttons::setVolume(double rms)
{
    constexpr double FLOOR_DB = -54.0;
    constexpr double CEIL_DB  = -6.0;

    const double db = 20.0 * std::log10(std::max(rms, 1e-9));
    const int lit = qBound(0, qRound((db - FLOOR_DB) / (CEIL_DB - FLOOR_DB) * 10.0), 10);

    if (lit == litLines) return;
    litLines = lit;

    QString html = "Inputted volume &nbsp;";
    for (int i = 0; i < 10; ++i)
    {
        const char *colour = i >= lit ? "black" : i < 7 ? "#00C800" : i < 9 ? "#FFE000" : "#FF0000";
        html += QString("<span style=\"color:%1\">|</span>").arg(colour);
    }
    volumeMeter->setText(html);
}
 
Buttons::Buttons(QWidget *parent)
    : QWidget(parent)
{

    setAutoFillBackground(true);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor("#2596BE"));
    setPalette(palette);

    setStyleSheet(
        "QPushButton, QComboBox, QLabel#volumeMeter {"
        "    background-color: #A0A0A0;"
        "    color: black;"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 8px 28px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
    );

    playPauseButton = makeButton("Pause", 120, "Space");
    clearButton     = makeButton("Clear", 120, "Backspace");
    bpmButton     = makeButton("? BPM", 150);
    bpmUpButton   = makeButton(QString(QChar(0x25B2)), 60, "Up");
    bpmDownButton = makeButton(QString(QChar(0x25BC)), 60, "Down");
    pitchButton = makeButton("Pitch: ?", 280);
    pitchButton->setFocusPolicy(Qt::NoFocus);

    bpmButton->setFocusPolicy(Qt::NoFocus);

    inputBox = new QComboBox(this);
    inputBox->setPlaceholderText("selected microphone");
    inputBox->setMinimumWidth(200);
    inputBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    inputBox->setCursor(Qt::PointingHandCursor);

    volumeMeter = new QLabel(this);
    volumeMeter->setObjectName("volumeMeter");
    volumeMeter->setTextFormat(Qt::RichText);
    volumeMeter->setAlignment(Qt::AlignCenter);
    volumeMeter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setVolume(0.0); 

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 3, 0, 5);
    layout->setSpacing(12);
    layout->addStretch();
    layout->addWidget(playPauseButton);
    layout->addWidget(clearButton);
    layout->addWidget(bpmButton);
    layout->addWidget(bpmUpButton);
    layout->addWidget(bpmDownButton);
    layout->addWidget(inputBox);
    layout->addWidget(volumeMeter);
    layout->addWidget(pitchButton);
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


void Buttons::setPitch(double freq)
{
    QString text = "Pitch: ?";
    if (freq >= 20.0)
    {
        static const char *names[] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};

        const double exact = 57 + 12 * std::log2(freq / 440.0);
        const int note = qRound(exact);
        const int cents = qRound((exact - note) * 100.0);

        text = QString::asprintf("Pitch: %s%d  %+d cents", names[note % 12], note / 12, cents);
    }
    pitchButton->setText(text);
}