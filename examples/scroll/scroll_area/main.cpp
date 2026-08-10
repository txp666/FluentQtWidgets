#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QtGlobal>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>

using namespace FluentQt;

class Demo : public SmoothScrollArea
{
  public:
    explicit Demo(QWidget *parent = nullptr) : SmoothScrollArea(parent)
    {
        auto *label = new PixmapLabel(this);
        label->setPixmap(QPixmap(QStringLiteral(":/scroll_area/shoko.jpg")));

        setScrollAnimation(Qt::Vertical, 400, QEasingCurve::OutQuint);
        setScrollAnimation(Qt::Horizontal, 400, QEasingCurve::OutQuint);
        enableTransparentBackground(true);
        horizontalScrollBar()->setValue(1900);
        setWidget(label);
        resize(1200, 800);

        QFile qss(QStringLiteral(":/scroll_area/demo.qss"));
        if (qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(QString::fromUtf8(qss.readAll()));
        }
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo window;
    window.show();
    return QApplication::exec();
}
