#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *label = new QLabel(QStringLiteral("Right-click system tray icon"), this);
        label->setAlignment(Qt::AlignCenter);
        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label);

        resize(500, 500);
        setStyleSheet(QStringLiteral("Demo{background:white} QLabel{font-size:20px}"));
        setWindowIcon(icon(FluentIcon::GitHub));

        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            label->setText(QStringLiteral("System tray is not available on this platform/session"));
            return;
        }

        m_menu = new SystemTrayMenu(this);
        m_menu->addAction(QStringLiteral("🎤   Sing"));
        m_menu->addAction(QStringLiteral("🕺   Dance"));
        m_menu->addAction(QStringLiteral("🤘   RAP"));
        m_menu->addAction(QStringLiteral("🎶   Music"));
        m_menu->addAction(QStringLiteral("🏀   Basketball"));

        m_trayIcon = new QSystemTrayIcon(windowIcon(), this);
        m_trayIcon->setToolTip(QStringLiteral("FluentQtWidgets"));
        m_trayIcon->setContextMenu(m_menu);
        m_trayIcon->show();
    }

  private:
    SystemTrayMenu *m_menu = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
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
