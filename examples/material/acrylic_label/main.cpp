#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>

using namespace FluentQt;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);

    AcrylicLabel label(20, QColor(105, 114, 168, 102));
    label.setImage(QStringLiteral(":/material/acrylic_label/resource/埃罗芒阿老师.jpg"));
    label.resize(label.sizeHint());
    label.show();

    return app.exec();
}
