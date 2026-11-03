#include <FluentQtWidgets/Window/FluentWindow.h>

#include <FluentQtWidgets/Navigation/NavigationBar.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/NavigationInterface.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Window/SplashScreen.h>

#include <QtCore/QEvent>
#include <QtCore/QOperatingSystemVersion>
#include <QtCore/QTimer>
#include <QtCore/QUuid>
#include <QtGui/QCursor>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QResizeEvent>
#include <QtGui/QWindow>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <functional>

#include <functional>

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

// Python's title bars place the window icon and title differently: FluentTitleBar starts at its
// left edge, MSFluentTitleBar 20 px in with 2 px before the title, and SplitTitleBar 12 px right of
// the navigation panel's 46 px. The title's side padding is 4, 10 and 5 px.
void setTitleBarIconPlacement(FluentTitleBar *titleBar, int leading, int iconGap, int titlePadding)
{
    auto *layout = titleBar ? qobject_cast<QHBoxLayout *>(titleBar->layout()) : nullptr;
    if (!layout) {
        return;
    }

    layout->setContentsMargins(leading, 0, 0, 0);
    if (iconGap > 0) {
        layout->insertSpacing(layout->indexOf(titleBar->iconLabel()) + 1, iconGap);
    }
    titleBar->titleLabel()->setContentsMargins(titlePadding, 0, titlePadding, 0);
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
    return 38; // Fallback for SDKs that do not define DWMWA_SYSTEMBACKDROP_TYPE
#endif
}();

constexpr UINT kDwmMicaEffectAttribute = []() constexpr
{
#ifdef DWMWA_MICA_EFFECT
    return DWMWA_MICA_EFFECT;
#else
    return 1029; // Fallback for SDKs that do not define DWMWA_MICA_EFFECT
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

// Python's FluentWidget background without Mica: the light or dark colour of the window, which also
// shows through the transparent title bar and navigation interface.
QColor windowBackgroundColor(const QColor &light, const QColor &dark)
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? dark : light;
}

QColor micaHitTestBackgroundColor()
{
    return QColor(0, 0, 0, 0);
}

// Before Windows 11 22H2 (build 22523, which added the system backdrop attribute) Mica only takes a
// new theme when it is set again; later builds cross-fade it to the dark-mode attribute by themselves.
bool micaNeedsRefresh()
{
#if defined(Q_OS_WIN)
    return QOperatingSystemVersion::current() <
           QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 22523);
#else
    return false;
#endif
}

// A theme change can finish several times in a row (the Gallery's theme button sets the theme
// three times); one update of the window's attributes follows the last, once the widgets have their
// new style, so that the backdrop starts changing with them. Setting Mica again where Windows
// recolours it by itself restarts its cross-fade with a jump, a visible flash.
QTimer *createThemeRefresh(QWidget *window, std::function<bool()> micaEnabled)
{
    const bool refreshMica = micaNeedsRefresh();
    auto *timer = new QTimer(window);
    timer->setSingleShot(true);
    timer->setInterval(refreshMica ? 100 : 0);
    QObject::connect(timer, &QTimer::timeout, window, [window, micaEnabled, refreshMica]() {
        syncDwmWindowAttributes(window, micaEnabled(), refreshMica);
        window->update();
    });
    return timer;
}

void beginThemeRefresh(QWidget *window, bool micaEnabled, QTimer *refresh)
{
    if (micaNeedsRefresh()) {
        syncDwmWindowAttributes(window, micaEnabled);
        window->update();
    }
    refresh->start();
}

