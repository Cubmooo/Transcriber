#include "pch/pch.h"
#include "buttons.h"
#include "pitchdial.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QFrame>
#include <QLabel>
#include <QFont>

QPushButton *Buttons::makeButton(const QString &text, int width, const QString &shortcut)
{
    auto *button = new QPushButton(text, this);
    button->setFixedWidth(width);
    button->setCursor(Qt::PointingHandCursor);

    if (!shortcut.isEmpty())
    {
        auto *key = new QShortcut(QKeySequence(shortcut, QKeySequence::PortableText), this);
        key->setAutoRepeat(false);
        connect(key, &QShortcut::activated, button, [button]() { button->animateClick(); });
    }
    return button;
}

void Buttons::setBPM(double bpm)
{
    bpmButton->setText(QString(QChar(0xE1D5)) + " = " + QString::number(qRound(bpm)));
}

void Buttons::setInputDevices(const std::vector<std::pair<int, std::string>> &devices, int selected)
{
    inputMenu->clear();

    for (const auto &device : devices)
    {
        const int id = device.first;
        const QString label = QString::fromUtf8(device.second.c_str());

        auto *action = inputMenu->addAction(label);
        action->setCheckable(true);
        action->setChecked(id == selected);

        connect(action, &QAction::triggered, this, [this, id, label]()
        {
            inputButton->setText(label);
            emit inputDeviceSelected(id);
        });

        if (id == selected)
            inputButton->setText(label);
    }
}

void Buttons::setPitch(double freq)
{
    QString text = "Pitch: ?";
    double cents = 0.0;

    if (freq >= 20.0)
    {
        static const char *names[] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};

        const double exact = 57 + 12 * std::log2(freq / 440.0);
        const int note = qRound(exact);
        cents = (exact - note) * 100.0;

        text = QString::asprintf("%s%d  %+d", names[note % 12], note / 12, qRound(cents)) + QChar(0x00A2);
    }

    pitchLabel->setText(text);
    pitchDial->setCents(cents);
}

