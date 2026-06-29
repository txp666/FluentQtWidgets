#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

QWidget *GalleryWindow::createDialogPage()
{
    auto *page = new GalleryInterface(navTx("Dialogs & flyouts"),
                                      QStringLiteral("qfluentwidgets.components.dialog_box"), this);
    const QString dialogSource = exampleSourceUrl("dialog_flyout/dialog");
    const QString messageDialogSource = exampleSourceUrl("dialog_flyout/message_dialog");
    const QString customMessageBoxSource = exampleSourceUrl("dialog_flyout/custom_message_box");
    const QString colorDialogSource = exampleSourceUrl("dialog_flyout/color_dialog");
    const QString flyoutSource = exampleSourceUrl("dialog_flyout/flyout");
    const QString teachingTipSource = exampleSourceUrl("dialog_flyout/teaching_tip");

    // -- Frameless Dialog --
    auto *dialogBtn = new PushButton(tx("DialogInterface", "Show dialog"));
    connect(dialogBtn, &QPushButton::clicked, page, [page]() {
        Dialog dialog(tx("DialogInterface", "This is a frameless message dialog"),
                      tx("DialogInterface",
                         "If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will "
                         "automatically wrap like this."),
                      page->window());
        dialog.setContentCopyable(true);
        dialog.exec();
    });
    page->addExampleCard(tx("DialogInterface", "A frameless message box"), dialogBtn, dialogSource);

    // -- MessageBox --
    auto *msgBoxBtn = new PushButton(tx("DialogInterface", "Show dialog"));
    connect(msgBoxBtn, &QPushButton::clicked, page, [page]() {
        MessageBox box(tx("DialogInterface", "This is a message dialog with mask"),
                       tx("DialogInterface",
                          "If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will "
                          "automatically wrap like this."),
                       page->window());
        box.setContentCopyable(true);
        box.exec();
    });
    page->addExampleCard(tx("DialogInterface", "A message box with mask"), msgBoxBtn, messageDialogSource);

    // -- Custom message box (demonstrates extending Dialog) --
    auto *customBtn = new PushButton(tx("DialogInterface", "Show dialog"));
    connect(customBtn, &QPushButton::clicked, page, [page]() {
        Dialog customBox(tx("CustomMessageBox", "Open URL"), QString(), page->window());
        customBox.acceptButton()->setText(tx("CustomMessageBox", "Open"));
        customBox.setMinimumWidth(360);
        customBox.contentLabel()->hide();

        auto *urlEdit = new LineEdit(customBox.view());
        urlEdit->setPlaceholderText(tx("CustomMessageBox", "Enter the URL of a file, stream, or playlist"));
        urlEdit->setClearButtonEnabled(true);
        urlEdit->setMinimumWidth(360);
        customBox.textLayout()->addWidget(urlEdit);
        customBox.acceptButton()->setEnabled(false);
        QObject::connect(urlEdit, &QLineEdit::textChanged, &customBox, [&customBox](const QString &text) {
            customBox.acceptButton()->setEnabled(QUrl(text).isValid());
        });

        if (customBox.exec() == QDialog::Accepted) {
            InfoBar::info(QStringLiteral("URL"), urlEdit->text(), Qt::Horizontal, true, 2000,
                          InfoBarPosition::TopRight, page);
        }
    });
    page->addExampleCard(tx("DialogInterface", "A custom message box"), customBtn, customMessageBoxSource);

    // -- Color dialog --
    auto *colorDialogBtn = new PushButton(tx("DialogInterface", "Show dialog"));
    connect(colorDialogBtn, &QPushButton::clicked, page, [page]() {
        ColorDialog dialog(Qt::cyan, tx("DialogInterface", "Choose color"), page->window());
        QObject::connect(&dialog, &ColorDialog::colorChanged, page,
                         [](const QColor &color) { qDebug() << __FUNCTION__ << color.name(); });
        dialog.exec();
    });
    page->addExampleCard(tx("DialogInterface", "A color dialog"), colorDialogBtn, colorDialogSource);

    // -- Simple Flyout --
    auto *simpleFlyoutBtn = new PushButton(tx("DialogInterface", "Show flyout"));
    connect(simpleFlyoutBtn, &QPushButton::clicked, page, [simpleFlyoutBtn]() {
        Flyout::make(QStringLiteral("Lesson 3"), tx("DialogInterface", "Believe in the spin, just keep believing!"),
                     simpleFlyoutBtn);
    });
    page->addExampleCard(tx("DialogInterface", "A simple flyout"), simpleFlyoutBtn, flyoutSource);

    // -- Complex Flyout --
    auto *complexFlyoutBtn = new PushButton(tx("DialogInterface", "Show flyout"));
    connect(complexFlyoutBtn, &QPushButton::clicked, page, [complexFlyoutBtn]() {
        auto *view = new FlyoutView(tx("DialogInterface", "Julius·Zeppeli"),
                                    tx("DialogInterface",
                                       "Where the tennis ball will land when it touches the net, no one can predict.\n"
                                       "If that moment comes, I hope the 'goddess' exists.\n"
                                       "In that case, I would accept it no matter which side the ball falls on."));
        view->setImagePath(QStringLiteral(":/gallery/images/chidanta.jpg"));
        view->setClosable(true);
        auto *actionButton = new PushButton(tx("DialogInterface", "Action"));
        actionButton->setFixedWidth(120);
        view->addWidget(actionButton, 0, Qt::AlignRight);
        Flyout::create(view, complexFlyoutBtn, FlyoutAnimationType::SlideRight);
    });
    page->addExampleCard(tx("DialogInterface", "A flyout with image and button"), complexFlyoutBtn, flyoutSource);

    // -- TeachingTip (Bottom) --
    auto *teachingBtn = new PushButton(tx("DialogInterface", "Show teaching tip"));
    connect(teachingBtn, &QPushButton::clicked, page, [teachingBtn]() {
        TeachingTip::create(QStringLiteral("Lesson 4"),
                            tx("DialogInterface", "With respect, let's advance towards a new stage of the spin."),
                            teachingBtn,
                            TeachingTipTailPosition::Bottom, 2500);
    });
    page->addExampleCard(tx("DialogInterface", "A teaching tip"), teachingBtn, teachingTipSource);

    // -- TeachingTip (Left Bottom) --
    auto *teachingLeftBtn = new PushButton(tx("DialogInterface", "Show teaching tip"));
    connect(teachingLeftBtn, &QPushButton::clicked, page, [page, teachingLeftBtn]() {
        auto *view =
            new TeachingTipView(QStringLiteral("Lesson 5"),
                                tx("DialogInterface", "The shortest shortcut is to take a detour."));
        view->setImagePath(QStringLiteral(":/gallery/images/chidanta2.jpg"));
        view->setClosable(true);
        auto *actionButton = new PushButton(tx("DialogInterface", "Action"));
        actionButton->setFixedWidth(120);
        view->addWidget(actionButton, 0, Qt::AlignRight);
        TeachingTip::make(view, teachingLeftBtn, TeachingTipTailPosition::LeftBottom, 3000, page);
    });
    page->addExampleCard(tx("DialogInterface", "A teaching tip with image and button"), teachingLeftBtn,
                         teachingTipSource);

    return page;
}
