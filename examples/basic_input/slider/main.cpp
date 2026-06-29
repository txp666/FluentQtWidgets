#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo1 : public QWidget
{
  public:
    explicit Demo1(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, QColor(184, 106, 106));
        setPalette(p);

        HollowHandleStyle::Config style;
        style.subPageColor = QColor(70, 23, 180);

        auto *slider = new QSlider(Qt::Horizontal, this);
        slider->setStyle(new HollowHandleStyle(style));
        slider->resize(200, 28);
        slider->move(50, 61);

        resize(300, 150);
    }
};

class Demo2 : public QWidget
{
  public:
    explicit Demo2(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *slider1 = new Slider(Qt::Horizontal, this);
        slider1->setFixedWidth(200);
        slider1->move(50, 30);

        auto *slider2 = new Slider(Qt::Vertical, this);
        slider2->setFixedHeight(150);
        slider2->move(140, 80);

        resize(300, 300);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    Demo1 w1;
    w1.show();

    Demo2 w2;
    w2.show();

    return QApplication::exec();
}
