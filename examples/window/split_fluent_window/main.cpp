#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>

using namespace FluentQt;

namespace {

class DemoPage : public QWidget
{
  public:
    explicit DemoPage(const QString &text, QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName(text.toLower().replace(QLatin1Char(' '), QLatin1Char('-')));

        auto *label = new SubtitleLabel(text, this);
        QFont font = label->font();
        font.setPixelSize(24);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 32, 0, 0);
        layout->addWidget(label, 1, Qt::AlignCenter);
    }
};

class DemoWindow : public SplitFluentWindow
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : SplitFluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::GitHub));

        auto *homeInterface = new DemoPage(QStringLiteral("Home Interface"), this);
        auto *musicInterface = new DemoPage(QStringLiteral("Music Interface"), this);
        auto *videoInterface = new DemoPage(QStringLiteral("Video Interface"), this);
        auto *folderInterface = new DemoPage(QStringLiteral("Folder Interface"), this);
        auto *settingInterface = new DemoPage(QStringLiteral("Setting Interface"), this);
        auto *albumInterface = new DemoPage(QStringLiteral("Album Interface"), this);
        auto *albumInterface1 = new DemoPage(QStringLiteral("Album Interface 1"), this);
        auto *albumInterface2 = new DemoPage(QStringLiteral("Album Interface 2"), this);
        auto *albumInterface11 = new DemoPage(QStringLiteral("Album Interface 1-1"), this);

        addSubInterface(homeInterface, icon(FluentIcon::Home), QStringLiteral("Home"), QStringLiteral("home-interface"));
        addSubInterface(musicInterface, icon(FluentIcon::Music), QStringLiteral("Music library"),
                        QStringLiteral("music-interface"));
        addSubInterface(videoInterface, icon(FluentIcon::Video), QStringLiteral("Video library"),
                        QStringLiteral("video-interface"));

        navigationInterface()->addSeparator();

        addSubInterface(albumInterface, icon(FluentIcon::Album), QStringLiteral("Albums"),
                        QStringLiteral("album-interface"), NavigationItemPosition::Scroll);
        addSubInterface(albumInterface1, icon(FluentIcon::Album), QStringLiteral("Album 1"),
                        QStringLiteral("album-interface-1"), NavigationItemPosition::Scroll,
                        QStringLiteral("album-interface"));
        addSubInterface(albumInterface11, icon(FluentIcon::Album), QStringLiteral("Album 1.1"),
                        QStringLiteral("album-interface-1-1"), NavigationItemPosition::Scroll,
                        QStringLiteral("album-interface-1"));
        addSubInterface(albumInterface2, icon(FluentIcon::Album), QStringLiteral("Album 2"),
                        QStringLiteral("album-interface-2"), NavigationItemPosition::Scroll,
                        QStringLiteral("album-interface"));
        addSubInterface(folderInterface, icon(FluentIcon::Folder), QStringLiteral("Folder library"),
                        QStringLiteral("folder-interface"), NavigationItemPosition::Scroll);

        auto *avatar = new NavigationAvatarWidget(QStringLiteral("zhiyiYo"),
                                                  QStringLiteral(":/window/split_fluent_window/resource/shoko.png"), this);
        navigationInterface()->addWidget(QStringLiteral("avatar"), avatar, NavigationItemPosition::Bottom);

        addSubInterface(settingInterface, icon(FluentIcon::Settings), QStringLiteral("Settings"),
                        QStringLiteral("setting-interface"), NavigationItemPosition::Bottom);

        resize(900, 700);
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
    ThemeManager::instance()->setTheme(Theme::Dark);
    DemoWindow window;
    window.show();
    return app.exec();
}