// Fills the window colour into the page stack's rounded top-left corner, which the content area
// behind it would otherwise show through.
class NavigationCornerBackdrop final : public QWidget
{
  public:
    NavigationCornerBackdrop(QWidget *parent, QStackedWidget *stack, std::function<QColor()> color)
        : QWidget(parent), m_stack(stack), m_color(std::move(color))
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAutoFillBackground(false);
        setFixedSize(12, 12);
        if (m_stack) {
            m_stack->installEventFilter(this);
        }
        connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) { update(); });
        syncGeometry();
        lower();
    }

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_stack && (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
                                   event->type() == QEvent::Show || event->type() == QEvent::DynamicPropertyChange)) {
            syncGeometry();
            update();
        }
        return QWidget::eventFilter(watched, event);
    }

    void paintEvent(QPaintEvent *) override
    {
        if (!m_stack || m_stack->property("isTransparent").toBool()) {
            return;
        }

        constexpr qreal radius = 10.0;
        QPainterPath surface;
        surface.moveTo(0.0, radius);
        surface.arcTo(QRectF(0.0, 0.0, radius * 2.0, radius * 2.0), 180.0, -90.0);
        surface.lineTo(width(), 0.0);
        surface.lineTo(width(), height());
        surface.lineTo(0.0, height());
        surface.closeSubpath();

        QPainterPath outside;
        outside.setFillRule(Qt::OddEvenFill);
        outside.addRect(QRectF(rect()));
        outside.addPath(surface);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.fillPath(outside, m_color());
    }

  private:
    void syncGeometry()
    {
        if (m_stack) {
            move(m_stack->pos());
        }
    }

    QPointer<QStackedWidget> m_stack;
    std::function<QColor()> m_color;
};

} // namespace

bool isMicaEffectAvailable()
{
#if defined(Q_OS_WIN)
    // QOperatingSystemVersion::Windows11 only exists from Qt 6.3; Windows 11 is build 22000.
    if (QOperatingSystemVersion::current() >=
        QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 22000)) {
        return true;
    }
#endif
    return false;
}

// --- FluentWidget ---

FluentWidget::FluentWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAutoFillBackground(false);
    configureRoundedWindowBackground(this);
    FluentStyleSheet::setRole(this, QStringLiteral("FluentWidget"));

    m_frameless = std::make_unique<FramelessWindowHelper>(this);
    m_titleBar = new FluentTitleBar(this);
    m_frameless->setTitleBar(m_titleBar);

    QTimer *themeRefresh = createThemeRefresh(this, [this]() { return m_isMicaEnabled; });
    connect(ThemeManager::instance(), &ThemeManager::themeChangedFinished, this,
            [this, themeRefresh]() { beginThemeRefresh(this, m_isMicaEnabled, themeRefresh); });

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

// --- FluentWindow ---

FluentWindow::FluentWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAutoFillBackground(false);
    configureRoundedWindowBackground(this);
    setMinimumSize(900, 620);
    FluentStyleSheet::setRole(this, QStringLiteral("FluentWindow"));

    m_frameless = std::make_unique<FramelessWindowHelper>(this);
    m_history = new NavigationHistory(this);
    m_titleBar = new FluentTitleBar(this);
    setTitleBarIconPlacement(m_titleBar, 0, 0, 4);
    m_frameless->setTitleBar(m_titleBar);

    m_container = new QWidget(this);
    m_container->setObjectName(QStringLiteral("FluentWindowContent"));
    configureContentBackground(m_container);
    auto *layout = new QHBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Python's FluentWindow: the navigation interface (48 px, or the expanded panel's width) beside
    // the page stack, which sits below the title bar.
    m_navigation = new NavigationInterface(m_container, true, true);
    m_stackedWidget = new PopUpAniStackedWidget(m_container);
    m_stackedWidget->setObjectName(QStringLiteral("NavigationStack"));
    m_stackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(m_stackedWidget, QStringLiteral("NavigationStack"));
    m_widgetLayout = new QHBoxLayout;
    m_widgetLayout->setContentsMargins(0, m_contentTopMargin, 0, 0);
    m_widgetLayout->addWidget(m_stackedWidget);
    layout->addWidget(m_navigation);
    layout->addLayout(m_widgetLayout, 1);
    new NavigationCornerBackdrop(m_container, m_stackedWidget, [this]() { return backgroundColor(); });
    setCentralWidget(m_container);

    connect(m_navigation->panel(), &NavigationPanel::itemClicked, this, [this](const QString &routeKey) {
        if (QWidget *page = findInterface(routeKey)) {
            switchTo(page);
        }
    });
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this, &FluentWindow::onCurrentInterfaceChanged);
    connect(m_navigation->panel(), &NavigationPanel::returnRequested, this, &FluentWindow::onReturnRequested);
    connect(m_navigation, &NavigationInterface::displayModeChanged, this,
            [this](NavigationDisplayMode) { m_titleBar->raise(); });
    connect(m_history, &NavigationHistory::emptyChanged, this, [this](bool isEmpty) {
        if (NavigationToolButton *button = m_navigation->panel()->returnButton()) {
            button->setDisabled(isEmpty);
        }
    });
    QTimer *themeRefresh = createThemeRefresh(this, [this]() { return m_isMicaEnabled; });
    connect(ThemeManager::instance(), &ThemeManager::themeChangedFinished, this,
            [this, themeRefresh]() { beginThemeRefresh(this, m_isMicaEnabled, themeRefresh); });

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

int FluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                                  bool isTransparent)
{
    return addSubInterface(widget, icon, text, routeKey, NavigationItemPosition::Top, isTransparent);
}

int FluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                                  NavigationItemPosition position, bool isTransparent)
{
    return addSubInterface(widget, icon, text, routeKey, position, QString(), isTransparent);
}

// Python keys a page by its objectName; a routeKey given here becomes the objectName.
int FluentWindow::addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                                  NavigationItemPosition position, const QString &parentRouteKey, bool isTransparent)
{
    if (!widget || m_stackedWidget->indexOf(widget) >= 0) {
        return -1;
    }

    QString key = routeKey;
    if (key.isEmpty()) {
        key = widget->objectName();
    }
    if (key.isEmpty()) {
        key = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    if (findInterface(key) || m_navigation->widget(key)) {
        return -1;
    }

    widget->setObjectName(key);
    widget->setProperty("isStackedTransparent", isTransparent);
    const int index = m_stackedWidget->addWidget(widget);
    m_navigation->addItem(key, icon, text, position, parentRouteKey);

    if (m_stackedWidget->count() == 1) {
        m_navigation->setCurrentItem(key);
        m_history->setDefaultRouteKey(m_stackedWidget, key);
    }

    updateStackedBackground();
    return index;
}

bool FluentWindow::switchTo(QWidget *widget)
{
    const int index = m_stackedWidget->indexOf(widget);
    if (index < 0) {
        return false;
    }

    if (index != m_stackedWidget->currentIndex()) {
        if (auto *scrollArea = qobject_cast<QAbstractScrollArea *>(widget)) {
            scrollArea->verticalScrollBar()->setValue(0);
        }
        m_stackedWidget->setCurrentIndex(index, false, true, 300, QEasingCurve::OutQuad);
    }
    return true;
}

bool FluentWindow::switchTo(const QString &routeKey) { return switchTo(findInterface(routeKey)); }

bool FluentWindow::removeInterface(QWidget *widget, bool deleteWidget)
{
    const int index = m_stackedWidget->indexOf(widget);
    if (index < 0) {
        return false;
    }

    const QString routeKey = widget->objectName();
    m_navigation->removeWidget(routeKey);
    m_stackedWidget->removeWidget(widget);
    m_history->remove(m_stackedWidget, routeKey);
    widget->hide();
    if (deleteWidget) {
        widget->deleteLater();
    } else {
        widget->setParent(nullptr);
    }
    return true;
}

bool FluentWindow::removeInterface(const QString &routeKey, bool deleteWidget)
{
    return removeInterface(findInterface(routeKey), deleteWidget);
}

bool FluentWindow::goBack()
{
    if (!canGoBack()) {
        return false;
    }

    m_suppressHistoryPush = true;
    switchTo(m_history->previousRouteKey(m_stackedWidget));
    m_history->pop(m_stackedWidget);
    m_suppressHistoryPush = false;
    return true;
}

bool FluentWindow::canGoBack() const { return m_history->canGoBack(m_stackedWidget); }

QStackedWidget *FluentWindow::stackedWidget() const { return m_stackedWidget; }

QWidget *FluentWindow::currentInterface() const { return m_stackedWidget->currentWidget(); }

QString FluentWindow::currentRouteKey() const
{
    QWidget *current = currentInterface();
    return current ? current->objectName() : QString();
}

QWidget *FluentWindow::findInterface(const QString &routeKey) const
{
    if (routeKey.isEmpty()) {
        return nullptr;
    }
    for (int i = 0; i < m_stackedWidget->count(); ++i) {
        if (m_stackedWidget->widget(i)->objectName() == routeKey) {
            return m_stackedWidget->widget(i);
        }
    }
    return nullptr;
}

bool FluentWindow::isMicaEffectEnabled() const { return m_isMicaEnabled; }

QColor FluentWindow::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor FluentWindow::darkBackgroundColor() const { return m_darkBackgroundColor; }

QColor FluentWindow::backgroundColor() const
{
    return m_isMicaEnabled ? micaHitTestBackgroundColor()
                           : windowBackgroundColor(m_lightBackgroundColor, m_darkBackgroundColor);
}

void FluentWindow::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

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
    const QColor background = backgroundColor();
    painter.setBrush(background);

    QPainterPath path;
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    path.addRect(QRectF(rect()));
    painter.drawRect(rect());
#else
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF backgroundRect = isMaximized() || isFullScreen() ? QRectF(rect()) : QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
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
    m_widgetLayout->setContentsMargins(0, m_contentTopMargin, 0, 0);
}

