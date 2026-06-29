#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
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

void addSubInterface(SegmentedToggleToolWidget *segmented, QStackedWidget *stackedWidget, QLabel *page, const QIcon &icon)
{
    stackedWidget->addWidget(page);
    segmented->addItem(page->objectName(), icon);
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
    auto *segmented = new SegmentedToggleToolWidget(&window);
    auto *stackedWidget = new QStackedWidget(&window);
    auto *toolLayout = new QHBoxLayout;
    auto *layout = new QVBoxLayout(&window);

    addSubInterface(segmented, stackedWidget,
                    createPage(QStringLiteral("Song Interface"), QStringLiteral("songInterface"), &window),
                    icon(FluentIcon::Music));
    addSubInterface(segmented, stackedWidget,
                    createPage(QStringLiteral("Album Interface"), QStringLiteral("albumInterface"), &window),
                    icon(FluentIcon::Album));
    addSubInterface(segmented, stackedWidget,
                    createPage(QStringLiteral("Artist Interface"), QStringLiteral("artistInterface"), &window),
                    icon(FluentIcon::People));

    toolLayout->addWidget(segmented, 0, Qt::AlignCenter);
    layout->addLayout(toolLayout);
    layout->addWidget(stackedWidget);
    layout->setContentsMargins(30, 10, 30, 30);

    stackedWidget->setCurrentIndex(0);
    segmented->setCurrentItem(QStringLiteral("songInterface"));
    QObject::connect(segmented, &SegmentedToggleToolWidget::currentItemChanged, stackedWidget,
                     [stackedWidget](const QString &key) {
                         if (auto *page = stackedWidget->findChild<QWidget *>(key)) {
                             stackedWidget->setCurrentWidget(page);
                         }
                     });

    window.show();
    return app.exec();
}
