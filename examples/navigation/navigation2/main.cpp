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
        setWindowIcon(icon(FluentIcon::Home));

        addSubInterface(new DemoPage(QStringLiteral("Search Interface"), this), icon(FluentIcon::Search),
                        QStringLiteral("Search"), QStringLiteral("Search-Interface"));
        addSubInterface(new DemoPage(QStringLiteral("Music Interface"), this), icon(FluentIcon::Music),
                        QStringLiteral("Music library"), QStringLiteral("Music-Interface"));
        addSubInterface(new DemoPage(QStringLiteral("Video Interface"), this), icon(FluentIcon::Video),
                        QStringLiteral("Video library"), QStringLiteral("Video-Interface"));

        navigationInterface()->addSeparator();

        addSubInterface(new DemoPage(QStringLiteral("Folder Interface"), this), icon(FluentIcon::Folder),
                        QStringLiteral("Folder library"), QStringLiteral("Folder-Interface"),
                        NavigationItemPosition::Scroll);

        auto *avatar = new NavigationAvatarWidget(QStringLiteral("zhiyiYo"), this);
        navigationInterface()->addWidget(QStringLiteral("avatar"), avatar, NavigationItemPosition::Bottom);
        connect(avatar, &NavigationWidget::clicked, this, [this]() {
            MessageBox box(QStringLiteral("Navigation"),
                           QStringLiteral("The return button is enabled by FluentWindow navigation history."), this);
            box.exec();
        });

        addSubInterface(new DemoPage(QStringLiteral("Setting Interface"), this), icon(FluentIcon::Settings),
                        QStringLiteral("Settings"), QStringLiteral("Setting-Interface"),
                        NavigationItemPosition::Bottom);
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
