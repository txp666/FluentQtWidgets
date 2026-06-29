#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QUuid>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *breadcrumbBar = new BreadcrumbBar(this);
        auto *stackedWidget = new QStackedWidget(this);
        auto *lineEdit = new LineEdit(this);
        auto *addButton = new PrimaryToolButton(icon(FluentIcon::Send), this);
        auto *lineEditLayout = new QHBoxLayout;
        auto *layout = new QVBoxLayout(this);

        lineEdit->setPlaceholderText(QStringLiteral("Enter the name of interface"));
        breadcrumbBar->setSpacing(20);
        QFont breadcrumbFont = breadcrumbBar->font();
        breadcrumbFont.setPixelSize(26);
        breadcrumbBar->setFont(breadcrumbFont);

        const auto addInterface = [breadcrumbBar, stackedWidget, lineEdit](const QString &text) {
            if (text.trimmed().isEmpty()) {
                return;
            }

            auto *page = new SubtitleLabel(text, stackedWidget);
            page->setObjectName(QUuid::createUuid().toString(QUuid::WithoutBraces));
            page->setAlignment(Qt::AlignCenter);

            lineEdit->clear();
            stackedWidget->addWidget(page);
            stackedWidget->setCurrentWidget(page);
            breadcrumbBar->addItem(page->objectName(), text);
        };

        connect(addButton, &QToolButton::clicked, this, [lineEdit, addInterface]() { addInterface(lineEdit->text()); });
        connect(lineEdit, &QLineEdit::returnPressed, this, [lineEdit, addInterface]() { addInterface(lineEdit->text()); });
        connect(breadcrumbBar, &BreadcrumbBar::currentItemChanged, stackedWidget,
                [stackedWidget](const QString &key) {
                    if (auto *page = stackedWidget->findChild<QWidget *>(key)) {
                        stackedWidget->setCurrentWidget(page);
                    }
                });

        layout->setContentsMargins(20, 20, 20, 20);
        layout->addWidget(breadcrumbBar);
        layout->addWidget(stackedWidget);
        layout->addLayout(lineEditLayout);
        lineEditLayout->addWidget(lineEdit, 1);
        lineEditLayout->addWidget(addButton);

        addInterface(QStringLiteral("Home"));
        addInterface(QStringLiteral("Documents"));
        resize(500, 500);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    Demo demo;
    demo.show();
    return app.exec();
}
