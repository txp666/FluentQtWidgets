#include <FluentQtWidgets/DateTime/PickerBase.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/CycleListWidget.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtGui/QRegion>
#include <QtGui/QResizeEvent>
#include <QtGui/QScreen>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

SeparatorWidget::SeparatorWidget(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
    , m_orientation(orientation)
{
    if (m_orientation == Qt::Horizontal) {
        setFixedHeight(1);
    } else {
        setFixedWidth(1);
    }
    setProperty("pickerSeparator", true);
    FluentStyleSheet::setRole(this, QStringLiteral("SeparatorWidget"));
    FluentStyleSheet::apply(this, FluentStyleSheetSource::TimePicker);
}

Qt::Orientation SeparatorWidget::orientation() const { return m_orientation; }

ItemMaskWidget::ItemMaskWidget(const QList<CycleListWidget *> &listWidgets, QWidget *parent)
    : QWidget(parent)
    , m_listWidgets(listWidgets)
{
    setFixedHeight(37);
    FluentStyleSheet::setRole(this, QStringLiteral("ItemMaskWidget"));
    FluentStyleSheet::apply(this, FluentStyleSheetSource::TimePicker);
}

QList<CycleListWidget *> ItemMaskWidget::listWidgets() const { return m_listWidgets; }

QColor ItemMaskWidget::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor ItemMaskWidget::darkBackgroundColor() const { return m_darkBackgroundColor; }

void ItemMaskWidget::setListWidgets(const QList<CycleListWidget *> &widgets)
{
    m_listWidgets = widgets;
    update();
}

void ItemMaskWidget::setLightBackgroundColor(const QColor &color)
{
    m_lightBackgroundColor = color;
    update();
}

void ItemMaskWidget::setDarkBackgroundColor(const QColor &color)
{
    m_darkBackgroundColor = color;
    update();
}

void ItemMaskWidget::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

void ItemMaskWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawRoundedRect(rect().adjusted(4, 0, -3, 0), 5, 5);

    painter.setPen(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? Qt::black : Qt::white);
    painter.setFont(font());
    int xOffset = 0;
    const int maskHeight = height();
    for (CycleListWidget *listWidget : std::as_const(m_listWidgets)) {
        if (!listWidget) {
            continue;
        }

        painter.save();

        const int x = listWidget->itemSize().width() / 2 + 4 + this->x();
        QListWidgetItem *item1 = listWidget->itemAt(QPoint(x, this->y() + 6));
        if (!item1) {
            painter.restore();
            continue;
        }

        const int itemWidth = item1->sizeHint().width();
        const int y = listWidget->visualItemRect(item1).y();
        painter.translate(xOffset, y - this->y() + 7);

        auto drawItemText = [&painter](QListWidgetItem *item, int yOffset) {
            if (!item) {
                return;
            }
            const Qt::Alignment align = Qt::Alignment(item->textAlignment());
            const int w = item->sizeHint().width();
            const int h = item->sizeHint().height();
            QRectF textRect;
            if (align & Qt::AlignLeft) {
                textRect = QRectF(15, yOffset, w, h);
            } else if (align & Qt::AlignRight) {
                textRect = QRectF(4, yOffset, qMax(0, w - 15), h);
            } else {
                textRect = QRectF(4, yOffset, w, h);
            }
            painter.drawText(textRect, align, item->text());
        };

        drawItemText(item1, 0);
        const QPoint secondItemPoint = pos() + QPoint(x, maskHeight - 6);
        drawItemText(listWidget->itemAt(secondItemPoint), maskHeight);

        painter.restore();
        xOffset += itemWidth + 8;
    }
}

QColor ItemMaskWidget::backgroundColor() const
{
    const QColor fallback = ThemeManager::instance()->accentColor();
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QColor custom = dark ? m_darkBackgroundColor : m_lightBackgroundColor;
    return custom.isValid() ? custom : fallback;
}

PickerColumnFormatter::PickerColumnFormatter(QObject *parent) : QObject(parent) {}

QString PickerColumnFormatter::encode(const QVariant &value) const { return value.toString(); }

QVariant PickerColumnFormatter::decode(const QString &value) const { return value; }

