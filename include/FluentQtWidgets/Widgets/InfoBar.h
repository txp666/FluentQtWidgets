#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/FluentIcon.h>

#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>

class QLabel;
class QPushButton;
class QToolButton;
class QPropertyAnimation;
class QGraphicsOpacityEffect;
class QVBoxLayout;
class QHBoxLayout;

namespace FluentQt {

enum class InfoBarSeverity
{
    Info,
    Success,
    Warning,
    Error
};

enum class InfoBarIcon
{
    Information,
    Success,
    Warning,
    Error
};

enum class InfoBarPosition
{
    Top,
    TopRight,
    TopLeft,
    Bottom,
    BottomRight,
    BottomLeft,
    None
};

class FQW_API InfoBarIconWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::InfoBarIcon icon READ icon WRITE setIcon)

public:
    explicit InfoBarIconWidget(InfoBarIcon icon, QWidget *parent = nullptr);
    explicit InfoBarIconWidget(InfoBarSeverity icon, QWidget *parent = nullptr);
    explicit InfoBarIconWidget(const QIcon &icon, QWidget *parent = nullptr);

    InfoBarIcon icon() const;
    QIcon customIcon() const;
    InfoBarSeverity severity() const;

public slots:
    void setIcon(InfoBarIcon icon);
    void setIcon(InfoBarSeverity icon);
    void setIcon(const QIcon &icon);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    InfoBarIcon m_icon = InfoBarIcon::Information;
    QIcon m_customIcon;
};

class FQW_API InfoBar : public QFrame
{
    Q_OBJECT

public:
    explicit InfoBar(InfoBarSeverity icon, const QString &title, const QString &content,
                     Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                     int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                     QWidget *parent = nullptr);
    explicit InfoBar(InfoBarIcon icon, const QString &title, const QString &content,
                     Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                     int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                     QWidget *parent = nullptr);
    explicit InfoBar(const QIcon &icon, const QString &title, const QString &content,
                     Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                     int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                     QWidget *parent = nullptr);
    explicit InfoBar(FluentIcon icon, const QString &title, const QString &content,
                     Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                     int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                     QWidget *parent = nullptr);

    static InfoBar *newInfoBar(InfoBarSeverity icon, const QString &title, const QString &content,
                               Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                               int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                               QWidget *parent = nullptr);
    static InfoBar *newInfoBar(InfoBarIcon icon, const QString &title, const QString &content,
                               Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                               int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                               QWidget *parent = nullptr);
    static InfoBar *newInfoBar(const QIcon &icon, const QString &title, const QString &content,
                               Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                               int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                               QWidget *parent = nullptr);
    static InfoBar *newInfoBar(FluentIcon icon, const QString &title, const QString &content,
                               Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                               int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                               QWidget *parent = nullptr);
    static InfoBar *info(const QString &title, const QString &content,
                         Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                         int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                         QWidget *parent = nullptr);
    static InfoBar *success(const QString &title, const QString &content,
                            Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                            int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                            QWidget *parent = nullptr);
    static InfoBar *warning(const QString &title, const QString &content,
                            Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                            int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                            QWidget *parent = nullptr);
    static InfoBar *error(const QString &title, const QString &content,
                          Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                          int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                          QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    InfoBarIcon iconType() const;
    QString title() const;
    QString content() const;
    Qt::Orientation orient() const;
    bool isClosable() const;
    int duration() const;
    InfoBarPosition position() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    InfoBarIconWidget *iconWidget() const;
    QToolButton *closeButton() const;

    void addWidget(QWidget *widget, int stretch = 0);
    void setCustomBackgroundColor(const QColor &light, const QColor &dark);

signals:
    void closedSignal();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void initWidget();
    void initLayout();
    void applyQss();
    void fadeOut();
    void adjustText();

    InfoBarSeverity m_severity = InfoBarSeverity::Info;
    InfoBarIcon m_icon = InfoBarIcon::Information;
    QIcon m_customIcon;
    QString m_title;
    QString m_content;
    Qt::Orientation m_orient = Qt::Horizontal;
    bool m_isClosable = true;
    int m_duration = 1000;
    InfoBarPosition m_position = InfoBarPosition::TopRight;
    QColor m_lightBackgroundColor;
    QColor m_darkBackgroundColor;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    InfoBarIconWidget *m_iconWidget = nullptr;
    QToolButton *m_closeButton = nullptr;

    QHBoxLayout *m_hBoxLayout = nullptr;
    QHBoxLayout *m_textLayout = nullptr;
    QVBoxLayout *m_textVLayout = nullptr;
    QHBoxLayout *m_widgetHLayout = nullptr;
    QVBoxLayout *m_widgetVLayout = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;
    QPropertyAnimation *m_opacityAni = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::InfoBarSeverity)
Q_DECLARE_METATYPE(FluentQt::InfoBarIcon)
Q_DECLARE_METATYPE(FluentQt::InfoBarPosition)
