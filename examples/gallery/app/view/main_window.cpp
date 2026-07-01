#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"
#include "../../GalleryTranslator.h"
#include "../../common/Icon.h"
#include "../../components/GalleryComponents.h"

#include <QtCore/QPointer>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtWidgets/QApplication>

using namespace FluentQt;

namespace {

QIcon galleryNavIcon(GalleryIcon icon)
{
    return QIcon(galleryIconPath(icon));
}

} // namespace

GalleryWindow::GalleryWindow(QWidget *parent, bool deferPageLoad) : FluentWindow(parent)
{
    setWindowTitle(QStringLiteral("FluentQtWidgets Gallery"));
    setWindowIcon(QIcon(QStringLiteral(":/gallery/images/logo.png")));
    setMinimumWidth(760);
    resize(960, 780);
    navigationInterface()->setAcrylicEnabled(true);

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
        addSubInterface(createIconPage(), galleryNavIcon(GalleryIcon::EmojiTabSymbols), navTx("Icons"),
                        QStringLiteral("iconInterface"));
        return true;
    case 1:
        navigationInterface()->addSeparator();
        addSubInterface(createChartPage(), icon(FluentIcon::PieSingle), navTx("Charts"),
                        QStringLiteral("chartInterface"));
        return true;
    case 2:
        addSubInterface(createBasicInputPage(), icon(FluentIcon::Checkbox), navTx("Basic input"),
                        QStringLiteral("basicInputInterface"));
        return true;
    case 3:
        addSubInterface(createDateTimePage(), icon(FluentIcon::DateTime), navTx("Date & time"),
                        QStringLiteral("dateTimeInterface"));
        return true;
    case 4:
        addSubInterface(createDialogPage(), icon(FluentIcon::Message), navTx("Dialogs & flyouts"),
                        QStringLiteral("dialogInterface"));
        return true;
    case 5:
        addSubInterface(createLayoutPage(), icon(FluentIcon::Layout), navTx("Layout"), QStringLiteral("layoutInterface"));
        return true;
    case 6:
        addSubInterface(createMaterialPage(), icon(FluentIcon::Palette), navTx("Material"),
                        QStringLiteral("materialInterface"));
        return true;
    case 7:
        addSubInterface(createMenuPage(), galleryNavIcon(GalleryIcon::Menu), navTx("Menus & toolbars"),
                        QStringLiteral("menuInterface"));
        return true;
    case 8:
        addSubInterface(createNavigationPage(), icon(FluentIcon::Menu), navTx("Navigation"),
                        QStringLiteral("navigationViewInterface"));
        return true;
    case 9:
        addSubInterface(createScrollPage(), icon(FluentIcon::Scroll), navTx("Scrolling"),
                        QStringLiteral("scrollInterface"));
        return true;
    case 10:
        addSubInterface(createStatusInfoPage(), icon(FluentIcon::Chat), navTx("Status & info"),
                        QStringLiteral("statusInfoInterface"));
        return true;
    case 11:
        addSubInterface(createTextPage(), galleryNavIcon(GalleryIcon::Text), navTx("Text"),
                        QStringLiteral("textInterface"));
        return true;
    case 12:
        addSubInterface(createViewsPage(), galleryNavIcon(GalleryIcon::Grid), navTx("View"),
                        QStringLiteral("viewInterface"));
        return true;
    case 13:
        addSubInterface(createSettingsPage(), icon(FluentIcon::Setting), mainTx("Settings"),
                        QStringLiteral("settingInterface"), NavigationItemPosition::Bottom);
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
