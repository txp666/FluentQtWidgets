#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

QString resourcePath(const QString &name)
{
    return QStringLiteral(FQW_EXAMPLE_RESOURCE_DIR) + QLatin1Char('/') + name;
}

} // namespace

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *imageLabel = new ImageLabel(resourcePath(QStringLiteral("Gyro.jpg")), this);
        auto *gifLabel = new ImageLabel(resourcePath(QStringLiteral("boqi.gif")), this);

        imageLabel->scaledToHeight(300);
        gifLabel->scaledToHeight(300);
        imageLabel->setBorderRadius(0, 30, 30, 0);
        gifLabel->setBorderRadius(10, 10, 10, 10);

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(imageLabel);
        layout->addWidget(gifLabel);
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
