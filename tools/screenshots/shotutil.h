// Helpers shared by galleryshots and showcase.
#pragma once
#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <cstdio>

// Runs the event loop for `ms` milliseconds, deferred deletes included.
inline void waitMs(int ms)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
}

// Grabs the window and paints every other visible top-level widget (popups) over it.
inline QImage grabComposite(QWidget *window, const QList<QWidget *> &popups)
{
    QPixmap base = window->grab();
    QPainter p(&base);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const QPoint origin = window->geometry().topLeft();
    for (QWidget *w : popups) {
        if (!w || !w->isVisible())
            continue;
        QPixmap pm = w->grab();
        const QPoint at = w->geometry().topLeft() - origin;
        std::fprintf(stderr, "popup %s at %d,%d size %dx%d\n", w->metaObject()->className(), at.x(), at.y(),
                     w->width(), w->height());
        p.drawPixmap(at, pm);
    }
    p.end();
    return base.toImage();
}

// The visible top-level widgets other than the window: its menus, pickers and flyouts.
inline QList<QWidget *> visiblePopups(QWidget *window)
{
    QList<QWidget *> out;
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (w != window && w->isVisible() && w->isWindow())
            out << w;
    }
    return out;
}

// Saves a 1280 x 800 JPEG, as the README images are.
inline bool saveShot(const QImage &img, const QString &path)
{
    QImage scaled = img.convertToFormat(QImage::Format_RGB32)
                        .scaled(1280, 800, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    const bool ok = scaled.save(path, "JPG", 90);
    std::fprintf(stderr, "saved %s (%dx%d from %dx%d): %s\n", qPrintable(path), scaled.width(), scaled.height(),
                 img.width(), img.height(), ok ? "ok" : "FAILED");
    return ok;
}
