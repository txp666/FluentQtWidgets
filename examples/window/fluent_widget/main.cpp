#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

void toggleTheme()
{
    auto *themeManager = ThemeManager::instance();
    themeManager->setTheme(themeManager->effectiveTheme() == Theme::Dark ? Theme::Light : Theme::Dark);
}

class DemoWindow : public FluentWidget
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::GitHub));

        auto *button = new PushButton(icon(FluentIcon::Constract), QStringLiteral("Toggle theme"), this);
        connect(button, &QPushButton::clicked, this, []() {
            toggleTheme();
        });

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, titleBar()->height(), 0, 0);
        layout->addWidget(button, 0, Qt::AlignCenter);

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
