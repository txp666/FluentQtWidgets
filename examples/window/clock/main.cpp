#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtGui/QDesktopServices>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

void addShadow(QWidget *widget)
{
    auto *effect = new QGraphicsDropShadowEffect(widget);
    effect->setBlurRadius(10);
    effect->setOffset(0, 0);
    effect->setColor(QColor(0, 0, 0, 15));
    widget->setGraphicsEffect(effect);
}

IconWidget *imageIcon(const QString &path, const QSize &size, QWidget *parent)
{
    auto *iconWidget = new IconWidget(QIcon(path), parent);
    iconWidget->setFixedSize(size);
    iconWidget->setIconSize(size);
    return iconWidget;
}

class FocusInterface : public QWidget
{
  public:
    explicit FocusInterface(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName(QStringLiteral("FocusInterface"));
        auto *grid = new QGridLayout(this);
        grid->setContentsMargins(20, 40, 20, 20);
        grid->setSpacing(12);

        grid->addWidget(createFocusCard(), 0, 0);
        grid->addWidget(createProgressCard(), 0, 1);
        grid->addWidget(createTaskCard(), 1, 0, 1, 2);
        grid->setColumnStretch(0, 1);
        grid->setColumnStretch(1, 1);
    }

  private:
    CardWidget *createFocusCard()
    {
        auto *card = new CardWidget(this);
        card->setMinimumSize(380, 410);
        card->setMaximumHeight(410);
        addShadow(card);

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 14, 14, 20);
        layout->setSpacing(10);

        auto *header = new QHBoxLayout;
        header->addWidget(imageIcon(QStringLiteral(":/window/clock/resource/images/alarms.png"), QSize(20, 20), card));
        header->addWidget(new StrongBodyLabel(tr("Focus session"), card));
        header->addStretch(1);
        header->addWidget(new TransparentToolButton(icon(FluentIcon::Pin), card));
        header->addWidget(new TransparentToolButton(icon(FluentIcon::More), card));
        layout->addLayout(header);

        layout->addStretch(1);
        auto *title = new SubtitleLabel(tr("Ready to focus"), card);
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        auto *hint = new BodyLabel(tr("Start a focus session and keep your current task visible."), card);
        hint->setAlignment(Qt::AlignCenter);
        hint->setWordWrap(true);
        hint->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));
        layout->addWidget(hint);

        auto *picker = new TimePicker(card);
        picker->setSecondVisible(true);
        layout->addWidget(picker, 0, Qt::AlignHCenter);
        layout->addWidget(new BodyLabel(tr("Focus for 45 minutes, then take a break."), card), 0, Qt::AlignHCenter);
        layout->addWidget(new CheckBox(tr("Skip breaks"), card), 0, Qt::AlignHCenter);
        auto *startButton = new PrimaryPushButton(icon(FluentIcon::PowerButton), tr("Start focus"), card);
        layout->addWidget(startButton, 0, Qt::AlignHCenter);
        layout->addStretch(1);
        return card;
    }

    CardWidget *createProgressCard()
    {
        auto *card = new CardWidget(this);
        card->setMinimumSize(380, 410);
        card->setMaximumHeight(410);
        addShadow(card);

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 14, 14, 20);

        auto *header = new QHBoxLayout;
        header->addWidget(imageIcon(QStringLiteral(":/window/clock/resource/images/tips.png"), QSize(18, 18), card));
        header->addWidget(new StrongBodyLabel(tr("Daily progress"), card));
        header->addStretch(1);
        header->addWidget(new TransparentToolButton(icon(FluentIcon::Edit), card));
        layout->addLayout(header);

        auto *body = new QHBoxLayout;
        auto *left = new QVBoxLayout;
        left->addStretch(1);
        left->addWidget(new BodyLabel(tr("Yesterday"), card), 0, Qt::AlignHCenter);
        left->addWidget(new LargeTitleLabel(QStringLiteral("2"), card), 0, Qt::AlignHCenter);
        left->addWidget(new BodyLabel(tr("hours"), card), 0, Qt::AlignHCenter);
        left->addStretch(1);

        auto *center = new QVBoxLayout;
        center->addStretch(1);
        auto *target = new SubtitleLabel(tr("Daily target"), card);
        target->setAlignment(Qt::AlignCenter);
        center->addWidget(target);
        auto *ring = new ProgressRing(card, false);
        ring->setRange(0, 24);
        ring->setValue(10);
        ring->setVal(10);
        ring->setTextVisible(true);
        ring->setStrokeWidth(15);
        ring->setMinimumSize(170, 170);
        ring->setMaximumSize(220, 220);
        center->addWidget(ring, 1, Qt::AlignCenter);
        center->addWidget(new BodyLabel(tr("Finished at 18:00"), card), 0, Qt::AlignHCenter);
        center->addStretch(1);

        auto *right = new QVBoxLayout;
        right->addStretch(1);
        right->addWidget(new BodyLabel(tr("Streak"), card), 0, Qt::AlignHCenter);
        right->addWidget(new LargeTitleLabel(QStringLiteral("10"), card), 0, Qt::AlignHCenter);
        right->addWidget(new BodyLabel(tr("days"), card), 0, Qt::AlignHCenter);
        right->addStretch(1);

        body->addLayout(left, 1);
        body->addLayout(center, 2);
        body->addLayout(right, 1);
        layout->addLayout(body, 1);
        return card;
    }

    CardWidget *createTaskCard()
    {
        auto *card = new CardWidget(this);
        card->setMinimumHeight(250);
        addShadow(card);

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(18, 14, 18, 18);
        layout->setSpacing(12);

        auto *header = new QHBoxLayout;
        header->addWidget(imageIcon(QStringLiteral(":/window/clock/resource/images/todo.png"), QSize(18, 18), card));
        header->addWidget(new StrongBodyLabel(tr("Tasks"), card));
        header->addStretch(1);
        header->addWidget(new TransparentToolButton(icon(FluentIcon::Add), card));
        header->addWidget(new TransparentToolButton(icon(FluentIcon::More), card));
        layout->addLayout(header);

        addTaskRow(layout, tr("Review Fluent window examples"), InfoBarSeverity::Success);
        addTaskRow(layout, tr("Align clock UI with Python version"), InfoBarSeverity::Warning);
        addTaskRow(layout, tr("Polish gallery settings cards"), InfoBarSeverity::Warning);
        layout->addStretch(1);
        return card;
    }

    void addTaskRow(QVBoxLayout *layout, const QString &text, InfoBarSeverity severity)
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        row->addWidget(new InfoBarIconWidget(severity, this));
        row->addWidget(new BodyLabel(text, this), 1);
        layout->addLayout(row);
    }
};

