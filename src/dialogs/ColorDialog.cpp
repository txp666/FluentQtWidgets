#include <FluentQtWidgets/Dialogs/ColorDialog.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtCore/QRegularExpression>
#include <QtCore/QAbstractAnimation>
#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSignalBlocker>
#include <QtGui/QIntValidator>
#include <QtGui/QLinearGradient>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QRegularExpressionValidator>
#include <QtGui/QShowEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

class ColorDialogScrollArea : public SingleDirectionScrollArea
{
  public:
    explicit ColorDialogScrollArea(QWidget *parent = nullptr) : SingleDirectionScrollArea(Qt::Vertical, parent) {}

    void setViewportInset(int left, int top, int right, int bottom) { setViewportMargins(left, top, right, bottom); }
};

int normalizedHue(const QColor &color)
{
    const int hue = color.hue();
    return hue < 0 ? 0 : hue;
}

QColor normalizedDialogColor(const QColor &color, bool alphaEnabled)
{
    QColor normalized = color.isValid() ? color : ThemeManager::instance()->accentColor();
    if (!alphaEnabled) {
        normalized.setAlpha(255);
    }
    return normalized;
}

QColor hsvColor(int hue, int saturation, int value, int alpha)
{
    QColor color;
    color.setHsv(qBound(0, hue, 359), qBound(0, saturation, 255), qBound(0, value, 255),
                 qBound(0, alpha, 255));
    return color;
}

QString brightnessSliderStyleSheet(const QColor &color)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QString handleBorder = dark ? QStringLiteral("rgb(55, 55, 55)") : QStringLiteral("rgb(222, 222, 222)");
    const QString handleGradient =
        dark ? QStringLiteral("stop:0 rgb(255, 255, 255), stop:0.55 rgb(255, 255, 255), stop:0.6 rgb(69, 69, 69), stop:1 rgb(69, 69, 69)")
             : QStringLiteral("stop:0 rgb(0, 0, 0), stop:0.5 rgb(0, 0, 0), stop:0.6 rgb(255, 255, 255), stop:1 rgb(255, 255, 255)");

    return QStringLiteral(R"(
QSlider:horizontal {
    min-width: 332px;
    min-height: 24px;
}

QSlider::groove:horizontal {
    height: 12px;
    border-radius: 6px;
    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,
            stop:0 hsv(%1, %2, 0),
            stop:1 hsv(%1, %2, 255));
}

QSlider::handle:horizontal {
    border: 1px solid %3;
    width: 16px;
    min-height: 10px;
    margin: -3px 0;
    border-radius: 9px;
    background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,
            %4);
}

QSlider::groove:horizontal:disabled {
    background-color: rgba(0, 0, 0, 75);
}

QSlider::handle:horizontal:disabled {
    background-color: #808080;
    border: 6px solid #cccccc;
}
)")
        .arg(normalizedHue(color))
        .arg(qBound(0, color.saturation(), 255))
        .arg(handleBorder, handleGradient);
}

} // namespace

HuePanel::HuePanel(const QColor &color, QWidget *parent) : QWidget(parent)
{
    setFixedSize(256, 256);
    m_huePixmap.load(QStringLiteral(":/qfluentwidgets/images/color_dialog/HuePanel.png"));
    setColor(color);
}

QColor HuePanel::color() const { return m_color; }

QPoint HuePanel::pickerPosition() const { return m_pickerPosition; }

int HuePanel::hue() const { return normalizedHue(m_color); }

int HuePanel::saturation() const { return qBound(0, m_color.saturation(), 255); }

void HuePanel::setColor(const QColor &color)
{
    const QColor normalized = normalizedDialogColor(color, true);
    m_color = hsvColor(normalizedHue(normalized), normalized.saturation(), 255, normalized.alpha());
    m_pickerPosition = QPoint(qRound(hue() / 359.0 * width()), qRound((255 - saturation()) / 255.0 * height()));
    update();
}

void HuePanel::mousePressEvent(QMouseEvent *event)
{
    setPickerPosition(event->pos());
    QWidget::mousePressEvent(event);
}

void HuePanel::mouseMoveEvent(QMouseEvent *event)
{
    setPickerPosition(event->pos());
    QWidget::mouseMoveEvent(event);
}

void HuePanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), 5.6, 5.6);
    painter.save();
    painter.setClipPath(clipPath);
    if (!m_huePixmap.isNull()) {
        painter.drawPixmap(rect(), m_huePixmap);
    } else {
        for (int x = 0; x < width(); ++x) {
            QLinearGradient columnGradient(0, 0, 0, height());
            const int hueValue = qRound(x / qMax(1.0, width() - 1.0) * 359);
            columnGradient.setColorAt(0, hsvColor(hueValue, 255, 255, 255));
            columnGradient.setColorAt(1, hsvColor(hueValue, 0, 255, 255));
            painter.fillRect(QRect(x, 0, 1, height()), columnGradient);
        }
    }
    painter.restore();

    painter.setPen(QPen(QColor(0, 0, 0, 15), 2.4));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5.6, 5.6);

    const QColor pickerColor = (saturation() > 153 || (hue() > 40 && hue() < 180)) ? Qt::black : QColor(255, 253, 254);
    painter.setPen(QPen(pickerColor, 3));
    painter.drawEllipse(m_pickerPosition, 8, 8);
}

void HuePanel::setPickerPosition(const QPoint &position)
{
    const int x = qBound(0, position.x(), width());
    const int y = qBound(0, position.y(), height());
    m_pickerPosition = QPoint(x, y);
    m_color = hsvColor(qRound(qBound(0.0, x / static_cast<double>(width()), 1.0) * 359),
                       qRound(qBound(0.0, (height() - y) / static_cast<double>(height()), 1.0) * 255), 255,
                       m_color.alpha());
    update();
    emit colorChanged(m_color);
}

BrightnessSlider::BrightnessSlider(const QColor &color, QWidget *parent) : ClickableSlider(Qt::Horizontal, parent)
{
    setRange(0, 255);
    setSingleStep(1);
    setColor(color);
    connect(this, &QSlider::valueChanged, this, [this](int value) {
        m_color = hsvColor(normalizedHue(m_color), m_color.saturation(), value, m_color.alpha());
        updateSliderStyle();
        emit colorChanged(m_color);
    });
}

QColor BrightnessSlider::color() const { return m_color; }

void BrightnessSlider::setColor(const QColor &color)
{
    const QSignalBlocker blocker(this);
    const QColor normalized = normalizedDialogColor(color, true);
    m_color = hsvColor(normalizedHue(normalized), normalized.saturation(), normalized.value(), normalized.alpha());
    setValue(m_color.value());
    updateSliderStyle();
}

void BrightnessSlider::updateSliderStyle() { setStyleSheet(brightnessSliderStyleSheet(m_color)); }

ColorCard::ColorCard(const QColor &color, QWidget *parent, bool enableAlpha) : QWidget(parent), m_alphaEnabled(enableAlpha)
{
    setFixedSize(44, 128);
    setColor(color);
}

QColor ColorCard::color() const { return m_color; }

bool ColorCard::isAlphaEnabled() const { return m_alphaEnabled; }

void ColorCard::setColor(const QColor &color)
{
    m_color = normalizedDialogColor(color, true);
    update();
}

void ColorCard::setAlphaEnabled(bool enabled)
{
    if (m_alphaEnabled == enabled) {
        return;
    }
    m_alphaEnabled = enabled;
    update();
}

void ColorCard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF frame = rect();
    if (m_alphaEnabled) {
        QPixmap tiledPixmap(8, 8);
        tiledPixmap.fill(Qt::transparent);
        QPainter tilePainter(&tiledPixmap);
        const int channel = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 255 : 0;
        const QColor tileColor(channel, channel, channel, 26);
        tilePainter.fillRect(4, 0, 4, 4, tileColor);
        tilePainter.fillRect(0, 4, 4, 4, tileColor);
        tilePainter.end();

        painter.setBrush(QBrush(tiledPixmap));
        painter.setPen(QColor(0, 0, 0, 13));
        painter.drawRoundedRect(frame, 4, 4);
    }

    painter.setBrush(m_color);
    painter.setPen(QColor(0, 0, 0, 13));
    painter.drawRoundedRect(frame, 4, 4);
}

ColorLineEdit::ColorLineEdit(int value, QWidget *parent) : LineEdit(QString::number(value), parent)
{
    setFixedSize(136, 33);
    setClearButtonEnabled(true);
    setValidator(new QIntValidator(0, 255, this));
    connect(this, &QLineEdit::textEdited, this, &ColorLineEdit::onTextEdited);
}

void ColorLineEdit::onTextEdited(const QString &text)
{
    if (!validator()) {
        emit valueChanged(text);
        return;
    }

    QString candidate = text;
    int position = 0;
    if (validator()->validate(candidate, position) == QValidator::Acceptable) {
        emit valueChanged(text);
    }
}

