#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        resize(400, 300);

        auto *layout = new QHBoxLayout(this);
        const QPixmap avatar(QStringLiteral(":/media/avatar_widget/resource/shoko.png"));

        const QList<int> sizes = {96, 48, 32, 24};
        for (int size : sizes) {
            auto *widget = new AvatarWidget(avatar, this);
            widget->setRadius(size / 2);
            layout->addWidget(widget);
        }

        auto *gifAvatar = new AvatarWidget(this);
        gifAvatar->setRadius(24);
        gifAvatar->setImagePath(QStringLiteral(":/media/avatar_widget/resource/boqi.gif"));
        layout->addWidget(gifAvatar);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    Demo demo;
    demo.show();
    return app.exec();
}
