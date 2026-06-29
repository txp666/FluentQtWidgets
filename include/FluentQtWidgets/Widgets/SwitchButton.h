#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

class QHBoxLayout;
class QLabel;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;

namespace FluentQt {

enum class IndicatorPosition
{
    Left = 0,
    Right = 1
};

class FQW_API SwitchIndicator : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(qreal sliderX READ sliderX WRITE setSliderX)
    Q_PROPERTY(QColor lightCheckedColor READ lightCheckedColor WRITE setLightCheckedColor)
    Q_PROPERTY(QColor darkCheckedColor READ darkCheckedColor WRITE setDarkCheckedColor)

  public:
    explicit SwitchIndicator(QWidget *parent = nullptr);

    qreal sliderX() const;
    QColor lightCheckedColor() const;
    QColor darkCheckedColor() const;

  signals:
    void checkedChanged(bool checked);

  public slots:
    void toggleIndicator();
    void setDown(bool isDown);
    void setHover(bool isHover);
    void setLightCheckedColor(const QColor &color);
    void setDarkCheckedColor(const QColor &color);
    void setCheckedColor(const QColor &light, const QColor &dark);
    void setSliderX(qreal x);

  protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void animateToState();
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor sliderColor() const;
    QColor fallbackCheckedColor() const;

    qreal m_sliderX = 5.0;
    bool m_isPressed = false;
    bool m_isHover = false;
    QColor m_lightCheckedColor;
    QColor m_darkCheckedColor;
};

class FQW_API SwitchButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString onText READ onText WRITE setOnText)
    Q_PROPERTY(QString offText READ offText WRITE setOffText)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(IndicatorPosition indicatorPosition READ indicatorPosition WRITE setIndicatorPosition)
    Q_PROPERTY(IndicatorPosition indicatorPos READ indicatorPos WRITE setIndicatorPos)
    Q_PROPERTY(QColor lightTextColor READ lightTextColor WRITE setLightTextColor)
    Q_PROPERTY(QColor darkTextColor READ darkTextColor WRITE setDarkTextColor)

  public:
    explicit SwitchButton(QWidget *parent = nullptr, IndicatorPosition pos = IndicatorPosition::Left);
    explicit SwitchButton(const QString &text, QWidget *parent = nullptr,
                          IndicatorPosition pos = IndicatorPosition::Left);

    bool isChecked() const;
    QString text() const;
    QString onText() const;
    QString offText() const;
    int spacing() const;
    IndicatorPosition indicatorPosition() const;
    IndicatorPosition indicatorPos() const;
    QColor lightTextColor() const;
    QColor darkTextColor() const;
    SwitchIndicator *indicator() const;
    QLabel *label() const;
    QHBoxLayout *hBox() const;
    QHBoxLayout *hBoxLayout() const;

  public slots:
    void setChecked(bool checked);
    void toggle();
    void toggleChecked();
    void setText(const QString &text);
    void setOnText(const QString &text);
    void setOffText(const QString &text);
    void setSpacing(int spacing);
    void setLightTextColor(const QColor &color);
    void setDarkTextColor(const QColor &color);
    void setTextColor(const QColor &light, const QColor &dark);
    void setCheckedIndicatorColor(const QColor &light, const QColor &dark);
    void setIndicatorPosition(IndicatorPosition pos);
    void setIndicatorPos(IndicatorPosition pos);

  signals:
    void checkedChanged(bool checked);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    void initWidget();
    void updateText();

    SwitchIndicator *m_indicator = nullptr;
    QLabel *m_label = nullptr;
    QHBoxLayout *m_layout = nullptr;
    IndicatorPosition m_indicatorPosition = IndicatorPosition::Left;
    QString m_onText;
    QString m_offText;
    QColor m_lightTextColor;
    QColor m_darkTextColor;
};

} // namespace FluentQt
