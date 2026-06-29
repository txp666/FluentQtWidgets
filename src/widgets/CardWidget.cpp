#include <FluentQtWidgets/Widgets/CardWidget.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/IconWidget.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtWidgets/QStyle>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {
bool isDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}
} // namespace

// ==========================================
// CardWidget
// ==========================================

CardWidget::CardWidget(QWidget *parent) : QFrame(parent)
{
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    FluentStyleSheet::setRole(this, QStringLiteral("CardWidget"));
}

int CardWidget::borderRadius() const { return m_borderRadius; }

void CardWidget::setBorderRadius(int radius)
{
    const int boundedRadius = qMax(0, radius);
    if (m_borderRadius == boundedRadius) {
        return;
    }
    m_borderRadius = boundedRadius;
    update();
}

void CardWidget::setClickEnabled(bool enabled)
{
    m_clickEnabled = enabled;
    update();
}

bool CardWidget::isClickEnabled() const { return m_clickEnabled; }

bool CardWidget::isSelected() const { return m_selected; }

void CardWidget::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }
    m_selected = selected;
    setProperty("selected", selected);
    style()->unpolish(this);
    style()->polish(this);
    update();
    emit selectedChanged(m_selected);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CardWidget::enterEvent(QEnterEvent *event)
#else
void CardWidget::enterEvent(QEvent *event)
#endif
{
    m_hover = true;
    // Trigger style transition
    style()->unpolish(this);
    style()->polish(this);
    update();
    QFrame::enterEvent(event);
}

void CardWidget::leaveEvent(QEvent *event)
{
    m_hover = false;
    m_pressed = false;
    // Trigger style transition
    style()->unpolish(this);
    style()->polish(this);
    update();
    QFrame::leaveEvent(event);
}

void CardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_clickEnabled) {
        m_pressed = true;
        style()->unpolish(this);
        style()->polish(this);
        update();
    }
    QFrame::mousePressEvent(event);
}

void CardWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_clickEnabled && m_pressed && rect().contains(event->pos())) {
        emit clicked();
    }
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        style()->unpolish(this);
        style()->polish(this);
        update();
    }
    QFrame::mouseReleaseEvent(event);
}

QColor CardWidget::normalBackgroundColor() const
{
    return isDarkTheme() ? QColor(255, 255, 255, 13) : QColor(255, 255, 255, 170);
}

QColor CardWidget::hoverBackgroundColor() const
{
    return isDarkTheme() ? QColor(255, 255, 255, 21) : QColor(255, 255, 255, 64);
}

QColor CardWidget::pressedBackgroundColor() const
{
    return isDarkTheme() ? QColor(255, 255, 255, 8) : QColor(255, 255, 255, 64);
}

QColor CardWidget::selectedBackgroundColor() const
{
    const Theme theme = ThemeManager::instance()->effectiveTheme();
    if (m_pressed) {
        return derivedThemeColor(themeColor(), ThemeColor::Dark1, theme);
    }
    if (m_hover) {
        return derivedThemeColor(themeColor(), ThemeColor::Light1, theme);
    }
    return themeColor();
}

QColor CardWidget::borderColor() const
{
    return isDarkTheme() ? QColor(0, 0, 0, 48) : QColor(0, 0, 0, 12);
}

void CardWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int r = m_borderRadius;
    const int d = 2 * r;
    const bool dark = isDarkTheme();

    // Draw top border
    QPainterPath topPath;
    topPath.arcMoveTo(1, h - d - 1, d, d, 240);
    topPath.arcTo(1, h - d - 1, d, d, 225, -60);
    topPath.lineTo(1, r);
    topPath.arcTo(1, 1, d, d, -180, -90);
    topPath.lineTo(w - r, 1);
    topPath.arcTo(w - d - 1, 1, d, d, 90, -90);
    topPath.lineTo(w - 1, h - r);
    topPath.arcTo(w - d - 1, h - d - 1, d, d, 0, -60);

    QColor topBorderColor = dark ? QColor(0, 0, 0, 20) : QColor(0, 0, 0, 15);
    if (dark) {
        if (m_pressed) {
            topBorderColor = QColor(255, 255, 255, 18);
        } else if (m_hover) {
            topBorderColor = QColor(255, 255, 255, 13);
        }
    }
    painter.setPen(topBorderColor);
    painter.setBrush(Qt::NoBrush);
    painter.strokePath(topPath, topBorderColor);

    // Draw bottom border
    QPainterPath bottomPath;
    bottomPath.arcMoveTo(1, h - d - 1, d, d, 240);
    bottomPath.arcTo(1, h - d - 1, d, d, 240, 30);
    bottomPath.lineTo(w - r - 1, h - 1);
    bottomPath.arcTo(w - d - 1, h - d - 1, d, d, 270, 30);

    QColor bottomBorderColor = topBorderColor;
    if (!dark && m_hover && !m_pressed) {
        bottomBorderColor = QColor(0, 0, 0, 27);
    }
    painter.strokePath(bottomPath, bottomBorderColor);

    // Draw background
    painter.setPen(Qt::NoPen);
    const QRect backgroundRect = rect().adjusted(1, 1, -1, -1);
    QColor bg = normalBackgroundColor();
    if (m_selected) {
        bg = selectedBackgroundColor();
    } else if (m_pressed) {
        bg = pressedBackgroundColor();
    } else if (m_hover) {
        bg = hoverBackgroundColor();
    }
    painter.setBrush(bg);
    painter.drawRoundedRect(backgroundRect, r, r);
}

