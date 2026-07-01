#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QPoint>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

class QEnterEvent;
class QEvent;
class QHBoxLayout;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QPropertyAnimation;

namespace FluentQt {

class RoundMenu;

enum class ButtonStatus
{
    Information,
    Success,
    Attention,
    Warning,
    Error
};

// --- Base Buttons ---

class FQW_API PushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)
  public:
    explicit PushButton(QWidget *parent = nullptr);
    explicit PushButton(const QString &text, QWidget *parent = nullptr);
    PushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    QIcon icon() const;
    bool isPressed() const;
    bool isHover() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  public slots:
    virtual void setIcon(const QIcon &icon);

  protected:
    void initFluentButton(const QString &role);
    virtual void drawIcon(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    QIcon m_storedIcon;
    bool m_isPressed = false;
    bool m_isHover = false;
};

class FQW_API PrimaryPushButton : public PushButton
{
  public:
    explicit PrimaryPushButton(QWidget *parent = nullptr);
    explicit PrimaryPushButton(const QString &text, QWidget *parent = nullptr);
    PrimaryPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    void setIcon(const QIcon &icon) override;

  protected:
    void drawIcon(QPainter *painter, const QRectF &rect) override;
};

class FQW_API TransparentPushButton : public PushButton
{
  public:
    explicit TransparentPushButton(QWidget *parent = nullptr);
    explicit TransparentPushButton(const QString &text, QWidget *parent = nullptr);
    TransparentPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
};

class FQW_API TogglePushButton : public PushButton
{
    Q_OBJECT
  public:
    explicit TogglePushButton(QWidget *parent = nullptr);
    explicit TogglePushButton(const QString &text, QWidget *parent = nullptr);
    TogglePushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

  protected:
    void drawIcon(QPainter *painter, const QRectF &rect) override;
};

class FQW_API ToggleButton : public TogglePushButton
{
    Q_OBJECT
  public:
    explicit ToggleButton(QWidget *parent = nullptr);
    explicit ToggleButton(const QString &text, QWidget *parent = nullptr);
    ToggleButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
};

class FQW_API TransparentTogglePushButton : public TogglePushButton
{
    Q_OBJECT
  public:
    explicit TransparentTogglePushButton(QWidget *parent = nullptr);
    explicit TransparentTogglePushButton(const QString &text, QWidget *parent = nullptr);
    TransparentTogglePushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
};

class FQW_API DropDownPushButton : public PushButton
{
    Q_OBJECT
  public:
    explicit DropDownPushButton(QWidget *parent = nullptr);
    explicit DropDownPushButton(const QString &text, QWidget *parent = nullptr);
    DropDownPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    void setMenu(RoundMenu *menu);
    RoundMenu *menu() const;
    void setDropDownMenu(RoundMenu *menu);
    RoundMenu *dropDownMenu() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void animateArrow(int endValue, int duration, QEasingCurve::Type easing);
    RoundMenu *m_menu = nullptr;
    QPropertyAnimation *m_arrowAnimation = nullptr;
    int m_arrowOffset = 0;
    Q_PROPERTY(int arrowOffset READ arrowOffset WRITE setArrowOffset)
    int arrowOffset() const;
    void setArrowOffset(int offset);
};

class FQW_API TransparentDropDownPushButton : public DropDownPushButton
{
    Q_OBJECT
  public:
    explicit TransparentDropDownPushButton(QWidget *parent = nullptr);
    explicit TransparentDropDownPushButton(const QString &text, QWidget *parent = nullptr);
    TransparentDropDownPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
};

class FQW_API PrimaryDropDownPushButton : public PrimaryPushButton
{
    Q_OBJECT
  public:
    explicit PrimaryDropDownPushButton(QWidget *parent = nullptr);
    explicit PrimaryDropDownPushButton(const QString &text, QWidget *parent = nullptr);
    PrimaryDropDownPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    void setMenu(RoundMenu *menu);
    RoundMenu *menu() const;
    void setDropDownMenu(RoundMenu *menu);
    RoundMenu *dropDownMenu() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void showMenu();
    void animateArrow(int endValue, int duration, QEasingCurve::Type easing);
    RoundMenu *m_menu = nullptr;
    QPropertyAnimation *m_arrowAnimation = nullptr;
    int m_arrowOffset = 0;
    Q_PROPERTY(int arrowOffset READ arrowOffset WRITE setArrowOffset)
    int arrowOffset() const;
    void setArrowOffset(int offset);
};