QString DigitFormatter::encode(const QVariant &value) const { return QString::number(value.toInt()); }

QVariant DigitFormatter::decode(const QString &value) const { return value.toInt(); }

PickerColumnButton::PickerColumnButton(const QString &name, const QVariantList &items, int width,
                                       Qt::Alignment alignment, PickerColumnFormatter *formatter, QWidget *parent)
    : QPushButton(name, parent)
    , m_name(name)
    , m_items(items)
    , m_alignment(alignment)
    , m_defaultFormatter(new PickerColumnFormatter(this))
{
    setObjectName(QStringLiteral("pickerButton"));
    setProperty("hasBorder", false);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFixedSize(width, 30);
    setFormatter(formatter);
    setAlignment(alignment);
    syncDisplayText();
}

QString PickerColumnButton::name() const { return m_name; }

QVariantList PickerColumnButton::rawItems() const { return m_items; }

QStringList PickerColumnButton::items() const
{
    QStringList result;
    result.reserve(m_items.size());
    for (const QVariant &item : m_items) {
        result.append(m_formatter->encode(item));
    }
    return result;
}

QVariant PickerColumnButton::value() const { return m_value; }

QVariant PickerColumnButton::initialValue() const { return m_initialValue; }

QString PickerColumnButton::displayValue() const
{
    return m_value.isValid() && !m_value.isNull() ? m_formatter->encode(m_value) : QString();
}

Qt::Alignment PickerColumnButton::alignment() const { return m_alignment; }

PickerColumnFormatter *PickerColumnButton::formatter() const { return m_formatter; }

void PickerColumnButton::setName(const QString &name)
{
    const bool showingName = text() == m_name;
    m_name = name;
    if (showingName) {
        setText(m_name);
    }
}

void PickerColumnButton::setItems(const QVariantList &items) { m_items = items; }

void PickerColumnButton::setValue(const QVariant &value)
{
    m_value = value;
    syncDisplayText();
}

void PickerColumnButton::setInitialValue(const QVariant &value) { m_initialValue = value; }

void PickerColumnButton::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    if (alignment & Qt::AlignLeft) {
        setProperty("align", QStringLiteral("left"));
    } else if (alignment & Qt::AlignRight) {
        setProperty("align", QStringLiteral("right"));
    } else {
        setProperty("align", QStringLiteral("center"));
    }
    FluentStyleSheet::polish(this);
}

void PickerColumnButton::setFormatter(PickerColumnFormatter *formatter)
{
    m_formatter = formatter ? formatter : m_defaultFormatter;
    syncDisplayText();
}

void PickerColumnButton::syncDisplayText()
{
    const bool hasValue = m_value.isValid() && !m_value.isNull();
    setText(hasValue ? m_formatter->encode(m_value) : m_name);
    setProperty("hasValue", hasValue);
    FluentStyleSheet::polish(this);
}

