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

QPushButton *Buttons::makeButton(const QString &content, int width, const QString &shortcut)
{
    auto *button = new QPushButton(this);
    button->setFixedWidth(width);
    button->setCursor(Qt::PointingHandCursor);

    if (content.startsWith(":/")){
        button->setIcon(QIcon(content));
        button->setIconSize(QSize(18, 18));
    }
    else{button->setText(content);}

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
            inputButton->setText("Input Device");
            emit inputDeviceSelected(id);
        });

        if (id == selected)
            inputButton->setText("Input Device: " + label + " ");
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
        html += QString("<span style=\"color:%1; font-size:24pt;\">|</span>").arg(colour);
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
        "QToolButton#inputButton::menu-indicator {"
        "   image: /images/downarrow.png;"
        "   subcontrol-position: right center;"
        "}"
        "QMenu#inputMenu {"
        "    background-color: #287DDF;"
        "    color: black;"
        "    border: 1px solid #1C5597;"
        "}"
        "QMenu#inputMenu::item { padding: 6px 20px; }"
        "QMenu#inputMenu::item:selected { background-color: #2971C3; }"
    );

    playPauseButton = makeButton("| |", 50, "Space");
    playPauseButton->setToolTip("Play/Pause (Space)");
    playPauseButton->setObjectName("symbolButton");
    playPauseButton->setStyleSheet(
        "QPushButton {"
        "    padding-top: 7px;"
        "    padding-bottom: 9px;"
        "}"
    );

    clearButton = makeButton(QString::fromUtf16(u"\u2715"), 50, "Backspace");
    clearButton->setToolTip("Clear Score (Backspace)");
    clearButton->setObjectName("symbolButton");

    QFont bpmFont("Leland");
    bpmFont.setPointSize(22);

    bpmButton = makeButton(QString(QChar(0xE1D5)) + " = ?", 70);
    bpmButton->setToolTip("Current Bpm");
    bpmButton->setFont(bpmFont);
    bpmButton->setStyleSheet(
        "QPushButton {"
        "    padding-top: 14px;"
        "    padding-bottom: 2px;"
        "}"
    );

    bpmUpButton = makeButton(":/images/upArrow.png", 28, "Up");
    bpmUpButton->setToolTip("Double Bpm (Up)");
    bpmDownButton = makeButton(":/images/downArrow.png", 28, "Down");
    bpmDownButton->setToolTip("Half Bpm (Down)");

    bpmButton->setFocusPolicy(Qt::NoFocus);
    bpmButton->setAttribute(Qt::WA_TransparentForMouseEvents);

    inputButton = new QToolButton(this);
    inputButton->setObjectName("inputButton");
    inputButton->setToolTip("Select Input Device");
    inputButton->setText("Input Device:");
    inputButton->setCursor(Qt::PointingHandCursor);
    inputButton->setPopupMode(QToolButton::InstantPopup);
    inputButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    inputButton->setMinimumWidth(200);

    inputMenu = new QMenu(inputButton);
    inputMenu->setObjectName("inputMenu");
    inputButton->setMenu(inputMenu);
    inputMenu->setStyleSheet(
        "QPushButton {"
        "    padding-top: 2px;"
        "    padding-bottom: 14px;"
        "}"
    );

    zoomOutButton = makeButton("-", 32);
    zoomOutButton->setToolTip("Zoom Out (Ctrl + Scrl)");
    zoomOutButton->setStyleSheet("QPushButton {font-size: 25px;}");
    zoomOutButton->setObjectName("symbolButton");

    zoomInButton = makeButton("+", 32);
    zoomInButton->setToolTip("Zoom In (Ctrl + Scrl)");
    zoomInButton->setStyleSheet("QPushButton {font-size: 25px;}");
    zoomInButton->setObjectName("symbolButton");

    viewToggleButton = makeButton("▢", 32, "v");
    viewToggleButton->setToolTip("Toggle View Mode (V)");
    viewToggleButton->setObjectName("symbolButton");

    pitchLabel = new QLabel("Pitch: ?", this);
    pitchLabel->setObjectName("pitchLabel");
    pitchLabel->setToolTip("Current Pitch");
    pitchLabel->setFixedWidth(80);
    pitchLabel->setFocusPolicy(Qt::NoFocus);

    pitchDial = new PitchDial(this);
    pitchDial->setColor(QColor("#FFFFFF"));

    volumeMeter = new QLabel(this);
    volumeMeter->setFixedWidth(110);
    volumeMeter->setToolTip("Current Volume");
    volumeMeter->setObjectName("volumeMeter");
    volumeMeter->setTextFormat(Qt::RichText);
    volumeMeter->setAlignment(Qt::AlignCenter);
    volumeMeter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setVolume(0.0); 

    auto *pitchLayout = new QHBoxLayout();
    pitchLayout->setContentsMargins(0, 0, 0, 0);
    pitchLayout->setSpacing(8);
    pitchLayout->addWidget(pitchLabel);
    pitchLayout->addWidget(pitchDial);

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

    constexpr int gapClearBpm    = 8;
    constexpr int gapBpmSpacing  = 6;
    constexpr int gapDownInput   = 12;
    constexpr int gapVolumePitch = 16;
    constexpr int gapZoom = 12;
    constexpr int gapPageType = 12;

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 3, 12, 5);
    layout->setSpacing(0);

    layout->addWidget(playPauseButton);
    layout->addWidget(clearButton);
    layout->addSpacing(gapClearBpm);
    layout->addWidget(makeSeparator());
    layout->addSpacing(gapClearBpm);

    layout->addWidget(bpmButton);
    layout->addWidget(bpmUpButton);
    layout->addWidget(bpmDownButton);
    layout->addSpacing(gapDownInput);
    layout->addWidget(makeSeparator());
    layout->addSpacing(gapDownInput);
    layout->addWidget(inputButton);

    layout->addStretch();

    layout->addWidget(viewToggleButton);
    layout->addSpacing(gapPageType);
    layout->addWidget(makeSeparator());
    layout->addSpacing(gapPageType);
    layout->addWidget(zoomInButton);
    layout->addWidget(zoomOutButton);
    layout->addSpacing(gapZoom);
    layout->addWidget(makeSeparator());
    layout->addSpacing(gapZoom);
    layout->addWidget(pitchContainer);
    layout->addSpacing(gapVolumePitch);
    layout->addWidget(makeSeparator());
    layout->addSpacing(gapVolumePitch);
    layout->addWidget(volumeMeter);    

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(80);
 
    connect(playPauseButton, &QPushButton::clicked, this, [this]()
    {
        paused = !paused;
        playPauseButton->setText(paused ? "▶" : "| |");
        playPauseButton->setFont(QFont("Impact"));
        emit pauseToggled(paused);
    });

    connect(clearButton, &QPushButton::clicked, this, &Buttons::clearRequested);

    connect(bpmUpButton,   &QPushButton::clicked, this, [this]() { emit bpmScaleRequested(2.0); });
    connect(bpmDownButton, &QPushButton::clicked, this, [this]() { emit bpmScaleRequested(0.5); });

    connect(zoomOutButton, &QPushButton::clicked, this, [this]() { emit zoomRequested(1.0 / 1.1); });
    connect(zoomInButton,  &QPushButton::clicked, this, [this]() { emit zoomRequested(1.1); });

    connect(viewToggleButton, &QPushButton::clicked, this, [this]() { emit viewToggleRequested(); });
}