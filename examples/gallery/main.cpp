#include "GalleryWindow.h"
#include "GalleryTranslator.h"

#include "app/update/GalleryUpdateChecker.h"

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <QtCore/QtGlobal>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QSystemTrayIcon>

namespace {

QIcon galleryAppIcon()
{
    return QIcon(QStringLiteral(":/gallery/images/logo.png"));
}

GalleryWindow *currentGalleryWindow()
{
    const QWidgetList windows = QApplication::topLevelWidgets();
    for (QWidget *widget : windows) {
        auto *window = qobject_cast<GalleryWindow *>(widget);
        if (window) {
            return window;
        }
    }
    return nullptr;
}

void showCurrentGalleryWindow()
{
    GalleryWindow *window = currentGalleryWindow();
    if (!window) {
        return;
    }

    if (window->isMinimized()) {
        window->showNormal();
    } else {
        window->show();
    }
    window->raise();
    window->activateWindow();
}

QString normalizedDpiScale(const QString &value)
{
    const QString cleaned = value.trimmed();
    if (cleaned.isEmpty() || cleaned.compare(QStringLiteral("Auto"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Auto");
    }

    bool ok = false;
    const double scale = cleaned.toDouble(&ok);
    if (!ok) {
        return QStringLiteral("Auto");
    }

    if (qAbs(scale - 1.0) < 0.001) {
        return QStringLiteral("1");
    }
    if (qAbs(scale - 1.25) < 0.001) {
        return QStringLiteral("1.25");
    }
    if (qAbs(scale - 1.5) < 0.001) {
        return QStringLiteral("1.5");
    }
    if (qAbs(scale - 1.75) < 0.001) {
        return QStringLiteral("1.75");
    }
    if (qAbs(scale - 2.0) < 0.001) {
        return QStringLiteral("2");
    }
    return QStringLiteral("Auto");
}

QString configuredDpiScale()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QString path =
        QDir(base.isEmpty() ? QDir::currentPath() : base).filePath(QStringLiteral("fluentqtwidgets.json"));
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QStringLiteral("Auto");
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return QStringLiteral("Auto");
    }

    const QJsonObject root = document.object();
    const QJsonObject fluentGroup = root.value(QStringLiteral("FluentQtWidgets")).toObject();
    const QJsonObject mainWindowGroup = root.value(QStringLiteral("MainWindow")).toObject();
    const QJsonValue value = fluentGroup.contains(QStringLiteral("DpiScale"))
                                ? fluentGroup.value(QStringLiteral("DpiScale"))
                                : mainWindowGroup.value(QStringLiteral("DpiScale"));
    return value.isUndefined() ? QStringLiteral("Auto") : normalizedDpiScale(value.toVariant().toString());
}

void applyDpiScaleSetting(const QString &dpiScale)
{
    if (dpiScale.compare(QStringLiteral("Auto"), Qt::CaseInsensitive) == 0 || dpiScale.trimmed().isEmpty()) {
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    } else {
        qputenv("QT_ENABLE_HIGHDPI_SCALING", QByteArrayLiteral("0"));
        qputenv("QT_SCALE_FACTOR", dpiScale.toUtf8());
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("FluentQtWidgets Gallery"));
    QCoreApplication::setOrganizationName(QStringLiteral("FluentQtWidgets"));
    QCoreApplication::setApplicationVersion(FluentQt::libraryVersion());

    applyDpiScaleSetting(configuredDpiScale());

    QApplication app(argc, argv);
    const QIcon appIcon = galleryAppIcon();
    QApplication::setWindowIcon(appIcon);
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    app.setApplicationName(QStringLiteral("FluentQtWidgets Gallery"));
    app.setOrganizationName(QStringLiteral("FluentQtWidgets"));

    FluentQt::FluentConfig::instance()->load();
    GalleryTranslation::installTranslators(&app, FluentQt::FluentConfig::instance()->localeName());
    FluentQt::ThemeManager::instance()->setTheme(FluentQt::FluentConfig::instance()->themeMode());
    FluentQt::ThemeManager::instance()->setAccentColor(FluentQt::FluentConfig::instance()->themeColor());

    const bool deferPageLoad = QCoreApplication::arguments().contains(QStringLiteral("--defer-page-load"));
    auto *window = new GalleryWindow(nullptr, deferPageLoad);
    window->setAttribute(Qt::WA_DeleteOnClose);

    QMenu trayMenu;
    QSystemTrayIcon trayIcon(appIcon);
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QAction *showAction = trayMenu.addAction(QCoreApplication::translate("GalleryTray", "Show Gallery"));
        QObject::connect(showAction, &QAction::triggered, &app, []() { showCurrentGalleryWindow(); });
        trayMenu.addSeparator();
        QAction *quitAction = trayMenu.addAction(QCoreApplication::translate("GalleryTray", "Quit"));
        QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

        trayIcon.setToolTip(app.applicationName());
        trayIcon.setContextMenu(&trayMenu);
        QObject::connect(&trayIcon, &QSystemTrayIcon::activated, &app,
                         [](QSystemTrayIcon::ActivationReason reason) {
                             if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick ||
                                 reason == QSystemTrayIcon::MiddleClick) {
                                 showCurrentGalleryWindow();
                             }
                         });
        trayIcon.show();
    }

    if (FluentQt::FluentConfig::instance()->isAutoUpdateEnabled()) {
        QTimer::singleShot(500, window, [window]() {
            FluentQt::checkGalleryUpdate(window, false, true);
        });
    }

    window->show();
    window->raise();
    window->activateWindow();
    return app.exec();
}
