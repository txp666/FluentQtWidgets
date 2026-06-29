#include <FluentQtWidgets/Settings/SettingCard.h>

#include <FluentQtWidgets/Dialogs/ColorDialog.h>
#include <FluentQtWidgets/Dialogs/FolderListDialog.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/ComboBox.h>
#include <FluentQtWidgets/Widgets/IconWidget.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Selection.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/SwitchButton.h>

#include <QtCore/QAbstractAnimation>
#include <QtCore/QDir>
#include <QtCore/QEasingCurve>
#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSignalBlocker>
#include <QtGui/QEnterEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include <utility>

namespace FluentQt {

namespace {

constexpr const char *kOptionValueProperty = "fqw-option-value";

QColor normalizedColor(const QColor &color)
{
    return color.isValid() ? color : ThemeManager::instance()->accentColor();
}

class SpaceWidget : public QWidget
{
  public:
    explicit SpaceWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedHeight(1);
    }
};

class ExpandBorderWidget : public QWidget
{
  public:
    explicit ExpandBorderWidget(ExpandSettingCard *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        if (parent) {
            parent->installEventFilter(this);
            resize(parent->size());
        }
    }

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == parentWidget() && event->type() == QEvent::Resize) {
            auto *resizeEvent = static_cast<QResizeEvent *>(event);
            resize(resizeEvent->size());
        }
        return QWidget::eventFilter(watched, event);
    }

    void paintEvent(QPaintEvent *) override
    {
        auto *expandCard = qobject_cast<ExpandSettingCard *>(parentWidget());
        const int cardHeight = expandCard && expandCard->card() ? expandCard->card()->height() : 0;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(0, 0, 0, 50)
                                                                                 : QColor(0, 0, 0, 19));
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);

        if (cardHeight > 0 && cardHeight < height()) {
            painter.drawLine(1, cardHeight, width() - 1, cardHeight);
        }
    }
};

} // namespace

SettingCard::SettingCard(QWidget *parent) : SettingCard(QIcon(), QString(), QString(), parent) {}

SettingCard::SettingCard(const QString &title, const QString &content, QWidget *parent)
    : SettingCard(QIcon(), title, content, parent)
{
}

SettingCard::SettingCard(FluentIcon fluentIcon, const QString &title, const QString &content, QWidget *parent)
    : SettingCard(FluentQt::icon(fluentIcon), title, content, parent)
{
}

SettingCard::SettingCard(const QIcon &icon, const QString &title, const QString &content, QWidget *parent)
    : QFrame(parent)
{
    init();
    setIcon(icon);
    setTitle(title);
    setContent(content);
}

QString SettingCard::title() const { return m_titleLabel ? m_titleLabel->text() : QString(); }

QString SettingCard::content() const { return m_contentLabel ? m_contentLabel->text() : QString(); }

QIcon SettingCard::icon() const { return m_icon; }

QSize SettingCard::iconSize() const { return m_iconSize; }

QLabel *SettingCard::iconLabel() const { return m_iconLabel; }

QLabel *SettingCard::titleLabel() const { return m_titleLabel; }

QLabel *SettingCard::contentLabel() const { return m_contentLabel; }

QHBoxLayout *SettingCard::mainLayout() const { return m_mainLayout; }

QVBoxLayout *SettingCard::textLayout() const { return m_textLayout; }

QHBoxLayout *SettingCard::actionLayout() const { return m_actionLayout; }

void SettingCard::setTitle(const QString &title)
{
    if (!m_titleLabel || m_titleLabel->text() == title) {
        return;
    }

    m_titleLabel->setText(title);
    emit titleChanged(title);
}

void SettingCard::setContent(const QString &content)
{
    if (!m_contentLabel || m_contentLabel->text() == content) {
        return;
    }

    m_contentLabel->setText(content);
    updateContentVisibility();
    emit contentChanged(content);
}

void SettingCard::setIcon(const QIcon &icon)
{
    if (m_icon.cacheKey() == icon.cacheKey()) {
        return;
    }

    m_icon = icon;
    refreshIcon();
}

void SettingCard::setIcon(FluentIcon icon) { setIcon(FluentQt::icon(icon)); }

void SettingCard::setIconSize(const QSize &size)
{
    if (!size.isValid() || m_iconSize == size) {
        return;
    }

    m_iconSize = size;
    if (m_iconLabel) {
        m_iconLabel->setFixedSize(m_iconSize);
    }
    refreshIcon();
}

void SettingCard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::PaletteChange) {
        refreshIcon();
    }

    QFrame::changeEvent(event);
}

QVBoxLayout *SettingCard::outerLayout() const { return m_outerLayout; }

void SettingCard::init()
{
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(m_iconSize);
    m_iconLabel->setScaledContents(false);
    m_iconLabel->setVisible(false);

    m_titleLabel = new BodyLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("SettingCardTitle"));

    m_contentLabel = new CaptionLabel(this);
    m_contentLabel->setObjectName(QStringLiteral("SettingCardContent"));
    m_contentLabel->setWordWrap(false);
    m_contentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_outerLayout = new QVBoxLayout(this);
    m_outerLayout->setContentsMargins(0, 0, 0, 0);
    m_outerLayout->setSpacing(0);

    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setContentsMargins(16, 0, 16, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setAlignment(Qt::AlignVCenter);

    m_textLayout = new QVBoxLayout;
    m_textLayout->setContentsMargins(0, 0, 0, 0);
    m_textLayout->setSpacing(0);
    m_textLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_actionLayout = new QHBoxLayout;
    m_actionLayout->setContentsMargins(0, 0, 0, 0);
    m_actionLayout->setSpacing(8);
    m_actionLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);
    m_iconSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_mainLayout->addItem(m_iconSpacer);
    m_mainLayout->addLayout(m_textLayout, 1);
    m_textLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    m_textLayout->addWidget(m_contentLabel, 0, Qt::AlignLeft);
    m_mainLayout->addSpacing(16);
    m_mainLayout->addLayout(m_actionLayout, 0);

    m_outerLayout->addLayout(m_mainLayout);

    updateContentVisibility();
    FluentStyleSheet::setRole(this, QStringLiteral("SettingCard"));

    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) { refreshIcon(); });
}

void SettingCard::updateContentVisibility()
{
    const bool hasContent = m_contentLabel && !m_contentLabel->text().isEmpty();
    if (m_contentLabel) {
        m_contentLabel->setVisible(hasContent);
    }

    setFixedHeight(hasContent ? 70 : 50);
}

