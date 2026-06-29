#include <FluentQtWidgets/Window/FramelessWindowHelper.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>

#include <QtCore/QtGlobal>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QWindow>
#include <QtGui/QCursor>
#include <QtWidgets/QWidget>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>
#endif

namespace FluentQt {

namespace {

constexpr int kFramelessResizeBorder = 8;

Qt::CursorShape cursorForFramelessArea(FramelessArea area)
{
    switch (area) {
    case FramelessArea::Top:
    case FramelessArea::Bottom:
        return Qt::SizeVerCursor;
    case FramelessArea::Left:
    case FramelessArea::Right:
        return Qt::SizeHorCursor;
    case FramelessArea::TopLeft:
    case FramelessArea::BottomRight:
        return Qt::SizeFDiagCursor;
    case FramelessArea::TopRight:
    case FramelessArea::BottomLeft:
        return Qt::SizeBDiagCursor;
    case FramelessArea::Caption:
        return Qt::SizeAllCursor;
    default:
        return Qt::ArrowCursor;
    }
}

Qt::Edges edgesForArea(FramelessArea area)
{
    Qt::Edges edges = Qt::Edges();
    if (area == FramelessArea::Left || area == FramelessArea::TopLeft || area == FramelessArea::BottomLeft)
        edges |= Qt::LeftEdge;
    if (area == FramelessArea::Right || area == FramelessArea::TopRight || area == FramelessArea::BottomRight)
        edges |= Qt::RightEdge;
    if (area == FramelessArea::Top || area == FramelessArea::TopLeft || area == FramelessArea::TopRight)
        edges |= Qt::TopEdge;
    if (area == FramelessArea::Bottom || area == FramelessArea::BottomLeft || area == FramelessArea::BottomRight)
        edges |= Qt::BottomEdge;
    return edges;
}

QRect constrainedResizeRect(const QRect &baseRect, const QRect &screenRect, Qt::Edges edges, int minW, int minH,
                            int dx, int dy)
{
    QRect target = baseRect;
    if (edges.testFlag(Qt::LeftEdge))
        target.setLeft(target.left() + dx);
    if (edges.testFlag(Qt::RightEdge))
        target.setRight(target.right() + dx);
    if (edges.testFlag(Qt::TopEdge))
        target.setTop(target.top() + dy);
    if (edges.testFlag(Qt::BottomEdge))
        target.setBottom(target.bottom() + dy);
    target = target.normalized();

    if (target.width() < minW) {
        if (edges.testFlag(Qt::LeftEdge))
            target.setLeft(target.right() - minW + 1);
        else
            target.setRight(target.left() + minW - 1);
    }
    if (target.height() < minH) {
        if (edges.testFlag(Qt::TopEdge))
            target.setTop(target.bottom() - minH + 1);
        else
            target.setBottom(target.top() + minH - 1);
    }

    if (screenRect.isValid()) {
        target.setLeft(qMax(target.left(), screenRect.left()));
        target.setTop(qMax(target.top(), screenRect.top()));
        target.setRight(qMin(target.right(), screenRect.right()));
        target.setBottom(qMin(target.bottom(), screenRect.bottom()));
    }

    return target;
}

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

} // anonymous namespace

FramelessWindowHelper::FramelessWindowHelper(QWidget *host)
    : m_host(host)
{
}

void FramelessWindowHelper::setTitleBar(FluentTitleBar *titleBar)
{
    m_titleBar = titleBar;
}

FluentTitleBar *FramelessWindowHelper::titleBar() const
{
    return m_titleBar;
}

bool FramelessWindowHelper::isProcessingFramelessAction() const
{
    return m_usingSystemMove || m_dragging || m_resizing;
}

bool FramelessWindowHelper::isMaximizedOrFullScreen() const
{
    return m_host->isMaximized() || m_host->isFullScreen();
}

QWindow *FramelessWindowHelper::windowHandle() const
{
    return m_host->windowHandle();
}

// ---- Windows native event handler ----

#if defined(Q_OS_WIN)
bool FramelessWindowHelper::handleNativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType)
    auto *msg = static_cast<MSG *>(message);
    if (!msg || msg->message != WM_NCHITTEST) {
        return false;
    }

    const QPoint globalPos(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam));
    const QPoint localPos = m_host->mapFromGlobal(globalPos);
    const bool resizable = !isMaximizedOrFullScreen();

    if (resizable) {
        const bool left = localPos.x() >= 0 && localPos.x() < kFramelessResizeBorder;
        const bool right = localPos.x() <= m_host->width() && localPos.x() > m_host->width() - kFramelessResizeBorder;
        const bool top = localPos.y() >= 0 && localPos.y() < kFramelessResizeBorder;
        const bool bottom = localPos.y() <= m_host->height() && localPos.y() > m_host->height() - kFramelessResizeBorder;

        if (top && left)       { *result = HTTOPLEFT;      return true; }
        if (top && right)      { *result = HTTOPRIGHT;     return true; }
        if (bottom && left)    { *result = HTBOTTOMLEFT;   return true; }
        if (bottom && right)   { *result = HTBOTTOMRIGHT;  return true; }
        if (left)              { *result = HTLEFT;         return true; }
        if (right)             { *result = HTRIGHT;        return true; }
        if (top)               { *result = HTTOP;          return true; }
        if (bottom)            { *result = HTBOTTOM;       return true; }
    }

    if (m_titleBar && m_titleBar->isVisible()) {
        const QPoint titlePos = m_titleBar->mapFromGlobal(globalPos);
        if (m_titleBar->rect().contains(titlePos) && !m_titleBar->childAt(titlePos)) {
            *result = HTCAPTION;
            return true;
        }
    }

    return false;
}
#else
bool FramelessWindowHelper::handleNativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    Q_UNUSED(result)
    return false;
}
#endif