PickerPanel::PickerPanel(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("PickerPanel"));

    m_view = new QFrame(this);
    FluentStyleSheet::setRole(m_view, QStringLiteral("TimePickerPanel"));
    auto *shadow = new QGraphicsDropShadowEffect(m_view);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 30));
    m_view->setGraphicsEffect(shadow);

    m_itemMaskWidget = new ItemMaskWidget(m_listWidgets, this);
    m_hSeparatorWidget = new SeparatorWidget(Qt::Horizontal, m_view);
    m_yesButton = new TransparentToolButton(icon(FluentIcon::Accept), m_view);
    m_resetButton = new TransparentToolButton(icon(FluentIcon::Remove), m_view);
    m_cancelButton = new TransparentToolButton(icon(FluentIcon::Close), m_view);

    m_yesButton->setObjectName(QStringLiteral("pickerConfirmButton"));
    m_resetButton->setObjectName(QStringLiteral("pickerResetButton"));
    m_cancelButton->setObjectName(QStringLiteral("pickerCancelButton"));
    for (auto *button : {m_yesButton, m_resetButton, m_cancelButton}) {
        button->setProperty("pickerOperation", true);
        button->setFixedHeight(33);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    m_yesButton->setIconSize(QSize(16, 16));
    m_resetButton->setIconSize(QSize(16, 16));
    m_cancelButton->setIconSize(QSize(13, 13));

    m_hBoxLayout = new QHBoxLayout(this);
    m_hBoxLayout->setContentsMargins(12, 8, 12, 20);
    m_hBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);
    m_hBoxLayout->addWidget(m_view, 1, Qt::AlignCenter);

    m_listLayout = new QHBoxLayout;
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setContentsMargins(3, 3, 3, 3);
    m_buttonLayout->setSpacing(6);
    m_buttonLayout->addWidget(m_yesButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addWidget(m_cancelButton);

    m_vBoxLayout = new QVBoxLayout(m_view);
    m_vBoxLayout->setSpacing(0);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_vBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);
    m_vBoxLayout->addLayout(m_listLayout, 1);
    m_vBoxLayout->addWidget(m_hSeparatorWidget);
    m_vBoxLayout->addLayout(m_buttonLayout, 1);

    connect(m_yesButton, &QToolButton::clicked, this, [this]() {
        emit confirmed(value());
        fadeOut();
    });
    connect(m_resetButton, &QToolButton::clicked, this, [this]() {
        emit resetted();
        fadeOut();
    });
    connect(m_cancelButton, &QToolButton::clicked, this, &PickerPanel::fadeOut);

    setResetEnabled(false);
}

QFrame *PickerPanel::view() const { return m_view; }

ItemMaskWidget *PickerPanel::itemMaskWidget() const { return m_itemMaskWidget; }

SeparatorWidget *PickerPanel::horizontalSeparatorWidget() const { return m_hSeparatorWidget; }

QHBoxLayout *PickerPanel::listLayout() const { return m_listLayout; }

QHBoxLayout *PickerPanel::buttonLayout() const { return m_buttonLayout; }

QVBoxLayout *PickerPanel::viewLayout() const { return m_vBoxLayout; }

QList<CycleListWidget *> PickerPanel::listWidgets() const { return m_listWidgets; }

TransparentToolButton *PickerPanel::yesButton() const { return m_yesButton; }

TransparentToolButton *PickerPanel::resetButton() const { return m_resetButton; }

TransparentToolButton *PickerPanel::cancelButton() const { return m_cancelButton; }

QStringList PickerPanel::value() const
{
    QStringList result;
    result.reserve(m_listWidgets.size());
    for (CycleListWidget *widget : m_listWidgets) {
        result.append(widget && widget->currentItem() ? widget->currentItem()->text() : QString());
    }
    return result;
}

QStringList PickerPanel::columnItems(int index) const
{
    CycleListWidget *widget = column(index);
    return widget ? widget->originItems() : QStringList();
}

QString PickerPanel::columnValue(int index) const
{
    CycleListWidget *widget = column(index);
    return widget && widget->currentItem() ? widget->currentItem()->text() : QString();
}

CycleListWidget *PickerPanel::column(int index) const
{
    if (index < 0 || index >= m_listWidgets.size()) {
        return nullptr;
    }
    return m_listWidgets.at(index);
}

bool PickerPanel::isResetEnabled() const { return m_resetButton && !m_resetButton->isHidden(); }

bool PickerPanel::isScrollButtonRepeatEnabled() const { return m_scrollButtonRepeatEnabled; }

