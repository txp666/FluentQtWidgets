#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtCore/QRect>
#include <QtCore/QByteArray>
#include <QtCore/QtGlobal>
#include <QtCore/qnamespace.h>

class QWidget;
class QWindow;

namespace FluentQt {

class FluentTitleBar;
class TransparentToolButton;

enum class FramelessArea
{
    None,
    Caption,
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

// The frameless window behaviour (moving, resizing, edge cursors) that FluentWidget and the
// FluentWindow classes share.
class FQW_API FramelessWindowHelper
{
public:
    explicit FramelessWindowHelper(QWidget *host);

    void setTitleBar(FluentTitleBar *titleBar);
    FluentTitleBar *titleBar() const;

    // --- Non-Windows event handlers ---
    // Call from the host's mousePressEvent(), mouseMoveEvent() and mouseReleaseEvent().
    void handleMousePress(const QPoint &globalPos, Qt::MouseButton button);
    void handleMouseMove(const QPoint &globalPos);
    void handleMouseRelease();

    // True while a frameless move or resize is under way; the host then skips its default handling.
    bool isProcessingFramelessAction() const;

    // --- Windows native event handler ---
    bool handleNativeEvent(const QByteArray &eventType, void *message, qintptr *result);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt 5's nativeEvent() hands out a long, which is 32 bits on 64-bit Windows.
    bool handleNativeEvent(const QByteArray &eventType, void *message, long *result);
#endif

    // Call from the host's changeEvent() on QEvent::WindowStateChange.
    void handleWindowStateChange();

    // Sets the resize cursor for the edge under localPos.
    void updateFramelessCursor(const QPoint &localPos);

    FramelessArea hitTestFramelessArea(const QPoint &localPos) const;

private:
    void applyManualMove(const QPoint &globalPos);
    void applyManualResize(const QPoint &globalPos);
    QWindow *windowHandle() const;
    bool isMaximizedOrFullScreen() const;

    QWidget *m_host;
    QPointer<FluentTitleBar> m_titleBar;
    QPointer<TransparentToolButton> m_pressedMaxButton;

    bool m_mousePressed = false;
    bool m_usingSystemMove = false;
    bool m_resizing = false;
    bool m_dragging = false;
    Qt::Edges m_resizeEdges = Qt::Edges();
    QPoint m_pressPos;
    QRect m_startGeometry;
};

} // namespace FluentQt
