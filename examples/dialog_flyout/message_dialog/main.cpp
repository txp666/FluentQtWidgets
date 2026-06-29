#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
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

        auto *button = new PrimaryPushButton(QStringLiteral("Click Me"), this);
        button->move(455, 25);
        connect(button, &QPushButton::clicked, this, &Demo::showDialog);
        resize(1000, 500);
    }

  private:
    void showDialog()
    {
        MessageBox box(QStringLiteral("Are you sure you want to delete the folder?"),
                       QStringLiteral("If you delete the \"Music\" folder from the list, the folder will no "
                                      "longer appear in the list, but will not be deleted."),
                       this);
        box.setClosableOnMaskClicked(true);
        box.setDraggable(true);
        if (box.exec() == QDialog::Accepted) {
            qDebug() << "Yes button is pressed";
        } else {
            qDebug() << "Cancel button is pressed";
        }
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