void SettingCard::refreshIcon()
{
    if (!m_iconLabel) {
        return;
    }

    if (m_icon.isNull()) {
        m_iconLabel->clear();
        m_iconLabel->setVisible(false);
        if (m_iconSpacer) {
            m_iconSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        }
        if (m_mainLayout) {
            m_mainLayout->invalidate();
        }
        return;
    }

    m_iconLabel->setVisible(true);
    m_iconLabel->setPixmap(m_icon.pixmap(m_iconSize, devicePixelRatioF(),
                                         isEnabled() ? QIcon::Normal : QIcon::Disabled));
    if (m_iconSpacer) {
        m_iconSpacer->changeSize(16, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    }
    if (m_mainLayout) {
        m_mainLayout->invalidate();
    }
}

SettingCardGroup::SettingCardGroup(const QString &title, QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    m_titleLabel = new SubtitleLabel(title, this);
    layout->addWidget(m_titleLabel);

    m_cardLayout = new QVBoxLayout;
    m_cardLayout->setContentsMargins(0, 0, 0, 0);
    m_cardLayout->setSpacing(2);
    layout->addLayout(m_cardLayout);

    FluentStyleSheet::setRole(this, QStringLiteral("SettingCardGroup"));
}

QString SettingCardGroup::title() const { return m_titleLabel ? m_titleLabel->text() : QString(); }

QLabel *SettingCardGroup::titleLabel() const { return m_titleLabel; }

QVBoxLayout *SettingCardGroup::cardLayout() const { return m_cardLayout; }

QList<QWidget *> SettingCardGroup::cards() const
{
    QList<QWidget *> result;
    if (!m_cardLayout) {
        return result;
    }

    for (int i = 0; i < m_cardLayout->count(); ++i) {
        if (QWidget *widget = m_cardLayout->itemAt(i)->widget()) {
            result.append(widget);
        }
    }
    return result;
}

void SettingCardGroup::setTitle(const QString &title)
{
    if (!m_titleLabel || m_titleLabel->text() == title) {
        return;
    }

    m_titleLabel->setText(title);
    emit titleChanged(title);
}

void SettingCardGroup::addSettingCard(QWidget *card)
{
    if (!card || !m_cardLayout) {
        return;
    }

    card->setParent(this);
    m_cardLayout->addWidget(card);
}

void SettingCardGroup::addSettingCards(const QList<QWidget *> &cards)
{
    for (QWidget *card : cards) {
        addSettingCard(card);
    }
}

PushSettingCard::PushSettingCard(const QString &buttonText, const QIcon &icon, const QString &title,
                                 const QString &content, QWidget *parent)
    : PushSettingCard(ButtonStyle::Default, buttonText, icon, title, content, parent)
{
}

PushSettingCard::PushSettingCard(const QString &buttonText, FluentIcon icon, const QString &title,
                                 const QString &content, QWidget *parent)
    : PushSettingCard(buttonText, FluentQt::icon(icon), title, content, parent)
{
}

PushSettingCard::PushSettingCard(ButtonStyle style, const QString &buttonText, const QIcon &icon, const QString &title,
                                 const QString &content, QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_button = style == ButtonStyle::Primary ? static_cast<QPushButton *>(new PrimaryPushButton(buttonText, this))
                                             : static_cast<QPushButton *>(new PushButton(buttonText, this));
    actionLayout()->addWidget(m_button, 0, Qt::AlignRight);
    connect(m_button, &QPushButton::clicked, this, &PushSettingCard::clicked);

    FluentStyleSheet::setRole(this, style == ButtonStyle::Primary ? QStringLiteral("PrimaryPushSettingCard")
                                                                  : QStringLiteral("PushSettingCard"));
}

QPushButton *PushSettingCard::button() const { return m_button; }

PrimaryPushSettingCard::PrimaryPushSettingCard(const QString &buttonText, const QIcon &icon, const QString &title,
                                               const QString &content, QWidget *parent)
    : PushSettingCard(ButtonStyle::Primary, buttonText, icon, title, content, parent)
{
}

PrimaryPushSettingCard::PrimaryPushSettingCard(const QString &buttonText, FluentIcon icon, const QString &title,
                                               const QString &content, QWidget *parent)
    : PrimaryPushSettingCard(buttonText, FluentQt::icon(icon), title, content, parent)
{
}

HyperlinkCard::HyperlinkCard(const QUrl &url, const QString &buttonText, const QIcon &icon, const QString &title,
                             const QString &content, QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_linkButton = new HyperlinkButton(url.toString(), buttonText, this);
    actionLayout()->addWidget(m_linkButton, 0, Qt::AlignRight);

    FluentStyleSheet::setRole(this, QStringLiteral("HyperlinkCard"));
}

HyperlinkCard::HyperlinkCard(const QString &url, const QString &buttonText, const QIcon &icon, const QString &title,
                             const QString &content, QWidget *parent)
    : HyperlinkCard(QUrl(url), buttonText, icon, title, content, parent)
{
}

HyperlinkCard::HyperlinkCard(const QUrl &url, const QString &buttonText, FluentIcon icon, const QString &title,
                             const QString &content, QWidget *parent)
    : HyperlinkCard(url, buttonText, FluentQt::icon(icon), title, content, parent)
{
}

HyperlinkCard::HyperlinkCard(const QString &url, const QString &buttonText, FluentIcon icon, const QString &title,
                             const QString &content, QWidget *parent)
    : HyperlinkCard(QUrl(url), buttonText, icon, title, content, parent)
{
}

HyperlinkButton *HyperlinkCard::linkButton() const { return m_linkButton; }

QUrl HyperlinkCard::url() const { return m_linkButton ? m_linkButton->url() : QUrl(); }

void HyperlinkCard::setUrl(const QUrl &url)
{
    if (m_linkButton) {
        m_linkButton->setUrl(url);
    }
}

void HyperlinkCard::setUrl(const QString &url)
{
    setUrl(QUrl(url));
}

SwitchSettingCard::SwitchSettingCard(const QIcon &icon, const QString &title, const QString &content, QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_switchButton = new SwitchButton(this);
    actionLayout()->addWidget(m_switchButton, 0, Qt::AlignRight);
    connect(m_switchButton, &SwitchButton::checkedChanged, this, &SwitchSettingCard::checkedChanged);

    FluentStyleSheet::setRole(this, QStringLiteral("SwitchSettingCard"));
}

SwitchSettingCard::SwitchSettingCard(FluentIcon icon, const QString &title, const QString &content, QWidget *parent)
    : SwitchSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

SwitchButton *SwitchSettingCard::switchButton() const { return m_switchButton; }

bool SwitchSettingCard::isChecked() const { return m_switchButton && m_switchButton->isChecked(); }

void SwitchSettingCard::setChecked(bool checked)
{
    if (m_switchButton) {
        m_switchButton->setChecked(checked);
    }
}

RangeSettingCard::RangeSettingCard(const QIcon &icon, const QString &title, const QString &content, QWidget *parent)
    : RangeSettingCard(0, 100, 0, icon, title, content, parent)
{
}

RangeSettingCard::RangeSettingCard(FluentIcon icon, const QString &title, const QString &content, QWidget *parent)
    : RangeSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

RangeSettingCard::RangeSettingCard(int minimum, int maximum, int value, const QIcon &icon, const QString &title,
                                   const QString &content, QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_valueLabel = new CaptionLabel(this);
    m_valueLabel->setObjectName(QStringLiteral("SettingCardValue"));
    m_valueLabel->setMinimumWidth(32);
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_slider = new ClickableSlider(Qt::Horizontal, this);
    m_slider->setMinimumWidth(220);
    m_slider->setRange(minimum, maximum);
    m_slider->setValue(value);
    updateValueLabel(m_slider->value());

    actionLayout()->addWidget(m_valueLabel, 0, Qt::AlignRight);
    actionLayout()->addWidget(m_slider, 0, Qt::AlignRight);

    connect(m_slider, &ClickableSlider::valueChanged, this, [this](int sliderValue) {
        updateValueLabel(sliderValue);
        emit valueChanged(sliderValue);
    });

    FluentStyleSheet::setRole(this, QStringLiteral("RangeSettingCard"));
}

RangeSettingCard::RangeSettingCard(int minimum, int maximum, int value, FluentIcon icon, const QString &title,
                                   const QString &content, QWidget *parent)
    : RangeSettingCard(minimum, maximum, value, FluentQt::icon(icon), title, content, parent)
{
}

ClickableSlider *RangeSettingCard::slider() const { return m_slider; }

QLabel *RangeSettingCard::valueLabel() const { return m_valueLabel; }

int RangeSettingCard::value() const { return m_slider ? m_slider->value() : 0; }

int RangeSettingCard::minimum() const { return m_slider ? m_slider->minimum() : 0; }

int RangeSettingCard::maximum() const { return m_slider ? m_slider->maximum() : 0; }

void RangeSettingCard::setValue(int value)
{
    if (!m_slider) {
        return;
    }

    if (m_slider->value() == value) {
        updateValueLabel(value);
        return;
    }

    m_slider->setValue(value);
}

void RangeSettingCard::setRange(int minimum, int maximum)
{
    if (m_slider) {
        m_slider->setRange(minimum, maximum);
        updateValueLabel(m_slider->value());
    }
}

void RangeSettingCard::updateValueLabel(int value)
{
    if (m_valueLabel) {
        m_valueLabel->setText(QString::number(value));
    }
}

ComboBoxSettingCard::ComboBoxSettingCard(const QIcon &icon, const QString &title, const QString &content,
                                         QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_comboBox = new ComboBox(this);
    m_comboBox->setMinimumWidth(160);
    actionLayout()->addWidget(m_comboBox, 0, Qt::AlignRight);

    connect(m_comboBox, &ComboBox::currentIndexChanged, this,
            &ComboBoxSettingCard::currentIndexChanged);
    connect(m_comboBox, &ComboBox::currentTextChanged, this, &ComboBoxSettingCard::currentTextChanged);

    FluentStyleSheet::setRole(this, QStringLiteral("ComboBoxSettingCard"));
}

ComboBoxSettingCard::ComboBoxSettingCard(FluentIcon icon, const QString &title, const QString &content, QWidget *parent)
    : ComboBoxSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

ComboBoxSettingCard::ComboBoxSettingCard(const QStringList &items, const QIcon &icon, const QString &title,
                                         const QString &content, QWidget *parent)
    : ComboBoxSettingCard(icon, title, content, parent)
{
    addItems(items);
}

ComboBoxSettingCard::ComboBoxSettingCard(const QStringList &items, FluentIcon icon, const QString &title,
                                         const QString &content, QWidget *parent)
    : ComboBoxSettingCard(items, FluentQt::icon(icon), title, content, parent)
{
}

ComboBox *ComboBoxSettingCard::comboBox() const { return m_comboBox; }

int ComboBoxSettingCard::currentIndex() const { return m_comboBox ? m_comboBox->currentIndex() : -1; }

QString ComboBoxSettingCard::currentText() const { return m_comboBox ? m_comboBox->currentText() : QString(); }

void ComboBoxSettingCard::addItem(const QString &text, const QVariant &userData)
{
    if (m_comboBox) {
        m_comboBox->addItem(text, QIcon(), userData);
    }
}

void ComboBoxSettingCard::addItems(const QStringList &texts)
{
    if (m_comboBox) {
        m_comboBox->addItems(texts);
    }
}

void ComboBoxSettingCard::setCurrentIndex(int index)
{
    if (m_comboBox) {
        m_comboBox->setCurrentIndex(index);
    }
}

ColorPickerButton::ColorPickerButton(const QColor &color, QWidget *parent) : QToolButton(parent)
{
    setFixedSize(96, 32);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    m_color = normalizedColor(color);

    FluentStyleSheet::setRole(this, QStringLiteral("ColorPickerButton"));
    connect(this, &QToolButton::clicked, this, &ColorPickerButton::showColorDialog);
}

ColorPickerButton::ColorPickerButton(const QColor &color, const QString &title, QWidget *parent, bool enableAlpha)
    : ColorPickerButton(color, parent)
{
    setTitle(title);
    setAlphaEnabled(enableAlpha);
}

QColor ColorPickerButton::color() const { return m_color; }

QString ColorPickerButton::title() const { return m_title; }

bool ColorPickerButton::isAlphaEnabled() const { return m_alphaEnabled; }

void ColorPickerButton::setColor(const QColor &color)
{
    const QColor nextColor = normalizedColor(color);
    if (m_color == nextColor) {
        return;
    }

    m_color = nextColor;
    update();
    emit colorChanged(m_color);
}

void ColorPickerButton::setTitle(const QString &title)
{
    m_title = title;
    setToolTip(title.isEmpty() ? QString() : tr("Choose %1").arg(title));
}

void ColorPickerButton::setAlphaEnabled(bool enabled) { m_alphaEnabled = enabled; }

void ColorPickerButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!isEnabled()) {
        painter.setOpacity(0.36);
    }

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QColor border = dark ? QColor(80, 80, 80) : QColor(214, 214, 214);
    QColor fill = m_color;
    if (!m_alphaEnabled) {
        fill.setAlpha(255);
    }

    painter.setPen(QPen(border, 1));
    painter.setBrush(fill);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5, 5);
}

