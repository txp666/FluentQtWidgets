#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QUrl>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->setSpacing(20);

        layout->addWidget(new HyperlinkLabel(QUrl(QStringLiteral(FQW_REPOSITORY_URL)),
                                             QStringLiteral("GitHub"), this));
        layout->addWidget(new CaptionLabel(QStringLiteral("Caption"), this));
        layout->addWidget(new BodyLabel(QStringLiteral("Body"), this));
        layout->addWidget(new StrongBodyLabel(QStringLiteral("Body Strong"), this));
        layout->addWidget(new SubtitleLabel(QStringLiteral("Subtitle"), this));
        layout->addWidget(new TitleLabel(QStringLiteral("Title"), this));
        layout->addWidget(new LargeTitleLabel(QStringLiteral("Title Large"), this));
        layout->addWidget(new DisplayLabel(QStringLiteral("Display"), this));
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
