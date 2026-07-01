#include <FluentQtWidgets/Widgets/InfoBadge.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/InfoBadgeManager.h>

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtWidgets/QSizePolicy>

namespace FluentQt {

namespace {

Theme badgeIconTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? Theme::Light : Theme::Dark;
}

template <typename Badge>
Badge *attachBadge(Badge *badge, QWidget *target, InfoBadgePosition position)
{
    if (target) {
        InfoBadgeManager::attach(badge, target, position);
    }
    return badge;
}

QString badgeText(int count)
{
    return QString::number(count);
}

} // namespace

InfoBadge::InfoBadge(QWidget *parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QFont badgeFont = font();
    badgeFont.setPixelSize(11);
    setFont(badgeFont);

    updateRole();
}

InfoBadge::InfoBadge(const QString &text, InfoLevel level, QWidget *parent) : InfoBadge(parent)
{
    setText(text);
    setLevel(level);
}

InfoBadge *InfoBadge::make(const QString &text, QWidget *parent, InfoLevel level, QWidget *target,
                           InfoBadgePosition position)
{
    auto *badge = new InfoBadge(text, level, parent);
    badge->adjustSize();
    return attachBadge(badge, target, position);
}

InfoBadge *InfoBadge::create(const QString &text, InfoLevel level, QWidget *parent, QWidget *target,
                             InfoBadgePosition position)
{
    return make(text, parent, level, target, position);
}

InfoBadge *InfoBadge::info(const QString &text, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return make(text, parent, InfoLevel::Info, target, position);
}

InfoBadge *InfoBadge::info(int count, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return info(badgeText(count), parent, target, position);
}

InfoBadge *InfoBadge::success(const QString &text, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return make(text, parent, InfoLevel::Success, target, position);
}

InfoBadge *InfoBadge::success(int count, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return success(badgeText(count), parent, target, position);
}

InfoBadge *InfoBadge::attention(const QString &text, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return make(text, parent, InfoLevel::Attention, target, position);
}

InfoBadge *InfoBadge::attention(int count, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return attention(badgeText(count), parent, target, position);
}

InfoBadge *InfoBadge::attension(const QString &text, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return attention(text, parent, target, position);
}

InfoBadge *InfoBadge::attension(int count, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return attention(count, parent, target, position);
}

InfoBadge *InfoBadge::warning(const QString &text, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return make(text, parent, InfoLevel::Warning, target, position);
}

InfoBadge *InfoBadge::warning(int count, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return warning(badgeText(count), parent, target, position);
}

InfoBadge *InfoBadge::error(const QString &text, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return make(text, parent, InfoLevel::Error, target, position);
}

InfoBadge *InfoBadge::error(int count, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return error(badgeText(count), parent, target, position);
}

InfoBadge *InfoBadge::custom(const QString &text, const QColor &lightBackground, const QColor &darkBackground,
                             QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    auto *badge = make(text, parent, InfoLevel::Info, target, position);
    badge->setCustomBackgroundColor(lightBackground, darkBackground);
    return badge;
}

InfoLevel InfoBadge::level() const { return m_level; }

QColor InfoBadge::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor InfoBadge::darkBackgroundColor() const { return m_darkBackgroundColor; }

InfoBadgeManager *InfoBadge::manager() const { return m_manager; }

void InfoBadge::setCount(int count) { setText(badgeText(count)); }

void InfoBadge::setLevel(InfoLevel level)
{
    if (m_level == level) {
        return;
    }

    m_level = level;
    updateRole();
    emit levelChanged(m_level);
}

void InfoBadge::setCustomBackgroundColor(const QColor &lightBackground, const QColor &darkBackground)
{
    m_lightBackgroundColor = lightBackground;
    m_darkBackgroundColor = darkBackground.isValid() ? darkBackground : lightBackground;

    update();
}

void InfoBadge::setLightBackgroundColor(const QColor &color)
{
    setCustomBackgroundColor(color, m_darkBackgroundColor);
}

void InfoBadge::setDarkBackgroundColor(const QColor &color)
{
    setCustomBackgroundColor(m_lightBackgroundColor, color);
}

void InfoBadge::setManager(InfoBadgeManager *manager) { m_manager = manager; }