void ColorPickerButton::showColorDialog()
{
    ColorDialog dialog(m_color, m_title.isEmpty() ? tr("Choose color") : m_title, window(), m_alphaEnabled);
    connect(&dialog, &ColorDialog::colorChanged, this, &ColorPickerButton::setColor);
    dialog.exec();
}

ColorSettingCard::ColorSettingCard(const QColor &color, const QIcon &icon, const QString &title, const QString &content,
                                   QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_colorPicker = new ColorPickerButton(color, this);
    actionLayout()->addWidget(m_colorPicker, 0, Qt::AlignRight);
    connect(m_colorPicker, &ColorPickerButton::colorChanged, this, &ColorSettingCard::colorChanged);

    FluentStyleSheet::setRole(this, QStringLiteral("ColorSettingCard"));
}

ColorSettingCard::ColorSettingCard(const QColor &color, FluentIcon icon, const QString &title, const QString &content,
                                   QWidget *parent)
    : ColorSettingCard(color, FluentQt::icon(icon), title, content, parent)
{
}

ColorPickerButton *ColorSettingCard::colorPicker() const { return m_colorPicker; }

QColor ColorSettingCard::color() const { return m_colorPicker ? m_colorPicker->color() : QColor(); }

void ColorSettingCard::setColor(const QColor &color)
{
    if (m_colorPicker) {
        m_colorPicker->setColor(color);
    }
}

