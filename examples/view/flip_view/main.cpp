#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QSize>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *flipView = new HorizontalFlipView(this);
        auto *pager = new HorizontalPipsPager(this);

        flipView->setAspectRatioMode(Qt::KeepAspectRatio);
        flipView->addImages(QStringList{
            QStringLiteral(":/flip_view/1.jpg"),
            QStringLiteral(":/flip_view/2.jpg"),
            QStringLiteral(":/flip_view/3.jpg"),
            QStringLiteral(":/flip_view/4.jpg"),
            QStringLiteral(":/flip_view/5.png"),
            QStringLiteral(":/flip_view/6.png"),
        });
        pager->setPageNumber(flipView->count());

        connect(pager, &PipsPager::currentIndexChanged, flipView, &FlipView::setCurrentIndex);
        connect(flipView, &FlipView::currentIndexChanged, pager, &PipsPager::setCurrentIndex);

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(flipView, 0, Qt::AlignCenter);
        layout->addWidget(pager, 0, Qt::AlignCenter);
        layout->setAlignment(Qt::AlignCenter);
        layout->setSpacing(20);
        resize(600, 600);
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