void Buttons::setVolume(double rms)
{
    constexpr double FLOOR_DB = -54.0;
    constexpr double CEIL_DB  = -6.0;

    const double db = 20.0 * std::log10(std::max(rms, 1e-9));
    const int lit = qBound(0, qRound((db - FLOOR_DB) / (CEIL_DB - FLOOR_DB) * 10.0), 10);

    if (lit == litLines) return;
    litLines = lit;

    QString html = "";
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
    palette.setColor(QPalette::Window, QColor("#287DDF"));
    setPalette(palette);

    setStyleSheet(
        "QPushButton, QComboBox, QLabel#volumeMeter, QLabel#pitchLabel, QToolButton#inputButton {"
        "    background-color: #287DDF;"
        "    color: black;"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 8px 0px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover, QComboBox:hover, QToolButton#inputButton:hover { background-color: #2971C3; }"
        "QPushButton:pressed { background-color: #1C5597; }"
        "QPushButton#symbolButton {"
        "    font-size: 22px;"
        "    font-family: 'Segoe UI Symbol', 'Noto Sans Symbols2', sans-serif;"
        "}"
        "QToolButton#inputButton {"
        "    background-color: transparent;"
        "    font-size: 20px;"
        "    padding: 8px 10px;"
        "}"
        "QToolButton#inputButton::menu-indicator { image: none; }"
        "QMenu#inputMenu {"
        "    background-color: #287DDF;"
        "    color: black;"
        "    border: 1px solid #1C5597;"
        "}"
        "QMenu#inputMenu::item { padding: 6px 20px; }"
        "QMenu#inputMenu::item:selected { background-color: #2971C3; }"
    );

    playPauseButton = makeButton(QString(QChar(0x2016)), 50, "Space");
    playPauseButton->setObjectName("symbolButton");

    clearButton = makeButton(QString::fromUtf16(u"\u2715"), 50, "Backspace");
    clearButton->setObjectName("symbolButton");

    QFont bpmFont("Leland");
    bpmFont.setPointSize(22);

    bpmButton = makeButton(QString(QChar(0xE1D5)) + " = ?", 70);
    bpmButton->setFont(bpmFont);

    bpmUpButton = makeButton(QString(QChar(0x25B2)), 40, "Up");
    bpmUpButton->setObjectName("symbolButton");
    bpmDownButton = makeButton(QString(QChar(0x25BC)), 40, "Down");
    bpmDownButton->setObjectName("symbolButton");

    bpmButton->setFocusPolicy(Qt::NoFocus);
    bpmButton->setAttribute(Qt::WA_TransparentForMouseEvents);

    inputButton = new QToolButton(this);
    inputButton->setObjectName("inputButton");
    inputButton->setText("selected microphone");
    inputButton->setCursor(Qt::PointingHandCursor);
    inputButton->setPopupMode(QToolButton::InstantPopup);
    inputButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    inputButton->setMinimumWidth(200);

    inputMenu = new QMenu(inputButton);
    inputMenu->setObjectName("inputMenu");
    inputButton->setMenu(inputMenu);

    volumeMeter = new QLabel(this);
    volumeMeter->setFixedWidth(80);
    volumeMeter->setObjectName("volumeMeter");
    volumeMeter->setTextFormat(Qt::RichText);
    volumeMeter->setAlignment(Qt::AlignCenter);
    volumeMeter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setVolume(0.0); 

    pitchDial = new PitchDial(this);
    pitchDial->setColor(QColor("#FFFFFF"));

    pitchLabel = new QLabel("Pitch: ?", this);
    pitchLabel->setObjectName("pitchLabel");
    pitchLabel->setFocusPolicy(Qt::NoFocus);

    auto *pitchLayout = new QHBoxLayout();
    pitchLayout->setContentsMargins(0, 0, 0, 0);
    pitchLayout->setSpacing(8);
    pitchLayout->addWidget(pitchDial);
    pitchLayout->addWidget(pitchLabel);

    auto *pitchContainer = new QWidget(this);
    pitchContainer->setLayout(pitchLayout);

    auto makeSeparator = [this]()
    {
        auto *line = new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Plain);
        line->setFixedWidth(1);
        line->setFixedHeight(25);
        line->setStyleSheet("background-color: #1C5597;");
        return line;
    };

    constexpr int kGapPlayClear   = 12;
    constexpr int kGapClearBpm    = 12;
    constexpr int kGapBpmSpacing  = 6;
    constexpr int kGapDownInput   = 12;
    constexpr int kGapVolumePitch = 12;

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 3, 12, 5);
    layout->setSpacing(0);

    layout->addWidget(playPauseButton);
    layout->addSpacing(kGapPlayClear);
    layout->addWidget(makeSeparator());
    layout->addSpacing(kGapPlayClear);

    layout->addWidget(clearButton);
    layout->addSpacing(kGapClearBpm);
    layout->addWidget(makeSeparator());
    layout->addSpacing(kGapClearBpm);

    layout->addWidget(bpmButton);
    layout->addSpacing(kGapBpmSpacing);
    layout->addWidget(bpmUpButton);
    layout->addSpacing(kGapBpmSpacing);
    layout->addWidget(bpmDownButton);
    layout->addSpacing(kGapDownInput);
    layout->addWidget(makeSeparator());
    layout->addSpacing(kGapDownInput);

    layout->addWidget(inputButton);
    layout->addStretch();
    layout->addWidget(volumeMeter);
    layout->addSpacing(kGapVolumePitch);
    layout->addWidget(makeSeparator());
    layout->addSpacing(kGapVolumePitch);
    layout->addWidget(pitchContainer);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(80);
 
    connect(playPauseButton, &QPushButton::clicked, this, [this]()
    {
        paused = !paused;
        playPauseButton->setText(QString(QChar(paused ? 0x25B6 : 0x2016)));
        emit pauseToggled(paused);
    });

    connect(clearButton, &QPushButton::clicked, this, &Buttons::clearRequested);

    connect(bpmUpButton,   &QPushButton::clicked, this, [this]() { emit bpmScaleRequested(2.0); });
    connect(bpmDownButton, &QPushButton::clicked, this, [this]() { emit bpmScaleRequested(0.5); });
}