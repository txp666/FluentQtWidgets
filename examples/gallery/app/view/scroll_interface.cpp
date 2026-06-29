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
    }
}
} // namespace

QWidget *GalleryWindow::createScrollPage()
{
    auto *page = new GalleryInterface(QStringLiteral("Scroll"), QStringLiteral("FluentQtWidgets::Widgets"),
                                      this);
    const QString scrollSource = exampleSourceUrl("scroll/scroll_area");
    const QString pipsSource = exampleSourceUrl("scroll/pips_pager");

    auto *scrollArea = new ScrollArea;
    auto *scrollLabel = new ImageLabel;
    scrollLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta2.jpg"));
    scrollLabel->scaledToWidth(775);
    scrollLabel->setBorderRadius(8);
    scrollArea->setWidgetResizable(false);
    scrollArea->setWidget(scrollLabel);
    scrollArea->setFixedSize(775, 430);
    scrollArea->horizontalScrollBar()->setValue(0);
    auto *card1 = page->addExampleCard(QStringLiteral("Smooth scroll area"), scrollArea, scrollSource);
    addCardToolTip(card1, QStringLiteral("Chitanda Eru is too hot 🥵"));

    auto *smoothScrollArea = new SmoothScrollArea;
    auto *smoothLabel = new ImageLabel;
    smoothLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta3.jpg"));
    smoothLabel->setBorderRadius(8);
    smoothScrollArea->setWidgetResizable(false);
    smoothScrollArea->setWidget(smoothLabel);
    smoothScrollArea->setFixedSize(660, 540);
    auto *card2 = page->addExampleCard(QStringLiteral("Smooth scroll area implemented by animation"), smoothScrollArea,
                         scrollSource);
    addCardToolTip(card2, QStringLiteral("Chitanda Eru is so hot 🥵🥵"));

    auto *singleDirectionArea = new SingleDirectionScrollArea(Qt::Horizontal);
    auto *singleDirectionLabel = new ImageLabel;
    singleDirectionLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta4.jpg"));
    singleDirectionLabel->setBorderRadius(8);
    singleDirectionArea->setWidgetResizable(false);
    singleDirectionArea->setWidget(singleDirectionLabel);
    singleDirectionArea->setFixedSize(660, 498);
    auto *card3 = page->addExampleCard(QStringLiteral("Single direction scroll scroll area"), singleDirectionArea, scrollSource);
    addCardToolTip(card3, QStringLiteral("Chitanda Eru is so hot 🥵🥵🥵"));

    auto *horizontalPips = new HorizontalPipsPager;
    horizontalPips->setPageNumber(15);
    horizontalPips->setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
    horizontalPips->setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
    page->addExampleCard(QStringLiteral("Pips pager"), horizontalPips, pipsSource);

    return page;
}
