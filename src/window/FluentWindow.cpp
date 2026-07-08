#include <FluentQtWidgets/Window/FluentWindow.h>

#include <FluentQtWidgets/Navigation/NavigationBar.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/NavigationInterface.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Window/SplashScreen.h>

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtCore/QUuid>
#include <QtCore/QOperatingSystemVersion>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QWindow>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
#include <QtGui/QRegion>
#endif

namespace FluentQt {

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
static QPoint eventGlobalPos(const QMouseEvent *event)
{
    return event ? event->globalPos() : QPoint();
}
#else
static QPoint eventGlobalPos(const QMouseEvent *event)
{
    return event ? event->globalPosition().toPoint() : QPoint();
}
#endif

#if defined(Q_OS_MACOS)
namespace Private {
void updateMacOSWindowCorner(QWidget *widget, int radius);
}
#endif

namespace {

constexpr int kWindowCornerRadius = 8;

void configureRoundedWindowBackground(QWidget *widget)
{
    if (!widget) {
        return;
    }

#if defined(Q_OS_MACOS)
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
#endif
}

void configureContentBackground(QWidget *widget)
{
    if (!widget) {
        return;
    }

#if defined(Q_OS_MACOS)
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
    widget->setAttribute(Qt::WA_StyledBackground, false);
#else
    widget->setAttribute(Qt::WA_StyledBackground, true);
#endif
}

void updateRoundedWindowMask(QWidget *widget)
{
    if (!widget) {
        return;
    }

#if defined(Q_OS_WIN)
    widget->clearMask();
#elif defined(Q_OS_MACOS)
    widget->clearMask();
    const bool rounded = !widget->isMaximized() && !widget->isFullScreen() && widget->width() > 0 && widget->height() > 0;
    Private::updateMacOSWindowCorner(widget, rounded ? kWindowCornerRadius : 0);
#else
    if (widget->isMaximized() || widget->isFullScreen() || widget->width() <= 0 || widget->height() <= 0) {
        widget->clearMask();
        return;
    }

    QPainterPath path;
    path.addRoundedRect(QRectF(widget->rect()), kWindowCornerRadius, kWindowCornerRadius);
    widget->setMask(QRegion(path.toFillPolygon().toPolygon()));
#endif
}

void setTitleBarNavigationBackground(FluentTitleBar *titleBar, bool visible)
{
    if (!titleBar) {
        return;
    }

    titleBar->setProperty("navigationBackground", visible);
    FluentStyleSheet::polish(titleBar);
}

#if defined(Q_OS_WIN)
void enableWindowAnimation(QWidget *widget)
{
    if (!widget) {
        return;
    }

    HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_THICKFRAME;
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
}

constexpr UINT kDwmSystemBackdropTypeAttribute = []() constexpr
{
#ifdef DWMWA_SYSTEMBACKDROP_TYPE
    return DWMWA_SYSTEMBACKDROP_TYPE;
#else
    return 38; // fallback for SDKs that do not define DWMWA_SYSTEMBACKDROP_TYPE
#endif
}();

constexpr UINT kDwmMicaEffectAttribute = []() constexpr
{
#ifdef DWMWA_MICA_EFFECT
    return DWMWA_MICA_EFFECT;
#else
    return 1029; // fallback for SDKs that do not define DWMWA_MICA_EFFECT
#endif
}();

constexpr UINT kDwmUseImmersiveDarkModeAttribute = 20; // DWMWA_USE_IMMERSIVE_DARK_MODE

constexpr UINT kDwmWindowCornerPreferenceAttribute = []() constexpr
{
#ifdef DWMWA_WINDOW_CORNER_PREFERENCE
    return DWMWA_WINDOW_CORNER_PREFERENCE;
#else
    return 33; // DWMWA_WINDOW_CORNER_PREFERENCE
#endif
}();

constexpr int kDwmBackdropNone = []() constexpr
{
#ifdef DWMSBT_NONE
    return DWMSBT_NONE;
#else
    return 1; // DWMSBT_NONE
#endif
}();

constexpr int kDwmBackdropMainWindow = []() constexpr
{
#ifdef DWMSBT_MAINWINDOW
    return DWMSBT_MAINWINDOW;
#else
    return 2; // DWMSBT_MAINWINDOW
#endif
}();

constexpr int kDwmWindowCornerRound = []() constexpr
{
#ifdef DWMWCP_ROUND
    return DWMWCP_ROUND;
#else
    return 2; // DWMWCP_ROUND
#endif
}();

constexpr int kAccentDisabled = 0;
constexpr int kAccentEnableHostBackdrop = 5;
constexpr DWORD kWindowCompositionAttributeAccentPolicy = 19;
constexpr DWORD kWindowCompositionAttributeUseDarkModeColors = 26;

struct AccentPolicy {
    int accentState = kAccentDisabled;
    int accentFlags = 0;
    DWORD gradientColor = 0;
    int animationId = 0;
};

struct WindowCompositionAttributeData {
    DWORD attribute = 0;
    PVOID data = nullptr;
    SIZE_T sizeOfData = 0;
};

using SetWindowCompositionAttributeProc = BOOL(WINAPI *)(HWND, WindowCompositionAttributeData *);

SetWindowCompositionAttributeProc setWindowCompositionAttributeProc()
{
    static const auto proc = reinterpret_cast<SetWindowCompositionAttributeProc>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
    return proc;
}

void setWindowAccentPolicy(HWND hwnd, int accentState)
{
    if (!hwnd) {
        return;
    }

    auto proc = setWindowCompositionAttributeProc();
    if (!proc) {
        return;
    }

    AccentPolicy accent;
    accent.accentState = accentState;

    WindowCompositionAttributeData data;
    data.attribute = kWindowCompositionAttributeAccentPolicy;
    data.data = &accent;
    data.sizeOfData = sizeof(accent);
    proc(hwnd, &data);
}

void setWindowCompositionDarkMode(HWND hwnd, bool darkMode)
{
    if (!hwnd) {
        return;
    }

    auto proc = setWindowCompositionAttributeProc();
    if (!proc) {
        return;
    }

    BOOL enabled = darkMode ? TRUE : FALSE;

    WindowCompositionAttributeData data;
    data.attribute = kWindowCompositionAttributeUseDarkModeColors;
    data.data = &enabled;
    data.sizeOfData = sizeof(enabled);
    proc(hwnd, &data);
}

void extendFrameIntoClientArea(HWND hwnd, bool enabled)
{
    if (!hwnd) {
        return;
    }

    MARGINS margins = enabled ? MARGINS{16777215, 16777215, 0, 0} : MARGINS{0, 0, 0, 0};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

void syncDwmWindowAttributes(QWidget *widget, bool micaEnabled, bool refreshMicaBackdrop = false)
{
    if (!widget) {
        return;
    }

    HWND hwnd = reinterpret_cast<HWND>(widget->winId());

    if (refreshMicaBackdrop && micaEnabled) {
        const int noBackdrop = kDwmBackdropNone;
        const BOOL noMica = FALSE;
        DwmSetWindowAttribute(hwnd, kDwmSystemBackdropTypeAttribute, &noBackdrop, sizeof(noBackdrop));
        DwmSetWindowAttribute(hwnd, kDwmMicaEffectAttribute, &noMica, sizeof(noMica));
    }

    const BOOL darkMode = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? TRUE : FALSE;
    setWindowCompositionDarkMode(hwnd, darkMode == TRUE);
    DwmSetWindowAttribute(hwnd, kDwmUseImmersiveDarkModeAttribute, &darkMode, sizeof(darkMode));

    const int cornerPreference = kDwmWindowCornerRound;
    DwmSetWindowAttribute(hwnd, kDwmWindowCornerPreferenceAttribute, &cornerPreference, sizeof(cornerPreference));

    setWindowAccentPolicy(hwnd, micaEnabled ? kAccentEnableHostBackdrop : kAccentDisabled);
    extendFrameIntoClientArea(hwnd, micaEnabled);

    const int backdrop = micaEnabled ? kDwmBackdropMainWindow : kDwmBackdropNone;
    DwmSetWindowAttribute(hwnd, kDwmSystemBackdropTypeAttribute, &backdrop, sizeof(backdrop));
    const BOOL mica = micaEnabled ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, kDwmMicaEffectAttribute, &mica, sizeof(mica));
}
#else
void enableWindowAnimation(QWidget *) {}

void syncDwmWindowAttributes(QWidget *, bool, bool = false) {}
#endif

QColor windowBackgroundColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(32, 32, 32) : QColor(249, 249, 249);
}

QColor micaHitTestBackgroundColor()
{
    return QColor(0, 0, 0, 0);
}

} // namespace

