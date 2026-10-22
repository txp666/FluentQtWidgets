#include <QtGui/QGuiApplication>
#include <QtWidgets/QWidget>

#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/QuartzCore.h>

namespace FluentQt::Private {

void updateMacOSWindowCorner(QWidget *widget, int radius)
{
    // winId() is only an NSView under the Cocoa platform plugin; offscreen and
    // minimal hand out plain integers.
    if (!widget || QGuiApplication::platformName() != QLatin1String("cocoa")) {
        return;
    }

    // internalWinId() rather than winId(): calls from the window's constructor must not create
    // the native window early. The first show sends a pending resize event, which calls this
    // again once the window exists.
    auto *widgetView = reinterpret_cast<NSView *>(widget->internalWinId());
    if (!widgetView) {
        return;
    }

    NSWindow *window = widgetView.window;
    if (!window) {
        return;
    }

    window.opaque = NO;
    window.backgroundColor = NSColor.clearColor;

    auto configureView = [radius, window](NSView *view) {
        if (!view) {
            return;
        }

        view.wantsLayer = YES;
        CALayer *layer = view.layer;
        if (!layer) {
            return;
        }

        layer.masksToBounds = radius > 0;
        layer.cornerRadius = radius;
        if (@available(macOS 10.13, *)) {
            layer.cornerCurve = kCACornerCurveContinuous;
        }
        layer.contentsScale = window.backingScaleFactor;
        layer.allowsEdgeAntialiasing = YES;
        layer.edgeAntialiasingMask = kCALayerLeftEdge | kCALayerRightEdge | kCALayerTopEdge | kCALayerBottomEdge;
    };

    configureView(window.contentView);
    if (widgetView != window.contentView) {
        configureView(widgetView);
    }
}

// Asks without prompting: CGRequestScreenCaptureAccess() or an actual capture would show the
// Screen Recording dialog. Before macOS 10.15 no permission exists.
bool isScreenCapturePermitted()
{
    if (@available(macOS 10.15, *)) {
        return CGPreflightScreenCaptureAccess();
    }
    return true;
}

} // namespace FluentQt::Private