void InfoBadge::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());

    const qreal radius = height() / 2.0;
    painter.drawRoundedRect(rect(), radius, radius);

    painter.setFont(font());
    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawText(rect(), Qt::AlignCenter, text());
}

QSize InfoBadge::sizeHint() const
{
    const QSize labelSize = QLabel::sizeHint();
    return QSize(qMax(labelSize.width(), labelSize.height()), labelSize.height());
}

QSize InfoBadge::minimumSizeHint() const
{
    const QSize labelSize = QLabel::minimumSizeHint();
    return QSize(qMax(labelSize.width(), labelSize.height()), labelSize.height());
}

QColor InfoBadge::backgroundColor() const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (m_lightBackgroundColor.isValid()) {
        return dark && m_darkBackgroundColor.isValid() ? m_darkBackgroundColor : m_lightBackgroundColor;
    }

    switch (m_level) {
    case InfoLevel::Info:
        return dark ? QColor(157, 157, 157) : QColor(138, 138, 138);
    case InfoLevel::Success:
        return dark ? QColor(108, 203, 95) : QColor(15, 123, 15);
    case InfoLevel::Attention:
        return ThemeManager::instance()->accentColor();
    case InfoLevel::Warning:
        return dark ? QColor(255, 244, 206) : QColor(157, 93, 0);
    case InfoLevel::Error:
        return dark ? QColor(255, 153, 164) : QColor(196, 43, 28);
    }
    return ThemeManager::instance()->accentColor();
}

QString InfoBadge::levelToken() const
{
    switch (m_level) {
    case InfoLevel::Info:
        return QStringLiteral("Info");
    case InfoLevel::Success:
        return QStringLiteral("Success");
    case InfoLevel::Attention:
        return QStringLiteral("Attention");
    case InfoLevel::Warning:
        return QStringLiteral("Warning");
    case InfoLevel::Error:
        return QStringLiteral("Error");
    }
    return QStringLiteral("Info");
}

void InfoBadge::updateRole()
{
    setProperty("level", levelToken());
    FluentStyleSheet::setRole(this, QStringLiteral("InfoBadge"));
}

DotInfoBadge::DotInfoBadge(InfoLevel level, QWidget *parent) : InfoBadge(parent)
{
    setLevel(level);
    setFixedSize(4, 4);
    setText(QString());
    FluentStyleSheet::setRole(this, QStringLiteral("DotInfoBadge"));
}

DotInfoBadge *DotInfoBadge::create(InfoLevel level, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return attachBadge(new DotInfoBadge(level, parent), target, position);
}

DotInfoBadge *DotInfoBadge::info(QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(InfoLevel::Info, parent, target, position);
}

DotInfoBadge *DotInfoBadge::success(QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(InfoLevel::Success, parent, target, position);
}

DotInfoBadge *DotInfoBadge::attention(QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(InfoLevel::Attention, parent, target, position);
}

DotInfoBadge *DotInfoBadge::attension(QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return attention(parent, target, position);
}

DotInfoBadge *DotInfoBadge::warning(QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(InfoLevel::Warning, parent, target, position);
}

DotInfoBadge *DotInfoBadge::error(QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(InfoLevel::Error, parent, target, position);
}

DotInfoBadge *DotInfoBadge::custom(const QColor &lightBackground, const QColor &darkBackground, QWidget *parent,
                                   QWidget *target, InfoBadgePosition position)
{
    auto *badge = new DotInfoBadge(InfoLevel::Info, parent);
    badge->setCustomBackgroundColor(lightBackground, darkBackground);
    return attachBadge(badge, target, position);
}

void DotInfoBadge::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawEllipse(rect());
}

IconInfoBadge::IconInfoBadge(QWidget *parent) : InfoBadge(parent)
{
    setText(QString());
    FluentStyleSheet::setRole(this, QStringLiteral("IconInfoBadge"));
    setFixedSize(16, 16);
    resize(16, 16);
}

IconInfoBadge::IconInfoBadge(const QIcon &icon, InfoLevel level, QWidget *parent) : IconInfoBadge(parent)
{
    setLevel(level);
    setIcon(icon);
}

