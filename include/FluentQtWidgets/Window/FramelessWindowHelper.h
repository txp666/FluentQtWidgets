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

/// Helper class that encapsulates frameless window behavior (resize, move, cursor updates).
/// Used by FluentWidget, FluentWindow, and MSFluentWindow to eliminate code duplication.
class FQW_API FramelessWindowHelper
{
public:
    explicit FramelessWindowHelper(QWidget *host);

    void setTitleBar(FluentTitleBar *titleBar);
    FluentTitleBar *titleBar() const;

    // ---- Non-Windows event handlers ----
    // Call from host's mousePressEvent / mouseMoveEvent / mouseReleaseEvent
    void handleMousePress(const QPoint &globalPos, Qt::MouseButton button);
    void handleMouseMove(const QPoint &globalPos);
    void handleMouseRelease();

    // Returns true while a frameless move/resize is active (base widget should skip default handling)
    bool isProcessingFramelessAction() const;

    // ---- Windows native event handler ----
    bool handleNativeEvent(const QByteArray &eventType, void *message, qintptr *result);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt 5's nativeEvent() hands out a long, which is 32 bits on 64-bit Windows.
    bool handleNativeEvent(const QByteArray &eventType, void *message, long *result);
#endif

    // Call from host's changeEvent (WindowStateChange)
    void handleWindowStateChange();

    // Update cursor based on position (for frameless edge detection)
    void updateFramelessCursor(const QPoint &localPos);

    // Hit test for frameless areas
    FramelessArea hitTestFramelessArea(const QPoint &localPos) const;

private:
    void applyManualMove(const QPoint &globalPos);
    void applyManualResize(const QPoint &globalPos);
    QWindow *windowHandle() const;
    bool isMaximizedOrFullScreen() const;

    QWidget *m_host;
    QPointer<FluentTitleBar> m_titleBar;
    QPointer<TransparentToolButton> m_pressedMaxButton;

    // Frameless interaction state
    bool m_mousePressed = false;
    bool m_usingSystemMove = false;
    bool m_resizing = false;
    bool m_dragging = false;
    Qt::Edges m_resizeEdges = Qt::Edges();
    QPoint m_pressPos;
    QRect m_startGeometry;
};

} // namespace FluentQt