HexColorLineEdit::HexColorLineEdit(const QColor &color, QWidget *parent, bool enableAlpha)
    : ColorLineEdit(0, parent), m_colorFormat(enableAlpha ? QColor::HexArgb : QColor::HexRgb)
{
    const QString pattern = enableAlpha ? QStringLiteral("[A-Fa-f0-9]{8}") : QStringLiteral("[A-Fa-f0-9]{6}");
    setValidator(new QRegularExpressionValidator(QRegularExpression(pattern), this));
    setTextMargins(4, 0, 33, 0);
    m_prefixLabel = new QLabel(QStringLiteral("#"), this);
    m_prefixLabel->move(7, 2);
    m_prefixLabel->setObjectName(QStringLiteral("prefixLabel"));
    setColor(color);
}

QColor::NameFormat HexColorLineEdit::colorFormat() const { return m_colorFormat; }

void HexColorLineEdit::setColor(const QColor &color)
{
    const QSignalBlocker blocker(this);
    setText(normalizedDialogColor(color, true).name(m_colorFormat).mid(1));
}

OpacityLineEdit::OpacityLineEdit(int alpha, QWidget *parent)
    : ColorLineEdit(qBound(0, qRound(alpha / 255.0 * 100), 100), parent)
{
    setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9][0-9]{0,1}|100")), this));
    setTextMargins(4, 0, 33, 0);
    m_suffixLabel = new QLabel(QStringLiteral("%"), this);
    m_suffixLabel->setObjectName(QStringLiteral("suffixLabel"));
    connect(this, &QLineEdit::textChanged, this, &OpacityLineEdit::adjustSuffixPosition);
}

void OpacityLineEdit::showEvent(QShowEvent *event)
{
    ColorLineEdit::showEvent(event);
    adjustSuffixPosition();
}

void OpacityLineEdit::adjustSuffixPosition()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    const int textWidth = fontMetrics().horizontalAdvance(text());
#else
    const int textWidth = fontMetrics().width(text());
#endif
    if (m_suffixLabel) {
        m_suffixLabel->move(textWidth + 18, 2);
    }
}

ColorDialog::ColorDialog(const QColor &color, const QString &title, QWidget *parent, bool enableAlpha)
    : MaskDialogBase(parent), m_alphaEnabled(enableAlpha), m_oldColor(normalizedDialogColor(color, enableAlpha)),
      m_color(m_oldColor)
{
    setWindowTitle(title);
    FluentStyleSheet::setRole(this, QStringLiteral("ColorDialogPopup"));
    setCenterWidgetRole(QStringLiteral("ColorDialog"));
    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 80));
    setMaskColor(QColor(0, 0, 0, 76));

    m_widget->setFixedSize(488, 696 + (m_alphaEnabled ? 40 : 0));

    auto *scrollArea = new ColorDialogScrollArea(m_widget);
    m_scrollArea = scrollArea;
    FluentStyleSheet::setRole(m_scrollArea, QStringLiteral("ColorDialogScrollArea"));
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setViewportInset(48, 24, 0, 24);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->enableTransparentBackground();

    m_scrollWidget = new QWidget(m_scrollArea);
    m_scrollWidget->setObjectName(QStringLiteral("ColorDialogScrollWidget"));
    m_scrollWidget->resize(440, 560 + (m_alphaEnabled ? 40 : 0));
    m_scrollArea->setWidget(m_scrollWidget);

    m_buttonGroup = new QFrame(m_widget);
    m_buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
    m_buttonGroup->setFixedSize(486, 81);
    m_yesButton = new PrimaryPushButton(tr("OK"), m_buttonGroup);
    m_cancelButton = new PushButton(tr("Cancel"), m_buttonGroup);
    m_yesButton->setObjectName(QStringLiteral("yesButton"));
    m_cancelButton->setObjectName(QStringLiteral("cancelButton"));
    m_yesButton->setFixedWidth(216);
    m_cancelButton->setFixedWidth(216);

    m_titleLabel = new QLabel(title, m_scrollWidget);
    m_huePanel = new HuePanel(m_color, m_scrollWidget);
    m_newColorCard = new ColorCard(m_color, m_scrollWidget, m_alphaEnabled);
    m_oldColorCard = new ColorCard(m_oldColor, m_scrollWidget, m_alphaEnabled);
    m_brightnessSlider = new BrightnessSlider(m_color, m_scrollWidget);

    m_editLabel = new QLabel(tr("Edit Color"), m_scrollWidget);
    m_redLabel = new QLabel(tr("Red"), m_scrollWidget);
    m_greenLabel = new QLabel(tr("Green"), m_scrollWidget);
    m_blueLabel = new QLabel(tr("Blue"), m_scrollWidget);
    m_opacityLabel = new QLabel(tr("Opacity"), m_scrollWidget);
    m_hexLineEdit = new HexColorLineEdit(m_color, m_scrollWidget, m_alphaEnabled);
    m_redLineEdit = new ColorLineEdit(m_color.red(), m_scrollWidget);
    m_greenLineEdit = new ColorLineEdit(m_color.green(), m_scrollWidget);
    m_blueLineEdit = new ColorLineEdit(m_color.blue(), m_scrollWidget);
    m_opacityLineEdit = new OpacityLineEdit(m_color.alpha(), m_scrollWidget);

    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_editLabel->setObjectName(QStringLiteral("editLabel"));

    initLayout();
    connectSignals();
}

