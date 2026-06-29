#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Window/FramelessWindowHelper.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

class QStackedWidget;
class QEvent;
class QPaintEvent;
class QResizeEvent;
class QWindow;
class QMouseEvent;

namespace FluentQt {

class FluentTitleBar;
class NavigationBar;
class NavigationHistory;
class NavigationInterface;
class SplashScreen;

FQW_API bool isMicaEffectAvailable();

class FQW_API FluentWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool micaEffectEnabled READ isMicaEffectEnabled WRITE setMicaEffectEnabled)
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor)

  public:
    explicit FluentWidget(QWidget *parent = nullptr);

    FluentTitleBar *titleBar() const;
    bool isMicaEffectEnabled() const;
    bool isSystemTitleBarButtonVisible() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;
    QColor backgroundColor() const;
    QRect systemTitleBarRect(const QSize &size) const;

  public slots:
    void setMicaEffectEnabled(bool enabled);
    void setSystemTitleBarButtonVisible(bool visible);
    void setCustomBackgroundColor(const QColor &light, const QColor &dark);
    void setTitleBar(FluentTitleBar *titleBar);

  protected:
    void changeEvent(QEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
#if !defined(Q_OS_WIN)
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#endif

  private:
    void updateTitleBarGeometry();
    void updateWindowMask();

    FluentTitleBar *m_titleBar = nullptr;
    FramelessWindowHelper *m_frameless = nullptr;
    QColor m_lightBackgroundColor = QColor(240, 244, 249);
    QColor m_darkBackgroundColor = QColor(32, 32, 32);
    bool m_isMicaEnabled = false;
    bool m_systemTitleBarButtonVisible = false;
};

class FQW_API FluentWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool micaEffectEnabled READ isMicaEffectEnabled WRITE setMicaEffectEnabled)

  public:
    explicit FluentWindow(QWidget *parent = nullptr);

    NavigationInterface *navigationInterface() const;
    NavigationHistory *navigationHistory() const;
    FluentTitleBar *titleBar() const;
    SplashScreen *splashScreen() const;

    int addPage(QWidget *page, const QString &title, const QIcon &icon = QIcon());
    int addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey = QString(),
                        bool isTransparent = false);
    int addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                        NavigationItemPosition position, bool isTransparent = false);
    int addSubInterface(QWidget *widget, const QIcon &icon, const QString &text, const QString &routeKey,
                        NavigationItemPosition position, const QString &parentRouteKey,
                        bool isTransparent = false);
    bool switchTo(QWidget *widget);
    bool switchTo(const QString &routeKey);
    bool removeInterface(QWidget *widget, bool deleteWidget = false);
    bool removeInterface(const QString &routeKey, bool deleteWidget = false);
    bool goBack();
    bool canGoBack() const;
    QStackedWidget *stackedWidget() const;
    QWidget *currentInterface() const;
    QString currentRouteKey() const;
    bool isMicaEffectEnabled() const;

  public slots:
    void setMicaEffectEnabled(bool enabled);
    void setSplashScreen(SplashScreen *splashScreen);

  protected:
    void changeEvent(QEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
#if !defined(Q_OS_WIN)
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#endif
    void setTitleBarLeftMargin(int margin);
    void setContentTopMargin(int margin);
    int titleBarLeftMargin() const;
    int contentTopMargin() const;

  private:
    void pushHistory(const QString &routeKey);
    void onCurrentRouteKeyChanged(const QString &routeKey);
    void onReturnRequested();
    void updateTitleBarGeometry();
    void updateStackedBackground();
    void updateWindowMask();

    QWidget *m_container = nullptr;
    NavigationInterface *m_navigation = nullptr;
    NavigationHistory *m_history = nullptr;
    FluentTitleBar *m_titleBar = nullptr;
    FramelessWindowHelper *m_frameless = nullptr;
    SplashScreen *m_splashScreen = nullptr;
    bool m_suppressHistoryPush = false;
    int m_titleBarLeftMargin = -1;
    int m_contentTopMargin = 48;
    bool m_isMicaEnabled = false;
};

class FQW_API SplitFluentWindow : public FluentWindow
{
    Q_OBJECT

  public:
    explicit SplitFluentWindow(QWidget *parent = nullptr);
};

class FQW_API MSFluentWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool micaEffectEnabled READ isMicaEffectEnabled WRITE setMicaEffectEnabled)

  public:
    explicit MSFluentWindow(QWidget *parent = nullptr);

    NavigationBar *navigationInterface() const;
    FluentTitleBar *titleBar() const;
    QStackedWidget *stackedWidget() const;
    QWidget *currentInterface() const;
    QString currentRouteKey() const;
    bool isMicaEffectEnabled() const;

    int addSubInterface(QWidget *widget, const QIcon &icon, const QString &text,
                        const QIcon &selectedIcon = QIcon(),
                        NavigationItemPosition position = NavigationItemPosition::Top,
                        bool isTransparent = false);
    bool switchTo(QWidget *widget);
    bool switchTo(const QString &routeKey);
    bool removeInterface(QWidget *widget, bool deleteWidget = false);
    bool removeInterface(const QString &routeKey, bool deleteWidget = false);

  public slots:
    void setMicaEffectEnabled(bool enabled);

  protected:
    void changeEvent(QEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
#if !defined(Q_OS_WIN)
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#endif

  private:
    QString ensureRouteKey(QWidget *widget) const;
    void updateTitleBarGeometry();
    void updateStackedBackground();
    void updateWindowMask();

    QWidget *m_container = nullptr;
    NavigationBar *m_navigationBar = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    FluentTitleBar *m_titleBar = nullptr;
    FramelessWindowHelper *m_frameless = nullptr;
    QList<QString> m_routeOrder;
    bool m_isMicaEnabled = false;
};

} // namespace FluentQt