bool isMicaEffectAvailable()
{
#if defined(Q_OS_WIN)
    QOperatingSystemVersion ver = QOperatingSystemVersion::current();
    if (ver >= QOperatingSystemVersion::Windows11) {
        return true;
    }
#endif
    return false;
}

// ============================================================
// FluentWidget
// ============================================================

FluentWidget::FluentWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAutoFillBackground(false);
    configureRoundedWindowBackground(this);
    FluentStyleSheet::setRole(this, QStringLiteral("FluentWidget"));

    m_frameless = new FramelessWindowHelper(this);
    m_titleBar = new FluentTitleBar(this);
    m_frameless->setTitleBar(m_titleBar);

    connect(ThemeManager::instance(), &ThemeManager::themeChangedFinished, this, [this]() {
        syncDwmWindowAttributes(this, m_isMicaEnabled);
        update();
        QTimer::singleShot(100, this, [this]() {
            syncDwmWindowAttributes(this, m_isMicaEnabled, true);
            update();
        });
    });

    setMicaEffectEnabled(true);
    updateTitleBarGeometry();
    updateWindowMask();
    enableWindowAnimation(this);
    syncDwmWindowAttributes(this, m_isMicaEnabled);
}

FluentTitleBar *FluentWidget::titleBar() const { return m_titleBar; }