class FQW_API ToolButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)
  public:
    explicit ToolButton(QWidget *parent = nullptr);
    explicit ToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit ToolButton(const QString &iconPath, QWidget *parent = nullptr);

    QIcon icon() const;
    bool isPressed() const;
    bool isHover() const;

  public slots:
    virtual void setIcon(const QIcon &icon);

  protected:
    void initFluentToolButton(const QString &role);
    virtual void drawIcon(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    QIcon m_storedIcon;
    bool m_isPressed = false;
    bool m_isHover = false;
};

class FQW_API PrimaryToolButton : public ToolButton
{
  public:
    explicit PrimaryToolButton(QWidget *parent = nullptr);
    explicit PrimaryToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit PrimaryToolButton(const QString &iconPath, QWidget *parent = nullptr);

    void setIcon(const QIcon &icon) override;

  protected:
    void drawIcon(QPainter *painter, const QRectF &rect) override;
};

class FQW_API TransparentToolButton : public ToolButton
{
  public:
    explicit TransparentToolButton(QWidget *parent = nullptr);
    explicit TransparentToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit TransparentToolButton(const QString &iconPath, QWidget *parent = nullptr);
};

class FQW_API ToggleToolButton : public ToolButton
{
    Q_OBJECT
  public:
    explicit ToggleToolButton(QWidget *parent = nullptr);
    explicit ToggleToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit ToggleToolButton(const QString &iconPath, QWidget *parent = nullptr);

  protected:
    void drawIcon(QPainter *painter, const QRectF &rect) override;
};

class FQW_API TransparentToggleToolButton : public ToggleToolButton
{
    Q_OBJECT
  public:
    explicit TransparentToggleToolButton(QWidget *parent = nullptr);
    explicit TransparentToggleToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit TransparentToggleToolButton(const QString &iconPath, QWidget *parent = nullptr);
};

class FQW_API DropDownToolButton : public ToolButton
{
    Q_OBJECT
  public:
    explicit DropDownToolButton(QWidget *parent = nullptr);
    explicit DropDownToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit DropDownToolButton(const QString &iconPath, QWidget *parent = nullptr);

    void setMenu(RoundMenu *menu);
    RoundMenu *menu() const;
    void setDropDownMenu(RoundMenu *menu);
    RoundMenu *dropDownMenu() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void showMenu();
    void animateArrow(int endValue, int duration, QEasingCurve::Type easing);
    RoundMenu *m_menu = nullptr;
    QPropertyAnimation *m_arrowAnimation = nullptr;
    int m_arrowOffset = 0;
    Q_PROPERTY(int arrowOffset READ arrowOffset WRITE setArrowOffset)
    int arrowOffset() const;
    void setArrowOffset(int offset);
};

class FQW_API TransparentDropDownToolButton : public DropDownToolButton
{
    Q_OBJECT
  public:
    explicit TransparentDropDownToolButton(QWidget *parent = nullptr);
    explicit TransparentDropDownToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit TransparentDropDownToolButton(const QString &iconPath, QWidget *parent = nullptr);
};

class FQW_API PrimaryDropDownToolButton : public PrimaryToolButton
{
    Q_OBJECT
  public:
    explicit PrimaryDropDownToolButton(QWidget *parent = nullptr);
    explicit PrimaryDropDownToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit PrimaryDropDownToolButton(const QString &iconPath, QWidget *parent = nullptr);

    void setMenu(RoundMenu *menu);
    RoundMenu *menu() const;
    void setDropDownMenu(RoundMenu *menu);
    RoundMenu *dropDownMenu() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void showMenu();
    void animateArrow(int endValue, int duration, QEasingCurve::Type easing);
    RoundMenu *m_menu = nullptr;
    QPropertyAnimation *m_arrowAnimation = nullptr;
    int m_arrowOffset = 0;
    Q_PROPERTY(int arrowOffset READ arrowOffset WRITE setArrowOffset)
    int arrowOffset() const;
    void setArrowOffset(int offset);
};

