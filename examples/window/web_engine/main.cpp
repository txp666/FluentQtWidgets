#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

class WebInterface : public QWidget
{
  public:
    explicit WebInterface(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName(QStringLiteral("homeInterface"));
        auto *webView = new QWebEngineView(this);
        webView->load(QUrl(QStringLiteral("https://www.baidu.com/")));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 48, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(webView);
    }
};

class WebEngineWindow : public SplitFluentWindow
{
  public:
    explicit WebEngineWindow(QWidget *parent = nullptr) : SplitFluentWindow(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::Home));
        addSubInterface(new WebInterface(this), icon(FluentIcon::Home), QStringLiteral("Home"),
                        QStringLiteral("homeInterface"));
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
    WebEngineWindow window;
    window.show();
    return app.exec();
}
