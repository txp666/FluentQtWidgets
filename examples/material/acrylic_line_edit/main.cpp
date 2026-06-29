#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.resize(400, 400);

    auto *layout = new QHBoxLayout(&window);
    layout->setAlignment(Qt::AlignCenter);

    auto *lineEdit = new AcrylicSearchLineEdit(&window);
    lineEdit->setFixedSize(200, 33);
    lineEdit->setClearButtonEnabled(true);
    lineEdit->setPlaceholderText(QStringLiteral("Search stand"));

    const QStringList stands = {
        QStringLiteral("Star Platinum"), QStringLiteral("Hierophant Green"),
        QStringLiteral("Made in Heaven"), QStringLiteral("King Crimson"),
        QStringLiteral("Silver Chariot"), QStringLiteral("Crazy Diamond"),
        QStringLiteral("Killer Queen"), QStringLiteral("The World"),
        QStringLiteral("Sticky Fingers"), QStringLiteral("Wonder of U")};

    auto *completer = new QCompleter(stands, lineEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);
    lineEdit->setCompleter(completer);

    layout->addWidget(lineEdit, 0, Qt::AlignCenter);
    layout->addWidget(new PushButton(QStringLiteral("Search"), &window), 0, Qt::AlignCenter);

    window.show();
    return app.exec();
}
