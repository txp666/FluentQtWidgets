#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QSize>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>
#include <QtWidgets/QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif
class QMouseEvent;
class QPaintEvent;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QPropertyAnimation;
class QGraphicsDropShadowEffect;

namespace FluentQt {

// Forward declarations
class IconWidget;

class FQW_API CardWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)

public:
    explicit CardWidget(QWidget *parent = nullptr);

    int borderRadius() const;

    void setClickEnabled(bool enabled);
    bool isClickEnabled() const;
    bool isSelected() const;

public slots:
    void setBorderRadius(int radius);
    void setSelected(bool selected);

signals:
    void clicked();
    void selectedChanged(bool selected);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    virtual QColor normalBackgroundColor() const;
    virtual QColor hoverBackgroundColor() const;
    virtual QColor pressedBackgroundColor() const;
    virtual QColor selectedBackgroundColor() const;
    QColor borderColor() const;

    bool m_hover = false;
    bool m_pressed = false;
    bool m_selected = false;
    int m_borderRadius = 5;
    bool m_clickEnabled = false;
};

class FQW_API SimpleCardWidget : public CardWidget
{
    Q_OBJECT

public:
    explicit SimpleCardWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API ElevatedCardWidget : public SimpleCardWidget
{
    Q_OBJECT

public:
    explicit ElevatedCardWidget(QWidget *parent = nullptr);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    QColor hoverBackgroundColor() const override;
    QColor pressedBackgroundColor() const override;

private:
    void startElevateAnimation(const QPoint &start, const QPoint &end);

    QGraphicsDropShadowEffect *m_shadowEffect = nullptr;
    QPropertyAnimation *m_elevateAni = nullptr;
    QPoint m_originalPos;
};

class FQW_API CardSeparator : public QWidget
{
    Q_OBJECT

public:
    explicit CardSeparator(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API HeaderCardWidget : public SimpleCardWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    explicit HeaderCardWidget(QWidget *parent = nullptr);
    explicit HeaderCardWidget(const QString &title, QWidget *parent = nullptr);

    QString title() const;
    QLabel *titleLabel() const;
    QLabel *headerLabel() const;
    QWidget *headerView() const;
    CardSeparator *separator() const;
    QWidget *view() const;
    QHBoxLayout *headerLayout() const;
    QHBoxLayout *viewLayout() const;

public slots:
    void setTitle(const QString &title);

private:
    QWidget *m_headerView = nullptr;
    QLabel *m_titleLabel = nullptr;
    CardSeparator *m_separator = nullptr;
    QWidget *m_view = nullptr;
    QHBoxLayout *m_headerLayout = nullptr;
    QHBoxLayout *m_viewLayout = nullptr;
};

class FQW_API CardGroupWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)

public:
    CardGroupWidget(const QIcon &icon, const QString &title, const QString &content, QWidget *parent = nullptr);

    QString title() const;
    QString content() const;
    QIcon icon() const;
    IconWidget *iconWidget() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    CardSeparator *separator() const;
    QHBoxLayout *hBoxLayout() const;
    QVBoxLayout *textLayout() const;

    void setTitle(const QString &text);
    void setContent(const QString &text);
    void setIcon(const QIcon &icon);
    void setIconSize(const QSize &size);
    void setSeparatorVisible(bool visible);
    bool isSeparatorVisible() const;
    void addWidget(QWidget *widget, int stretch = 0);

private:
    IconWidget *m_iconWidget = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    QHBoxLayout *m_hBoxLayout = nullptr;
    QVBoxLayout *m_textLayout = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    CardSeparator *m_separator = nullptr;
};

class FQW_API GroupHeaderCardWidget : public HeaderCardWidget
{
    Q_OBJECT

public:
    explicit GroupHeaderCardWidget(QWidget *parent = nullptr);
    explicit GroupHeaderCardWidget(const QString &title, QWidget *parent = nullptr);

    CardGroupWidget *addGroup(const QIcon &icon, const QString &title, const QString &content,
                              QWidget *widget, int stretch = 0);
    int groupCount() const;
    QList<CardGroupWidget *> groupWidgets() const;

private:
    QList<CardGroupWidget *> m_groupWidgets;
    QVBoxLayout *m_groupLayout = nullptr;
};

} // namespace FluentQt
