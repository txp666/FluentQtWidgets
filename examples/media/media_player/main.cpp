#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

class Demo1 : public QWidget
{
  public:
    explicit Demo1(QWidget *parent = nullptr) : QWidget(parent)
    {
        ThemeManager::instance()->setTheme(Theme::Dark);
        auto *vBoxLayout = new QVBoxLayout(this);
        resize(500, 300);

        auto *simplePlayBar = new SimpleMediaPlayBar(this);
        auto *standardPlayBar = new StandardMediaPlayBar(this);

        vBoxLayout->addWidget(simplePlayBar);
        vBoxLayout->addWidget(standardPlayBar);

        // Online music
        simplePlayBar->player()->setSource(
            QUrl(QStringLiteral("https://files.cnblogs.com/files/blogs/677826/beat.zip?t=1693900324")));

        // Local music
        standardPlayBar->player()->setSource(
            QUrl(QStringLiteral("qrc:/media/media_player/resource/aiko-shiawase.mp3")));

    }
};

class Demo2 : public QWidget
{
  public:
    explicit Demo2(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *vBoxLayout = new QVBoxLayout(this);
        auto *videoWidget = new VideoWidget(this);

        videoWidget->setVideo(QUrl(QStringLiteral("https://media.w3.org/2010/05/sintel/trailer.mp4")));
        videoWidget->play();

        vBoxLayout->setContentsMargins(0, 0, 0, 0);
        vBoxLayout->addWidget(videoWidget);
        resize(800, 450);
    }
};

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Demo1 demo1;
    demo1.show();
    Demo2 demo2;
    demo2.show();
    return app.exec();
}
