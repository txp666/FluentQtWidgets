#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/ConfigItem.h>

using namespace FluentQt;

QWidget *GalleryWindow::createMaterialPage()
{
    auto *page = new GalleryInterface(navTx("Material"), QStringLiteral("FluentQtWidgets::Widgets"),
                                      this);
    const QString acrylicLabelSource = exampleSourceUrl("material/acrylic_label");

    auto *label = new AcrylicLabel(FluentConfig::instance()->acrylicBlurRadius(), QColor(105, 114, 168, 102), page);
    label->setImagePath(QStringLiteral(":/gallery/images/chidanta.jpg"));
    label->setMinimumSize(197, 145);
    label->setMaximumSize(787, 579);
    connect(FluentConfig::instance(), &FluentConfig::acrylicBlurRadiusChanged, label, [label](int radius) {
        label->setBlurRadius(radius);
    });
    page->addExampleCard(QCoreApplication::translate("MaterialInterface", "Acrylic label"), label, acrylicLabelSource, 1);

    return page;
}
