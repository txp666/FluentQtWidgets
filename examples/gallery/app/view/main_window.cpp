#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"
#include "../../GalleryTranslator.h"
#include "../../components/GalleryComponents.h"

#include <QtCore/QPointer>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtWidgets/QApplication>

using namespace FluentQt;

GalleryWindow::GalleryWindow(QWidget *parent, bool deferPageLoad) : FluentWindow(parent)
{
    setWindowTitle(QStringLiteral("FluentQtWidgets Gallery"));
    setWindowIcon(QIcon(QStringLiteral(":/gallery/images/logo.png")));
    setMinimumWidth(760);
    resize(960, 780);

    // Create and show splash screen
    auto *splash = new SplashScreen(windowIcon(), this);
    splash->setIconSize(QSize(106, 106));
    setSplashScreen(splash);

    const QRect desktop = QApplication::primaryScreen()->availableGeometry();
    move((desktop.width() - width()) / 2, (desktop.height() - height()) / 2);

    auto *home = new HomeInterface(this);
    connect(home, &HomeInterface::sampleCardClicked, this, &GalleryWindow::switchToSample);
    addSubInterface(home, icon(FluentIcon::Home), mainTx("Home"), QStringLiteral("home"));

    if (deferPageLoad) {
        QTimer::singleShot(100, this, &GalleryWindow::addNextDeferredInterface);
    } else {
        populateInterfaces();
        scheduleSplashFinish();
    }
}

GalleryWindow::~GalleryWindow() = default;

bool GalleryWindow::addGalleryInterface(int index)
{
    switch (index) {
    case 0:
        addSubInterface(createIconPage(), icon(FluentIcon::Star), navTx("Icons"), QStringLiteral("iconInterface"));
        return true;
    case 1:
        navigationInterface()->addSeparator();
        addSubInterface(createBasicInputPage(), icon(FluentIcon::Check), navTx("Basic input"),
                        QStringLiteral("basicInputInterface"));
        return true;
    case 2:
        addSubInterface(createDateTimePage(), icon(FluentIcon::Calendar), navTx("Date & time"),
                        QStringLiteral("dateTimeInterface"));
        return true;
    case 3:
        addSubInterface(createDialogPage(), icon(FluentIcon::Info), navTx("Dialogs & flyouts"),
                        QStringLiteral("dialogInterface"));
        return true;
    case 4:
        addSubInterface(createLayoutPage(), icon(FluentIcon::View), navTx("Layout"), QStringLiteral("layoutInterface"));
        return true;
    case 5:
        addSubInterface(createMaterialPage(), icon(FluentIcon::Palette), navTx("Material"),
                        QStringLiteral("materialInterface"));
        return true;
    case 6:
        addSubInterface(createMenuPage(), icon(FluentIcon::More), navTx("Menus & toolbars"),
                        QStringLiteral("menuInterface"));
        return true;
    case 7:
        addSubInterface(createNavigationPage(), icon(FluentIcon::More), navTx("Navigation"),
                        QStringLiteral("navigationViewInterface"));
        return true;
    case 8:
        addSubInterface(createScrollPage(), icon(FluentIcon::Scroll), navTx("Scrolling"),
                        QStringLiteral("scrollInterface"));
        return true;
    case 9:
        addSubInterface(createStatusInfoPage(), icon(FluentIcon::Feedback), navTx("Status & info"),
                        QStringLiteral("statusInfoInterface"));
        return true;
    case 10:
        addSubInterface(createTextPage(), icon(FluentIcon::Font), navTx("Text"), QStringLiteral("textInterface"));
        return true;
    case 11:
        addSubInterface(createViewsPage(), icon(FluentIcon::Folder), navTx("View"), QStringLiteral("viewInterface"));
        return true;
    case 12:
        addSubInterface(createSettingsPage(), icon(FluentIcon::Settings), mainTx("Settings"),
                        QStringLiteral("settings"), NavigationItemPosition::Bottom);
        return true;
    default:
        return false;
    }
}

void GalleryWindow::populateInterfaces()
{
    if (m_interfacesPopulated) {
        return;
    }

    while (addGalleryInterface(m_nextDeferredInterface)) {
        ++m_nextDeferredInterface;
    }
    m_interfacesPopulated = true;
}

void GalleryWindow::addNextDeferredInterface()
{
    if (m_interfacesPopulated) {
        return;
    }

    if (addGalleryInterface(m_nextDeferredInterface)) {
        ++m_nextDeferredInterface;
        QTimer::singleShot(1, this, &GalleryWindow::addNextDeferredInterface);
        return;
    }

    m_interfacesPopulated = true;
    scheduleSplashFinish();
}

void GalleryWindow::scheduleSplashFinish()
{
    if (m_splashFinishScheduled) {
        return;
    }

    if (splashScreen()) {
        m_splashFinishScheduled = true;
        QTimer::singleShot(0, this, [this]() {
            if (auto *splash = splashScreen()) {
                splash->finish();
            }
        });
    }
}

void GalleryWindow::reloadForLanguageChange()
{
    if (m_reloadingLanguage) {
        return;
    }
    m_reloadingLanguage = true;

    const QPointer<GalleryWindow> windowToReload(this);
    QTimer::singleShot(0, this, [windowToReload]() {
        if (!windowToReload) {
            return;
        }

        for (int i = 0; i < 8; ++i) {
            QWidget *popup = QApplication::activePopupWidget();
            if (!popup) {
                break;
            }
            popup->close();
        }
        GalleryTranslation::installTranslators(qApp, FluentConfig::instance()->localeName());

        const QString routeKey = windowToReload->currentRouteKey();
        QRect targetGeometry = windowToReload->normalGeometry();
        if (!targetGeometry.isValid()) {
            targetGeometry = windowToReload->geometry();
        }
        const Qt::WindowStates previousState = windowToReload->windowState();

        auto *newWindow = new GalleryWindow;
        newWindow->setAttribute(Qt::WA_DeleteOnClose);
        if (targetGeometry.isValid()) {
            newWindow->setGeometry(targetGeometry);
        }
        if (!routeKey.isEmpty()) {
            newWindow->switchTo(routeKey);
        }

        if (previousState.testFlag(Qt::WindowFullScreen)) {
            newWindow->showFullScreen();
        } else if (previousState.testFlag(Qt::WindowMaximized)) {
            newWindow->showMaximized();
        } else {
            newWindow->show();
        }

        const bool deleteOldWindow = windowToReload->testAttribute(Qt::WA_DeleteOnClose);
        windowToReload->hide();
        if (deleteOldWindow) {
            windowToReload->deleteLater();
        } else {
            windowToReload->m_reloadingLanguage = false;
        }
    });
}

void GalleryWindow::switchToSample(const QString &routeKey, int index)
{
    if (!switchTo(routeKey)) {
        return;
    }

    auto *gallery = qobject_cast<GalleryInterface *>(currentInterface());
    if (!gallery) {
        return;
    }
    gallery->scrollToCard(index);
}

void GalleryWindow::showEvent(QShowEvent *event)
{
    FluentWindow::showEvent(event);
}

void GalleryWindow::resizeEvent(QResizeEvent *event)
{
    FluentWindow::resizeEvent(event);
    if (auto *s = splashScreen()) {
        s->resize(size());
    }
}

void GalleryWindow::closeEvent(QCloseEvent *event)
{
    FluentWindow::closeEvent(event);
}