int FluentWindow::titleBarLeftMargin() const { return m_titleBarLeftMargin; }

int FluentWindow::contentTopMargin() const { return m_contentTopMargin; }

// Python's _onCurrentInterfaceChanged: select the page's item, record it in the history and match
// the stack's background to the page.
void FluentWindow::onCurrentInterfaceChanged(int index)
{
    QWidget *page = m_stackedWidget->widget(index);
    if (!page) {
        return;
    }

    m_navigation->setCurrentItem(page->objectName());
    if (!m_suppressHistoryPush) {
        m_history->push(m_stackedWidget, page->objectName());
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
    QWidget *current = m_stackedWidget->currentWidget();
    if (!current) {
        return;
    }

    const bool isTransparent = current->property("isStackedTransparent").toBool();
    QStackedWidget *stack = m_stackedWidget;
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

// --- SplitFluentWindow ---

SplitFluentWindow::SplitFluentWindow(QWidget *parent) : FluentWindow(parent)
{
    setTitleBarIconPlacement(titleBar(), 46 + 12, 0, 5);
    setTitleBarLeftMargin(0);
    setContentTopMargin(0);
    titleBar()->raise();
}

// --- MSFluentWindow ---

MSFluentWindow::MSFluentWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAutoFillBackground(false);
    configureRoundedWindowBackground(this);
    setMinimumSize(900, 620);
    FluentStyleSheet::setRole(this, QStringLiteral("MSFluentWindow"));

    m_frameless = std::make_unique<FramelessWindowHelper>(this);
    m_titleBar = new FluentTitleBar(this);
    setTitleBarIconPlacement(m_titleBar, 20, 2, 10);
    m_frameless->setTitleBar(m_titleBar);

    m_container = new QWidget(this);
    m_container->setObjectName(QStringLiteral("FluentWindowContent"));
    configureContentBackground(m_container);

    auto *layout = new QHBoxLayout(m_container);
    layout->setContentsMargins(0, 48, 0, 0);
    layout->setSpacing(0);

    m_navigationBar = new NavigationBar(m_container);
    // FluentWindow's page stack, as in Python: a translucent layer with a rounded top-left corner.
    m_stackedWidget = new QStackedWidget(m_container);
    m_stackedWidget->setObjectName(QStringLiteral("NavigationStack"));
    m_stackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(m_stackedWidget, QStringLiteral("NavigationStack"));
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
    QTimer *themeRefresh = createThemeRefresh(this, [this]() { return m_isMicaEnabled; });
    connect(ThemeManager::instance(), &ThemeManager::themeChangedFinished, this,
            [this, themeRefresh]() { beginThemeRefresh(this, m_isMicaEnabled, themeRefresh); });

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

QColor MSFluentWindow::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor MSFluentWindow::darkBackgroundColor() const { return m_darkBackgroundColor; }

QColor MSFluentWindow::backgroundColor() const
{
    return m_isMicaEnabled ? micaHitTestBackgroundColor()
                           : windowBackgroundColor(m_lightBackgroundColor, m_darkBackgroundColor);
}

void MSFluentWindow::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

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
    const int index = static_cast<int>(m_routeOrder.indexOf(routeKey));
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
    const int index = static_cast<int>(m_routeOrder.indexOf(routeKey));
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
    const QColor background = backgroundColor();
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
