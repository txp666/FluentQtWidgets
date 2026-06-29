#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtWidgets/QCheckBox>

class QPaintEvent;
class QEnterEvent;
class QMouseEvent;
class QEvent;

namespace FluentQt {

class FQW_API CheckBox : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(QColor lightTextColor READ lightTextColor WRITE setLightTextColor)
    Q_PROPERTY(QColor darkTextColor READ darkTextColor WRITE setDarkTextColor)
    Q_PROPERTY(QColor lightCheckedColor READ lightCheckedColor WRITE setLightCheckedColor)
    Q_PROPERTY(QColor darkCheckedColor READ darkCheckedColor WRITE setDarkCheckedColor)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit CheckBox(QWidget *parent = nullptr);
    explicit CheckBox(const QString &text, QWidget *parent = nullptr);

    QColor lightTextColor() const;
    QColor darkTextColor() const;
    QColor lightCheckedColor() const;
    QColor darkCheckedColor() const;
    bool isPressed() const;
    bool isHover() const;

  public slots:
    void setLightTextColor(const QColor &color);
    void setDarkTextColor(const QColor &color);
    void setLightCheckedColor(const QColor &color);
    void setDarkCheckedColor(const QColor &color);
    void setCheckedColor(const QColor &light, const QColor &dark);
    void setTextColor(const QColor &light, const QColor &dark);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

  private:
    enum class State
    {
        Normal,
        Hover,
        Pressed,
        Checked,
        CheckedHover,
        CheckedPressed,
        Disabled,
        CheckedDisabled
    };

    void init();
    State state() const;
    QColor textColor() const;
    QRect indicatorRect() const;
    QRect textRect() const;
    QColor borderColor(State state) const;
    QColor backgroundColor(State state) const;
    QString indicatorIconPath() const;
    void updateTextStyle();

    bool m_isPressed = false;
    bool m_isHover = false;
    QColor m_lightCheckedColor;
    QColor m_darkCheckedColor;
    QColor m_lightTextColor = QColor(0, 0, 0);
    QColor m_darkTextColor = QColor(255, 255, 255);
};

} // namespace FluentQt
