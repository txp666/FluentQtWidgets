#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPoint>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.resize(420, 260);

    auto *layout = new QHBoxLayout(&window);
    layout->setAlignment(Qt::AlignCenter);

    auto *button = new PushButton(QStringLiteral("Show acrylic menu"), &window);
    layout->addWidget(button, 0, Qt::AlignCenter);

    QObject::connect(button, &QPushButton::clicked, button, [button]() {
        auto *menu = new AcrylicMenu(button);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addAction(icon(FluentIcon::Copy), QStringLiteral("Copy"));
        menu->addAction(icon(FluentIcon::Cut), QStringLiteral("Cut"));

        auto *subMenu = new AcrylicMenu(QStringLiteral("Add to"), menu);
        subMenu->setIcon(icon(FluentIcon::Add));
        subMenu->addAction(icon(FluentIcon::Video), QStringLiteral("Video"));
        subMenu->addAction(icon(FluentIcon::Music), QStringLiteral("Music"));
        menu->addMenu(subMenu);

        menu->addAction(icon(FluentIcon::Paste), QStringLiteral("Paste"));
        menu->addAction(icon(FluentIcon::Cancel), QStringLiteral("Undo"));
        menu->addSeparator();
        menu->addAction(icon(FluentIcon::Setting), QStringLiteral("Settings"));

        menu->exec(button->mapToGlobal(QPoint(0, button->height())),
                   true, MenuAnimationType::DropDown);
    });

    window.show();
    return app.exec();
}
