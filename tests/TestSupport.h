#pragma once

// Helpers shared by the widget tests.

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QAbstractAnimation>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtGui/QColor>
#include <QtGui/QFontInfo>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtGui/QWheelEvent>
#include <QtTest/QTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <functional>
#include <memory>

// Skips the test unless Qt runs on the named platform plugin ("windows", "cocoa", ...): native-API tests
// need the real plugin, and on another one (offscreen in CI) the window handles are not native.
#define FQW_REQUIRE_PLATFORM(name)                                                                                     \
    do {                                                                                                               \
        if (QGuiApplication::platformName() != QLatin1String(name)) {                                                  \
            QSKIP("Needs the \"" name "\" platform plugin");                                                           \
        }                                                                                                              \
    } while (false)

// Skips the test unless the primary screen has the room a placement test needs around what it places: on a
// smaller screen the library moves it back on the screen, as it should, and the expected position no
// longer holds.
#define FQW_REQUIRE_SCREEN_ROOM(minimumWidth, minimumHeight)                                                           \
    do {                                                                                                               \
        const QScreen *fqwScreen = QGuiApplication::primaryScreen();                                                   \
        if (!fqwScreen || fqwScreen->availableGeometry().width() < (minimumWidth) ||                                   \
            fqwScreen->availableGeometry().height() < (minimumHeight)) {                                               \
            QSKIP("The screen is too small for this placement test");                                                  \
        }                                                                                                              \
    } while (false)

// Skips the test when the platform does not maximize windows (a bare X server without a window manager,
// a session that refuses it): a maximize button then has nothing to toggle.
#define FQW_REQUIRE_MAXIMIZE()                                                                                         \
    do {                                                                                                               \
        if (!FqwTest::platformMaximizesWindows()) {                                                                    \
            QSKIP("This platform does not maximize windows");                                                          \
        }                                                                                                              \
    } while (false)

namespace FqwTest {

// Python's sizes in these tests were measured on macOS, where the Fluent font families resolve to PingFang SC.
// Text-dependent pixel values hold only where a widget gets that font; elsewhere (CI on Linux and Windows)
// tests check font-independent relations instead.
inline bool hasPythonMeasurementFont(const QWidget *widget)
{
    return QFontInfo(widget->font()).family() == QLatin1String("PingFang SC");
}

// Whether a plain window becomes maximized when asked. Probed once, with a widget of no Fluent class, so a
// library bug cannot turn a failing maximize test into a skip.
inline bool platformMaximizesWindows()
{
    static const bool maximizes = [] {
        QWidget probe;
        probe.resize(200, 150);
        probe.show();
        if (!QTest::qWaitForWindowExposed(&probe)) {
            return false;
        }
        probe.showMaximized();
        const bool maximized = QTest::qWaitFor([&probe] { return probe.isMaximized(); }, 2000);
        probe.close();
        return maximized;
    }();
    return maximizes;
}

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

    // Points the configuration at a file in a temporary directory, so that code which saves it (the
    // Gallery's setting cards) never writes the user's real configuration file.
    void useTemporaryConfigFile()
    {
        m_configDir = std::make_unique<QTemporaryDir>();
        FluentQt::FluentConfig::instance()->setFileName(m_configDir->filePath(QStringLiteral("fluentqtwidgets.json")));
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
    std::unique_ptr<QTemporaryDir> m_configDir;
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

// The visible widget with the given Fluent role anywhere in the application, top-level or not, or nullptr.
// Mask dialogs (MessageBox, MessageBoxBase, Dialog's mask, ColorDialog...) are child overlays of their
// parent window, which findVisibleTopLevelByRole() cannot see.
inline QWidget *findVisibleWidgetByRole(const QString &role)
{
    const auto widgets = QApplication::allWidgets();
    for (QWidget *widget : widgets) {
        if (widget->isVisible() && widget->property("fqw").toString() == role) {
            return widget;
        }
    }
    return nullptr;
}

// The visible widget of type T anywhere in the application, top-level or not, or nullptr.
template <typename T> T *findVisibleWidget()
{
    const auto widgets = QApplication::allWidgets();
    for (QWidget *widget : widgets) {
        if (auto *candidate = qobject_cast<T *>(widget); candidate && candidate->isVisible()) {
            return candidate;
        }
    }
    return nullptr;
}

// The widget painted at one image pixel per logical pixel, on a transparent background by default.
inline QImage render(QWidget &widget, const QColor &background = Qt::transparent,
                     QWidget::RenderFlags flags = QWidget::DrawWindowBackground | QWidget::DrawChildren)
{
    widget.ensurePolished();
    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(background);
    widget.render(&image, QPoint(), QRegion(), flags);
    return image;
}

inline QImage render(QWidget *widget, const QColor &background = Qt::transparent,
                     QWidget::RenderFlags flags = QWidget::DrawWindowBackground | QWidget::DrawChildren)
{
    return render(*widget, background, flags);
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

// The average grey of the painted pixels of one area whose grey lies in [minimumGray, maximumGray], or -1
// when there are none. Used to tell an icon's colour from the background around it.
inline int averageGray(const QImage &image, const QRect &area, int minimumGray, int maximumGray)
{
    const QRect bounds = area.intersected(image.rect());
    int sampled = 0;
    int graySum = 0;
    for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            const QColor color = image.pixelColor(x, y);
            const int gray = qGray(color.rgb());
            if (color.alpha() > 0 && gray >= minimumGray && gray <= maximumGray) {
                ++sampled;
                graySum += gray;
            }
        }
    }
    return sampled > 0 ? graySum / sampled : -1;
}

// Waits until no animation owned by the object or its children runs; false if one still runs after the
// timeout. Call it right after the call that starts the animation.
inline bool waitForAnimations(const QObject *object, int timeout = 5000)
{
    return QTest::qWaitFor(
        [object] {
            const auto animations = object->findChildren<QAbstractAnimation *>();
            for (const QAbstractAnimation *animation : animations) {
                if (animation->state() != QAbstractAnimation::Stopped) {
                    return false;
                }
            }
            return true;
        },
        timeout);
}

// Closes every top-level widget of type T and runs the deletions that causes.
template <typename T> void closeTopLevels()
{
    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (auto *candidate = qobject_cast<T *>(widget)) {
            candidate->close();
        }
    }
    drainDeferredDeletes();
}

// A 16 x 16 icon filled with one colour, easy to find in a rendered widget.
inline QIcon solidIcon(const QColor &color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    return QIcon(pixmap);
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