// ---- Non-Windows event handlers ----

#if !defined(Q_OS_WIN)

void FramelessWindowHelper::handleMousePress(const QPoint &globalPos, Qt::MouseButton button)
{
    m_mousePressed = false;
    m_resizing = false;
    m_dragging = false;
    m_usingSystemMove = false;
    m_resizeEdges = Qt::Edges();

    if (button != Qt::LeftButton || isMaximizedOrFullScreen()) {
        return;
    }

    // Wayland: only system-level move/resize works (manual setGeometry is not allowed)
    static const bool isWayland = !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
    if (isWayland) {
        const FramelessArea area = hitTestFramelessArea(m_host->mapFromGlobal(globalPos));
        if (area == FramelessArea::Caption) {
            if (QWindow *handle = windowHandle()) {
                handle->startSystemMove();
            }
        } else if (area != FramelessArea::None) {
            const Qt::Edges edges = edgesForArea(area);
            if (edges != Qt::Edges()) {
                if (QWindow *handle = windowHandle()) {
                    handle->startSystemResize(edges);
                }
            }
        }
        return; // No manual fallback on Wayland
    }

    const FramelessArea area = hitTestFramelessArea(m_host->mapFromGlobal(globalPos));
    if (area == FramelessArea::None) {
        return;
    }

    if (area == FramelessArea::Caption) {
        m_mousePressed = true;
        m_dragging = true;
        m_pressPos = globalPos;
        m_startGeometry = m_host->geometry();

        if (QWindow *handle = windowHandle()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (handle->startSystemMove()) {
                m_usingSystemMove = true;
                return;
            }
#endif
        }
        return;
    }

    const Qt::Edges edges = edgesForArea(area);
    if (edges == Qt::Edges()) {
        return;
    }

    m_mousePressed = true;
    m_resizing = true;
    m_resizeEdges = edges;
    m_pressPos = globalPos;
    m_startGeometry = m_host->geometry();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (QWindow *handle = windowHandle()) {
        if (handle->startSystemResize(edges)) {
            m_usingSystemMove = true;
        }
    }
#endif
}