ExpandButton::ExpandButton(QWidget *parent) : QToolButton(parent)
{
    setFixedSize(30, 30);
    setCursor(Qt::PointingHandCursor);
    setAutoRaise(true);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TranslucentBackground);

    m_rotateAnimation = new QPropertyAnimation(this, "angle", this);
    m_rotateAnimation->setDuration(200);
    connect(this, &QToolButton::clicked, this, [this]() { setExpand(m_angle < 180); });

    FluentStyleSheet::setRole(this, QStringLiteral("ExpandButton"));
}

qreal ExpandButton::angle() const { return m_angle; }

bool ExpandButton::isHover() const { return m_hover; }

bool ExpandButton::isPressed() const { return m_pressed; }

void ExpandButton::setAngle(qreal angle)
{
    if (qFuzzyCompare(m_angle, angle)) {
        return;
    }
    m_angle = angle;
    update();
}

void ExpandButton::setHover(bool hover)
{
    if (m_hover == hover) {
        return;
    }
    m_hover = hover;
    update();
}

void ExpandButton::setPressed(bool pressed)
{
    if (m_pressed == pressed) {
        return;
    }
    m_pressed = pressed;
    update();
}

void ExpandButton::setExpand(bool expanded)
{
    if (!m_rotateAnimation) {
        setAngle(expanded ? 180 : 0);
        return;
    }

    m_rotateAnimation->stop();
    m_rotateAnimation->setStartValue(m_angle);
    m_rotateAnimation->setEndValue(expanded ? 180 : 0);
    m_rotateAnimation->start();
}

void ExpandButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const int channel = dark ? 255 : 0;
    QColor background = Qt::transparent;
    if (isEnabled()) {
        if (m_pressed) {
            background = QColor(channel, channel, channel, 10);
        } else if (m_hover) {
            background = QColor(channel, channel, channel, 14);
        }
    } else {
        painter.setOpacity(0.36);
    }
    painter.setBrush(background);
    painter.drawRoundedRect(rect(), 4, 4);

    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(m_angle);
    FluentQt::icon(FluentIcon::ArrowDown).paint(&painter, QRect(-5, -5, 10, 10));
}

void ExpandButton::enterEvent(QEnterEvent *event)
{
    setHover(true);
    QToolButton::enterEvent(event);
}

void ExpandButton::leaveEvent(QEvent *event)
{
    setHover(false);
    QToolButton::leaveEvent(event);
}

void ExpandButton::mousePressEvent(QMouseEvent *event)
{
    setPressed(true);
    QToolButton::mousePressEvent(event);
}

void ExpandButton::mouseReleaseEvent(QMouseEvent *event)
{
    QToolButton::mouseReleaseEvent(event);
    setPressed(false);
}

HeaderSettingCard::HeaderSettingCard(const QIcon &icon, const QString &title, const QString &content,
                                     QWidget *parent)
    : SettingCard(icon, title, content, parent)
{
    m_expandButton = new ExpandButton(this);
    if (mainLayout()) {
        mainLayout()->setContentsMargins(16, 0, 0, 0);
    }
    if (actionLayout()) {
        actionLayout()->setSpacing(0);
        actionLayout()->addWidget(m_expandButton, 0, Qt::AlignRight);
        actionLayout()->addSpacing(8);
    }
    titleLabel()->setObjectName(QStringLiteral("titleLabel"));
    installEventFilter(this);

    FluentStyleSheet::setRole(this, QStringLiteral("HeaderSettingCard"));
}

HeaderSettingCard::HeaderSettingCard(FluentIcon icon, const QString &title, const QString &content, QWidget *parent)
    : HeaderSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

ExpandButton *HeaderSettingCard::expandButton() const { return m_expandButton; }

void HeaderSettingCard::addWidget(QWidget *widget)
{
    if (!widget || !actionLayout()) {
        return;
    }

    if (actionLayout()->count() > 0) {
        delete actionLayout()->takeAt(actionLayout()->count() - 1);
    }
    actionLayout()->removeWidget(m_expandButton);
    actionLayout()->addWidget(widget, 0, Qt::AlignRight | Qt::AlignVCenter);
    actionLayout()->addSpacing(19);
    actionLayout()->addWidget(m_expandButton, 0, Qt::AlignRight);
    actionLayout()->addSpacing(8);
}

bool HeaderSettingCard::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && m_expandButton) {
        switch (event->type()) {
        case QEvent::Enter:
            m_expandButton->setHover(true);
            break;
        case QEvent::Leave:
            m_expandButton->setHover(false);
            break;
        case QEvent::MouseButtonPress:
            if (auto *mouseEvent = static_cast<QMouseEvent *>(event); mouseEvent->button() == Qt::LeftButton) {
                m_expandButton->setPressed(true);
            }
            break;
        case QEvent::MouseButtonRelease:
            if (auto *mouseEvent = static_cast<QMouseEvent *>(event); mouseEvent->button() == Qt::LeftButton) {
                m_expandButton->setPressed(false);
                m_expandButton->click();
            }
            break;
        default:
            break;
        }
    }

    return SettingCard::eventFilter(watched, event);
}

void HeaderSettingCard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255, 13)
                                                                               : QColor(255, 255, 255, 170));

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(rect().adjusted(1, 1, -1, -1)), 6, 6);

    auto *expandCard = qobject_cast<ExpandSettingCard *>(parentWidget());
    if (expandCard && expandCard->isExpanded()) {
        path.addRect(1, height() - 8, width() - 2, 8);
    }

    painter.drawPath(path.simplified());
}

ExpandSettingCard::ExpandSettingCard(const QIcon &icon, const QString &title, const QString &content, QWidget *parent)
    : QScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);
    setFixedHeight(content.isEmpty() ? 50 : 70);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_scrollWidget = new QFrame(this);
    m_scrollWidget->setObjectName(QStringLiteral("scrollWidget"));
    m_view = new QFrame(m_scrollWidget);
    m_view->setObjectName(QStringLiteral("view"));
    m_spaceWidget = new SpaceWidget(m_scrollWidget);
    m_card = new HeaderSettingCard(icon, title, content, this);
    m_expandButton = m_card->expandButton();
    m_expandButton->setCheckable(true);
    m_expandButton->setToolTip(tr("Expand"));

    m_scrollLayout = new QVBoxLayout(m_scrollWidget);
    m_scrollLayout->setContentsMargins(0, 0, 0, 0);
    m_scrollLayout->setSpacing(0);
    m_scrollLayout->addWidget(m_view);
    m_scrollLayout->addWidget(m_spaceWidget);

    m_viewLayout = new QVBoxLayout(m_view);
    m_viewLayout->setContentsMargins(9, 9, 9, 9);

    setWidget(m_scrollWidget);
    setViewportMargins(0, m_card->height(), 0, 0);

    m_borderWidget = new ExpandBorderWidget(this);
    m_borderWidget->raise();

    m_expandAnimation = new QPropertyAnimation(verticalScrollBar(), "value", this);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_expandAnimation->setDuration(200);

    setProperty("isExpand", false);
    FluentStyleSheet::setRole(this, QStringLiteral("ExpandSettingCard"));
    FluentStyleSheet::apply(m_card, FluentStyleSheetSource::ExpandSettingCard);

    connect(m_expandAnimation, &QVariantAnimation::valueChanged, this, [this]() { onExpandValueChanged(); });
    connect(m_expandButton, &QToolButton::clicked, this, [this]() { toggleExpanded(); });

    refreshExpandedGeometry();
}

