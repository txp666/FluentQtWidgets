#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *commandBar = new CommandBar(this);
        commandBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        commandBar->addAction(FluentIcon::Add, QStringLiteral("Add"));
        commandBar->addSeparator();

        auto *editAction = new QAction(icon(FluentIcon::Edit), QStringLiteral("Edit"), commandBar);
        editAction->setCheckable(true);
        commandBar->addAction(editAction);
        commandBar->addAction(FluentIcon::Copy, QStringLiteral("Copy"));
        commandBar->addAction(FluentIcon::Feedback, QStringLiteral("Share"));

        auto *dropDownButton = createDropDownButton();
        commandBar->addWidget(dropDownButton);

        commandBar->addHiddenAction(FluentIcon::Scroll, QStringLiteral("Sort"));
        auto *settings = commandBar->addHiddenAction(FluentIcon::Settings, QStringLiteral("Settings"));
        settings->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(commandBar, 0);
        resize(240, 40);
        setWindowTitle(QStringLiteral("Drag window"));
    }

  private:
    TransparentDropDownPushButton *createDropDownButton()
    {
        auto *button =
            new TransparentDropDownPushButton(icon(FluentIcon::More), QStringLiteral("Menu"), this);
        button->setFixedHeight(34);
        auto font = button->font();
        font.setPixelSize(12);
        button->setFont(font);

        auto *menu = new RoundMenu(button);
        menu->addAction(FluentIcon::Copy, QStringLiteral("Copy"));
        menu->addAction(FluentIcon::Cut, QStringLiteral("Cut"));
        menu->addAction(FluentIcon::Paste, QStringLiteral("Paste"));
        menu->addAction(FluentIcon::Cancel, QStringLiteral("Cancel"));
        menu->addAction(QStringLiteral("Select all"));
        button->setMenu(menu);
        return button;
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo window;
    window.show();
    return QApplication::exec();
}
