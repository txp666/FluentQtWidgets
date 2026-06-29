#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new FlowLayout(this, true);
        layout->setAnimation(250, QEasingCurve::OutQuad);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->setVerticalSpacing(20);
        layout->setHorizontalSpacing(10);

        layout->addWidget(new PushButton(QStringLiteral("aiko"), this));
        layout->addWidget(new PushButton(QStringLiteral("刘静爱"), this));
        layout->addWidget(new PushButton(QStringLiteral("柳井爱子"), this));
        layout->addWidget(new PushButton(QStringLiteral("aiko 赛高"), this));
        layout->addWidget(new PushButton(QStringLiteral("aiko 太爱啦😘"), this));
        layout->insertWidget(1, new PrimaryPushButton(QStringLiteral("西宫硝子"), this));

        setStyleSheet(QStringLiteral("Demo{background:white} PushButton{padding:5px 10px; font:15px \"Microsoft YaHei\"}"));
        resize(250, 300);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo window;
    window.show();
    return QApplication::exec();
}