ExpandSettingCard::ExpandSettingCard(FluentIcon icon, const QString &title, const QString &content, QWidget *parent)
    : ExpandSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

HeaderSettingCard *ExpandSettingCard::card() const { return m_card; }

QWidget *ExpandSettingCard::view() const { return m_view; }

QVBoxLayout *ExpandSettingCard::viewLayout() const { return m_viewLayout; }

ExpandButton *ExpandSettingCard::expandButton() const { return m_expandButton; }

bool ExpandSettingCard::isExpanded() const { return m_expanded; }

QLabel *ExpandSettingCard::iconLabel() const { return m_card ? m_card->iconLabel() : nullptr; }

QLabel *ExpandSettingCard::titleLabel() const { return m_card ? m_card->titleLabel() : nullptr; }

QLabel *ExpandSettingCard::contentLabel() const { return m_card ? m_card->contentLabel() : nullptr; }

QHBoxLayout *ExpandSettingCard::mainLayout() const { return m_card ? m_card->mainLayout() : nullptr; }

QVBoxLayout *ExpandSettingCard::textLayout() const { return m_card ? m_card->textLayout() : nullptr; }

QHBoxLayout *ExpandSettingCard::actionLayout() const { return m_card ? m_card->actionLayout() : nullptr; }

void ExpandSettingCard::addWidget(QWidget *widget)
{
    if (!widget || !m_card) {
        return;
    }

    m_card->addWidget(widget);
    adjustViewSize();
}

void ExpandSettingCard::setExpanded(bool expanded)
{
    if (m_expanded == expanded) {
        return;
    }

    adjustViewSize();
    m_expanded = expanded;
    setProperty("isExpand", m_expanded);
    FluentStyleSheet::polish(this);
    if (m_card) {
        m_card->setProperty("isExpand", m_expanded);
        FluentStyleSheet::polish(m_card);
    }
    if (m_expandButton) {
        const QSignalBlocker blocker(m_expandButton);
        m_expandButton->setChecked(m_expanded);
        m_expandButton->setToolTip(m_expanded ? tr("Collapse") : tr("Expand"));
        m_expandButton->setExpand(m_expanded);
    }

    if (m_expandAnimation) {
        m_expandAnimation->stop();
        const int height = expandedContentHeight();
        if (m_expanded) {
            verticalScrollBar()->setValue(height);
            m_expandAnimation->setStartValue(height);
            m_expandAnimation->setEndValue(0);
        } else {
            m_expandAnimation->setStartValue(0);
            m_expandAnimation->setEndValue(verticalScrollBar()->maximum());
        }
        m_expandAnimation->start();
    } else {
        refreshExpandedGeometry();
    }

    emit expandedChanged(m_expanded);
}

void ExpandSettingCard::toggleExpanded() { setExpanded(!m_expanded); }

void ExpandSettingCard::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    if (m_card) {
        m_card->resize(width(), m_card->height());
        setViewportMargins(0, m_card->height(), 0, 0);
    }
    if (m_scrollWidget) {
        m_scrollWidget->resize(width(), m_scrollWidget->height());
    }
    if (m_borderWidget) {
        m_borderWidget->resize(size());
        m_borderWidget->raise();
    }
}

void ExpandSettingCard::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void ExpandSettingCard::refreshExpandedGeometry()
{
    adjustViewSize();
}

int ExpandSettingCard::expandedContentHeight() const
{
    return m_viewLayout ? m_viewLayout->sizeHint().height() : 0;
}

void ExpandSettingCard::adjustViewSize()
{
    if (!m_card || !m_viewLayout || !m_spaceWidget) {
        return;
    }

    m_viewLayout->activate();
    const int headerHeight = m_card->height();
    const int viewHeight = qMax(0, expandedContentHeight());
    m_spaceWidget->setFixedHeight(viewHeight);
    if (m_expanded) {
        setFixedHeight(headerHeight + viewHeight);
    } else if (!m_expandAnimation || m_expandAnimation->state() != QAbstractAnimation::Running) {
        setFixedHeight(headerHeight);
    }

    if (layout()) {
        layout()->invalidate();
    }
    updateGeometry();
    if (QWidget *p = parentWidget()) {
        if (p->layout()) {
            p->layout()->invalidate();
        }
        p->updateGeometry();
    }
}

void ExpandSettingCard::onExpandValueChanged()
{
    if (!m_card) {
        return;
    }

    const int viewHeight = qMax(0, expandedContentHeight());
    const int headerHeight = m_card->height();
    setFixedHeight(qMax(headerHeight + viewHeight - verticalScrollBar()->value(), headerHeight));
    if (m_borderWidget) {
        m_borderWidget->update();
    }
    updateGeometry();
}

GroupSeparator::GroupSeparator(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(3);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("GroupSeparator"));
}

void GroupSeparator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setPen(dark ? QColor(0, 0, 0, 50) : QColor(0, 0, 0, 19));
    painter.drawLine(0, 1, width(), 1);
}

GroupWidget::GroupWidget(const QIcon &icon, const QString &title, const QString &content, QWidget *widget,
                         int stretch, QWidget *parent)
    : QWidget(parent), m_widget(widget)
{
    setMinimumHeight(60);

    m_iconWidget = new IconWidget(icon, this);
    m_iconWidget->setFixedSize(16, 16);

    m_titleLabel = new BodyLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_contentLabel = new CaptionLabel(content, this);
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));
    m_contentLabel->setVisible(!content.isEmpty());

    m_hBoxLayout = new QHBoxLayout(this);
    m_hBoxLayout->setSpacing(16);
    m_hBoxLayout->setContentsMargins(48, 12, 48, 12);
    m_hBoxLayout->setAlignment(Qt::AlignVCenter);

    m_textLayout = new QVBoxLayout;
    m_textLayout->setSpacing(0);
    m_textLayout->setContentsMargins(0, 0, 0, 0);
    m_textLayout->setAlignment(Qt::AlignVCenter);
    m_textLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    m_textLayout->addWidget(m_contentLabel, 0, Qt::AlignLeft);

    m_hBoxLayout->addWidget(m_iconWidget, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_hBoxLayout->addLayout(m_textLayout);
    m_hBoxLayout->addStretch(1);
    if (m_widget) {
        m_widget->setParent(this);
        m_hBoxLayout->addWidget(m_widget, stretch);
    }

    setIcon(icon);
    FluentStyleSheet::setRole(this, QStringLiteral("GroupWidget"));
}