// --- HyperlinkButton ---

class FQW_API HyperlinkButton : public PushButton
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit HyperlinkButton(QWidget *parent = nullptr);
    explicit HyperlinkButton(const QString &url, const QString &text, QWidget *parent = nullptr);
    HyperlinkButton(const QIcon &icon, const QString &url, const QString &text, QWidget *parent = nullptr);

    QUrl url() const;
    bool isPressed() const;
    bool isHover() const;

  public slots:
    void setUrl(const QUrl &url);
    void setUrl(const QString &url);
    void setIcon(const QIcon &icon) override;

  protected:
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    void onLinkClicked();
    QUrl m_url;
};

class FQW_API HyperlinkToolButton : public ToolButton
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

  public:
    explicit HyperlinkToolButton(QWidget *parent = nullptr);
    HyperlinkToolButton(const QIcon &icon, const QString &url, QWidget *parent = nullptr);
    HyperlinkToolButton(const QString &iconPath, const QString &url, QWidget *parent = nullptr);

    QUrl url() const;

  public slots:
    void setUrl(const QUrl &url);
    void setUrl(const QString &url);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    void onLinkClicked();
    QUrl m_url;
};

class FQW_API FilledPushButton : public PrimaryPushButton
{
    Q_OBJECT

  public:
    explicit FilledPushButton(QWidget *parent = nullptr);
    explicit FilledPushButton(const QString &text, QWidget *parent = nullptr);
    FilledPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    ButtonStatus status() const;

