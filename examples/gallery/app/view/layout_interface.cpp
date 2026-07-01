#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

QWidget *GalleryWindow::createLayoutPage()
{
    auto *page = new GalleryInterface(navTx("Layout"), QStringLiteral("qfluentwidgets.components.layout"), this);
    page->setObjectName(QStringLiteral("layoutInterface"));
    const QString flowLayoutSource = exampleSourceUrl("layout/flow_layout");

    const QStringList texts = {tx("LayoutInterface", "Star Platinum"),
                               tx("LayoutInterface", "Hierophant Green"),
                               tx("LayoutInterface", "Silver Chariot"),
                               tx("LayoutInterface", "Crazy diamond"),
                               tx("LayoutInterface", "Heaven's Door"),
                               tx("LayoutInterface", "Killer Queen"),
                               tx("LayoutInterface", "Gold Experience"),
                               tx("LayoutInterface", "Sticky Fingers"),
                               tx("LayoutInterface", "Sex Pistols"),
                               tx("LayoutInterface", "Dirty Deeds Done Dirt Cheap")};

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

    page->addExampleCard(tx("LayoutInterface", "Flow layout without animation"), createFlowWidget(false), flowLayoutSource, 1);
    page->addExampleCard(tx("LayoutInterface", "Flow layout with animation"), createFlowWidget(true), flowLayoutSource, 1);

    return page;
}
