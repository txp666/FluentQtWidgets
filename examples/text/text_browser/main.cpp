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
        auto *textBrowser = new TextBrowser(this);
        textBrowser->setPlaceholderText(QStringLiteral("Search stand"));
        textBrowser->setMarkdown(QStringLiteral("## Steel Ball Run\n * Johnny Joestar\n * Gyro Zeppeli"));

        auto *layout = new QHBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->addWidget(textBrowser, 0, Qt::AlignCenter);
        resize(400, 400);
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
