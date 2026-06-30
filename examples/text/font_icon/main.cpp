#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *button1 = new PushButton(icon(FluentIcon::GitHub), QStringLiteral("Default"), this);
        auto *button2 = new PushButton(icon(FluentIcon::Link), QStringLiteral("Custom"), this);
        auto *button3 = new TogglePushButton(icon(FluentIcon::Heart), QStringLiteral("Toggle"), this);
        auto *button4 = new HyperlinkButton(icon(FluentIcon::Play),
                                            QStringLiteral(FQW_REPOSITORY_URL),
                                            QStringLiteral("Hyperlink"), this);
        auto *themeButton = new SwitchButton(this);
        themeButton->move(200, 50);
        themeButton->setOnText(QStringLiteral("Dark"));
        themeButton->setOffText(QStringLiteral("Light"));

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(button1);
        layout->addWidget(button2);
        layout->addWidget(button3);
        layout->addWidget(button4);
        resize(500, 500);

        connect(themeButton, &SwitchButton::checkedChanged, this, [this](bool checked) {
            ThemeManager::instance()->setTheme(checked ? Theme::Dark : Theme::Light);
            QPalette p = palette();
            p.setColor(QPalette::Window, checked ? QColor(32, 32, 32) : QColor(242, 242, 242));
            setAutoFillBackground(true);
            setPalette(p);
        });
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo w;
    w.show();
    return QApplication::exec();
}
