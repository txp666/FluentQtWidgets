#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

QLabel *createPage(const QString &text, const QString &objectName, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(objectName);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("font: 20px \"%1\"; background: rgb(242,242,242); border-radius: 8px;")
                             .arg(QApplication::font().family()));
    return label;
}

void addSubInterface(SegmentedWidget *segmented, QStackedWidget *stackedWidget, QLabel *page, const QString &text)
{
    stackedWidget->addWidget(page);
    segmented->addItem(page->objectName(), text);
}

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);

    QWidget window;
    window.resize(400, 400);
    auto *segmented = new SegmentedWidget(&window);
    auto *stackedWidget = new QStackedWidget(&window);
    auto *layout = new QVBoxLayout(&window);

    addSubInterface(segmented, stackedWidget,
                    createPage(QStringLiteral("Song Interface"), QStringLiteral("songInterface"), &window),
                    QStringLiteral("Song"));
    addSubInterface(segmented, stackedWidget,
                    createPage(QStringLiteral("Album Interface"), QStringLiteral("albumInterface"), &window),
                    QStringLiteral("Album"));
    addSubInterface(segmented, stackedWidget,
                    createPage(QStringLiteral("Artist Interface"), QStringLiteral("artistInterface"), &window),
                    QStringLiteral("Artist"));

    layout->addWidget(segmented);
    layout->addWidget(stackedWidget);
    layout->setContentsMargins(30, 10, 30, 30);

    stackedWidget->setCurrentIndex(0);
    segmented->setCurrentItem(QStringLiteral("songInterface"));
    QObject::connect(segmented, &SegmentedWidget::currentItemChanged, stackedWidget,
                     [stackedWidget](const QString &key) {
                         if (auto *page = stackedWidget->findChild<QWidget *>(key)) {
                             stackedWidget->setCurrentWidget(page);
                         }
                     });

    window.show();
    return app.exec();
}
