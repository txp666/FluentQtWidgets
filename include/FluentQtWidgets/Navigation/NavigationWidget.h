#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Label.h>

#include <QtCore/QRectF>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QParallelAnimationGroup;
class QPropertyAnimation;
class QVariantAnimation;

namespace FluentQt {

class FQW_API NavigationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString routeKey READ routeKey WRITE setRouteKey)
    Q_PROPERTY(bool compacted READ isCompacted WRITE setCompacted)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    Q_PROPERTY(bool aboutSelected READ isAboutSelected WRITE setAboutSelected)

  public:
    static constexpr int kCompactWidth = 40;
    static constexpr int kExpandWidth = 312;
    static constexpr int kItemHeight = 36;

    explicit NavigationWidget(bool selectable = true, QWidget *parent = nullptr);

    QString routeKey() const;
    bool isSelectable() const;
    bool isCompacted() const;
    bool isSelected() const;
    bool isAboutSelected() const;
    virtual QRectF indicatorRect() const;
    QSize sizeHint() const override;
    // The width when expanded: kExpandWidth, or the panel's expand width minus 10 (Python's
    // NavigationWidget.EXPAND_WIDTH, which NavigationPanel.setExpandWidth() sets).
    int expandWidth() const;
    void setExpandWidth(int width);

  public slots:
    void setRouteKey(const QString &routeKey);
    virtual void setCompacted(bool compacted);
    virtual void setSelected(bool selected);
    virtual void setAboutSelected(bool selected);

  signals:
    void clicked(bool triggeredByUser = true);
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

    QColor textColor(int alpha = 255) const;
    QColor hoverBackgroundColor() const;
    void drawHoverBackground(QPainter *painter);

    QString m_routeKey;
    int m_expandWidth = kExpandWidth;
    bool m_selectable = true;
    bool m_compacted = true;
    bool m_selected = false;
    bool m_aboutSelected = false;
    bool m_pressed = false;
    bool m_hovered = false;
};

class FQW_API NavigationAvatarWidget : public NavigationWidget
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

  public:
    explicit NavigationAvatarWidget(const QString &name = QString(), QWidget *parent = nullptr);
    NavigationAvatarWidget(const QString &name, const QString &avatarPath, QWidget *parent = nullptr);
    NavigationAvatarWidget(const QString &name, const QPixmap &avatar, QWidget *parent = nullptr);
    NavigationAvatarWidget(const QString &name, const QImage &avatar, QWidget *parent = nullptr);

    QString name() const;
    AvatarWidget *avatar() const;

  public slots:
    void setName(const QString &name);
    bool setAvatar(const QString &avatarPath);
    void setAvatar(const QPixmap &avatar);
    void setAvatar(const QImage &avatar);
    void setCompacted(bool compacted) override;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void updateAvatarGeometry();

    QString m_name;
    AvatarWidget *m_avatar = nullptr;
};

class FQW_API NavigationUserCard : public NavigationAvatarWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString subtitle READ subtitle WRITE setSubtitle)
    Q_PROPERTY(qreal textOpacity READ textOpacity WRITE setTextOpacity)

  public:
    explicit NavigationUserCard(QWidget *parent = nullptr);

    // Python's: compacting and expanding animate the avatar's radius (12 <-> 32, OutCubic) and,
    // in 80 % of the time, the text's opacity (InOutQuad), 250 ms by default.
    int animationDuration() const;
    void setAnimationDuration(int duration);
    qreal textOpacity() const;
    void setTextOpacity(qreal opacity);

    QString title() const;
    QString subtitle() const;
    int titleFontSize() const;
    int subtitleFontSize() const;

  public slots:
    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);
    void setTitleFontSize(int size);
    void setSubtitleFontSize(int size);
    void setAvatarIcon(const QIcon &icon);
    void setAvatarBackgroundColor(const QColor &light, const QColor &dark);
    void setCompacted(bool compacted) override;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void updateAvatarGeometry();

  private:
    QString m_title;
    QString m_subtitle;
    int m_titleFontSize = 14;
    int m_subtitleFontSize = 12;
    int m_animationDuration = 250;
    qreal m_textOpacity = 0.0;
    QPropertyAnimation *m_radiusAnimation = nullptr;
    QPropertyAnimation *m_opacityAnimation = nullptr;
    QParallelAnimationGroup *m_animationGroup = nullptr;
};

class FQW_API NavigationItemHeader : public NavigationWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)

  public:
    explicit NavigationItemHeader(const QString &text = QString(), QWidget *parent = nullptr);

    QString text() const;

  public slots:
    void setText(const QString &text);
    void setCompacted(bool compacted) override;

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

  private:
    QVariantAnimation *m_heightAnimation = nullptr;
    QString m_text;
};

} // namespace FluentQt