GroupWidget::GroupWidget(FluentIcon icon, const QString &title, const QString &content, QWidget *widget, int stretch,
                         QWidget *parent)
    : GroupWidget(FluentQt::icon(icon), title, content, widget, stretch, parent)
{
}

QString GroupWidget::title() const { return m_titleLabel ? m_titleLabel->text() : QString(); }

QString GroupWidget::content() const { return m_contentLabel ? m_contentLabel->text() : QString(); }

QIcon GroupWidget::icon() const { return m_iconWidget ? m_iconWidget->icon() : QIcon(); }

IconWidget *GroupWidget::iconWidget() const { return m_iconWidget; }

QLabel *GroupWidget::titleLabel() const { return m_titleLabel; }

QLabel *GroupWidget::contentLabel() const { return m_contentLabel; }

QWidget *GroupWidget::widget() const { return m_widget; }

QHBoxLayout *GroupWidget::hBoxLayout() const { return m_hBoxLayout; }

QVBoxLayout *GroupWidget::textLayout() const { return m_textLayout; }

void GroupWidget::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void GroupWidget::setContent(const QString &content)
{
    if (m_contentLabel) {
        m_contentLabel->setText(content);
        m_contentLabel->setVisible(!content.isEmpty());
    }
}

void GroupWidget::setIcon(const QIcon &icon)
{
    if (m_iconWidget) {
        m_iconWidget->setIcon(icon);
        m_iconWidget->setHidden(icon.isNull());
    }
}

void GroupWidget::setIcon(FluentIcon icon)
{
    setIcon(FluentQt::icon(icon));
}

void GroupWidget::setIconSize(const QSize &size)
{
    if (m_iconWidget) {
        m_iconWidget->setFixedSize(size);
        m_iconWidget->setIconSize(size);
    }
}

ExpandGroupSettingCard::ExpandGroupSettingCard(const QIcon &icon, const QString &title, const QString &content,
                                               QWidget *parent)
    : ExpandSettingCard(icon, title, content, parent)
{
    if (viewLayout()) {
        viewLayout()->setContentsMargins(0, 0, 0, 0);
        viewLayout()->setSpacing(0);
    }
    FluentStyleSheet::setRole(this, QStringLiteral("ExpandGroupSettingCard"));
}

ExpandGroupSettingCard::ExpandGroupSettingCard(FluentIcon icon, const QString &title, const QString &content,
                                               QWidget *parent)
    : ExpandGroupSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

QList<QWidget *> ExpandGroupSettingCard::groupWidgets() const { return m_groupWidgets; }

int ExpandGroupSettingCard::groupCount() const { return m_groupWidgets.size(); }

GroupWidget *ExpandGroupSettingCard::addGroup(const QIcon &icon, const QString &title, const QString &content,
                                              QWidget *widget, int stretch)
{
    auto *group = new GroupWidget(icon, title, content, widget, stretch, view());
    addGroupWidget(group);
    return group;
}

GroupWidget *ExpandGroupSettingCard::addGroup(FluentIcon icon, const QString &title, const QString &content,
                                              QWidget *widget, int stretch)
{
    return addGroup(FluentQt::icon(icon), title, content, widget, stretch);
}

void ExpandGroupSettingCard::addGroupWidget(QWidget *widget)
{
    if (!widget || m_groupWidgets.contains(widget) || !viewLayout()) {
        return;
    }

    widget->setParent(view());
    m_groupWidgets.append(widget);
    viewLayout()->addWidget(widget);
    rebuildGroupSeparators();
    refreshExpandedGeometry();
}

void ExpandGroupSettingCard::removeGroupWidget(QWidget *widget)
{
    if (!widget || !m_groupWidgets.contains(widget) || !viewLayout()) {
        return;
    }

    m_groupWidgets.removeOne(widget);
    viewLayout()->removeWidget(widget);
    widget->setParent(nullptr);
    rebuildGroupSeparators();
    refreshExpandedGeometry();
}

int ExpandGroupSettingCard::expandedContentHeight() const
{
    int height = 0;
    for (QWidget *widget : m_groupWidgets) {
        if (widget) {
            height += widget->sizeHint().height() + 3;
        }
    }
    return height;
}

void ExpandGroupSettingCard::rebuildGroupSeparators()
{
    QVBoxLayout *layout = viewLayout();
    if (!layout) {
        return;
    }

    QList<GroupSeparator *> separators;
    for (int i = 0; i < layout->count(); ++i) {
        if (auto *separator = qobject_cast<GroupSeparator *>(layout->itemAt(i)->widget())) {
            separators.append(separator);
        }
    }
    for (GroupSeparator *separator : separators) {
        layout->removeWidget(separator);
        separator->deleteLater();
    }

    for (int i = 1; i < m_groupWidgets.size(); ++i) {
        const int index = layout->indexOf(m_groupWidgets.at(i));
        if (index >= 0) {
            layout->insertWidget(index, new GroupSeparator(view()));
        }
    }
}

SimpleExpandGroupSettingCard::SimpleExpandGroupSettingCard(const QIcon &icon, const QString &title,
                                                           const QString &content, QWidget *parent)
    : ExpandGroupSettingCard(icon, title, content, parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SimpleExpandGroupSettingCard"));
}

SimpleExpandGroupSettingCard::SimpleExpandGroupSettingCard(FluentIcon icon, const QString &title,
                                                           const QString &content, QWidget *parent)
    : SimpleExpandGroupSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

int SimpleExpandGroupSettingCard::expandedContentHeight() const
{
    return viewLayout() ? viewLayout()->sizeHint().height() : 0;
}

OptionsSettingCard::OptionsSettingCard(const QIcon &icon, const QString &title, const QString &content, QWidget *parent)
    : ExpandSettingCard(icon, title, content, parent)
{
    m_choiceLabel = new CaptionLabel(this);
    m_choiceLabel->setObjectName(QStringLiteral("titleLabel"));
    addWidget(m_choiceLabel);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    if (viewLayout()) {
        viewLayout()->setSpacing(19);
        viewLayout()->setContentsMargins(48, 18, 0, 18);
    }

    FluentStyleSheet::setRole(this, QStringLiteral("OptionsSettingCard"));
}

OptionsSettingCard::OptionsSettingCard(FluentIcon icon, const QString &title, const QString &content, QWidget *parent)
    : OptionsSettingCard(FluentQt::icon(icon), title, content, parent)
{
}

OptionsSettingCard::OptionsSettingCard(const QStringList &texts, const QVariantList &values, const QIcon &icon,
                                       const QString &title, const QString &content, QWidget *parent)
    : OptionsSettingCard(icon, title, content, parent)
{
    for (int i = 0; i < texts.size(); ++i) {
        addOption(texts.at(i), i < values.size() ? values.at(i) : QVariant(texts.at(i)));
    }
}

OptionsSettingCard::OptionsSettingCard(const QStringList &texts, const QVariantList &values, FluentIcon icon,
                                       const QString &title, const QString &content, QWidget *parent)
    : OptionsSettingCard(texts, values, FluentQt::icon(icon), title, content, parent)
{
}

QVariant OptionsSettingCard::value() const { return m_value; }

