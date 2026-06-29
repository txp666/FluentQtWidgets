#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QUrl>
#include <QtCore/QtGlobal>
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
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *button1 = new PushButton(QStringLiteral("キラキラ"), this);
        auto *button2 = new PushButton(QStringLiteral("食べた愛"), this);
        auto *button3 = new PushButton(QStringLiteral("シアワセ"), this);

        button1->setToolTip(QStringLiteral("aiko - キラキラ ✨"));
        button2->setToolTip(QStringLiteral("aiko - 食べた愛 🥰"));
        button3->setToolTip(QStringLiteral("aiko - シアワセ 😊"));
        button1->setToolTipDuration(1000);

        button1->installEventFilter(new ToolTipFilter(button1, 0, ToolTipPosition::Top));
        button2->installEventFilter(new ToolTipFilter(button2, 0, ToolTipPosition::Bottom));
        button3->installEventFilter(new ToolTipFilter(button3, 300, ToolTipPosition::Right));

        connect(button1, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.youtube.com/watch?v=S0bXDRY1DGM&list=RDMM&index=1")));
        });
        connect(button2, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.youtube.com/watch?v=CZLs8GuCq2U&list=RDMM&index=4")));
        });
        connect(button3, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.youtube.com/watch?v=fp-yJUB7sS8&list=RDMM&index=3")));
        });

        auto *layout = new QHBoxLayout(this);
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
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo w;
    w.show();
    return QApplication::exec();
}