IconInfoBadge::IconInfoBadge(FluentIcon icon, InfoLevel level, QWidget *parent)
    : IconInfoBadge(FluentQt::icon(icon, badgeIconTheme()), level, parent)
{
}

IconInfoBadge *IconInfoBadge::create(const QIcon &icon, InfoLevel level, QWidget *parent, QWidget *target,
                                     InfoBadgePosition position)
{
    return attachBadge(new IconInfoBadge(icon, level, parent), target, position);
}

IconInfoBadge *IconInfoBadge::create(FluentIcon icon, InfoLevel level, QWidget *parent, QWidget *target,
                                     InfoBadgePosition position)
{
    return attachBadge(new IconInfoBadge(icon, level, parent), target, position);
}

IconInfoBadge *IconInfoBadge::info(const QIcon &icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Info, parent, target, position);
}

IconInfoBadge *IconInfoBadge::info(FluentIcon icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Info, parent, target, position);
}

IconInfoBadge *IconInfoBadge::success(const QIcon &icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Success, parent, target, position);
}

IconInfoBadge *IconInfoBadge::success(FluentIcon icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Success, parent, target, position);
}

IconInfoBadge *IconInfoBadge::attention(const QIcon &icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Attention, parent, target, position);
}

IconInfoBadge *IconInfoBadge::attention(FluentIcon icon, QWidget *parent, QWidget *target,
                                        InfoBadgePosition position)
{
    return create(icon, InfoLevel::Attention, parent, target, position);
}

IconInfoBadge *IconInfoBadge::attension(const QIcon &icon, QWidget *parent, QWidget *target,
                                        InfoBadgePosition position)
{
    return attention(icon, parent, target, position);
}

IconInfoBadge *IconInfoBadge::attension(FluentIcon icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return attention(icon, parent, target, position);
}

IconInfoBadge *IconInfoBadge::warning(const QIcon &icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Warning, parent, target, position);
}

IconInfoBadge *IconInfoBadge::warning(FluentIcon icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Warning, parent, target, position);
}

IconInfoBadge *IconInfoBadge::error(const QIcon &icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Error, parent, target, position);
}

IconInfoBadge *IconInfoBadge::error(FluentIcon icon, QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return create(icon, InfoLevel::Error, parent, target, position);
}

IconInfoBadge *IconInfoBadge::custom(const QIcon &icon, const QColor &lightBackground, const QColor &darkBackground,
                                     QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    auto *badge = create(icon, InfoLevel::Info, parent);
    badge->setCustomBackgroundColor(lightBackground, darkBackground);
    return attachBadge(badge, target, position);
}

IconInfoBadge *IconInfoBadge::custom(FluentIcon icon, const QColor &lightBackground, const QColor &darkBackground,
                                     QWidget *parent, QWidget *target, InfoBadgePosition position)
{
    return custom(FluentQt::icon(icon, badgeIconTheme()), lightBackground, darkBackground, parent, target, position);
}

QIcon IconInfoBadge::icon() const { return m_icon; }

QSize IconInfoBadge::iconSize() const { return m_iconSize; }

QSize IconInfoBadge::sizeHint() const { return QSize(16, 16); }

void IconInfoBadge::setIcon(const QIcon &icon)
{
    if (m_icon.cacheKey() == icon.cacheKey()) {
        return;
    }

    m_icon = icon;
    update();
    emit iconChanged(m_icon);
}

void IconInfoBadge::setIcon(FluentIcon icon) { setIcon(FluentQt::icon(icon, badgeIconTheme())); }

void IconInfoBadge::setIconSize(const QSize &size)
{
    const QSize boundedSize(qMax(1, size.width()), qMax(1, size.height()));
    if (m_iconSize == boundedSize) {
        return;
    }

    m_iconSize = boundedSize;
    update();
}

void IconInfoBadge::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawEllipse(rect());

    if (m_icon.isNull()) {
        return;
    }

    const QSize boundedSize(qMin(width(), m_iconSize.width()), qMin(height(), m_iconSize.height()));
    const QRect iconRect((width() - boundedSize.width()) / 2, (height() - boundedSize.height()) / 2,
                         boundedSize.width(), boundedSize.height());
    m_icon.paint(&painter, iconRect, Qt::AlignCenter);
}

} // namespace FluentQt
