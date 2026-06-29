#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

QWidget *GalleryWindow::createLayoutPage()
{
    auto *page = new GalleryInterface(QStringLiteral("Layout"), QStringLiteral("FluentQtWidgets::Layout"),
                                      this);
    const QString flowLayoutSource = exampleSourceUrl("layout/flow_layout");
    const QString adaptiveFlowLayoutSource = exampleSourceUrl("layout/adaptive_flow_layout");

    const QStringList texts = {QStringLiteral("Star Platinum"),  QStringLiteral("Hierophant Green"),
                               QStringLiteral("Silver Chariot"), QStringLiteral("Crazy diamond"),
                               QStringLiteral("Heaven's Door"),  QStringLiteral("Killer Queen"),
                               QStringLiteral("Gold Experience"), QStringLiteral("Sticky Fingers"),
                               QStringLiteral("Sex Pistols"),    QStringLiteral("Dirty Deeds Done Dirt Cheap")};

    auto createFlowWidget = [texts](bool animation) {
        auto *widget = new QWidget;
        auto *flowLayout = new FlowLayout(widget, animation);
        flowLayout->setContentsMargins(0, 0, 0, 0);
        flowLayout->setVerticalSpacing(20);
        flowLayout->setHorizontalSpacing(10);
        for (const QString &text : texts) {
            flowLayout->addWidget(new PushButton(text, widget));
        }
        return widget;
    };

    page->addExampleCard(QStringLiteral("Flow layout without animation"), createFlowWidget(false), flowLayoutSource, 1);
    page->addExampleCard(QStringLiteral("Flow layout with animation"), createFlowWidget(true), adaptiveFlowLayoutSource,
                         1);

    return page;
}