QLabel *OptionsSettingCard::choiceLabel() const { return m_choiceLabel; }

QButtonGroup *OptionsSettingCard::buttonGroup() const { return m_buttonGroup; }

void OptionsSettingCard::addOption(const QString &text, const QVariant &value)
{
    if (!m_buttonGroup || !viewLayout()) {
        return;
    }

    const QVariant optionValue = value.isValid() ? value : QVariant(text);
    auto *button = new RadioButton(text, view());
    button->setProperty(kOptionValueProperty, optionValue);
    m_buttonGroup->addButton(button);
    viewLayout()->addWidget(button);

    connect(button, &QAbstractButton::clicked, this,
            [this, button]() { setValue(button->property(kOptionValueProperty)); });

    if (!m_value.isValid()) {
        setValue(optionValue);
    }
    refreshExpandedGeometry();
}

void OptionsSettingCard::setValue(const QVariant &value)
{
    if (m_value == value) {
        updateChoiceLabel();
        return;
    }

    m_value = value;
    if (m_buttonGroup) {
        for (QAbstractButton *button : m_buttonGroup->buttons()) {
            const bool checked = button->property(kOptionValueProperty) == m_value;
            button->setChecked(checked);
        }
    }
    updateChoiceLabel();
    emit valueChanged(m_value);
}

void OptionsSettingCard::updateChoiceLabel()
{
    if (!m_choiceLabel || !m_buttonGroup) {
        return;
    }

    QString selectedText;
    for (QAbstractButton *button : m_buttonGroup->buttons()) {
        if (button->property(kOptionValueProperty) == m_value) {
            selectedText = button->text();
            break;
        }
    }

    m_choiceLabel->setText(selectedText);
}

