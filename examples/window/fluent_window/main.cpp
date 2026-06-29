#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

class DemoPage : public QWidget
{
  public:
    explicit DemoPage(const QString &text, QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *label = new SubtitleLabel(text, this);
        QFont labelFont = label->font();
        labelFont.setPixelSize(24);
        label->setFont(labelFont);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label, 1, Qt::AlignCenter);
    }
};

} // namespace

class DemoWindow : public FluentWindow
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::GitHub));

        auto *homeInterface = new DemoPage(QStringLiteral("Search Interface"), this);
        auto *musicInterface = new DemoPage(QStringLiteral("Music Interface"), this);
        auto *videoInterface = new DemoPage(QStringLiteral("Video Interface"), this);
        auto *folderInterface = new DemoPage(QStringLiteral("Folder Interface"), this);
        auto *settingInterface = new DemoPage(QStringLiteral("Setting Interface"), this);
        auto *albumInterface = new DemoPage(QStringLiteral("Album Interface"), this);
        auto *albumInterface1 = new DemoPage(QStringLiteral("Album Interface 1"), this);
        auto *albumInterface2 = new DemoPage(QStringLiteral("Album Interface 2"), this);
        auto *albumInterface11 = new DemoPage(QStringLiteral("Album Interface 1-1"), this);

        addSubInterface(homeInterface, icon(FluentIcon::Home), QStringLiteral("Home"), QStringLiteral("home"));
        addSubInterface(musicInterface, icon(FluentIcon::Play), QStringLiteral("Music library"), QStringLiteral("music"));
        addSubInterface(videoInterface, icon(FluentIcon::View), QStringLiteral("Video library"), QStringLiteral("video"));

        navigationInterface()->addSeparator();

        addSubInterface(albumInterface, icon(FluentIcon::Folder), QStringLiteral("Albums"), QStringLiteral("albums"),
                        NavigationItemPosition::Scroll);
        addSubInterface(albumInterface1, icon(FluentIcon::Folder), QStringLiteral("Album 1"), QStringLiteral("album1"),
                        NavigationItemPosition::Scroll, QStringLiteral("albums"));
        addSubInterface(albumInterface11, icon(FluentIcon::Folder), QStringLiteral("Album 1.1"),
                        QStringLiteral("album1_1"), NavigationItemPosition::Scroll, QStringLiteral("album1"));
        addSubInterface(albumInterface2, icon(FluentIcon::Folder), QStringLiteral("Album 2"), QStringLiteral("album2"),
                        NavigationItemPosition::Scroll, QStringLiteral("albums"));
        addSubInterface(folderInterface, icon(FluentIcon::Folder), QStringLiteral("Folder library"),
                        QStringLiteral("folder"), NavigationItemPosition::Scroll);
        addSubInterface(settingInterface, icon(FluentIcon::Settings), QStringLiteral("Settings"),
                        QStringLiteral("settings"), NavigationItemPosition::Bottom);

        if (auto *videoItem = navigationInterface()->navigationPanel()->widget(QStringLiteral("video"))) {
            InfoBadge::attention(9, nullptr, videoItem, InfoBadgePosition::NavigationItem);
        }

        resize(900, 700);
    }
};

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
