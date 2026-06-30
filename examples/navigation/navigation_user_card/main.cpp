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
        layout->addWidget(label, 1, Qt::AlignCenter);
    }
};

class DemoWindow : public FluentWindow
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("Navigation User Card"));
        setWindowIcon(icon(FluentIcon::People));

        auto *userCard = navigationInterface()->addUserCard(
            QStringLiteral("userCard"), QStringLiteral(":/navigation/navigation_user_card/resource/shoko.png"),
            QStringLiteral("zhiyiYo"), QStringLiteral("shokokawaii@outlook.com"), NavigationItemPosition::Top);
        userCard->setTitleFontSize(14);
        userCard->setSubtitleFontSize(12);

        addSubInterface(new DemoPage(QStringLiteral("Home Interface"), this), icon(FluentIcon::Home),
                        QStringLiteral("Home"), QStringLiteral("home-interface"));
        addSubInterface(new DemoPage(QStringLiteral("Music Interface"), this), icon(FluentIcon::Music),
                        QStringLiteral("Music library"), QStringLiteral("music-interface"));

        navigationInterface()->addSeparator();

        addSubInterface(new DemoPage(QStringLiteral("Video Interface"), this), icon(FluentIcon::Video),
                        QStringLiteral("Video library"), QStringLiteral("video-interface"));
        addSubInterface(new DemoPage(QStringLiteral("Setting Interface"), this), icon(FluentIcon::Setting),
                        QStringLiteral("Settings"), QStringLiteral("setting-interface"),
                        NavigationItemPosition::Bottom);

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
