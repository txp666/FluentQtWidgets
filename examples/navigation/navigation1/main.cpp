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
        QString objectName = text;
        setObjectName(objectName.replace(QLatin1Char(' '), QLatin1Char('-')));

        auto *label = new SubtitleLabel(text, this);
        QFont font = label->font();
        font.setPixelSize(24);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label, 1, Qt::AlignCenter);
    }
};

class DemoWindow : public FluentWindow
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::Home));

        auto *search = new DemoPage(QStringLiteral("Search Interface"), this);
        auto *music = new DemoPage(QStringLiteral("Music Interface"), this);
        auto *video = new DemoPage(QStringLiteral("Video Interface"), this);
        auto *folder = new DemoPage(QStringLiteral("Folder Interface"), this);
        auto *settings = new DemoPage(QStringLiteral("Setting Interface"), this);
        auto *albums = new DemoPage(QStringLiteral("Album Interface"), this);
        auto *album1 = new DemoPage(QStringLiteral("Album Interface 1"), this);
        auto *album2 = new DemoPage(QStringLiteral("Album Interface 2"), this);
        auto *album11 = new DemoPage(QStringLiteral("Album Interface 1-1"), this);

        addSubInterface(search, icon(FluentIcon::Search), QStringLiteral("Search"), QStringLiteral("Search-Interface"));
        addSubInterface(music, icon(FluentIcon::Music), QStringLiteral("Music library"),
                        QStringLiteral("Music-Interface"));
        addSubInterface(video, icon(FluentIcon::Video), QStringLiteral("Video library"),
                        QStringLiteral("Video-Interface"));

        navigationInterface()->addSeparator();

        addSubInterface(albums, icon(FluentIcon::Album), QStringLiteral("Albums"),
                        QStringLiteral("Album-Interface"), NavigationItemPosition::Scroll);
        addSubInterface(album1, icon(FluentIcon::Album), QStringLiteral("Album 1"),
                        QStringLiteral("Album-Interface-1"), NavigationItemPosition::Scroll,
                        QStringLiteral("Album-Interface"));
        addSubInterface(album11, icon(FluentIcon::Album), QStringLiteral("Album 1.1"),
                        QStringLiteral("Album-Interface-1-1"), NavigationItemPosition::Scroll,
                        QStringLiteral("Album-Interface-1"));
        addSubInterface(album2, icon(FluentIcon::Album), QStringLiteral("Album 2"),
                        QStringLiteral("Album-Interface-2"), NavigationItemPosition::Scroll,
                        QStringLiteral("Album-Interface"));

        addSubInterface(folder, icon(FluentIcon::Folder), QStringLiteral("Folder library"),
                        QStringLiteral("Folder-Interface"), NavigationItemPosition::Scroll);

        auto *avatar = new NavigationAvatarWidget(QStringLiteral("zhiyiYo"), this);
        navigationInterface()->addWidget(QStringLiteral("avatar"), avatar, NavigationItemPosition::Bottom);
        connect(avatar, &NavigationWidget::clicked, this, [this]() {
            MessageBox box(QStringLiteral("Navigation"),
                           QStringLiteral("This custom bottom item is backed by NavigationAvatarWidget."), this);
            box.exec();
        });

        addSubInterface(settings, icon(FluentIcon::Setting), QStringLiteral("Settings"),
                        QStringLiteral("Setting-Interface"), NavigationItemPosition::Bottom);
        switchTo(QStringLiteral("Music-Interface"));
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
    DemoWindow window;
    window.show();
    return app.exec();
}
