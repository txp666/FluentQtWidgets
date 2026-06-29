#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *lineEdit = new SearchLineEdit(this);
        auto *button = new PushButton(QStringLiteral("Search"), this);

        const QStringList stands = {
            QStringLiteral("Star Platinum"), QStringLiteral("Hierophant Green"),
            QStringLiteral("Made in Heaven"), QStringLiteral("King Crimson"),
            QStringLiteral("Silver Chariot"), QStringLiteral("Crazy Diamond"),
            QStringLiteral("Metallica"), QStringLiteral("Another One Bites The Dust"),
            QStringLiteral("Heaven's Door"), QStringLiteral("Killer Queen"),
            QStringLiteral("The Grateful Dead"), QStringLiteral("Stone Free"),
            QStringLiteral("The World"), QStringLiteral("Sticky Fingers"),
            QStringLiteral("Ozone Baby"), QStringLiteral("Love Love Deluxe"),
            QStringLiteral("Hermit Purple"), QStringLiteral("Gold Experience"),
            QStringLiteral("King Nothing"), QStringLiteral("Paper Moon King"),
            QStringLiteral("Scary Monster"), QStringLiteral("Mandom"),
            QStringLiteral("20th Century Boy"), QStringLiteral("Tusk Act 4"),
            QStringLiteral("Ball Breaker"), QStringLiteral("Sex Pistols"),
            QStringLiteral("D4C Love Train"), QStringLiteral("Born This Way"),
            QStringLiteral("SOFT & WET"), QStringLiteral("Paisley Park"),
            QStringLiteral("Wonder of U"), QStringLiteral("Walking Heart"),
            QStringLiteral("Cream Starter"), QStringLiteral("November Rain"),
            QStringLiteral("Smooth Operators"), QStringLiteral("The Matte Kudasai")};
        auto *completer = new QCompleter(stands, lineEdit);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setMaxVisibleItems(10);
        lineEdit->setCompleter(completer);

        lineEdit->setFixedSize(200, 33);
        lineEdit->setClearButtonEnabled(true);
        lineEdit->setPlaceholderText(QStringLiteral("Search stand"));

        auto *layout = new QHBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->addWidget(lineEdit, 0, Qt::AlignCenter);
        layout->addWidget(button, 0, Qt::AlignCenter);
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