  public slots:
    virtual void setSeverity(InfoBarSeverity severity);
    virtual void setStatus(ButtonStatus status);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API FilledToolButton : public PrimaryToolButton
{
    Q_OBJECT

  public:
    explicit FilledToolButton(QWidget *parent = nullptr);
    explicit FilledToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit FilledToolButton(const QString &iconPath, QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    ButtonStatus status() const;

  public slots:
    virtual void setSeverity(InfoBarSeverity severity);
    virtual void setStatus(ButtonStatus status);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API TextPushButton : public PushButton
{
    Q_OBJECT

  public:
    explicit TextPushButton(QWidget *parent = nullptr);
    explicit TextPushButton(const QString &text, QWidget *parent = nullptr);
    TextPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    ButtonStatus status() const;

  public slots:
    virtual void setSeverity(InfoBarSeverity severity);
    virtual void setStatus(ButtonStatus status);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API TextToolButton : public ToolButton
{
    Q_OBJECT

  public:
    explicit TextToolButton(QWidget *parent = nullptr);
    explicit TextToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit TextToolButton(const QString &iconPath, QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    ButtonStatus status() const;

  public slots:
    virtual void setSeverity(InfoBarSeverity severity);
    virtual void setStatus(ButtonStatus status);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API LuminaPushButton : public FilledPushButton
{
    Q_OBJECT

  public:
    explicit LuminaPushButton(QWidget *parent = nullptr);
    explicit LuminaPushButton(const QString &text, QWidget *parent = nullptr);
    LuminaPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

  public slots:
    void setSeverity(InfoBarSeverity severity) override;
    void setStatus(ButtonStatus status) override;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void updateGlow();
};

class FQW_API OutlinedPushButton : public PushButton
{
    Q_OBJECT

  public:
    explicit OutlinedPushButton(QWidget *parent = nullptr);
    explicit OutlinedPushButton(const QString &text, QWidget *parent = nullptr);
    OutlinedPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    ButtonStatus status() const;

  public slots:
    virtual void setSeverity(InfoBarSeverity severity);
    virtual void setStatus(ButtonStatus status);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API OutlinedToolButton : public ToolButton
{
    Q_OBJECT

  public:
    explicit OutlinedToolButton(QWidget *parent = nullptr);
    explicit OutlinedToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit OutlinedToolButton(const QString &iconPath, QWidget *parent = nullptr);

    InfoBarSeverity severity() const;
    ButtonStatus status() const;

  public slots:
    virtual void setSeverity(InfoBarSeverity severity);
    virtual void setStatus(ButtonStatus status);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawIcon(QPainter *painter, const QRectF &rect) override;

  private:
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API RoundPushButton : public PushButton
{
    Q_OBJECT

  public:
    explicit RoundPushButton(QWidget *parent = nullptr);
    explicit RoundPushButton(const QString &text, QWidget *parent = nullptr);
    RoundPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API RoundToolButton : public ToolButton
{
    Q_OBJECT

  public:
    explicit RoundToolButton(QWidget *parent = nullptr);
    explicit RoundToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit RoundToolButton(const QString &iconPath, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API Chip : public PushButton
{
    Q_OBJECT
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)

  public:
    explicit Chip(QWidget *parent = nullptr);
    explicit Chip(const QString &text, QWidget *parent = nullptr);
    Chip(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    bool isClosable() const;
    ButtonStatus status() const;
    QSize sizeHint() const override;

  public slots:
    void setClosable(bool closable);
    void setStatus(ButtonStatus status);

  signals:
    void closedSignal();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    QRect closeButtonRect() const;
    bool m_closable = true;
    bool m_closePressed = false;
    ButtonStatus m_status = ButtonStatus::Information;
};

class FQW_API Tag : public OutlinedPushButton
{
    Q_OBJECT

  public:
    explicit Tag(QWidget *parent = nullptr);
    explicit Tag(const QString &text, QWidget *parent = nullptr);
    Tag(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API RadioButton : public QRadioButton
{
    Q_OBJECT
    Q_PROPERTY(QColor lightTextColor READ lightTextColor WRITE setLightTextColor)
    Q_PROPERTY(QColor darkTextColor READ darkTextColor WRITE setDarkTextColor)
    Q_PROPERTY(QColor lightIndicatorColor READ lightIndicatorColor WRITE setLightIndicatorColor)
    Q_PROPERTY(QColor darkIndicatorColor READ darkIndicatorColor WRITE setDarkIndicatorColor)
    Q_PROPERTY(QPoint indicatorPos READ indicatorPos WRITE setIndicatorPos)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit RadioButton(QWidget *parent = nullptr);
    explicit RadioButton(const QString &text, QWidget *parent = nullptr);

    QColor lightTextColor() const;
    QColor darkTextColor() const;
    QColor lightIndicatorColor() const;
    QColor darkIndicatorColor() const;
    QPoint indicatorPos() const;
    bool isHover() const;

  public slots:
    void setLightTextColor(const QColor &color);
    void setDarkTextColor(const QColor &color);
    void setLightIndicatorColor(const QColor &color);
    void setDarkIndicatorColor(const QColor &color);
    void setTextColor(const QColor &light, const QColor &dark);
    void setIndicatorColor(const QColor &light, const QColor &dark);
    void setIndicatorPos(const QPoint &pos);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

  private:
    void drawIndicator(QPainter *painter);
    void drawText(QPainter *painter);
    void drawCircle(QPainter *painter, const QPointF &center, int radius, int thickness, const QColor &borderColor,
                    const QColor &filledColor);
    QColor textColor() const;

    QColor m_lightTextColor = QColor(0, 0, 0);
    QColor m_darkTextColor = QColor(255, 255, 255);
    QColor m_lightIndicatorColor;
    QColor m_darkIndicatorColor;
    QPoint m_indicatorPos = QPoint(11, 12);
    bool m_isHover = false;
};

// --- Pill Buttons ---

class FQW_API PillPushButton : public TogglePushButton
{
    Q_OBJECT
  public:
    explicit PillPushButton(QWidget *parent = nullptr);
    explicit PillPushButton(const QString &text, QWidget *parent = nullptr);
    PillPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API PillToolButton : public ToggleToolButton
{
    Q_OBJECT
  public:
    explicit PillToolButton(QWidget *parent = nullptr);
    explicit PillToolButton(const QIcon &icon, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

// --- Split Buttons ---

class FQW_API SplitDropButton : public ToolButton
{
    Q_OBJECT
    Q_PROPERTY(qreal arrowOffset READ arrowOffset WRITE setArrowOffset)
    Q_PROPERTY(bool customDropIcon READ hasCustomDropIcon)

  public:
    explicit SplitDropButton(QWidget *parent = nullptr);

    qreal arrowOffset() const;
    bool hasCustomDropIcon() const;
    QIcon dropIcon() const;

  public slots:
    void setArrowOffset(qreal offset);
    void setDropIcon(const QIcon &icon);
    void setDropIconSize(const QSize &size);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual bool isPrimaryDropButton() const;
    virtual qreal idleOpacity() const;
    virtual qreal hoverOpacity() const;
    virtual qreal pressedOpacity() const;

  private:
    void animateArrow(qreal endValue, int duration, QEasingCurve::Type easing);

    bool m_hasCustomDropIcon = false;
    QIcon m_customDropIcon;
    qreal m_arrowOffset = 0.0;
    QPropertyAnimation *m_arrowAnimation = nullptr;
};

class FQW_API PrimarySplitDropButton : public SplitDropButton
{
    Q_OBJECT

  public:
    explicit PrimarySplitDropButton(QWidget *parent = nullptr);

  protected:
    bool isPrimaryDropButton() const override;
    qreal idleOpacity() const override;
    qreal hoverOpacity() const override;
    qreal pressedOpacity() const override;
};

class FQW_API SplitWidgetBase : public QWidget
{
    Q_OBJECT

  public:
    explicit SplitWidgetBase(QWidget *parent = nullptr);

    QToolButton *dropButton() const;
    QWidget *contentWidget() const;
    QHBoxLayout *hBoxLayout() const;
    RoundMenu *flyout() const;
    RoundMenu *dropDownMenu() const;

  public slots:
    void setWidget(QWidget *widget);
    void setDropButton(QToolButton *button);
    void setDropIcon(const QIcon &icon);
    void setDropIconSize(const QSize &size);
    void setFlyout(RoundMenu *flyout);
    void showFlyout();
    void setDropDownMenu(RoundMenu *menu);

  signals:
    void dropDownClicked();

  protected:
    void showMenu();
    void syncDropButtonSize();

    QWidget *m_contentWidget = nullptr;
    QToolButton *m_dropButton = nullptr;
    RoundMenu *m_menu = nullptr;
    QHBoxLayout *m_layout = nullptr;
};

class FQW_API SplitPushButton : public SplitWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit SplitPushButton(QWidget *parent = nullptr);
    explicit SplitPushButton(const QString &text, QWidget *parent = nullptr);
    SplitPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    QString text() const;
    QIcon icon() const;
    QSize iconSize() const;
    QPushButton *button() const;

  public slots:
    void setText(const QString &text);
    void setIcon(const QIcon &icon);
    void setIconSize(const QSize &size);

  signals:
    void clicked();

  private:
    QPushButton *m_button = nullptr;
};

class FQW_API SplitToolButton : public SplitWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit SplitToolButton(QWidget *parent = nullptr);
    explicit SplitToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit SplitToolButton(const QString &iconPath, QWidget *parent = nullptr);

    QIcon icon() const;
    QSize iconSize() const;
    QToolButton *button() const;

  public slots:
    void setIcon(const QIcon &icon);
    void setIconSize(const QSize &size);

  signals:
    void clicked();

  private:
    QToolButton *m_button = nullptr;
};

class FQW_API PrimarySplitPushButton : public SplitWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit PrimarySplitPushButton(QWidget *parent = nullptr);
    explicit PrimarySplitPushButton(const QString &text, QWidget *parent = nullptr);
    PrimarySplitPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    QString text() const;
    QIcon icon() const;
    QSize iconSize() const;
    QPushButton *button() const;

  public slots:
    void setText(const QString &text);
    void setIcon(const QIcon &icon);
    void setIconSize(const QSize &size);

  signals:
    void clicked();

  private:
    QPushButton *m_button = nullptr;
};

class FQW_API PrimarySplitToolButton : public SplitWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit PrimarySplitToolButton(QWidget *parent = nullptr);
    explicit PrimarySplitToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit PrimarySplitToolButton(const QString &iconPath, QWidget *parent = nullptr);

    QIcon icon() const;
    QSize iconSize() const;
    QToolButton *button() const;

  public slots:
    void setIcon(const QIcon &icon);
    void setIconSize(const QSize &size);

  signals:
    void clicked();

  private:
    QToolButton *m_button = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::ButtonStatus)
