#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *label = new QLabel(QStringLiteral("Right-click your mouse"), this);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label);
        resize(400, 400);
        setStyleSheet(QStringLiteral("Demo{background:white} QLabel{font-size:20px}"));
    }

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override
    {
        RoundMenu menu(this);

        auto *copy = menu.addAction(FluentIcon::Copy, QStringLiteral("Copy"));
        auto *cut = menu.addAction(FluentIcon::Cut, QStringLiteral("Cut"));
        Q_UNUSED(cut)
        copy->setCheckable(true);
        copy->setChecked(true);

        auto *submenu = new RoundMenu(QStringLiteral("Add to"), &menu);
        submenu->setIcon(icon(FluentIcon::Add));
        submenu->addAction(FluentIcon::Video, QStringLiteral("Video"));
        submenu->addAction(FluentIcon::Music, QStringLiteral("Music"));
        menu.addMenu(submenu);

        menu.addAction(FluentIcon::Paste, QStringLiteral("Paste"));
        menu.addAction(FluentIcon::Cancel, QStringLiteral("Undo"));
        menu.addSeparator();

        auto *selectAll = new QAction(QStringLiteral("Select all"), &menu);
        selectAll->setShortcut(QKeySequence(QStringLiteral("Ctrl+A")));
        menu.addAction(selectAll);

        auto *settings = new QAction(icon(FluentIcon::Setting), QStringLiteral("Settings"), &menu);
        settings->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));
        menu.insertAction(selectAll, settings);

        auto *help = new QAction(icon(FluentIcon::Info), QStringLiteral("Help"), &menu);
        help->setShortcut(QKeySequence(QStringLiteral("Ctrl+H")));
        auto *feedback = new QAction(icon(FluentIcon::Feedback), QStringLiteral("Feedback"), &menu);
        feedback->setShortcut(QKeySequence(QStringLiteral("Ctrl+F")));
        menu.insertActions(selectAll, {help, feedback});
        help->setCheckable(true);
        help->setChecked(true);

        menu.exec(event->globalPos(), true, MenuAnimationType::DropDown);
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
