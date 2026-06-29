#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *vPager = new VerticalPipsPager(this);
        auto *hPager = new HorizontalPipsPager(this);

        hPager->setPageNumber(15);
        vPager->setPageNumber(15);

        hPager->setVisibleNumber(8);
        hPager->setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
        hPager->setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::Always);

        vPager->setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
        vPager->setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::OnHover);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(hPager);
        layout->addWidget(vPager);

        resize(500, 500);
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