void PickerPanel::exec(const QPoint &pos, bool animated)
{
    adjustSize();
    move(clampedPosition(pos));
    show();
    raise();

    if (!animated) {
        return;
    }

    m_isExpanded = false;
    auto *animation = new QPropertyAnimation(m_view, "windowOpacity", this);
    connect(animation, &QPropertyAnimation::valueChanged, this, [this](const QVariant &value) {
        updateAnimationMask(value.toReal());
    });
    animation->setDuration(150);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PickerPanel::addColumn(const QStringList &items, int width, Qt::Alignment alignment)
{
    if (!m_listWidgets.isEmpty()) {
        m_listLayout->addWidget(new SeparatorWidget(Qt::Vertical, m_view));
    }

    const int itemHeight = 37;
    const int columnIndex = m_listWidgets.size();
    auto *widget = new CycleListWidget(items, QSize(width, itemHeight), alignment, m_view);
    widget->setScrollButtonRepeatEnabled(m_scrollButtonRepeatEnabled);
    connect(widget, &CycleListWidget::currentItemChanged, this, [this, columnIndex](QListWidgetItem *item) {
        if (item) {
            emit columnValueChanged(columnIndex, item->text());
        }
        m_itemMaskWidget->update();
    });

    m_listWidgets.append(widget);
    m_itemMaskWidget->setListWidgets(m_listWidgets);
    m_listLayout->addWidget(widget);
}

void PickerPanel::setPanelRole(const QString &role)
{
    FluentStyleSheet::setRole(this, role);
}

void PickerPanel::setViewRole(const QString &role)
{
    FluentStyleSheet::setRole(m_view, role);
}

void PickerPanel::setValue(const QStringList &value)
{
    if (value.size() != m_listWidgets.size()) {
        return;
    }
    for (int i = 0; i < value.size(); ++i) {
        setColumnValue(i, value.at(i));
    }
}

void PickerPanel::setColumnItems(int index, const QStringList &items)
{
    CycleListWidget *widget = column(index);
    if (!widget) {
        return;
    }

    const QString previous = columnValue(index);
    widget->setItems(items);
    if (items.contains(previous)) {
        widget->setSelectedItem(previous);
    } else if (!items.isEmpty()) {
        widget->setSelectedItem(items.last());
    }
    m_itemMaskWidget->update();
}

void PickerPanel::setColumnValue(int index, const QString &value)
{
    if (auto *widget = column(index)) {
        widget->setSelectedItem(value);
    }
}

void PickerPanel::setResetEnabled(bool enabled)
{
    if (m_resetButton) {
        m_resetButton->setVisible(enabled);
    }
}

void PickerPanel::setScrollButtonRepeatEnabled(bool enabled)
{
    m_scrollButtonRepeatEnabled = enabled;
    for (CycleListWidget *widget : m_listWidgets) {
        widget->setScrollButtonRepeatEnabled(enabled);
    }
}

void PickerPanel::setSelectedBackgroundColor(const QColor &light, const QColor &dark)
{
    m_itemMaskWidget->setCustomBackgroundColor(light, dark);
}

void PickerPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_itemMaskWidget || !m_view) {
        return;
    }
    const QMargins margins = m_hBoxLayout->contentsMargins();
    m_itemMaskWidget->resize(qMax(0, m_view->width() - 3), 37);
    m_itemMaskWidget->move(margins.left() + 2, margins.top() + 148);
    m_itemMaskWidget->raise();
}

QPoint PickerPanel::clampedPosition(const QPoint &preferred) const
{
    QScreen *screen = QGuiApplication::screenAt(preferred);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return preferred;
    }

    const QRect available = screen->availableGeometry();
    QPoint pos = preferred;
    pos.setX(qBound(available.left(), pos.x(), available.right() - width()));
    pos.setY(qBound(available.top(), pos.y(), available.bottom() - height()));
    return pos;
}

void PickerPanel::updateAnimationMask(qreal opacity)
{
    if (!layout() || !m_view) {
        return;
    }

    const QMargins margins = layout()->contentsMargins();
    const int maskWidth = qMax(1, m_view->width() + margins.left() + margins.right() + 120);
    const int maskHeight = qMax(1, m_view->height() + margins.top() + margins.bottom() + 12);
    const int y = m_isExpanded ? int(maskHeight / 3.0 * (1.0 - opacity))
                               : int(maskHeight / 2.0 * (1.0 - opacity));
    setMask(QRegion(0, qMax(0, y), maskWidth, qMax(1, maskHeight - y * 2)));
}

void PickerPanel::fadeOut()
{
    m_isExpanded = true;
    auto *animation = new QPropertyAnimation(this, "windowOpacity", this);
    connect(animation, &QPropertyAnimation::valueChanged, this, [this](const QVariant &value) {
        updateAnimationMask(value.toReal());
    });
    connect(animation, &QPropertyAnimation::finished, this, &QObject::deleteLater);
    animation->setDuration(150);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace FluentQt
