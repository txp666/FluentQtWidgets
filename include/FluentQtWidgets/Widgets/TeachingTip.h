#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Flyout.h>

#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

class QCloseEvent;
class QEvent;
class QLabel;
class QObject;
class QPaintEvent;
class QPropertyAnimation;
class QShowEvent;
class QTimer;
class QToolButton;
class QVBoxLayout;
class QGraphicsDropShadowEffect;

namespace FluentQt {

class ImageLabel;

enum class TeachingTipTailPosition
{
    Top = 0,
    Bottom = 1,
    Left = 2,
    Right = 3,
    TopLeft = 4,
    TopRight = 5,
    BottomLeft = 6,
    BottomRight = 7,
    LeftTop = 8,
    LeftBottom = 9,
    RightTop = 10,
    RightBottom = 11,
    None = 12
};

enum class TeachingTipImagePosition
{
    Top,
    Bottom,
    Left,
    Right
};

class FQW_API TeachingTipView : public FlyoutViewBase
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)

public:
    TeachingTipView(const QString &title, const QString &content, QWidget *parent = nullptr);
    TeachingTipView(const QString &title, const QString &content, const QIcon &icon,
                    const QPixmap &image = QPixmap(), bool isClosable = true,
                    TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom,
                    QWidget *parent = nullptr);
    TeachingTipView(const QString &title, const QString &content, const QIcon &icon,
                    const QString &imagePath, bool isClosable = true,
                    TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom,
                    QWidget *parent = nullptr);

    QString title() const;
    QString content() const;
    bool isClosable() const;
    void setTitle(const QString &title);
    void setContent(const QString &content);
    void setIcon(const QIcon &icon);
    void setClosable(bool closable);
    void setImage(const QPixmap &pixmap);
    void setImage(const QImage &image);
    bool setImagePath(const QString &path);
    void setTailHint(TeachingTipTailPosition tail);
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::AlignLeft) override;
    QVBoxLayout *bodyLayout() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateImageLayout();
    TeachingTipImagePosition imagePositionForTail(TeachingTipTailPosition tail) const;

    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    ImageLabel *m_imageLabel = nullptr;
    QToolButton *m_closeButton = nullptr;
    QVBoxLayout *m_rootLayout = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_bodyLayout = nullptr;
    bool m_closable = true;
    TeachingTipTailPosition m_tailHint = TeachingTipTailPosition::Bottom;
};

class FQW_API TeachingTip : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration WRITE setDuration)

public:
    TeachingTip(FlyoutViewBase *view, QWidget *target, int durationMs = 1000,
                TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom, QWidget *parent = nullptr);

    static TeachingTip *make(FlyoutViewBase *view, QWidget *target,
                             TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom, int durationMs = 1000,
                             QWidget *parent = nullptr);
    static TeachingTip *create(const QString &title, const QString &content, QWidget *target,
                               TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom, int durationMs = 1000,
                               QWidget *parent = nullptr);
    static TeachingTip *create(const QString &title, const QString &content, const QIcon &icon,
                               const QPixmap &image, bool isClosable, QWidget *target,
                               TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom,
                               int durationMs = 1000, QWidget *parent = nullptr,
                               bool isDeleteOnClose = true);
    static TeachingTip *create(const QString &title, const QString &content, const QIcon &icon,
                               const QString &imagePath, bool isClosable, QWidget *target,
                               TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom,
                               int durationMs = 1000, QWidget *parent = nullptr,
                               bool isDeleteOnClose = true);

    FlyoutViewBase *view() const;
    QWidget *target() const;
    int duration() const;
    TeachingTipTailPosition tailPosition() const;
    bool isDeleteOnClose() const;
    void setIsDeleteOnClose(bool value);

public slots:
    void setDuration(int durationMs);
    void setTailPosition(FluentQt::TeachingTipTailPosition position);
    void reposition();
    void fadeOut();

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPoint calculatePosition() const;
    void closeAndDelete();
    void applyTailMargins();
    void setShadowEffect();
    void resizeToSizeHint();

    FlyoutViewBase *m_view = nullptr;
    QWidget *m_target = nullptr;
    QWidget *m_bubble = nullptr;
    int m_duration = 1000;
    TeachingTipTailPosition m_tailPosition = TeachingTipTailPosition::Bottom;
    bool m_isDeleteOnClose = false;
    QGraphicsDropShadowEffect *m_shadowEffect = nullptr;
    QPropertyAnimation *m_opacityAni = nullptr;
};

class FQW_API PopupTeachingTip : public TeachingTip
{
    Q_OBJECT

public:
    PopupTeachingTip(FlyoutViewBase *view, QWidget *target, int durationMs = 1000,
                     TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom,
                     QWidget *parent = nullptr, bool isDeleteOnClose = true);

    static PopupTeachingTip *make(FlyoutViewBase *view, QWidget *target,
                                  TeachingTipTailPosition tail = TeachingTipTailPosition::Bottom,
                                  int durationMs = 1000, QWidget *parent = nullptr,
                                  bool isDeleteOnClose = true);
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::TeachingTipTailPosition)