QColor ColorDialog::color() const { return m_color; }

QColor ColorDialog::oldColor() const { return m_oldColor; }

bool ColorDialog::isAlphaEnabled() const { return m_alphaEnabled; }

SingleDirectionScrollArea *ColorDialog::scrollArea() const { return m_scrollArea; }

QWidget *ColorDialog::scrollWidget() const { return m_scrollWidget; }

QFrame *ColorDialog::buttonGroup() const { return m_buttonGroup; }

QPushButton *ColorDialog::yesButton() const { return m_yesButton; }

QPushButton *ColorDialog::cancelButton() const { return m_cancelButton; }

QLabel *ColorDialog::titleLabel() const { return m_titleLabel; }

QLabel *ColorDialog::editLabel() const { return m_editLabel; }

QLabel *ColorDialog::redLabel() const { return m_redLabel; }

QLabel *ColorDialog::greenLabel() const { return m_greenLabel; }

QLabel *ColorDialog::blueLabel() const { return m_blueLabel; }

QLabel *ColorDialog::opacityLabel() const { return m_opacityLabel; }

HuePanel *ColorDialog::huePanel() const { return m_huePanel; }

ColorCard *ColorDialog::newColorCard() const { return m_newColorCard; }

ColorCard *ColorDialog::oldColorCard() const { return m_oldColorCard; }

BrightnessSlider *ColorDialog::brightnessSlider() const { return m_brightnessSlider; }

HexColorLineEdit *ColorDialog::hexLineEdit() const { return m_hexLineEdit; }

ColorLineEdit *ColorDialog::redLineEdit() const { return m_redLineEdit; }

ColorLineEdit *ColorDialog::greenLineEdit() const { return m_greenLineEdit; }

ColorLineEdit *ColorDialog::blueLineEdit() const { return m_blueLineEdit; }

OpacityLineEdit *ColorDialog::opacityLineEdit() const { return m_opacityLineEdit; }

void ColorDialog::setColor(const QColor &color) { setColor(color, true); }

void ColorDialog::setColor(const QColor &color, bool movePicker)
{
    m_color = normalizedDialogColor(color, m_alphaEnabled);
    if (m_brightnessSlider) {
        m_brightnessSlider->setColor(m_color);
    }
    if (m_newColorCard) {
        m_newColorCard->setColor(m_color);
    }
    if (m_hexLineEdit) {
        m_hexLineEdit->setColor(m_color);
    }
    if (m_redLineEdit) {
        const QSignalBlocker blocker(m_redLineEdit);
        m_redLineEdit->setText(QString::number(m_color.red()));
    }
    if (m_greenLineEdit) {
        const QSignalBlocker blocker(m_greenLineEdit);
        m_greenLineEdit->setText(QString::number(m_color.green()));
    }
    if (m_blueLineEdit) {
        const QSignalBlocker blocker(m_blueLineEdit);
        m_blueLineEdit->setText(QString::number(m_color.blue()));
    }
    if (m_opacityLineEdit) {
        const QSignalBlocker blocker(m_opacityLineEdit);
        m_opacityLineEdit->setText(QString::number(qBound(0, qRound(m_color.alpha() / 255.0 * 100), 100)));
    }
    if (movePicker && m_huePanel) {
        m_huePanel->setColor(m_color);
    }
}