bool FluentWidget::isMicaEffectEnabled() const { return m_isMicaEnabled; }

bool FluentWidget::isSystemTitleBarButtonVisible() const { return m_systemTitleBarButtonVisible; }

QColor FluentWidget::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor FluentWidget::darkBackgroundColor() const { return m_darkBackgroundColor; }

QColor FluentWidget::backgroundColor() const
{
    if (m_isMicaEnabled) {
        return micaHitTestBackgroundColor();
    }
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? m_darkBackgroundColor
                                                                     : m_lightBackgroundColor;
}

QRect FluentWidget::systemTitleBarRect(const QSize &size) const
{
#if defined(Q_OS_MACOS)
    return QRect(0, isFullScreen() ? 0 : 2, 75, size.height());
#else
    return QRect(size.width() - 75, isFullScreen() ? 0 : 8, 75, size.height());
#endif
}

void FluentWidget::setMicaEffectEnabled(bool enabled)
{
    const bool nextEnabled = enabled && isMicaEffectAvailable();
    if (m_isMicaEnabled == nextEnabled) {
        syncDwmWindowAttributes(this, m_isMicaEnabled);
        update();
        return;
    }
    m_isMicaEnabled = nextEnabled;

    syncDwmWindowAttributes(this, m_isMicaEnabled);
    update();
}

void FluentWidget::setSystemTitleBarButtonVisible(bool visible)
{
    m_systemTitleBarButtonVisible = visible;
}

void FluentWidget::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

void FluentWidget::setTitleBar(FluentTitleBar *titleBar)
{
    if (!titleBar || titleBar == m_titleBar) {
        return;
    }
    if (m_titleBar) {
        m_titleBar->hide();
        m_titleBar->deleteLater();
    }
    m_titleBar = titleBar;
    m_titleBar->setParent(this);
    m_titleBar->show();
    if (m_frameless) {
        m_frameless->setTitleBar(m_titleBar);
    }
    updateTitleBarGeometry();
}

void FluentWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (m_frameless) {
            m_frameless->handleWindowStateChange();
        }
        updateWindowMask();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool FluentWidget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool FluentWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    if (m_frameless && m_frameless->handleNativeEvent(eventType, message, result)) {
        return true;
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void FluentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    painter.drawRect(rect());
#else
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF backgroundRect = isMaximized() || isFullScreen() ? QRectF(rect()) : QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath path;
    if (isMaximized() || isFullScreen()) {
        path.addRect(backgroundRect);
    } else {
        path.addRoundedRect(backgroundRect, kWindowCornerRadius, kWindowCornerRadius);
    }
    painter.drawPath(path);
#endif
}

void FluentWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateTitleBarGeometry();
    updateWindowMask();
}

#if !defined(Q_OS_WIN)
void FluentWidget::mousePressEvent(QMouseEvent *event)
{
    if (event && m_frameless) {
        m_frameless->handleMousePress(eventGlobalPos(event), event->button());
    }
    if (!m_frameless || !m_frameless->isProcessingFramelessAction()) {
        QWidget::mousePressEvent(event);
    }
}

void FluentWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!event) {
        return;
    }
    if (m_frameless) {
        m_frameless->handleMouseMove(eventGlobalPos(event));
    }
    QWidget::mouseMoveEvent(event);
}

void FluentWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_frameless) {
        m_frameless->handleMouseRelease();
    }
    QWidget::mouseReleaseEvent(event);
}
#endif

void FluentWidget::updateTitleBarGeometry()
{
    if (!m_titleBar) {
        return;
    }
    m_titleBar->move(0, 0);
    m_titleBar->resize(width(), m_titleBar->height());
    m_titleBar->raise();
}

void FluentWidget::updateWindowMask()
{
    updateRoundedWindowMask(this);
#if !defined(Q_OS_WIN)
    update();
#endif
}

// ============================================================
// FluentWindow
// ============================================================

FluentWindow::FluentWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAutoFillBackground(false);
    configureRoundedWindowBackground(this);
    setMinimumSize(900, 620);
    FluentStyleSheet::setRole(this, QStringLiteral("FluentWindow"));

    m_frameless = new FramelessWindowHelper(this);
    m_history = new NavigationHistory(this);
    m_titleBar = new FluentTitleBar(this);
    setTitleBarNavigationBackground(m_titleBar, true);
    m_frameless->setTitleBar(m_titleBar);

    m_container = new QWidget(this);
    m_container->setObjectName(QStringLiteral("FluentWindowContent"));
    configureContentBackground(m_container);
    auto *layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigation = new NavigationInterface(m_container, true);
    m_navigation->setContentTopMargin(m_contentTopMargin);
    layout->addWidget(m_navigation);
    setCentralWidget(m_container);

    connect(m_navigation, &NavigationInterface::currentRouteKeyChanged, this,
            &FluentWindow::onCurrentRouteKeyChanged);
    connect(m_navigation->navigationPanel(), &NavigationPanel::returnRequested, this, &FluentWindow::onReturnRequested);
    connect(m_navigation->navigationPanel(), &NavigationPanel::displayModeChanged, this,
            [this](NavigationDisplayMode) { m_titleBar->raise(); });
    connect(m_history, &NavigationHistory::emptyChanged, this, [this](bool isEmpty) {
        if (NavigationToolButton *button = m_navigation->navigationPanel()->returnButton()) {
            button->setDisabled(isEmpty);
        }
    });
    connect(ThemeManager::instance(), &ThemeManager::themeChangedFinished, this, [this]() {
        syncDwmWindowAttributes(this, m_isMicaEnabled);
        update();
        QTimer::singleShot(100, this, [this]() {
            syncDwmWindowAttributes(this, m_isMicaEnabled, true);
            update();
        });
    });

    m_titleBar->raise();
    updateTitleBarGeometry();
    updateWindowMask();
    enableWindowAnimation(this);
    syncDwmWindowAttributes(this, m_isMicaEnabled);
}

NavigationInterface *FluentWindow::navigationInterface() const { return m_navigation; }

NavigationHistory *FluentWindow::navigationHistory() const { return m_history; }

FluentTitleBar *FluentWindow::titleBar() const { return m_titleBar; }

SplashScreen *FluentWindow::splashScreen() const { return m_splashScreen; }

int FluentWindow::addPage(QWidget *page, const QString &title, const QIcon &icon)
{
    return m_navigation->addPage(page, title, icon);
}

int FluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                                  bool isTransparent)
{
    return addSubInterface(widget, icon, text, routeKey, NavigationItemPosition::Scroll, isTransparent);
}

int FluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                                  NavigationItemPosition position, bool isTransparent)
{
    return addSubInterface(widget, icon, text, routeKey, position, QString(), isTransparent);
}

int FluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                                  NavigationItemPosition position, const QString &parentRouteKey, bool isTransparent)
{
    if (widget) {
        widget->setProperty("isStackedTransparent", isTransparent);
    }

    const int index = m_navigation->addPage(widget, text, icon, routeKey, position, parentRouteKey);
    if (index == 0 && widget) {
        const QString key = m_navigation->routeKey(index);
        m_history->setDefaultRouteKey(m_navigation->stackedWidget(), key);
    }

    updateStackedBackground();
    return index;
}

bool FluentWindow::switchTo(QWidget *widget)
{
    const int index = m_navigation->indexOf(widget);
    if (index < 0) {
        return false;
    }

    m_navigation->setCurrentIndex(index);
    return true;
}

bool FluentWindow::switchTo(const QString &routeKey)
{
    if (!m_navigation->contains(routeKey)) {
        return false;
    }

    m_navigation->setCurrentRouteKey(routeKey);
    return true;
}

bool FluentWindow::removeInterface(QWidget *widget, bool deleteWidget)
{
    const QString routeKey = m_navigation->routeKey(m_navigation->indexOf(widget));
    const bool removed = m_navigation->removePage(m_navigation->indexOf(widget), deleteWidget);
    if (removed && !routeKey.isEmpty()) {
        m_history->remove(m_navigation->stackedWidget(), routeKey);
    }
    return removed;
}

bool FluentWindow::removeInterface(const QString &routeKey, bool deleteWidget)
{
    const bool removed = m_navigation->removePage(routeKey, deleteWidget);
    if (removed) {
        m_history->remove(m_navigation->stackedWidget(), routeKey);
    }
    return removed;
}

bool FluentWindow::goBack()
{
    if (!canGoBack()) {
        return false;
    }

    m_suppressHistoryPush = true;
    m_history->pop(m_navigation->stackedWidget());
    m_navigation->setCurrentRouteKey(m_history->topRouteKey(m_navigation->stackedWidget()));
    m_suppressHistoryPush = false;
    updateStackedBackground();
    return true;
}

bool FluentWindow::canGoBack() const { return m_history->canGoBack(m_navigation->stackedWidget()); }

QStackedWidget *FluentWindow::stackedWidget() const { return m_navigation->stackedWidget(); }

QWidget *FluentWindow::currentInterface() const { return m_navigation->currentWidget(); }

QString FluentWindow::currentRouteKey() const { return m_navigation->currentRouteKey(); }

bool FluentWindow::isMicaEffectEnabled() const { return m_isMicaEnabled; }

void FluentWindow::setMicaEffectEnabled(bool enabled)
{
    const bool nextEnabled = enabled && isMicaEffectAvailable();
    if (m_isMicaEnabled == nextEnabled) {
        syncDwmWindowAttributes(this, m_isMicaEnabled);
        update();
        return;
    }
    m_isMicaEnabled = nextEnabled;

    syncDwmWindowAttributes(this, m_isMicaEnabled);
    update();
}

void FluentWindow::setSplashScreen(SplashScreen *splashScreen)
{
    if (m_splashScreen == splashScreen) {
        return;
    }

    if (m_splashScreen) {
        m_splashScreen->hide();
    }

    m_splashScreen = splashScreen;
    if (m_splashScreen) {
        m_splashScreen->setParent(this);
        m_splashScreen->resize(size());
        m_splashScreen->show();
        m_splashScreen->raise();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool FluentWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool FluentWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    if (m_frameless && m_frameless->handleNativeEvent(eventType, message, result)) {
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

#if !defined(Q_OS_WIN)
void FluentWindow::mousePressEvent(QMouseEvent *event)
{
    if (event && m_frameless) {
        m_frameless->handleMousePress(eventGlobalPos(event), event->button());
    }
    if (!m_frameless || !m_frameless->isProcessingFramelessAction()) {
        QMainWindow::mousePressEvent(event);
    }
}

void FluentWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event) {
        return;
    }
    if (m_frameless) {
        m_frameless->handleMouseMove(eventGlobalPos(event));
    }
    QMainWindow::mouseMoveEvent(event);
}

void FluentWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_frameless) {
        m_frameless->handleMouseRelease();
    }
    QMainWindow::mouseReleaseEvent(event);
}
#endif

void FluentWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (m_frameless) {
            m_frameless->handleWindowStateChange();
        }
        updateWindowMask();
    }
}

void FluentWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    const QColor background = m_isMicaEnabled ? micaHitTestBackgroundColor() : windowBackgroundColor();
    painter.setBrush(background);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    painter.drawRect(rect());
#else
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF backgroundRect = isMaximized() || isFullScreen() ? QRectF(rect()) : QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath path;
    if (isMaximized() || isFullScreen()) {
        path.addRect(backgroundRect);
    } else {
        path.addRoundedRect(backgroundRect, kWindowCornerRadius, kWindowCornerRadius);
    }
    painter.drawPath(path);
#endif
}

void FluentWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateTitleBarGeometry();
    updateWindowMask();
    if (m_splashScreen && m_splashScreen->isVisible()) {
        m_splashScreen->resize(size());
    }
}

void FluentWindow::setTitleBarLeftMargin(int margin)
{
    m_titleBarLeftMargin = margin;
    updateTitleBarGeometry();
}

void FluentWindow::setContentTopMargin(int margin)
{
    m_contentTopMargin = qMax(0, margin);
    if (m_navigation) {
        m_navigation->setContentTopMargin(m_contentTopMargin);
    }
}

int FluentWindow::titleBarLeftMargin() const { return m_titleBarLeftMargin; }

int FluentWindow::contentTopMargin() const { return m_contentTopMargin; }

void FluentWindow::pushHistory(const QString &routeKey)
{
    if (routeKey.isEmpty()) {
        return;
    }

    m_history->push(m_navigation->stackedWidget(), routeKey);
}

void FluentWindow::onCurrentRouteKeyChanged(const QString &routeKey)
{
    if (!m_suppressHistoryPush) {
        pushHistory(routeKey);
    }

    if (m_navigation->navigationPanel()->currentRouteKey() != routeKey) {
        m_navigation->navigationPanel()->setCurrentItem(routeKey);
    }

    updateStackedBackground();
}

void FluentWindow::onReturnRequested() { goBack(); }

void FluentWindow::updateTitleBarGeometry()
{
    const int left = m_titleBarLeftMargin >= 0 ? m_titleBarLeftMargin : NavigationPanel::kCompactWidth;
    m_titleBar->move(left, 0);
    m_titleBar->resize(qMax(0, width() - left), m_titleBar->height());
    m_titleBar->raise();
}

void FluentWindow::updateStackedBackground()
{
    QWidget *current = m_navigation->currentWidget();
    if (!current) {
        return;
    }

    const bool isTransparent = current->property("isStackedTransparent").toBool();
    QStackedWidget *stack = m_navigation->stackedWidget();
    if (stack->property("isTransparent").toBool() == isTransparent) {
        return;
    }

    stack->setProperty("isTransparent", isTransparent);
    if (stack->style()) {
        stack->style()->unpolish(stack);
        stack->style()->polish(stack);
    }
}

void FluentWindow::updateWindowMask()
{
    updateRoundedWindowMask(this);
#if !defined(Q_OS_WIN)
    update();
#endif
}

// ============================================================
// SplitFluentWindow
// ============================================================

SplitFluentWindow::SplitFluentWindow(QWidget *parent) : FluentWindow(parent)
{
    setTitleBarNavigationBackground(titleBar(), false);
    setTitleBarLeftMargin(0);
    setContentTopMargin(0);
    titleBar()->raise();
}

// ============================================================
// MSFluentWindow
// ============================================================

