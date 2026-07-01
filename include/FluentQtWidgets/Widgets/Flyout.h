#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPaintEvent>
#include <QtGui/QPixmap>
#include <QtWidgets/QFrame>

class QCloseEvent;
class QEvent;
class QLabel;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class QPropertyAnimation;
class QGraphicsDropShadowEffect;

namespace FluentQt {

class AcrylicBrush;
class ImageLabel;
class IconWidget;

enum class FlyoutAnimationType
{
    None = 5,
    PullUp = 0,
    DropDown = 1,
    SlideLeft = 2,
    SlideRight = 3,
    FadeIn = 4
};

class FQW_API FlyoutViewBase : public QFrame
{
    Q_OBJECT

public:
    explicit FlyoutViewBase(QWidget *parent = nullptr);
    virtual void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::AlignLeft);

signals:
    void closed();
};

class FQW_API FlyoutView : public FlyoutViewBase
{
    Q_OBJECT

public:
    explicit FlyoutView(QWidget *parent = nullptr);
    FlyoutView(const QString &title, const QString &content, QWidget *parent = nullptr);
    FlyoutView(const QString &title, const QString &content, const QIcon &icon,
               const QImage &image, bool isClosable, QWidget *parent = nullptr);
    FlyoutView(const QString &title, const QString &content, const QIcon &icon,
               const QPixmap &image, bool isClosable, QWidget *parent = nullptr);
    FlyoutView(const QString &title, const QString &content, const QIcon &icon,
               const QString &imagePath, bool isClosable, QWidget *parent = nullptr);

    QString title() const;
    QString content() const;
    QIcon viewIcon() const;
    bool isClosable() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    IconWidget *iconWidget() const;
    ImageLabel *imageLabel() const;
    QToolButton *closeButton() const;

    void setTitle(const QString &title);
    void setContent(const QString &content);
    void setIcon(const QIcon &icon);
    void setClosable(bool closable);
    void setImage(const QPixmap &pixmap);
    void setImage(const QImage &image);
    bool setImagePath(const QString &path);
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::AlignLeft) override;
    QVBoxLayout *widgetLayout() const;
    QVBoxLayout *bodyLayout() const;

protected:
    void showEvent(QShowEvent *event) override;

private:
    void initWidgets();
    void initLayout();
    void adjustText();
    void adjustImage();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    IconWidget *m_iconWidget = nullptr;
    ImageLabel *m_imageLabel = nullptr;
    QToolButton *m_closeButton = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QHBoxLayout *m_viewLayout = nullptr;
    QVBoxLayout *m_widgetLayout = nullptr;
    QVBoxLayout *m_bodyLayout = nullptr;
    QString m_title;
    QString m_content;
    bool m_closable = false;
};

class FQW_API AcrylicFlyoutViewBase : public FlyoutViewBase
{
    Q_OBJECT

public:
    explicit AcrylicFlyoutViewBase(QWidget *parent = nullptr);
    ~AcrylicFlyoutViewBase() override;

    AcrylicBrush *acrylicBrush() const;
    virtual QPainterPath acrylicClipPath() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void drawAcrylic(QPainter *painter);
    QColor acrylicBorderColor() const;
    void updateAcrylicColor();

private:
    AcrylicBrush *m_acrylicBrush = nullptr;
};

class FQW_API AcrylicFlyoutView : public FlyoutView
{
    Q_OBJECT

public:
    explicit AcrylicFlyoutView(QWidget *parent = nullptr);
    AcrylicFlyoutView(const QString &title, const QString &content, QWidget *parent = nullptr);
    AcrylicFlyoutView(const QString &title, const QString &content, const QIcon &icon,
                      const QImage &image, bool isClosable, QWidget *parent = nullptr);
    AcrylicFlyoutView(const QString &title, const QString &content, const QIcon &icon,
                      const QPixmap &image, bool isClosable, QWidget *parent = nullptr);
    AcrylicFlyoutView(const QString &title, const QString &content, const QIcon &icon,
                      const QString &imagePath, bool isClosable, QWidget *parent = nullptr);
    ~AcrylicFlyoutView() override;

    AcrylicBrush *acrylicBrush() const;
    virtual QPainterPath acrylicClipPath() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void drawAcrylic(QPainter *painter);
    QColor acrylicBorderColor() const;
    void updateAcrylicColor();

private:
    AcrylicBrush *m_acrylicBrush = nullptr;
};

class FQW_API Flyout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::FlyoutAnimationType animationType READ animationType WRITE setAnimationType)
    Q_PROPERTY(bool popupMode READ isPopupMode WRITE setPopupMode)