// ==========================================
// SimpleCardWidget
// ==========================================

SimpleCardWidget::SimpleCardWidget(QWidget *parent) : CardWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);
    FluentStyleSheet::setRole(this, QStringLiteral("SimpleCardWidget"));
}

void SimpleCardWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(CardWidget::normalBackgroundColor());

    if (isDarkTheme()) {
        painter.setPen(QColor(0, 0, 0, 48));
    } else {
        painter.setPen(QColor(0, 0, 0, 12));
    }

    const int r = borderRadius();
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), r, r);
}

// ==========================================
// ElevatedCardWidget
// ==========================================

ElevatedCardWidget::ElevatedCardWidget(QWidget *parent) : SimpleCardWidget(parent)
{
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setOffset(0, 5);
    m_shadowEffect->setBlurRadius(38);
    m_shadowEffect->setColor(QColor(0, 0, 0, 20));
    setGraphicsEffect(m_shadowEffect);

    m_elevateAni = new QPropertyAnimation(this, "pos", this);
    m_elevateAni->setDuration(100);

    setBorderRadius(8);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ElevatedCardWidget::enterEvent(QEnterEvent *event)
#else
void ElevatedCardWidget::enterEvent(QEvent *event)
#endif
{
    SimpleCardWidget::enterEvent(event);

    if (m_elevateAni->state() != QPropertyAnimation::Running) {
        m_originalPos = pos();
    }

    startElevateAnimation(pos(), pos() - QPoint(0, 3));
}

void ElevatedCardWidget::leaveEvent(QEvent *event)
{
    SimpleCardWidget::leaveEvent(event);
    startElevateAnimation(pos(), m_originalPos);
}

void ElevatedCardWidget::mousePressEvent(QMouseEvent *event)
{
    SimpleCardWidget::mousePressEvent(event);
    startElevateAnimation(pos(), m_originalPos);
}

void ElevatedCardWidget::startElevateAnimation(const QPoint &start, const QPoint &end)
{
    m_elevateAni->setStartValue(start);
    m_elevateAni->setEndValue(end);
    m_elevateAni->start();
}

QColor ElevatedCardWidget::hoverBackgroundColor() const
{
    return isDarkTheme() ? QColor(255, 255, 255, 16) : QColor(255, 255, 255);
}

QColor ElevatedCardWidget::pressedBackgroundColor() const
{
    return isDarkTheme() ? QColor(255, 255, 255, 6) : QColor(255, 255, 255, 118);
}

// ==========================================
// CardSeparator
// ==========================================

CardSeparator::CardSeparator(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(3);
}

void CardSeparator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (isDarkTheme()) {
        painter.setPen(QColor(255, 255, 255, 46));
    } else {
        painter.setPen(QColor(0, 0, 0, 12));
    }

    painter.drawLine(2, 1, width() - 2, 1);
}

// ==========================================
// HeaderCardWidget
// ==========================================

HeaderCardWidget::HeaderCardWidget(QWidget *parent) : SimpleCardWidget(parent)
{
    auto *mainLayout = qobject_cast<QVBoxLayout *>(layout());
    if (mainLayout) {
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
    }

    m_headerView = new QWidget(this);
    m_headerView->setFixedHeight(48);
    m_headerView->setObjectName(QStringLiteral("headerView"));

    m_headerLayout = new QHBoxLayout(m_headerView);
    m_headerLayout->setContentsMargins(24, 0, 16, 0);

    m_titleLabel = new QLabel(m_headerView);
    m_titleLabel->setObjectName(QStringLiteral("headerLabel"));
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(15);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    m_headerLayout->addWidget(m_titleLabel);

    m_separator = new CardSeparator(this);

    m_view = new QWidget(this);
    m_view->setObjectName(QStringLiteral("view"));

    m_viewLayout = new QHBoxLayout(m_view);
    m_viewLayout->setContentsMargins(24, 24, 24, 24);

    if (mainLayout) {
        mainLayout->addWidget(m_headerView);
        mainLayout->addWidget(m_separator);
        mainLayout->addWidget(m_view);
    }

    FluentStyleSheet::setRole(this, QStringLiteral("HeaderCardWidget"));
}

HeaderCardWidget::HeaderCardWidget(const QString &title, QWidget *parent) : HeaderCardWidget(parent)
{
    setTitle(title);
}

QString HeaderCardWidget::title() const { return m_titleLabel ? m_titleLabel->text() : QString(); }

void HeaderCardWidget::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

QLabel *HeaderCardWidget::titleLabel() const { return m_titleLabel; }

