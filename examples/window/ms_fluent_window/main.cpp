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

class DemoWindow : public MSFluentWindow
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : MSFluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::Home));

        addSubInterface(new DemoPage(QStringLiteral("Home Interface"), this), icon(FluentIcon::Home),
                        QStringLiteral("Home"));
        addSubInterface(new DemoPage(QStringLiteral("Application Interface"), this), icon(FluentIcon::Code),
                        QStringLiteral("Apps"));
        addSubInterface(new DemoPage(QStringLiteral("Video Interface"), this), icon(FluentIcon::Video),
                        QStringLiteral("Video"));
        addSubInterface(new DemoPage(QStringLiteral("Library Interface"), this), icon(FluentIcon::Folder),
                        QStringLiteral("Library"), QIcon(), NavigationItemPosition::Bottom);

        auto *help = navigationInterface()->addItem(QStringLiteral("help"), icon(FluentIcon::Info),
                                                    QStringLiteral("Help"), false, QIcon(),
                                                    NavigationItemPosition::Bottom);
        connect(help, &NavigationWidget::clicked, this, [this]() {
            MessageBox box(QStringLiteral("MS Fluent Window"),
                           QStringLiteral("This command item is hosted by the NavigationBar."), this);
            box.exec();
        });

        navigationInterface()->setCurrentItem(currentRouteKey());
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
