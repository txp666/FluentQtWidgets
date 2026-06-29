#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Window : public QWidget
{
  public:
    explicit Window(QWidget *parent = nullptr) : QWidget(parent)
    {
        resize(160, 80);

        auto *switchButton = new SwitchButton(this);
        switchButton->move(48, 24);
        connect(switchButton, &SwitchButton::checkedChanged, this, [switchButton](bool checked) {
            switchButton->setText(checked ? QStringLiteral("On") : QStringLiteral("Off"));
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
    Window w;
    w.show();
    return QApplication::exec();
}
