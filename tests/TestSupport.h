#pragma once

// Helpers shared by the widget tests.

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <functional>

namespace FqwTest {

// The theme, the accent colour and the Fluent configuration are global: a test that changes them
// would change what every later test sees. Saved in init() and put back in cleanup().
class GlobalState
{
  public:
    GlobalState()
        : m_theme(FluentQt::ThemeManager::instance()->theme())
        , m_accentColor(FluentQt::ThemeManager::instance()->accentColor())
    {
        const auto *config = FluentQt::FluentConfig::instance();
        m_fileName = config->fileName();
        m_themeMode = config->themeMode();
        m_themeColor = config->themeColor();
        m_fontFamilies = config->fontFamilies();
        m_localeName = config->localeName();
        m_dpiScale = config->dpiScale();
        m_downloadFolder = config->downloadFolder();
        m_micaEnabled = config->isMicaEnabled();
        m_acrylicBlurRadius = config->acrylicBlurRadius();
        m_autoUpdateEnabled = config->isAutoUpdateEnabled();
    }

    GlobalState(const GlobalState &) = delete;
    GlobalState &operator=(const GlobalState &) = delete;

    ~GlobalState()
    {
        auto *config = FluentQt::FluentConfig::instance();
        config->setFileName(m_fileName);
        config->setThemeMode(m_themeMode);
        config->setThemeColor(m_themeColor);
        config->setFontFamilies(m_fontFamilies);
        config->setLocaleName(m_localeName);
        config->setDpiScale(m_dpiScale);
        config->setDownloadFolder(m_downloadFolder);
        config->setMicaEnabled(m_micaEnabled);
        config->setAcrylicBlurRadius(m_acrylicBlurRadius);
        config->setAutoUpdateEnabled(m_autoUpdateEnabled);

        // Only when changed: setting the theme restyles every widget.
        auto *manager = FluentQt::ThemeManager::instance();
        if (manager->theme() != m_theme) {
            manager->setTheme(m_theme);
        }
        if (manager->accentColor() != m_accentColor) {
            manager->setAccentColor(m_accentColor);
        }
    }

  private:
    FluentQt::Theme m_theme;
    QColor m_accentColor;
    QString m_fileName;
    FluentQt::Theme m_themeMode;
    QColor m_themeColor;
    QStringList m_fontFamilies;
    QString m_localeName;
    QString m_dpiScale;
    QString m_downloadFolder;
    bool m_micaEnabled = false;
    int m_acrylicBlurRadius = 0;
    bool m_autoUpdateEnabled = false;
};

// Only the posted deleteLater() calls.
inline void flushDeletes() { QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); }

// Runs deleteLater() and the events it causes until done() holds, or for a few rounds.
inline void drainDeferredDeletes(const std::function<bool()> &done = {})
{
    for (int i = 0; i < 8; ++i) {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        if (done && done()) {
            return;
        }
    }
}

// A mouse event sent straight to the widget, with any buttons held (QTest moves carry none).
inline void sendMouse(QWidget *widget, QEvent::Type type, const QPoint &pos, Qt::MouseButton button,
                      Qt::MouseButtons buttons)
{
    QMouseEvent event(type, QPointF(pos), QPointF(widget->mapToGlobal(pos)), button, buttons, Qt::NoModifier);
    QApplication::sendEvent(widget, &event);
}

// Ctrl+wheel over the widget; whether it took the event.
inline bool sendCtrlWheel(QWidget *widget, int delta)
{
    const QPoint pos(20, 20);
    QWheelEvent event(QPointF(pos), QPointF(widget->mapToGlobal(pos)), QPoint(), QPoint(0, delta), Qt::NoButton,
                      Qt::ControlModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(widget, &event);
    return event.isAccepted();
}

// The visible top-level widget of type T (a popup menu, a flyout), or nullptr.
template <typename T>
T *findVisibleTopLevel()
{
    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (auto *candidate = qobject_cast<T *>(widget); candidate && candidate->isVisible()) {
            return candidate;
        }
    }
    return nullptr;
}

// The visible top-level widget with the given Fluent role (its "fqw" property), or nullptr.
inline QWidget *findVisibleTopLevelByRole(const QString &role)
{
    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (widget->isVisible() && widget->property("fqw").toString() == role) {
            return widget;
        }
    }
    return nullptr;
}

// The widget painted at one image pixel per logical pixel, on a transparent background.
inline QImage render(QWidget &widget, const QColor &background = Qt::transparent)
{
    widget.ensurePolished();
    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(background);
    widget.render(&image);
    return image;
}

// How many pixels of the image (or of one area of it) match.
inline int countPixels(const QImage &image, const std::function<bool(const QColor &)> &matches,
                       const QRect &area = QRect())
{
    const QRect bounds = area.isNull() ? image.rect() : area.intersected(image.rect());
    int count = 0;
    for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            if (matches(image.pixelColor(x, y))) {
                ++count;
            }
        }
    }
    return count;
}

// Whether anything is painted: a pixel that is not fully transparent.
inline bool hasPaintedPixel(const QImage &image)
{
    return countPixels(image, [](const QColor &color) { return color.alpha() > 0; }) > 0;
}

// A text resource or file, or an empty string when it cannot be read.
inline QString readText(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace FqwTest
