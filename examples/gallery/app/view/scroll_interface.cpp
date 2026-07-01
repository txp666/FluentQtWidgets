#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

namespace {
void addCardToolTip(QWidget *exampleCard, const QString &text)
{
    auto *innerCard = exampleCard->findChild<QFrame *>(QStringLiteral("card"));
    if (innerCard) {
        innerCard->installEventFilter(new ToolTipFilter(innerCard, 500));
        innerCard->setToolTip(text);
        innerCard->setToolTipDuration(2000);
    }
}

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

QWidget *GalleryWindow::createScrollPage()
{
    auto *page = new GalleryInterface(navTx("Scrolling"), QStringLiteral("qfluentwidgets.components.widgets"),
                                      this);
    page->setObjectName(QStringLiteral("scrollInterface"));
    const QString scrollSource = exampleSourceUrl("scroll/scroll_area");
    const QString pipsSource = exampleSourceUrl("scroll/pips_pager");

    auto *scrollArea = new ScrollArea;
    auto *scrollLabel = new ImageLabel;
    scrollLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta2.jpg"));
    scrollLabel->scaledToWidth(775);
    scrollLabel->setBorderRadius(8);
    scrollArea->setWidget(scrollLabel);
    scrollArea->setFixedSize(775, 430);
    scrollArea->horizontalScrollBar()->setValue(0);
    auto *card1 = page->addExampleCard(tx("ScrollInterface", "Smooth scroll area"), scrollArea, scrollSource);
    addCardToolTip(card1, tx("ScrollInterface", "Chitanda Eru is too hot 🥵"));

    auto *smoothScrollArea = new SmoothScrollArea;
    auto *smoothLabel = new ImageLabel;
    smoothLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta3.jpg"));
    smoothLabel->setBorderRadius(8);
    smoothScrollArea->setWidget(smoothLabel);
    smoothScrollArea->setFixedSize(660, 540);
    auto *card2 = page->addExampleCard(tx("ScrollInterface", "Smooth scroll area implemented by animation"), smoothScrollArea,
                         scrollSource);
    addCardToolTip(card2, tx("ScrollInterface", "Chitanda Eru is so hot 🥵🥵"));

    auto *singleDirectionArea = new SingleDirectionScrollArea(Qt::Horizontal);
    auto *singleDirectionLabel = new ImageLabel;
    singleDirectionLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta4.jpg"));
    singleDirectionLabel->setBorderRadius(8);
    singleDirectionArea->setWidget(singleDirectionLabel);
    singleDirectionArea->setFixedSize(660, 498);
    auto *card3 = page->addExampleCard(tx("ScrollInterface", "Single direction scroll scroll area"), singleDirectionArea, scrollSource);
    addCardToolTip(card3, tx("ScrollInterface", "Chitanda Eru is so hot 🥵🥵🥵"));

    auto *horizontalPips = new HorizontalPipsPager;
    horizontalPips->setPageNumber(15);
    horizontalPips->setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
    horizontalPips->setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
    auto *card4 = page->addExampleCard(tx("ScrollInterface", "Pips pager"), horizontalPips, pipsSource);
    setCardTopMargins(card4, QMargins(12, 20, 12, 20));

    return page;
}
