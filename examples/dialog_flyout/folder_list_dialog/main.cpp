#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *button = new PrimaryPushButton(QStringLiteral("Click Me"), this);
        button->move(352, 300);
        connect(button, &QPushButton::clicked, this, &Demo::showDialog);
        resize(800, 720);
    }

  private:
    void showDialog()
    {
        QStringList folderPaths;
        const QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        if (!musicPath.isEmpty()) {
            folderPaths << musicPath;
        }

        const QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        if (!homePath.isEmpty() && !folderPaths.contains(homePath)) {
            folderPaths << homePath;
        }

        if (folderPaths.isEmpty()) {
            folderPaths << QDir::homePath();
        }

        FolderListDialog dialog(folderPaths, QStringLiteral("Build your collection from your local music files"),
                                QStringLiteral("Right now, we're watching these folders:"), this);
        connect(&dialog, &FolderListDialog::folderChanged, this,
                [](const QStringList &folders) { qDebug() << folders; });
        dialog.exec();
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
