#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

class PlayBarDemo : public FluentWidget
{
  public:
    explicit PlayBarDemo(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        ThemeManager::instance()->setTheme(Theme::Dark);
        setWindowTitle(QStringLiteral("Media Play Bar"));
        setWindowIcon(icon(FluentIcon::Music));

        auto *title = new SubtitleLabel(tr("Media play bars"), this);
        auto *simplePlayBar = new SimpleMediaPlayBar(this);
        auto *standardPlayBar = new StandardMediaPlayBar(this);

        simplePlayBar->setSource(QUrl(QStringLiteral("https://files.cnblogs.com/files/blogs/677826/beat.zip?t=1693900324")));
        standardPlayBar->setSource(QUrl(QStringLiteral("qrc:/media/media_player/resource/aiko-shiawase.mp3")));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, titleBar()->height() + 24, 24, 24);
        layout->setSpacing(16);
        layout->addWidget(title);
        layout->addStretch(1);
        layout->addWidget(simplePlayBar);
        layout->addWidget(standardPlayBar);
        layout->addStretch(1);

        resize(520, 320);
    }
};

class VideoDemo : public FluentWidget
{
  public:
    explicit VideoDemo(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        setWindowTitle(QStringLiteral("Video Widget"));
        setWindowIcon(icon(FluentIcon::Video));

        auto *videoWidget = new VideoWidget(this);
        videoWidget->setVideo(QUrl(QStringLiteral("https://media.w3.org/2010/05/sintel/trailer.mp4")));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, titleBar()->height(), 0, 0);
        layout->setSpacing(0);
        layout->addWidget(videoWidget);

        resize(800, 450);
        videoWidget->play();
    }
};

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    PlayBarDemo playBarDemo;
    VideoDemo videoDemo;
    playBarDemo.show();
    videoDemo.show();
    return app.exec();
}
