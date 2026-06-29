#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtWidgets/QCompleter>

using namespace FluentQt;

QWidget *GalleryWindow::createTextPage()
{
    auto *page = new GalleryInterface(navTx("Text"),
                                      tx("TextInterface", "LineEdit, SpinBox, TimeEdit and TextEdit"), this);
    const QString lineEditSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/text/line_edit/main.cpp");
    const QString spinBoxSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/text/spin_box/main.cpp");
    const QString textBrowserSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/text/text_browser/main.cpp");

    auto *lineEdit = new LineEdit(page);
    lineEdit->setText(tx("TextInterface", "ko no dio da！"));
    lineEdit->setClearButtonEnabled(true);
    lineEdit->setFixedWidth(260);
    page->addExampleCard(tx("TextInterface", "A LineEdit with a clear button"), lineEdit, lineEditSource);

    auto *searchEdit = new SearchLineEdit(page);
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
    auto *completer = new QCompleter(stands, searchEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);
    searchEdit->setCompleter(completer);
    searchEdit->setPlaceholderText(tx("TextInterface", "Type a stand name"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setFixedWidth(230);
    page->addExampleCard(tx("TextInterface", "A autosuggest line edit"), searchEdit, lineEditSource);

    auto *passwordEdit = new PasswordLineEdit(page);
    passwordEdit->setPlaceholderText(tx("TextInterface", "Enter your password"));
    passwordEdit->setFixedWidth(230);
    page->addExampleCard(tx("TextInterface", "A password line edit"), passwordEdit, lineEditSource);

    auto *spinBox = new SpinBox(page);
    page->addExampleCard(tx("TextInterface", "A SpinBox with a spin button"), spinBox, spinBoxSource);

    auto *doubleSpin = new DoubleSpinBox(page);
    page->addExampleCard(tx("TextInterface", "A DoubleSpinBox with a spin button"), doubleSpin, spinBoxSource);

    auto *dateEdit = new DateEdit(page);
    page->addExampleCard(tx("TextInterface", "A DateEdit with a spin button"), dateEdit, spinBoxSource);

    auto *timeEdit = new TimeEdit(page);
    page->addExampleCard(tx("TextInterface", "A TimeEdit with a spin button"), timeEdit, spinBoxSource);

    auto *dateTimeEdit = new DateTimeEdit(page);
    page->addExampleCard(tx("TextInterface", "A DateTimeEdit with a spin button"), dateTimeEdit, spinBoxSource);

    auto *textEdit = new TextEdit(page);
    textEdit->setMarkdown(QStringLiteral("## Steel Ball Run \n * Johnny Joestar 🦄 \n * Gyro Zeppeli 🐴 "));
    textEdit->setFixedHeight(150);
    page->addExampleCard(tx("TextInterface", "A simple TextEdit"), textEdit, textBrowserSource, 1);

    return page;
}