void ColorDialog::updateStyle()
{
    FluentStyleSheet::polish(this);
    FluentStyleSheet::polish(m_widget);
    if (m_titleLabel) {
        m_titleLabel->adjustSize();
    }
    if (m_editLabel) {
        m_editLabel->adjustSize();
    }
    if (m_redLabel) {
        m_redLabel->adjustSize();
    }
    if (m_greenLabel) {
        m_greenLabel->adjustSize();
    }
    if (m_blueLabel) {
        m_blueLabel->adjustSize();
    }
    if (m_opacityLabel) {
        m_opacityLabel->adjustSize();
    }
}

void ColorDialog::showEvent(QShowEvent *event)
{
    updateStyle();
    MaskDialogBase::showEvent(event);
}

void ColorDialog::initLayout()
{
    m_huePanel->move(0, 46);
    m_newColorCard->move(288, 46);
    m_oldColorCard->move(288, m_newColorCard->geometry().bottom() + 1);
    m_brightnessSlider->move(0, 324);

    m_editLabel->move(0, 385);
    m_redLineEdit->move(0, 426);
    m_greenLineEdit->move(0, 470);
    m_blueLineEdit->move(0, 515);
    m_redLabel->move(144, 434);
    m_greenLabel->move(144, 478);
    m_blueLabel->move(144, 524);
    m_hexLineEdit->move(196, 381);

    if (m_alphaEnabled) {
        m_opacityLineEdit->move(0, 560);
        m_opacityLabel->move(144, 567);
    } else {
        m_opacityLineEdit->hide();
        m_opacityLabel->hide();
    }

    auto *layout = new QVBoxLayout(m_widget);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_scrollArea, 1);
    layout->addWidget(m_buttonGroup, 0, Qt::AlignBottom);

    m_yesButton->move(24, 25);
    m_cancelButton->move(250, 25);
}

void ColorDialog::connectSignals()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_yesButton, &QPushButton::clicked, this, &ColorDialog::onYesButtonClicked);
    connect(m_huePanel, &HuePanel::colorChanged, this, &ColorDialog::onHueChanged);
    connect(m_brightnessSlider, &BrightnessSlider::colorChanged, this, &ColorDialog::onBrightnessChanged);
    connect(m_redLineEdit, &ColorLineEdit::valueChanged, this, &ColorDialog::onRedChanged);
    connect(m_greenLineEdit, &ColorLineEdit::valueChanged, this, &ColorDialog::onGreenChanged);
    connect(m_blueLineEdit, &ColorLineEdit::valueChanged, this, &ColorDialog::onBlueChanged);
    connect(m_opacityLineEdit, &ColorLineEdit::valueChanged, this, &ColorDialog::onOpacityChanged);
    connect(m_hexLineEdit, &ColorLineEdit::valueChanged, this, &ColorDialog::onHexColorChanged);
}

void ColorDialog::onHueChanged(const QColor &color)
{
    setColor(hsvColor(normalizedHue(color), color.saturation(), m_color.value(), m_color.alpha()));
}

void ColorDialog::onBrightnessChanged(const QColor &color)
{
    setColor(hsvColor(normalizedHue(m_color), m_color.saturation(), color.value(), m_color.alpha()), false);
}

void ColorDialog::onRedChanged(const QString &red)
{
    QColor next = m_color;
    next.setRed(red.toInt());
    setColor(next);
}

void ColorDialog::onGreenChanged(const QString &green)
{
    QColor next = m_color;
    next.setGreen(green.toInt());
    setColor(next);
}

void ColorDialog::onBlueChanged(const QString &blue)
{
    QColor next = m_color;
    next.setBlue(blue.toInt());
    setColor(next);
}

void ColorDialog::onOpacityChanged(const QString &opacity)
{
    QColor next = m_color;
    next.setAlpha(qBound(0, qRound(opacity.toInt() / 100.0 * 255), 255));
    setColor(next);
}

void ColorDialog::onHexColorChanged(const QString &color)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    const QColor next = QColor::fromString(QStringLiteral("#") + color);
#else
    QColor next;
    next.setNamedColor(QStringLiteral("#") + color);
#endif
    if (next.isValid()) {
        setColor(next);
    }
}

void ColorDialog::onYesButtonClicked()
{
    const QColor acceptedColor = m_color;
    accept();
    if (acceptedColor != m_oldColor) {
        emit colorChanged(acceptedColor);
    }
}

} // namespace FluentQt