MSFluentWindow::MSFluentWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAutoFillBackground(false);
    configureRoundedWindowBackground(this);
    setMinimumSize(900, 620);
    FluentStyleSheet::setRole(this, QStringLiteral("MSFluentWindow"));

    m_frameless = new FramelessWindowHelper(this);
    m_titleBar = new FluentTitleBar(this);
    m_frameless->setTitleBar(m_titleBar);

    m_container = new QWidget(this);
    m_container->setObjectName(QStringLiteral("FluentWindowContent"));
    configureContentBackground(m_container);

    auto *layout = new QHBoxLayout(m_container);
    layout->setContentsMargins(0, 48, 0, 0);
    layout->setSpacing(0);

    m_navigationBar = new NavigationBar(m_container);
    m_stackedWidget = new QStackedWidget(m_container);
    m_stackedWidget->setObjectName(QStringLiteral("NavigationStack"));
    layout->addWidget(m_navigationBar);
    layout->addWidget(m_stackedWidget, 1);
    setCentralWidget(m_container);

    connect(m_navigationBar, &NavigationBar::itemClicked, this, [this](const QString &routeKey) {
        switchTo(routeKey);
    });
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this, [this](int index) {
        const QString key = index >= 0 && index < m_routeOrder.size() ? m_routeOrder.at(index) : QString();
        if (!key.isEmpty() && m_navigationBar->currentRouteKey() != key) {
            m_navigationBar->setCurrentItem(key);
        }
        updateStackedBackground();
    });
    connect(ThemeManager::instance(), &ThemeManager::themeChangedFinished, this, [this]() {
        syncDwmWindowAttributes(this, m_isMicaEnabled);
        update();
        QTimer::singleShot(100, this, [this]() {
            syncDwmWindowAttributes(this, m_isMicaEnabled, true);
            update();
        });
    });

    m_titleBar->raise();
    updateTitleBarGeometry();
    updateWindowMask();
    enableWindowAnimation(this);
    syncDwmWindowAttributes(this, m_isMicaEnabled);
}

NavigationBar *MSFluentWindow::navigationInterface() const { return m_navigationBar; }

FluentTitleBar *MSFluentWindow::titleBar() const { return m_titleBar; }

QStackedWidget *MSFluentWindow::stackedWidget() const { return m_stackedWidget; }

QWidget *MSFluentWindow::currentInterface() const { return m_stackedWidget->currentWidget(); }

QString MSFluentWindow::currentRouteKey() const
{
    const int index = m_stackedWidget->currentIndex();
    return index >= 0 && index < m_routeOrder.size() ? m_routeOrder.at(index) : QString();
}

bool MSFluentWindow::isMicaEffectEnabled() const { return m_isMicaEnabled; }

void MSFluentWindow::setMicaEffectEnabled(bool enabled)
{
    const bool nextEnabled = enabled && isMicaEffectAvailable();
    if (m_isMicaEnabled == nextEnabled) {
        syncDwmWindowAttributes(this, m_isMicaEnabled);
        update();
        return;
    }
    m_isMicaEnabled = nextEnabled;

    syncDwmWindowAttributes(this, m_isMicaEnabled);
    update();
}

int MSFluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text,
                                    const QIcon &selectedIcon, NavigationItemPosition position, bool isTransparent)
{
    if (!widget) {
        return -1;
    }

    const QString routeKey = ensureRouteKey(widget);
    if (routeKey.isEmpty() || m_routeOrder.contains(routeKey) || m_navigationBar->contains(routeKey)) {
        return -1;
    }

    widget->setProperty("isStackedTransparent", isTransparent);
    widget->setProperty("routeKey", routeKey);
    widget->setObjectName(routeKey);

    const int index = m_stackedWidget->addWidget(widget);
    m_routeOrder.append(routeKey);
    m_navigationBar->addItem(routeKey, icon, text, true, selectedIcon, position);

    if (index == 0) {
        m_stackedWidget->setCurrentIndex(0);
        m_navigationBar->setCurrentItem(routeKey);
    }

    updateStackedBackground();
    return index;
}

bool MSFluentWindow::switchTo(QWidget *widget)
{
    const int index = m_stackedWidget->indexOf(widget);
    if (index < 0) {
        return false;
    }

    m_stackedWidget->setCurrentIndex(index);
    const QString key = m_routeOrder.value(index);
    if (!key.isEmpty()) {
        m_navigationBar->setCurrentItem(key);
    }
    return true;
}