class StopWatchInterface : public QWidget
{
  public:
    explicit StopWatchInterface(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->addStretch(1);

        auto *timeLabel = new BodyLabel(QStringLiteral("00:00:00"), this);
        timeLabel->setPixelFontSize(100);
        timeLabel->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));
        layout->addWidget(timeLabel, 0, Qt::AlignHCenter);

        auto *labels = new QHBoxLayout;
        labels->addStretch(1);
        labels->addWidget(new TitleLabel(tr("Hours"), this));
        labels->addSpacing(60);
        labels->addWidget(new TitleLabel(tr("Minutes"), this));
        labels->addSpacing(90);
        labels->addWidget(new TitleLabel(tr("Seconds"), this));
        labels->addStretch(1);
        layout->addLayout(labels);
        layout->addSpacing(50);

        auto *buttons = new QHBoxLayout;
        buttons->setSpacing(24);
        buttons->addStretch(1);
        auto *start = new PillToolButton(icon(FluentIcon::PowerButton), this);
        auto *flag = new PillToolButton(icon(FluentIcon::Flag), this);
        auto *restart = new PillToolButton(icon(FluentIcon::Cancel), this);
        for (auto *button : {start, flag, restart}) {
            button->setMinimumSize(68, 68);
            button->setIconSize(QSize(21, 21));
            buttons->addWidget(button);
        }
        start->setChecked(true);
        flag->setEnabled(false);
        restart->setEnabled(false);
        buttons->addStretch(1);
        layout->addLayout(buttons);
        layout->addStretch(1);
    }
};

class ClockWindow : public SplitFluentWindow
{
  public:
    explicit ClockWindow(QWidget *parent = nullptr) : SplitFluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::Ringer));

        addSubInterface(new FocusInterface(this), icon(FluentIcon::Ringer), tr("Focus sessions"),
                        QStringLiteral("focusInterface"));
        addSubInterface(new StopWatchInterface(this), icon(FluentIcon::StopWatch), tr("Stopwatch"),
                        QStringLiteral("stopWatchInterface"));

        auto *avatar = new NavigationAvatarWidget(QStringLiteral("zhiyiYo"),
                                                  QStringLiteral(":/window/clock/resource/images/shoko.png"), this);
        navigationInterface()->addWidget(QStringLiteral("avatar"), avatar, NavigationItemPosition::Bottom);
        connect(avatar, &NavigationWidget::clicked, this, [this]() {
            MessageBox box(tr("Support the author"),
                           tr("This C++ demo keeps the Python example structure and points links to this repository."),
                           this);
            box.exec();
        });

        navigationInterface()->addItem(QStringLiteral("settingInterface"), icon(FluentIcon::Settings), tr("Settings"),
                                       NavigationItemPosition::Bottom);
        navigationInterface()->navigationPanel()->setExpandWidth(280);
        resize(900, 700);
    }
};

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    ClockWindow window;
    window.show();
    return app.exec();
}
