#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QHBoxLayout(this);
        auto *button1 = new PushButton(QStringLiteral("Kirakira"), this);
        auto *button2 = new PushButton(QStringLiteral("Tabeta Ai"), this);
        auto *button3 = new PushButton(QStringLiteral("Shiawase"), this);

        button1->setToolTip(QStringLiteral("aiko - Kirakira"));
        button2->setToolTip(QStringLiteral("aiko - Tabeta Ai"));
        button3->setToolTip(QStringLiteral("aiko - Shiawase"));
        button1->setToolTipDuration(1000);

        button1->installEventFilter(new AcrylicToolTipFilter(button1, 0, ToolTipPosition::Top));
        button2->installEventFilter(new AcrylicToolTipFilter(button2, 0, ToolTipPosition::Bottom));
        button3->installEventFilter(new AcrylicToolTipFilter(button3, 300, ToolTipPosition::Right));

        connect(button1, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.youtube.com/watch?v=S0bXDRY1DGM")));
        });
        connect(button2, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.youtube.com/watch?v=CZLs8GuCq2U")));
        });
        connect(button3, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.youtube.com/watch?v=fp-yJUB7sS8")));
        });

        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);
        layout->addWidget(button1);
        layout->addWidget(button2);
        layout->addWidget(button3);
        resize(480, 240);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    Demo demo;
    demo.show();
    return app.exec();
}
