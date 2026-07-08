#include <QtWidgets/QWidget>

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

namespace FluentQt::Private {

void updateMacOSWindowCorner(QWidget *widget, int radius)
{
    if (!widget) {
        return;
    }

    auto *widgetView = reinterpret_cast<NSView *>(widget->winId());
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

} // namespace FluentQt::Private
