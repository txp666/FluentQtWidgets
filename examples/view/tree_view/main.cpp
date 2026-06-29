#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QFileSystemModel>

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
        auto *view = new TreeView(this);
        auto *model = new QFileSystemModel(view);
        model->setRootPath(QStringLiteral("."));
        view->setModel(model);
        view->setBorderVisible(true);
        view->setBorderRadius(8);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(50, 30, 50, 30);
        layout->addWidget(view);
        setStyleSheet(QStringLiteral("Demo{background:rgb(255,255,255)}"));
        resize(800, 660);
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