bool MSFluentWindow::switchTo(const QString &routeKey)
{
    const int index = m_routeOrder.indexOf(routeKey);
    if (index < 0) {
        return false;
    }

    m_stackedWidget->setCurrentIndex(index);
    m_navigationBar->setCurrentItem(routeKey);
    return true;
}

bool MSFluentWindow::removeInterface(QWidget *widget, bool deleteWidget)
{
    const int index = m_stackedWidget->indexOf(widget);
    if (index < 0) {
        return false;
    }
    return removeInterface(m_routeOrder.value(index), deleteWidget);
}

bool MSFluentWindow::removeInterface(const QString &routeKey, bool deleteWidget)
{
    const int index = m_routeOrder.indexOf(routeKey);
    if (index < 0) {
        return false;
    }

    QWidget *page = m_stackedWidget->widget(index);
    m_navigationBar->removeWidget(routeKey);
    m_routeOrder.removeAt(index);
    m_stackedWidget->removeWidget(page);

    if (deleteWidget) {
        page->deleteLater();
    } else {
        page->setParent(nullptr);
    }

    if (m_stackedWidget->count() > 0) {
        const int nextIndex = qMin(index, m_stackedWidget->count() - 1);
        m_stackedWidget->setCurrentIndex(nextIndex);
        m_navigationBar->setCurrentItem(m_routeOrder.value(nextIndex));
    }
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool MSFluentWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool MSFluentWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    if (m_frameless && m_frameless->handleNativeEvent(eventType, message, result)) {
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

#if !defined(Q_OS_WIN)
void MSFluentWindow::mousePressEvent(QMouseEvent *event)
{
    if (event && m_frameless) {
        m_frameless->handleMousePress(eventGlobalPos(event), event->button());
    }
    if (!m_frameless || !m_frameless->isProcessingFramelessAction()) {
        QMainWindow::mousePressEvent(event);
    }
}

void MSFluentWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event) {
        return;
    }
    if (m_frameless) {
        m_frameless->handleMouseMove(eventGlobalPos(event));
    }
    QMainWindow::mouseMoveEvent(event);
}

void MSFluentWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_frameless) {
        m_frameless->handleMouseRelease();
    }
    QMainWindow::mouseReleaseEvent(event);
}
#endif

void MSFluentWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (m_frameless) {
            m_frameless->handleWindowStateChange();
        }
        updateWindowMask();
    }
}

void MSFluentWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    const QColor background = m_isMicaEnabled ? micaHitTestBackgroundColor() : windowBackgroundColor();
    painter.setBrush(background);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    painter.drawRect(rect());
#else
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF backgroundRect =
        isMaximized() || isFullScreen() ? QRectF(rect()) : QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath path;
    if (isMaximized() || isFullScreen()) {
        path.addRect(backgroundRect);
    } else {
        path.addRoundedRect(backgroundRect, kWindowCornerRadius, kWindowCornerRadius);
    }
    painter.drawPath(path);
#endif
}

void MSFluentWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateTitleBarGeometry();
    updateWindowMask();
}

QString MSFluentWindow::ensureRouteKey(QWidget *widget) const
{
    if (!widget) {
        return QString();
    }
    return widget->objectName().isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                                          : widget->objectName();
}

void MSFluentWindow::updateTitleBarGeometry()
{
    if (!m_titleBar) {
        return;
    }
    m_titleBar->move(0, 0);
    m_titleBar->resize(width(), m_titleBar->height());
    m_titleBar->raise();
}

void MSFluentWindow::updateStackedBackground()
{
    QWidget *current = currentInterface();
    if (!current || !m_stackedWidget) {
        return;
    }

    const bool isTransparent = current->property("isStackedTransparent").toBool();
    if (m_stackedWidget->property("isTransparent").toBool() == isTransparent) {
        return;
    }

    m_stackedWidget->setProperty("isTransparent", isTransparent);
    if (m_stackedWidget->style()) {
        m_stackedWidget->style()->unpolish(m_stackedWidget);
        m_stackedWidget->style()->polish(m_stackedWidget);
    }
}

void MSFluentWindow::updateWindowMask()
{
    updateRoundedWindowMask(this);
#if !defined(Q_OS_WIN)
    update();
#endif
}

} // namespace FluentQt