FolderItem::FolderItem(const QString &path, QWidget *parent) : QWidget(parent)
{
    setFixedHeight(53);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(48, 0, 60, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignVCenter);

    m_pathLabel = new BodyLabel(this);
    m_pathLabel->setObjectName(QStringLiteral("titleLabel"));
    m_pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_removeButton = new ToolButton(icon(FluentIcon::Close), this);
    FluentStyleSheet::setRole(m_removeButton, QStringLiteral("FolderRemoveButton"));
    m_removeButton->setFixedSize(39, 29);
    m_removeButton->setIconSize(QSize(12, 12));
    m_removeButton->setToolTip(tr("Remove"));

    layout->addWidget(m_pathLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addSpacing(16);
    layout->addStretch(1);
    layout->addWidget(m_removeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    FluentStyleSheet::setRole(this, QStringLiteral("FolderItem"));
    setPath(path);

    connect(m_removeButton, &QToolButton::clicked, this, [this]() { emit removeRequested(m_path); });
}

QString FolderItem::path() const { return m_path; }

QLabel *FolderItem::pathLabel() const { return m_pathLabel; }

QToolButton *FolderItem::removeButton() const { return m_removeButton; }

void FolderItem::setPath(const QString &path)
{
    if (m_path == path) {
        return;
    }

    m_path = path;
    if (m_pathLabel) {
        m_pathLabel->setText(m_path);
    }
    emit pathChanged(m_path);
}

FolderListSettingCard::FolderListSettingCard(const QStringList &folders, const QIcon &icon, const QString &title,
                                             const QString &content, const QString &dialogDirectory, QWidget *parent)
    : ExpandSettingCard(icon, title, content, parent), m_dialogDirectory(dialogDirectory)
{
    m_addFolderButton = new PushButton(FluentQt::icon(FluentIcon::FolderAdd), tr("Add folder"), this);
    m_addFolderButton->setMinimumWidth(128);
    addWidget(m_addFolderButton);

    if (viewLayout()) {
        viewLayout()->setContentsMargins(0, 0, 0, 0);
        viewLayout()->setSpacing(0);
        viewLayout()->setAlignment(Qt::AlignTop);
    }

    connect(m_addFolderButton, &QPushButton::clicked, this, &FolderListSettingCard::chooseFolder);
    setFolders(folders);

    FluentStyleSheet::setRole(this, QStringLiteral("FolderListSettingCard"));
}

FolderListSettingCard::FolderListSettingCard(const QStringList &folders, FluentIcon icon, const QString &title,
                                             const QString &content, const QString &dialogDirectory, QWidget *parent)
    : FolderListSettingCard(folders, FluentQt::icon(icon), title, content, dialogDirectory, parent)
{
}

QStringList FolderListSettingCard::folders() const { return m_folders; }

QString FolderListSettingCard::dialogDirectory() const { return m_dialogDirectory; }

QPushButton *FolderListSettingCard::addFolderButton() const { return m_addFolderButton; }

QList<FolderItem *> FolderListSettingCard::folderItems() const { return m_folderItems; }

void FolderListSettingCard::setFolders(const QStringList &folders)
{
    QStringList uniqueFolders;
    for (const QString &folder : folders) {
        if (!folder.isEmpty() && !uniqueFolders.contains(folder)) {
            uniqueFolders.append(folder);
        }
    }

    if (m_folders == uniqueFolders) {
        return;
    }

    m_folders = uniqueFolders;
    rebuildFolderItems();
    emit foldersChanged(m_folders);
}

void FolderListSettingCard::setDialogDirectory(const QString &directory) { m_dialogDirectory = directory; }

bool FolderListSettingCard::addFolder(const QString &folder)
{
    if (folder.isEmpty() || m_folders.contains(folder)) {
        return false;
    }
    m_folders.append(folder);
    insertFolderItem(folder);
    emit folderAdded(folder);
    emit foldersChanged(m_folders);
    return true;
}

bool FolderListSettingCard::removeFolder(const QString &folder)
{
    const int index = m_folders.indexOf(folder);
    if (index < 0) {
        return false;
    }
    m_folders.removeAt(index);
    if (index < m_folderItems.size()) {
        FolderItem *item = m_folderItems.takeAt(index);
        if (viewLayout()) {
            viewLayout()->removeWidget(item);
        }
        item->deleteLater();
    }
    refreshExpandedGeometry();
    emit folderRemoved(folder);
    emit foldersChanged(m_folders);
    return true;
}

void FolderListSettingCard::chooseFolder()
{
    const QString startDirectory = m_dialogDirectory.isEmpty() ? QDir::homePath() : m_dialogDirectory;
    auto *dialog = new FolderPickerDialog(startDirectory, window());

    QObject::disconnect(dialog->acceptButton(), &QPushButton::clicked, dialog, &QDialog::accept);
    connect(dialog->acceptButton(), &QPushButton::clicked, this, [this, dialog]() {
        const QString folder = dialog->selectedFolder();
        if (!folder.isEmpty()) {
            addFolder(folder);
            m_dialogDirectory = folder;
        }
        dialog->accept();
    });

    dialog->exec();
    dialog->deleteLater();
}

void FolderListSettingCard::rebuildFolderItems()
{
    for (FolderItem *item : m_folderItems) {
        if (viewLayout()) {
            viewLayout()->removeWidget(item);
        }
        item->deleteLater();
    }
    m_folderItems.clear();

    for (const QString &folder : m_folders) {
        insertFolderItem(folder);
    }
    refreshExpandedGeometry();
}

void FolderListSettingCard::insertFolderItem(const QString &folder)
{
    auto *item = new FolderItem(folder, view());

    connect(item, &FolderItem::removeRequested, this, [this](const QString &path) {
        removeFolder(path);
    });

    m_folderItems.append(item);
    if (viewLayout()) {
        viewLayout()->addWidget(item);
    }
    refreshExpandedGeometry();
}

CustomColorSettingCard::CustomColorSettingCard(const QColor &defaultColor, const QColor &customColor,
                                               const QIcon &icon, const QString &title, const QString &content,
                                               QWidget *parent)
    : ExpandGroupSettingCard(icon, title, content, parent), m_defaultColor(normalizedColor(defaultColor)),
      m_customColor(normalizedColor(customColor))
{
    m_customColorEnabled = m_customColor != m_defaultColor;

    m_choiceLabel = new CaptionLabel(this);
    m_choiceLabel->setObjectName(QStringLiteral("titleLabel"));
    addWidget(m_choiceLabel);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_defaultRadioButton = new RadioButton(tr("Default color"), view());
    m_customRadioButton = new RadioButton(tr("Custom color"), view());
    m_buttonGroup->addButton(m_customRadioButton);
    m_buttonGroup->addButton(m_defaultRadioButton);

    auto *radioWidget = new QWidget(view());
    auto *radioLayout = new QVBoxLayout(radioWidget);
    radioLayout->setContentsMargins(48, 18, 0, 18);
    radioLayout->setSpacing(19);
    radioLayout->setAlignment(Qt::AlignTop);
    radioLayout->addWidget(m_customRadioButton);
    radioLayout->addWidget(m_defaultRadioButton);
    radioLayout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *customRow = new QWidget(view());
    auto *customLayout = new QHBoxLayout(customRow);
    customLayout->setContentsMargins(48, 18, 44, 18);
    auto *customLabel = new BodyLabel(tr("Custom color"), customRow);
    customLabel->setObjectName(QStringLiteral("titleLabel"));
    customLayout->addWidget(customLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    customLayout->addStretch(1);

    m_colorPicker = new ColorPickerButton(m_customColor, customRow);
    customLayout->addWidget(m_colorPicker, 0, Qt::AlignRight | Qt::AlignVCenter);
    customLayout->setSizeConstraint(QLayout::SetMinimumSize);

    if (viewLayout()) {
        viewLayout()->setSpacing(0);
        viewLayout()->setContentsMargins(0, 0, 0, 0);
    }
    addGroupWidget(radioWidget);
    addGroupWidget(customRow);

    connect(m_defaultRadioButton, &QAbstractButton::clicked, this,
            [this]() { setCustomColorEnabled(false); });
    connect(m_customRadioButton, &QAbstractButton::clicked, this,
            [this]() { setCustomColorEnabled(true); });
    connect(m_colorPicker, &ColorPickerButton::colorChanged, this, [this](const QColor &color) {
        setCustomColor(color);
        setCustomColorEnabled(true);
    });

    updateColorControls();
    FluentStyleSheet::setRole(this, QStringLiteral("CustomColorSettingCard"));
}

CustomColorSettingCard::CustomColorSettingCard(const QColor &defaultColor, const QColor &customColor, FluentIcon icon,
                                               const QString &title, const QString &content, QWidget *parent)
    : CustomColorSettingCard(defaultColor, customColor, FluentQt::icon(icon), title, content, parent)
{
}

QColor CustomColorSettingCard::color() const { return m_customColorEnabled ? m_customColor : m_defaultColor; }

QColor CustomColorSettingCard::defaultColor() const { return m_defaultColor; }

QColor CustomColorSettingCard::customColor() const { return m_customColor; }

bool CustomColorSettingCard::isCustomColorEnabled() const { return m_customColorEnabled; }

bool CustomColorSettingCard::isAlphaEnabled() const { return m_colorPicker && m_colorPicker->isAlphaEnabled(); }

QLabel *CustomColorSettingCard::choiceLabel() const { return m_choiceLabel; }

RadioButton *CustomColorSettingCard::defaultRadioButton() const { return m_defaultRadioButton; }

RadioButton *CustomColorSettingCard::customRadioButton() const { return m_customRadioButton; }

ColorPickerButton *CustomColorSettingCard::colorPicker() const { return m_colorPicker; }

void CustomColorSettingCard::setColor(const QColor &color)
{
    const QColor nextColor = normalizedColor(color);
    if (nextColor == m_defaultColor) {
        setCustomColorEnabled(false);
        return;
    }

    setCustomColor(nextColor);
    setCustomColorEnabled(true);
}

void CustomColorSettingCard::setDefaultColor(const QColor &color)
{
    const QColor nextColor = normalizedColor(color);
    if (m_defaultColor == nextColor) {
        return;
    }

    const QColor previousColor = this->color();
    m_defaultColor = nextColor;
    if (!m_customColor.isValid()) {
        m_customColor = m_defaultColor;
    }

    updateColorControls();
    emit defaultColorChanged(m_defaultColor);
    if (previousColor != this->color()) {
        emit colorChanged(this->color());
    }
}

void CustomColorSettingCard::setCustomColor(const QColor &color)
{
    const QColor nextColor = normalizedColor(color);
    if (m_customColor == nextColor) {
        return;
    }

    const QColor previousColor = this->color();
    m_customColor = nextColor;
    updateColorControls();
    emit customColorChanged(m_customColor);
    if (m_customColorEnabled && previousColor != this->color()) {
        emit colorChanged(this->color());
    }
}

void CustomColorSettingCard::setCustomColorEnabled(bool enabled)
{
    if (m_customColorEnabled == enabled) {
        updateColorControls();
        return;
    }

    const QColor previousColor = this->color();
    m_customColorEnabled = enabled;
    updateColorControls();
    emit customColorEnabledChanged(m_customColorEnabled);
    if (previousColor != this->color()) {
        emit colorChanged(this->color());
    }
}

void CustomColorSettingCard::setAlphaEnabled(bool enabled)
{
    if (m_colorPicker) {
        m_colorPicker->setAlphaEnabled(enabled);
    }
}

void CustomColorSettingCard::updateColorControls()
{
    if (m_choiceLabel) {
        m_choiceLabel->setText(m_customColorEnabled ? tr("Custom color") : tr("Default color"));
    }
    if (m_defaultRadioButton) {
        const QSignalBlocker blocker(m_defaultRadioButton);
        m_defaultRadioButton->setChecked(!m_customColorEnabled);
    }
    if (m_customRadioButton) {
        const QSignalBlocker blocker(m_customRadioButton);
        m_customRadioButton->setChecked(m_customColorEnabled);
    }
    if (m_colorPicker) {
        const QSignalBlocker blocker(m_colorPicker);
        m_colorPicker->setColor(m_customColor);
        m_colorPicker->setEnabled(m_customColorEnabled);
    }
}

} // namespace FluentQt
