#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

QWidget *GalleryWindow::createMenuPage()
{
    auto *page = new GalleryInterface(navTx("Menus & toolbars"), QStringLiteral("qfluentwidgets.components.widgets"),
                                      this);
    const QString menuSource = exampleSourceUrl("menu/menu");
    const QString widgetMenuSource = exampleSourceUrl("menu/widget_menu");
    const QString commandBarSource = exampleSourceUrl("menu/command_bar");

    auto *menuBtn = new PushButton(tx("MenuInterface", "Show menu"));
    connect(menuBtn, &QPushButton::clicked, page, [menuBtn]() {
        auto *menu = new RoundMenu(menuBtn);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addAction(FluentIcon::Copy, tx("MenuInterface", "Copy"));
        menu->addAction(FluentIcon::Cut, tx("MenuInterface", "Cut"));
        auto *submenu = new RoundMenu(tx("MenuInterface", "Add to"), menu);
        submenu->setIcon(icon(FluentIcon::Add));
        submenu->addAction(FluentIcon::Video, tx("MenuInterface", "Video"));
        submenu->addAction(FluentIcon::Music, tx("MenuInterface", "Music"));
        menu->addMenu(submenu);
        menu->addAction(FluentIcon::Paste, tx("MenuInterface", "Paste"));
        menu->addAction(FluentIcon::Cancel, tx("MenuInterface", "Undo"));
        menu->addSeparator();
        menu->addAction(FluentIcon::Setting, tx("MenuInterface", "Settings"));
        menu->addAction(FluentIcon::Help, tx("MenuInterface", "Help"));
        menu->addAction(FluentIcon::Feedback, tx("MenuInterface", "Feedback"));
        menu->addAction(tx("MenuInterface", "Select all"));
        menu->exec(menuBtn->mapToGlobal(QPoint(menuBtn->width() + 5, -100)), true, MenuAnimationType::DropDown);
    });
    page->addExampleCard(tx("MenuInterface", "Rounded corners menu"), menuBtn, menuSource);

    auto *profileMenuBtn = new PushButton(tx("MenuInterface", "Show menu"));
    connect(profileMenuBtn, &QPushButton::clicked, page, [profileMenuBtn]() {
        auto *menu = new RoundMenu(profileMenuBtn);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addWidget(new ProfileCard(menu), false);
        menu->addSeparator();
        menu->addAction(FluentIcon::People, tx("MenuInterface", "Manage account profile"));
        menu->addAction(FluentIcon::ShoppingCart, tx("MenuInterface", "Payment method"));
        menu->addAction(FluentIcon::Code, tx("MenuInterface", "Redemption code and gift card"));
        menu->addSeparator();
        menu->addAction(FluentIcon::Setting, tx("MenuInterface", "Settings"));
        menu->exec(profileMenuBtn->mapToGlobal(QPoint(profileMenuBtn->width() + 5, -100)), true,
                  MenuAnimationType::DropDown);
    });
    page->addExampleCard(tx("MenuInterface", "Rounded corners menu with custom widget"), profileMenuBtn,
                         widgetMenuSource);

    auto *checkableMenuBtn = new PushButton(tx("MenuInterface", "Show menu"));
    connect(checkableMenuBtn, &QPushButton::clicked, page, [checkableMenuBtn]() {
        auto *menu = new CheckableMenu(QString(), checkableMenuBtn, MenuIndicatorType::Radio);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        auto *group1 = new QActionGroup(menu);
        auto *createTime = new QAction(icon(FluentIcon::Calendar), tx("MenuInterface", "Create Date"), group1);
        auto *shootTime = new QAction(icon(FluentIcon::Camera), tx("MenuInterface", "Shooting Date"), group1);
        auto *modifiedTime = new QAction(icon(FluentIcon::Edit), tx("MenuInterface", "Modified time"), group1);
        auto *name = new QAction(icon(FluentIcon::Font), tx("MenuInterface", "Name"), group1);
        for (auto *action : {createTime, shootTime, modifiedTime, name}) {
            action->setCheckable(true);
            group1->addAction(action);
        }
        shootTime->setChecked(true);
        menu->addActions({createTime, shootTime, modifiedTime, name});
        menu->addSeparator();
        auto *group2 = new QActionGroup(menu);
        auto *ascending = new QAction(icon(FluentIcon::Up), tx("MenuInterface", "Ascending"), group2);
        auto *descending = new QAction(icon(FluentIcon::Download), tx("MenuInterface", "Descending"), group2);
        ascending->setCheckable(true);
        descending->setCheckable(true);
        ascending->setChecked(true);
        group2->addAction(ascending);
        group2->addAction(descending);
        menu->addActions({ascending, descending});
        menu->exec(checkableMenuBtn->mapToGlobal(QPoint(checkableMenuBtn->width() + 5, -100)), true,
                  MenuAnimationType::DropDown);
    });
    page->addExampleCard(tx("MenuInterface", "Checkable menu"), checkableMenuBtn, menuSource);

    auto *commandBar = new CommandBar(page);
    commandBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    commandBar->addAction(FluentIcon::Add, tx("MenuInterface", "Add"));
    commandBar->addAction(FluentIcon::Rotate, tx("MenuInterface", "Rotate"));
    commandBar->addAction(FluentIcon::ZoomIn, tx("MenuInterface", "Zoom in"));
    commandBar->addAction(FluentIcon::ZoomOut, tx("MenuInterface", "Zoom out"));
    commandBar->addSeparator();
    auto *editAction = new QAction(icon(FluentIcon::Edit), tx("MenuInterface", "Edit"), commandBar);
    editAction->setCheckable(true);
    commandBar->addAction(editAction);
    commandBar->addAction(FluentIcon::Info, tx("MenuInterface", "Info"));
    commandBar->addAction(FluentIcon::Delete, tx("MenuInterface", "Delete"));
    commandBar->addAction(FluentIcon::Share, tx("MenuInterface", "Share"));
    auto *dropBtn = new TransparentDropDownPushButton(icon(FluentIcon::Scroll), tx("MenuInterface", "Sort"));
    auto *sortMenu = new RoundMenu(dropBtn);
    sortMenu->addCheckableAction(tx("MenuInterface", "Ascending"), true);
    sortMenu->addCheckableAction(tx("MenuInterface", "Descending"));
    dropBtn->setDropDownMenu(sortMenu);
    dropBtn->setFixedHeight(34);
    commandBar->addWidget(dropBtn);
    auto *settingsAction = commandBar->addHiddenAction(FluentIcon::Setting, tx("MenuInterface", "Settings"));
    settingsAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+I")));
    page->addExampleCard(tx("MenuInterface", "Command bar"), commandBar, commandBarSource, 1);

    auto *flyoutWidget = new QWidget(page);
    auto *flyoutLayout = new QVBoxLayout(flyoutWidget);
    flyoutLayout->setContentsMargins(0, 0, 0, 0);
    flyoutLayout->setSpacing(10);
    flyoutLayout->addWidget(
        new BodyLabel(tx("MenuInterface", "Click the image to open a command bar flyout 👇️🥵")));
    auto *imageLabel = new ImageLabel;
    imageLabel->setImagePath(QStringLiteral(":/gallery/images/chidanta5.jpg"));
    imageLabel->scaledToWidth(350);
    imageLabel->setBorderRadius(8);
    flyoutLayout->addWidget(imageLabel, 0, Qt::AlignLeft);
    connect(imageLabel, &ImageLabel::clicked, page, [page, imageLabel]() {
        auto *view = new CommandBarView;
        view->addAction(FluentIcon::Share, tx("MenuInterface", "Share"));
        auto *saveAction = view->addAction(FluentIcon::Save, tx("MenuInterface", "Save"));
        QObject::connect(saveAction, &QAction::triggered, page, [page, imageLabel]() {
            const QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            const QString initialPath =
                QDir(desktop.isEmpty() ? QDir::homePath() : desktop).filePath(QStringLiteral("fluent-gallery-image.png"));
            Dialog dialog(tx("MenuInterface", "Save image"),
                          tx("MenuInterface", "Choose where to save this PNG image."), page->window());
            auto *pathEdit = new LineEdit(dialog.view());
            pathEdit->setClearButtonEnabled(true);
            pathEdit->setText(QDir::toNativeSeparators(initialPath));
            pathEdit->setMinimumWidth(420);
            dialog.textLayout()->addWidget(pathEdit);

            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            QString path = QDir::fromNativeSeparators(pathEdit->text().trimmed());
            if (path.isEmpty()) {
                return;
            }
            if (!path.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)) {
                path.append(QStringLiteral(".png"));
            }
            path = QDir::cleanPath(path);

            if (imageLabel->image().save(path, "PNG")) {
                InfoBar::success(tx("MenuInterface", "Saved"), tx("MenuInterface", "Image saved to %1").arg(path),
                                 Qt::Horizontal, true, 2000, InfoBarPosition::TopRight, page);
            } else {
                InfoBar::error(tx("MenuInterface", "Save failed"),
                               tx("MenuInterface", "Could not save image to %1").arg(path), Qt::Horizontal, true, 3000,
                               InfoBarPosition::TopRight, page);
            }
        });
        view->addAction(FluentIcon::Heart, tx("MenuInterface", "Add to favorate"));
        view->addAction(FluentIcon::Delete, tx("MenuInterface", "Delete"));
        auto *printAction = view->addHiddenAction(FluentIcon::Print, tx("MenuInterface", "Print"));
        printAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+P")));
        auto *settings = view->addHiddenAction(FluentIcon::Setting, tx("MenuInterface", "Settings"));
        settings->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));
        view->resizeToSuitableWidth();
        Flyout::create(view, imageLabel->mapToGlobal(QPoint(imageLabel->width(), 0)), FlyoutAnimationType::FadeIn);
    });
    page->addExampleCard(tx("MenuInterface", "Command bar flyout"), flyoutWidget, commandBarSource, 1);

    return page;
}
