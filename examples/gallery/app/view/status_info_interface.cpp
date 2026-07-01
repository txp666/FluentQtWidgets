#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

namespace {

void setCardTopMargins(QWidget *exampleCard, const QMargins &margins)
{
    auto *innerCard = exampleCard ? exampleCard->findChild<QFrame *>(QStringLiteral("card")) : nullptr;
    auto *cardLayout = innerCard ? qobject_cast<QVBoxLayout *>(innerCard->layout()) : nullptr;
    QLayoutItem *topItem = cardLayout ? cardLayout->itemAt(0) : nullptr;
    if (topItem && topItem->layout()) {
        topItem->layout()->setContentsMargins(margins);
    }
}

} // namespace

QWidget *GalleryWindow::createStatusInfoPage()
{
    auto *page = new GalleryInterface(navTx("Status & info"),
                                      QStringLiteral("qfluentwidgets.components.widgets"), this);
    page->setObjectName(QStringLiteral("statusInfoInterface"));
    const QString stateToolTipSource = exampleSourceUrl("status_info/state_tool_tip");
    const QString toolTipSource = exampleSourceUrl("status_info/tool_tip");
    const QString infoBarSource = exampleSourceUrl("status_info/info_bar");
    const QString progressBarSource = exampleSourceUrl("status_info/progress_bar");
    const QString progressRingSource = exampleSourceUrl("status_info/progress_ring");

    // -- StateToolTip --
    auto *stateBtn = new PushButton(tx("StatusInfoInterface", "Show StateToolTip"));
    QPointer<StateToolTip> stateTip;
    connect(stateBtn, &QPushButton::clicked, page, [stateBtn, stateTip]() mutable {
        if (stateTip) {
            stateTip->setContent(tx("StatusInfoInterface", "The model training is complete!")
                                 + QString::fromUtf8(" \xf0\x9f\x98\x85"));
            stateTip->setState(true);
            stateBtn->setText(tx("StatusInfoInterface", "Show StateToolTip"));
            stateTip.clear();
        } else {
            auto *tip = StateToolTip::showIn(stateBtn->window(), tx("StatusInfoInterface", "Training model"),
                                             tx("StatusInfoInterface", "Please wait patiently"));
            stateTip = tip;
            stateBtn->setText(tx("StatusInfoInterface", "Hide StateToolTip"));
        }
    });
    page->addExampleCard(tx("StatusInfoInterface", "State tool tip"), stateBtn, stateToolTipSource);

    // -- ToolTip (with ToolTipFilter) --
    {
        auto *tooltipBtn = new PushButton(tx("StatusInfoInterface", "Button with a simple ToolTip"));
        tooltipBtn->setToolTip(tx("StatusInfoInterface", "Simple ToolTip"));
        tooltipBtn->installEventFilter(new ToolTipFilter(tooltipBtn));
        page->addExampleCard(tx("StatusInfoInterface", "A button with a simple ToolTip"), tooltipBtn, toolTipSource);
    }

    // -- Label with ToolTip --
    {
        auto *tooltipLabel = new PixmapLabel(page);
        tooltipLabel->setPixmap(QPixmap(QStringLiteral(":/gallery/images/kunkun.png"))
                                    .scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        tooltipLabel->installEventFilter(new ToolTipFilter(tooltipLabel, 500));
        tooltipLabel->setToolTip(tx("StatusInfoInterface", "Label with a ToolTip"));
        tooltipLabel->setToolTipDuration(2000);
        tooltipLabel->setFixedSize(160, 160);
        page->addExampleCard(tx("StatusInfoInterface", "A label with a ToolTip"), tooltipLabel, toolTipSource);
    }

    // -- InfoBadge --
    auto *badgeWidget = new QWidget(page);
    auto *badgeLayout = new QHBoxLayout(badgeWidget);
    badgeLayout->setContentsMargins(0, 10, 0, 10);
    badgeLayout->setSpacing(20);
    badgeLayout->addWidget(InfoBadge::info(1));
    badgeLayout->addWidget(InfoBadge::success(10));
    badgeLayout->addWidget(InfoBadge::attention(100));
    badgeLayout->addWidget(InfoBadge::warning(1000));
    badgeLayout->addWidget(InfoBadge::error(10000));
    badgeLayout->addWidget(InfoBadge::custom(QStringLiteral("1w+"), QColor(QStringLiteral("#005fb8")),
                                             QColor(QStringLiteral("#60cdff"))));
    page->addExampleCard(tx("StatusInfoInterface", "InfoBadge in different styles"), badgeWidget, infoBarSource);

    // -- Short InfoBar --
    auto *infoBarShort = InfoBar::success(
        tx("StatusInfoInterface", "Success"),
        tx("StatusInfoInterface", "The Anthem of man is the Anthem of courage."), Qt::Horizontal, true,
        -1, InfoBarPosition::None, page);
    page->addExampleCard(tx("StatusInfoInterface", "A closable InfoBar"), infoBarShort, infoBarSource);

    // -- Long InfoBar --
    auto *infoBarLong = InfoBar::warning(
        tx("StatusInfoInterface", "Warning"),
        tx("StatusInfoInterface", "My name is kira yoshikake, 33 years old. Living in the villa area northeast of duwangting, unmarried. I work in Guiyou chain store. Every day I have to work overtime until 8 p.m. to go home. I don't smoke. The wine is only for a taste. Sleep at 11 p.m. for 8 hours a day. Before I go to bed, I must drink a cup of warm milk, then do 20 minutes of soft exercise, get on the bed, and immediately fall asleep. Never leave fatigue and stress until the next day. Doctors say I'm normal."),
        Qt::Vertical, true, -1, InfoBarPosition::None, page);
    page->addExampleCard(tx("StatusInfoInterface", "A closable InfoBar with long message"), infoBarLong, infoBarSource);

    // -- InfoBar with custom icon and widget --
    {
        auto *customInfoBar = new InfoBar(icon(FluentIcon::GitHub),
            tx("StatusInfoInterface", "GitHub"),
            tx("StatusInfoInterface", "When you look long into an abyss, the abyss looks into you."),
            Qt::Horizontal, true, -1, InfoBarPosition::None, page);
        customInfoBar->addWidget(new PushButton(tx("StatusInfoInterface", "Action")));
        customInfoBar->setCustomBackgroundColor(QColor(QStringLiteral("white")), QColor(QStringLiteral("#2a2a2a")));
        page->addExampleCard(tx("StatusInfoInterface", "An InfoBar with custom icon, background color and widget."), customInfoBar, infoBarSource);
    }

    // -- InfoBar different positions --
    auto *infoBarPosWidget = new QWidget(page);
    auto *posLayout = new QHBoxLayout(infoBarPosWidget);
    posLayout->setContentsMargins(0, 0, 0, 0);
    posLayout->setSpacing(15);
    auto makeInfoBarBtn = [page](const QString &text, InfoBarPosition pos, InfoBarSeverity severity) {
        auto *btn = new PushButton(text);
        connect(btn, &QPushButton::clicked, page, [page, pos, severity]() {
            switch (pos) {
            case InfoBarPosition::TopRight:
                InfoBar::info(tx("StatusInfoInterface", "Lesson 3"),
                              tx("StatusInfoInterface", "Believe in the spin, just keep believing!"), Qt::Horizontal,
                              true, 2000, pos, page);
                break;
            case InfoBarPosition::Top:
                InfoBar::success(tx("StatusInfoInterface", "Lesson 4"),
                                 tx("StatusInfoInterface", "With respect, let's advance towards a new stage of the spin."),
                                 Qt::Horizontal, true, 2000, pos, page);
                break;
            case InfoBarPosition::TopLeft:
                InfoBar::warning(tx("StatusInfoInterface", "Lesson 5"),
                                 tx("StatusInfoInterface", "The shortest shortcut is to take a detour."), Qt::Horizontal,
                                 false, 2000, pos, page);
                break;
            case InfoBarPosition::BottomRight:
                InfoBar::error(tx("StatusInfoInterface", "No Internet"),
                               tx("StatusInfoInterface", "An error message which won't disappear automatically."),
                               Qt::Horizontal, true, -1, pos, page);
                break;
            case InfoBarPosition::Bottom:
                InfoBar::success(tx("StatusInfoInterface", "Lesson 1"),
                                 tx("StatusInfoInterface", "Don't have any strange expectations of me."),
                                 Qt::Horizontal, true, 2000, pos, page);
                break;
            case InfoBarPosition::BottomLeft:
                InfoBar::warning(tx("StatusInfoInterface", "Lesson 2"),
                                 tx("StatusInfoInterface", "Don't let your muscles notice."), Qt::Horizontal,
                                 true, 1500, pos, page);
                break;
            default:
                InfoBar::newInfoBar(severity, QString(), QString(), Qt::Horizontal, true, 2000, pos, page);
                break;
            }
        });
        return btn;
    };
    posLayout->addWidget(
        makeInfoBarBtn(tx("StatusInfoInterface", "Top right"), InfoBarPosition::TopRight, InfoBarSeverity::Info));
    posLayout->addWidget(makeInfoBarBtn(tx("StatusInfoInterface", "Top"), InfoBarPosition::Top, InfoBarSeverity::Success));
    posLayout->addWidget(
        makeInfoBarBtn(tx("StatusInfoInterface", "Top left"), InfoBarPosition::TopLeft, InfoBarSeverity::Warning));
    posLayout->addWidget(
        makeInfoBarBtn(tx("StatusInfoInterface", "Bottom right"), InfoBarPosition::BottomRight, InfoBarSeverity::Error));
    posLayout->addWidget(
        makeInfoBarBtn(tx("StatusInfoInterface", "Bottom"), InfoBarPosition::Bottom, InfoBarSeverity::Success));
    posLayout->addWidget(
        makeInfoBarBtn(tx("StatusInfoInterface", "Bottom left"), InfoBarPosition::BottomLeft, InfoBarSeverity::Warning));
    posLayout->addStretch();
    page->addExampleCard(tx("StatusInfoInterface", "InfoBar with different pop-up locations"), infoBarPosWidget, infoBarSource);

    // -- Indeterminate progress bar --
    auto *indeterminateBar = new IndeterminateProgressBar(page);
    indeterminateBar->setFixedWidth(200);
    auto *indeterminateBarCard = page->addExampleCard(tx("StatusInfoInterface", "An indeterminate progress bar"),
                                                      indeterminateBar, progressBarSource);
    setCardTopMargins(indeterminateBarCard, QMargins(12, 24, 12, 24));

    // -- Determinate progress bar --
    {
        auto *barRow = new QWidget(page);
        auto *barRowLayout = new QHBoxLayout(barRow);
        barRowLayout->setContentsMargins(0, 0, 0, 0);
        auto *bar = new ProgressBar(page);
        bar->setFixedWidth(200);
        auto *barSpin = new SpinBox(page);
        barSpin->setRange(0, 100);
        barSpin->setValue(0);
        barRowLayout->addWidget(bar);
        barRowLayout->addSpacing(50);
        barRowLayout->addWidget(new QLabel(tx("StatusInfoInterface", "Progress")));
        barRowLayout->addSpacing(5);
        barRowLayout->addWidget(barSpin);
        connect(barSpin, QOverload<int>::of(&SpinBox::valueChanged), bar, &ProgressBar::setValue);
        page->addExampleCard(tx("StatusInfoInterface", "An determinate progress bar"), barRow, progressBarSource);
    }

    // -- Indeterminate progress ring --
    auto *indeterminateRing = new IndeterminateProgressRing(page);
    indeterminateRing->setFixedSize(70, 70);
    page->addExampleCard(tx("StatusInfoInterface", "An indeterminate progress ring"), indeterminateRing,
                         progressRingSource);

    // -- Determinate progress ring --
    {
        auto *ringRow = new QWidget(page);
        auto *ringRowLayout = new QHBoxLayout(ringRow);
        ringRowLayout->setContentsMargins(0, 0, 0, 0);
        auto *progressRing = new ProgressRing(page);
        progressRing->setFixedSize(80, 80);
        progressRing->setTextVisible(true);
        auto *ringSpin = new SpinBox(page);
        ringSpin->setRange(0, 100);
        ringRowLayout->addWidget(progressRing);
        ringRowLayout->addSpacing(50);
        ringRowLayout->addWidget(new QLabel(tx("StatusInfoInterface", "Progress")));
        ringRowLayout->addSpacing(5);
        ringRowLayout->addWidget(ringSpin);
        connect(ringSpin, QOverload<int>::of(&SpinBox::valueChanged), progressRing, &ProgressRing::setValue);
        ringSpin->setValue(0);
        page->addExampleCard(tx("StatusInfoInterface", "An determinate progress ring"), ringRow, progressRingSource);
    }

    return page;
}
