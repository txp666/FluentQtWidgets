#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>

using namespace FluentQt;

namespace {

class DemoWindow : public FluentWindow
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::GitHub));
        resize(700, 600);

        auto *splash = new SplashScreen(windowIcon(), this);
        splash->setIconSize(QSize(102, 102));
        setSplashScreen(splash);

        QTimer::singleShot(1200, this, [this, splash]() {
            addSubInterface(new BodyLabel(QStringLiteral("Home Interface"), this), icon(FluentIcon::Home),
                            QStringLiteral("Home"), QStringLiteral("home"));
            splash->finish();
        });
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