QLabel *HeaderCardWidget::headerLabel() const { return m_titleLabel; }

QWidget *HeaderCardWidget::headerView() const { return m_headerView; }

CardSeparator *HeaderCardWidget::separator() const { return m_separator; }

QWidget *HeaderCardWidget::view() const { return m_view; }

QHBoxLayout *HeaderCardWidget::headerLayout() const { return m_headerLayout; }

QHBoxLayout *HeaderCardWidget::viewLayout() const { return m_viewLayout; }

// ==========================================
// CardGroupWidget
// ==========================================

CardGroupWidget::CardGroupWidget(const QIcon &icon, const QString &title, const QString &content, QWidget *parent)
    : QWidget(parent)
{
    m_vBoxLayout = new QVBoxLayout(this);
    m_vBoxLayout->setSpacing(0);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);

    m_hBoxLayout = new QHBoxLayout;
    m_hBoxLayout->setSpacing(15);
    m_hBoxLayout->setContentsMargins(24, 10, 24, 10);
    m_hBoxLayout->setAlignment(Qt::AlignLeft);

    m_textLayout = new QVBoxLayout;
    m_textLayout->setContentsMargins(0, 0, 0, 0);
    m_textLayout->setSpacing(0);
    m_textLayout->setAlignment(Qt::AlignCenter);

    m_separator = new CardSeparator(this);
    m_separator->hide();

    m_iconWidget = new IconWidget(icon, this);
    m_iconWidget->setFixedSize(20, 20);

    m_titleLabel = new BodyLabel(title, this);
    m_contentLabel = new CaptionLabel(content, this);
    static_cast<FluentLabelBase *>(m_contentLabel)->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));

    m_textLayout->addWidget(m_titleLabel);
    m_textLayout->addWidget(m_contentLabel);

    m_hBoxLayout->addWidget(m_iconWidget);
    m_hBoxLayout->addLayout(m_textLayout);
    m_hBoxLayout->addStretch(1);

    m_vBoxLayout->addLayout(m_hBoxLayout);
    m_vBoxLayout->addWidget(m_separator);
}

QString CardGroupWidget::title() const { return m_titleLabel->text(); }

QString CardGroupWidget::content() const { return m_contentLabel->text(); }

QIcon CardGroupWidget::icon() const { return m_iconWidget->icon(); }

IconWidget *CardGroupWidget::iconWidget() const { return m_iconWidget; }

QLabel *CardGroupWidget::titleLabel() const { return m_titleLabel; }

QLabel *CardGroupWidget::contentLabel() const { return m_contentLabel; }

CardSeparator *CardGroupWidget::separator() const { return m_separator; }

QHBoxLayout *CardGroupWidget::hBoxLayout() const { return m_hBoxLayout; }

QVBoxLayout *CardGroupWidget::textLayout() const { return m_textLayout; }

void CardGroupWidget::setTitle(const QString &text) { m_titleLabel->setText(text); }

void CardGroupWidget::setContent(const QString &text) { m_contentLabel->setText(text); }

void CardGroupWidget::setIcon(const QIcon &icon) { m_iconWidget->setIcon(icon); }

void CardGroupWidget::setIconSize(const QSize &size) { m_iconWidget->setFixedSize(size); }

void CardGroupWidget::setSeparatorVisible(bool visible) { m_separator->setVisible(visible); }

bool CardGroupWidget::isSeparatorVisible() const { return !m_separator->isHidden(); }

void CardGroupWidget::addWidget(QWidget *widget, int stretch)
{
    m_hBoxLayout->addWidget(widget, stretch);
}

// ==========================================
// GroupHeaderCardWidget
// ==========================================

GroupHeaderCardWidget::GroupHeaderCardWidget(QWidget *parent) : HeaderCardWidget(parent)
{
    m_groupLayout = new QVBoxLayout;
    m_groupLayout->setSpacing(0);
    m_groupLayout->setContentsMargins(0, 0, 0, 0);

    auto *viewLayout = this->viewLayout();
    if (viewLayout) {
        viewLayout->setContentsMargins(0, 0, 0, 0);
        viewLayout->addLayout(m_groupLayout);
    }
}

GroupHeaderCardWidget::GroupHeaderCardWidget(const QString &title, QWidget *parent)
    : GroupHeaderCardWidget(parent)
{
    setTitle(title);
}

CardGroupWidget *GroupHeaderCardWidget::addGroup(const QIcon &icon, const QString &title,
                                                  const QString &content, QWidget *widget, int stretch)
{
    auto *group = new CardGroupWidget(icon, title, content, this);
    group->addWidget(widget, stretch);

    if (!m_groupWidgets.isEmpty()) {
        m_groupWidgets.last()->setSeparatorVisible(true);
    }

    m_groupLayout->addWidget(group);
    m_groupWidgets.append(group);
    return group;
}

int GroupHeaderCardWidget::groupCount() const { return m_groupWidgets.size(); }

QList<CardGroupWidget *> GroupHeaderCardWidget::groupWidgets() const { return m_groupWidgets; }

} // namespace FluentQt
