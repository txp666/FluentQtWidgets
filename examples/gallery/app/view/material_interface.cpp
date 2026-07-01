#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/ConfigItem.h>

using namespace FluentQt;

QWidget *GalleryWindow::createMaterialPage()
{
    auto *page = new GalleryInterface(navTx("Material"), QStringLiteral("qfluentwidgets.components.widgets"), this);
    page->setObjectName(QStringLiteral("materialInterface"));
    const QString acrylicLabelSource = exampleSourceUrl("material/acrylic_label");
    const QString imagePath = QStringLiteral(":/gallery/images/chidanta.jpg");

    auto *label = new AcrylicLabel(FluentConfig::instance()->acrylicBlurRadius(), QColor(105, 114, 168, 102), page);
    label->setImagePath(imagePath);
    label->setMinimumSize(197, 145);
    label->setMaximumSize(787, 579);
    connect(FluentConfig::instance(), &FluentConfig::acrylicBlurRadiusChanged, label, [label, imagePath](int radius) {
        label->setBlurRadius(radius);
        label->setImagePath(imagePath);
    });
    page->addExampleCard(tx("MaterialInterface", "Acrylic label"), label, acrylicLabelSource, 1);

    return page;
}