void FramelessWindowHelper::handleMouseMove(const QPoint &globalPos)
{
    if (!m_mousePressed) {
        updateFramelessCursor(m_host->mapFromGlobal(globalPos));
        return;
    }

    if (m_dragging) {
        applyManualMove(globalPos);
    } else if (m_resizing) {
        applyManualResize(globalPos);
    }
}

void FramelessWindowHelper::handleMouseRelease()
{
    m_mousePressed = false;
    m_usingSystemMove = false;
    m_dragging = false;
    m_resizing = false;
    m_resizeEdges = Qt::Edges();
    m_host->unsetCursor();
    updateFramelessCursor(m_host->mapFromGlobal(QCursor::pos()));
}

#else // Q_OS_WIN

void FramelessWindowHelper::handleMousePress(const QPoint &, Qt::MouseButton) {}
void FramelessWindowHelper::handleMouseMove(const QPoint &) {}
void FramelessWindowHelper::handleMouseRelease() {}

#endif

void FramelessWindowHelper::applyManualMove(const QPoint &globalPos)
{
    if (m_usingSystemMove) {
        return;
    }
    const QPoint offset = globalPos - m_pressPos;
    m_host->move(m_startGeometry.topLeft() + offset);
}

void FramelessWindowHelper::applyManualResize(const QPoint &globalPos)
{
    if (m_usingSystemMove) {
        return;
    }
    const QPoint delta = globalPos - m_pressPos;
    const QRect screenRect = QGuiApplication::screenAt(globalPos)
                                 ? QGuiApplication::screenAt(globalPos)->availableGeometry()
                                 : QRect();
    const QRect target = constrainedResizeRect(m_startGeometry, screenRect, m_resizeEdges,
                                               m_host->minimumWidth(), m_host->minimumHeight(),
                                               delta.x(), delta.y());
    m_host->setGeometry(target);
}

void FramelessWindowHelper::updateFramelessCursor(const QPoint &localPos)
{
    if (isMaximizedOrFullScreen() || m_mousePressed) {
        return;
    }
    m_host->setCursor(cursorForFramelessArea(hitTestFramelessArea(localPos)));
}

FramelessArea FramelessWindowHelper::hitTestFramelessArea(const QPoint &localPos) const
{
    if (isMaximizedOrFullScreen()) {
        return FramelessArea::None;
    }

    const bool left = localPos.x() >= 0 && localPos.x() < kFramelessResizeBorder;
    const bool right = localPos.x() <= m_host->width() && localPos.x() > m_host->width() - kFramelessResizeBorder;
    const bool top = localPos.y() >= 0 && localPos.y() < kFramelessResizeBorder;
    const bool bottom = localPos.y() <= m_host->height() && localPos.y() > m_host->height() - kFramelessResizeBorder;

    if (top && left)       return FramelessArea::TopLeft;
    if (top && right)      return FramelessArea::TopRight;
    if (bottom && left)    return FramelessArea::BottomLeft;
    if (bottom && right)   return FramelessArea::BottomRight;
    if (left)              return FramelessArea::Left;
    if (right)             return FramelessArea::Right;
    if (top)               return FramelessArea::Top;
    if (bottom)            return FramelessArea::Bottom;

    if (m_titleBar && m_titleBar->isVisible()) {
        const QPoint titlePos = m_titleBar->mapFrom(m_host, localPos);
        if (m_titleBar->rect().contains(titlePos) && !m_titleBar->childAt(titlePos)) {
            return FramelessArea::Caption;
        }
    }

    return FramelessArea::None;
}

void FramelessWindowHelper::handleWindowStateChange()
{
    // Reset interaction state on window state changes
    m_mousePressed = false;
    m_usingSystemMove = false;
    m_dragging = false;
    m_resizing = false;
    m_resizeEdges = Qt::Edges();
}

} // namespace FluentQt