public:
    explicit Flyout(QWidget *parent = nullptr);
    explicit Flyout(FlyoutViewBase *view, QWidget *parent = nullptr, bool isDeleteOnClose = true);

    static Flyout *create(FlyoutViewBase *view, QWidget *target,
                          FlyoutAnimationType animation = FlyoutAnimationType::DropDown);
    static Flyout *create(FlyoutViewBase *view, const QPoint &pos,
                          FlyoutAnimationType animation = FlyoutAnimationType::PullUp);
    static Flyout *make(FlyoutViewBase *view, QWidget *target = nullptr, QWidget *parent = nullptr,
                        FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                        bool isDeleteOnClose = true);
    static Flyout *make(FlyoutViewBase *view, const QPoint &pos, QWidget *parent = nullptr,
                        FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                        bool isDeleteOnClose = true);
    static Flyout *make(const QString &title, const QString &content, QWidget *target,
                        FlyoutAnimationType animation = FlyoutAnimationType::DropDown);
    static Flyout *create(const QString &title, const QString &content, const QIcon &icon = QIcon(),
                          const QPixmap &image = QPixmap(), bool isClosable = false,
                          QWidget *target = nullptr, QWidget *parent = nullptr,
                          FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                          bool isDeleteOnClose = true);
    static Flyout *create(const QString &title, const QString &content, const QIcon &icon,
                          const QString &imagePath, bool isClosable, QWidget *target = nullptr,
                          QWidget *parent = nullptr,
                          FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                          bool isDeleteOnClose = true);
    static Flyout *create(const QString &title, const QString &content, const QIcon &icon,
                          const QPixmap &image, bool isClosable, const QPoint &pos,
                          QWidget *parent = nullptr,
                          FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                          bool isDeleteOnClose = true);
    static Flyout *create(const QString &title, const QString &content, const QIcon &icon,
                          const QString &imagePath, bool isClosable, const QPoint &pos,
                          QWidget *parent = nullptr,
                          FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                          bool isDeleteOnClose = true);

    FlyoutAnimationType animationType() const;
    void setAnimationType(FlyoutAnimationType type);
    FlyoutViewBase *view() const;
    void exec(QWidget *target);
    void exec(const QPoint &pos, FlyoutAnimationType animationType = FlyoutAnimationType::PullUp);
    void fadeOut();

    bool isDeleteOnClose() const;
    void setIsDeleteOnClose(bool value);
    bool isPopupMode() const;
    void setPopupMode(bool value);
    void setShadowEffect(int blurRadius = 35, const QPoint &offset = QPoint(0, 8));

signals:
    void closed();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QPoint calculatePosition(QWidget *target) const;

    FlyoutAnimationType m_animationType = FlyoutAnimationType::DropDown;
    FlyoutViewBase *m_view = nullptr;
    bool m_isDeleteOnClose = true;
    bool m_popupMode = true;
    bool m_eventFilterInstalled = false;
    QGraphicsDropShadowEffect *m_shadowEffect = nullptr;
    QHBoxLayout *m_layout = nullptr;
};

class FQW_API AcrylicFlyout : public Flyout
{
    Q_OBJECT

public:
    explicit AcrylicFlyout(QWidget *parent = nullptr);
    explicit AcrylicFlyout(FlyoutViewBase *view, QWidget *parent = nullptr,
                           bool isDeleteOnClose = true);

    static AcrylicFlyout *create(FlyoutViewBase *view, QWidget *target,
                                 FlyoutAnimationType animation = FlyoutAnimationType::DropDown);
    static AcrylicFlyout *create(FlyoutViewBase *view, const QPoint &pos,
                                 FlyoutAnimationType animation = FlyoutAnimationType::PullUp);
    static AcrylicFlyout *make(FlyoutViewBase *view, QWidget *target = nullptr, QWidget *parent = nullptr,
                               FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                               bool isDeleteOnClose = true);
    static AcrylicFlyout *make(FlyoutViewBase *view, const QPoint &pos, QWidget *parent = nullptr,
                               FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                               bool isDeleteOnClose = true);
    static AcrylicFlyout *create(const QString &title, const QString &content, const QIcon &icon = QIcon(),
                                 const QPixmap &image = QPixmap(), bool isClosable = false,
                                 QWidget *target = nullptr, QWidget *parent = nullptr,
                                 FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                                 bool isDeleteOnClose = true);
    static AcrylicFlyout *create(const QString &title, const QString &content, const QIcon &icon,
                                 const QString &imagePath, bool isClosable, QWidget *target = nullptr,
                                 QWidget *parent = nullptr,
                                 FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                                 bool isDeleteOnClose = true);
    static AcrylicFlyout *create(const QString &title, const QString &content, const QIcon &icon,
                                 const QPixmap &image, bool isClosable, const QPoint &pos,
                                 QWidget *parent = nullptr,
                                 FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                                 bool isDeleteOnClose = true);
    static AcrylicFlyout *create(const QString &title, const QString &content, const QIcon &icon,
                                 const QString &imagePath, bool isClosable, const QPoint &pos,
                                 QWidget *parent = nullptr,
                                 FlyoutAnimationType animation = FlyoutAnimationType::PullUp,
                                 bool isDeleteOnClose = true);
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::FlyoutAnimationType)
