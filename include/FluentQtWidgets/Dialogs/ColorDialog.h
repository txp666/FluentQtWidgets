#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>

#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class QMouseEvent;
class QPaintEvent;
class QShowEvent;

namespace FluentQt {

class FQW_API HuePanel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QPoint pickerPosition READ pickerPosition)

  public:
    explicit HuePanel(const QColor &color = QColor(255, 0, 0), QWidget *parent = nullptr);

    QColor color() const;
    QPoint pickerPosition() const;
    int hue() const;
    int saturation() const;

  public slots:
    void setColor(const QColor &color);

  signals:
    void colorChanged(const QColor &color);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    void setPickerPosition(const QPoint &position);

    QColor m_color;
    QPoint m_pickerPosition;
    QPixmap m_huePixmap;
};

class FQW_API BrightnessSlider : public ClickableSlider
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

  public:
    explicit BrightnessSlider(const QColor &color, QWidget *parent = nullptr);

    QColor color() const;

  public slots:
    void setColor(const QColor &color);

  signals:
    void colorChanged(const QColor &color);

  private:
    void updateSliderStyle();

    QColor m_color;
};

class FQW_API ColorCard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(bool alphaEnabled READ isAlphaEnabled WRITE setAlphaEnabled)

  public:
    explicit ColorCard(const QColor &color, QWidget *parent = nullptr, bool enableAlpha = false);

    QColor color() const;
    bool isAlphaEnabled() const;

  public slots:
    void setColor(const QColor &color);
    void setAlphaEnabled(bool enabled);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QColor m_color;
    bool m_alphaEnabled = false;
};

class FQW_API ColorLineEdit : public LineEdit
{
    Q_OBJECT

  public:
    explicit ColorLineEdit(int value, QWidget *parent = nullptr);

  signals:
    void valueChanged(const QString &value);

  private:
    void onTextEdited(const QString &text);
};

class FQW_API HexColorLineEdit : public ColorLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QColor::NameFormat colorFormat READ colorFormat)

  public:
    explicit HexColorLineEdit(const QColor &color, QWidget *parent = nullptr, bool enableAlpha = false);

    QColor::NameFormat colorFormat() const;

  public slots:
    void setColor(const QColor &color);

  private:
    QColor::NameFormat m_colorFormat = QColor::HexRgb;
    QLabel *m_prefixLabel = nullptr;
};

class FQW_API OpacityLineEdit : public ColorLineEdit
{
    Q_OBJECT

  public:
    explicit OpacityLineEdit(int alpha, QWidget *parent = nullptr);

  protected:
    void showEvent(QShowEvent *event) override;

  private:
    void adjustSuffixPosition();

    QLabel *m_suffixLabel = nullptr;
};

class FQW_API ColorDialog : public MaskDialogBase
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor oldColor READ oldColor)
    Q_PROPERTY(bool alphaEnabled READ isAlphaEnabled)

  public:
    explicit ColorDialog(const QColor &color, const QString &title, QWidget *parent = nullptr,
                         bool enableAlpha = false);

    QColor color() const;
    QColor oldColor() const;
    bool isAlphaEnabled() const;

    SingleDirectionScrollArea *scrollArea() const;
    QWidget *scrollWidget() const;
    QFrame *buttonGroup() const;
    QPushButton *yesButton() const;
    QPushButton *cancelButton() const;
    QLabel *titleLabel() const;
    QLabel *editLabel() const;
    QLabel *redLabel() const;
    QLabel *greenLabel() const;
    QLabel *blueLabel() const;
    QLabel *opacityLabel() const;
    HuePanel *huePanel() const;
    ColorCard *newColorCard() const;
    ColorCard *oldColorCard() const;
    BrightnessSlider *brightnessSlider() const;
    HexColorLineEdit *hexLineEdit() const;
    ColorLineEdit *redLineEdit() const;
    ColorLineEdit *greenLineEdit() const;
    ColorLineEdit *blueLineEdit() const;
    OpacityLineEdit *opacityLineEdit() const;

  public slots:
    void setColor(const QColor &color);
    void setColor(const QColor &color, bool movePicker);
    void updateStyle();

  signals:
    void colorChanged(const QColor &color);

  protected:
    void showEvent(QShowEvent *event) override;

  private:
    void initLayout();
    void connectSignals();
    void onHueChanged(const QColor &color);
    void onBrightnessChanged(const QColor &color);
    void onRedChanged(const QString &red);
    void onGreenChanged(const QString &green);
    void onBlueChanged(const QString &blue);
    void onOpacityChanged(const QString &opacity);
    void onHexColorChanged(const QString &color);
    void onYesButtonClicked();

    bool m_alphaEnabled = false;
    QColor m_oldColor;
    QColor m_color;
    SingleDirectionScrollArea *m_scrollArea = nullptr;
    QWidget *m_scrollWidget = nullptr;
    QFrame *m_buttonGroup = nullptr;
    QPushButton *m_yesButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_editLabel = nullptr;
    QLabel *m_redLabel = nullptr;
    QLabel *m_greenLabel = nullptr;
    QLabel *m_blueLabel = nullptr;
    QLabel *m_opacityLabel = nullptr;
    HuePanel *m_huePanel = nullptr;
    ColorCard *m_newColorCard = nullptr;
    ColorCard *m_oldColorCard = nullptr;
    BrightnessSlider *m_brightnessSlider = nullptr;
    HexColorLineEdit *m_hexLineEdit = nullptr;
    ColorLineEdit *m_redLineEdit = nullptr;
    ColorLineEdit *m_greenLineEdit = nullptr;
    ColorLineEdit *m_blueLineEdit = nullptr;
    OpacityLineEdit *m_opacityLineEdit = nullptr;
};

} // namespace FluentQt
