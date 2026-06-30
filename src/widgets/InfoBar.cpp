#include <FluentQtWidgets/Widgets/InfoBar.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/InfoBarManager.h>

#include <QtCore/QEvent>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

InfoBarIcon iconFromSeverity(InfoBarSeverity severity)
{
    switch (severity) {
    case InfoBarSeverity::Info:
        return InfoBarIcon::Information;
    case InfoBarSeverity::Success:
        return InfoBarIcon::Success;
    case InfoBarSeverity::Warning:
        return InfoBarIcon::Warning;
    case InfoBarSeverity::Error:
        return InfoBarIcon::Error;
    }

    return InfoBarIcon::Information;
}

InfoBarSeverity severityFromIcon(InfoBarIcon icon)
{
    switch (icon) {
    case InfoBarIcon::Information:
        return InfoBarSeverity::Info;
    case InfoBarIcon::Success:
        return InfoBarSeverity::Success;
    case InfoBarIcon::Warning:
        return InfoBarSeverity::Warning;
    case InfoBarIcon::Error:
        return InfoBarSeverity::Error;
    }

    return InfoBarSeverity::Info;
}

QString infoBarIconResourceName(InfoBarIcon icon)
{
    switch (icon) {
    case InfoBarIcon::Information:
        return QStringLiteral("Info");
    case InfoBarIcon::Success:
        return QStringLiteral("Success");
    case InfoBarIcon::Warning:
        return QStringLiteral("Warning");
    case InfoBarIcon::Error:
        return QStringLiteral("Error");
    }

    return QStringLiteral("Info");
}

QIcon infoBarIcon(InfoBarIcon icon)
{
    const QString themeToken = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QStringLiteral("dark")
                                                                                        : QStringLiteral("light");
    return QIcon(QStringLiteral(":/qfluentwidgets/images/info_bar/%1_%2.svg")
                     .arg(infoBarIconResourceName(icon), themeToken));
}

QString typeNameFromInfoBarIcon(InfoBarIcon icon)
{
    switch (icon) {
    case InfoBarIcon::Information:
        return QStringLiteral("Info");
    case InfoBarIcon::Success:
        return QStringLiteral("Success");
    case InfoBarIcon::Warning:
        return QStringLiteral("Warning");
    case InfoBarIcon::Error:
        return QStringLiteral("Error");
    }

    return QStringLiteral("Info");
}

QString wrapInfoBarText(const QString &text, int maxChars)
{
    if (text.size() <= maxChars || maxChars <= 0) {
        return text;
    }

    QStringList lines;
    QString remaining = text;
    while (remaining.size() > maxChars) {
        int breakIndex = maxChars;
        for (int i = maxChars; i >= maxChars / 2; --i) {
            if (remaining.at(i).isSpace()) {
                breakIndex = i;
                break;
            }
        }
        lines.append(remaining.left(breakIndex).trimmed());
        remaining = remaining.mid(breakIndex).trimmed();
    }

    if (!remaining.isEmpty()) {
        lines.append(remaining);
    }
    return lines.join(QLatin1Char('\n'));
}

} // namespace

// --- InfoBarIconWidget ---

InfoBarIconWidget::InfoBarIconWidget(InfoBarIcon icon, QWidget *parent)
    : QWidget(parent), m_icon(icon)
{
    setFixedSize(36, 36);
}

InfoBarIconWidget::InfoBarIconWidget(InfoBarSeverity icon, QWidget *parent)
    : InfoBarIconWidget(iconFromSeverity(icon), parent)
{
}

InfoBarIconWidget::InfoBarIconWidget(const QIcon &icon, QWidget *parent)
    : InfoBarIconWidget(InfoBarIcon::Information, parent)
{
    m_customIcon = icon;
}

InfoBarIcon InfoBarIconWidget::icon() const
{
    return m_icon;
}

QIcon InfoBarIconWidget::customIcon() const
{
    return m_customIcon;
}

InfoBarSeverity InfoBarIconWidget::severity() const
{
    return severityFromIcon(m_icon);
}

void InfoBarIconWidget::setIcon(InfoBarIcon icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    m_customIcon = QIcon();
    update();
}

void InfoBarIconWidget::setIcon(InfoBarSeverity icon)
{
    setIcon(iconFromSeverity(icon));
}

void InfoBarIconWidget::setIcon(const QIcon &icon)
{
    m_customIcon = icon;
    update();
}

void InfoBarIconWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QRect rect(10, 10, 15, 15);
    const QIcon qicon = !m_customIcon.isNull() ? m_customIcon : infoBarIcon(m_icon);
    qicon.paint(&painter, rect);
}

// --- InfoBar ---

InfoBar::InfoBar(InfoBarSeverity icon, const QString &title, const QString &content,
                 Qt::Orientation orient, bool isClosable, int duration,
                 InfoBarPosition position, QWidget *parent)
    : InfoBar(iconFromSeverity(icon), title, content, orient, isClosable, duration, position, parent)
{
}

InfoBar::InfoBar(InfoBarIcon icon, const QString &title, const QString &content,
                 Qt::Orientation orient, bool isClosable, int duration,
                 InfoBarPosition position, QWidget *parent)
    : QFrame(parent)
    , m_severity(severityFromIcon(icon))
    , m_icon(icon)
    , m_title(title)
    , m_content(content)
    , m_orient(orient)
    , m_isClosable(isClosable)
    , m_duration(duration)
    , m_position(position)
{
    initWidget();
}

InfoBar::InfoBar(const QIcon &icon, const QString &title, const QString &content,
                 Qt::Orientation orient, bool isClosable, int duration,
                 InfoBarPosition position, QWidget *parent)
    : QFrame(parent)
    , m_severity(InfoBarSeverity::Info)
    , m_icon(InfoBarIcon::Information)
    , m_customIcon(icon)
    , m_title(title)
    , m_content(content)
    , m_orient(orient)
    , m_isClosable(isClosable)
    , m_duration(duration)
    , m_position(position)
{
    initWidget();
}

InfoBar::InfoBar(FluentIcon icon, const QString &title, const QString &content,
                 Qt::Orientation orient, bool isClosable, int duration,
                 InfoBarPosition position, QWidget *parent)
    : InfoBar(FluentQt::icon(icon, Theme::Auto), title, content, orient, isClosable, duration, position, parent)
{
}

InfoBar *InfoBar::newInfoBar(InfoBarSeverity icon, const QString &title, const QString &content,
                             Qt::Orientation orient, bool isClosable, int duration,
                             InfoBarPosition position, QWidget *parent)
{
    return newInfoBar(iconFromSeverity(icon), title, content, orient, isClosable, duration, position, parent);
}

InfoBar *InfoBar::newInfoBar(InfoBarIcon icon, const QString &title, const QString &content,
                             Qt::Orientation orient, bool isClosable, int duration,
                             InfoBarPosition position, QWidget *parent)
{
    auto *w = new InfoBar(icon, title, content, orient, isClosable, duration, position, parent);
    w->show();
    return w;
}

InfoBar *InfoBar::newInfoBar(const QIcon &icon, const QString &title, const QString &content,
                             Qt::Orientation orient, bool isClosable, int duration,
                             InfoBarPosition position, QWidget *parent)
{
    auto *w = new InfoBar(icon, title, content, orient, isClosable, duration, position, parent);
    w->show();
    return w;
}

InfoBar *InfoBar::newInfoBar(FluentIcon icon, const QString &title, const QString &content,
                             Qt::Orientation orient, bool isClosable, int duration,
                             InfoBarPosition position, QWidget *parent)
{
    return newInfoBar(FluentQt::icon(icon, Theme::Auto), title, content, orient, isClosable, duration, position, parent);
}

InfoBar *InfoBar::info(const QString &title, const QString &content,
                       Qt::Orientation orient, bool isClosable, int duration,
                       InfoBarPosition position, QWidget *parent)
{
    return newInfoBar(InfoBarIcon::Information, title, content, orient, isClosable, duration, position, parent);
}

InfoBar *InfoBar::success(const QString &title, const QString &content,
                          Qt::Orientation orient, bool isClosable, int duration,
                          InfoBarPosition position, QWidget *parent)
{
    return newInfoBar(InfoBarIcon::Success, title, content, orient, isClosable, duration, position, parent);
}

InfoBar *InfoBar::warning(const QString &title, const QString &content,
                          Qt::Orientation orient, bool isClosable, int duration,
                          InfoBarPosition position, QWidget *parent)
{
    return newInfoBar(InfoBarIcon::Warning, title, content, orient, isClosable, duration, position, parent);
}

InfoBar *InfoBar::error(const QString &title, const QString &content,
                        Qt::Orientation orient, bool isClosable, int duration,
                        InfoBarPosition position, QWidget *parent)
{
    return newInfoBar(InfoBarIcon::Error, title, content, orient, isClosable, duration, position, parent);
}

InfoBarSeverity InfoBar::severity() const
{
    return m_severity;
}

InfoBarIcon InfoBar::iconType() const
{
    return m_icon;
}

QString InfoBar::title() const
{
    return m_title;
}

QString InfoBar::content() const
{
    return m_content;
}

Qt::Orientation InfoBar::orient() const
{
    return m_orient;
}

bool InfoBar::isClosable() const
{
    return m_isClosable;
}

int InfoBar::duration() const
{
    return m_duration;
}

InfoBarPosition InfoBar::position() const
{
    return m_position;
}

QLabel *InfoBar::titleLabel() const
{
    return m_titleLabel;
}

QLabel *InfoBar::contentLabel() const
{
    return m_contentLabel;
}

InfoBarIconWidget *InfoBar::iconWidget() const
{
    return m_iconWidget;
}

QToolButton *InfoBar::closeButton() const
{
    return m_closeButton;
}

void InfoBar::initWidget()
{
    setFrameShape(QFrame::NoFrame);

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);

    m_opacityAni = new QPropertyAnimation(m_opacityEffect, "opacity", this);

    m_hBoxLayout = new QHBoxLayout(this);

    m_iconWidget = m_customIcon.isNull() ? new InfoBarIconWidget(m_icon, this)
                                         : new InfoBarIconWidget(m_customIcon, this);
    m_titleLabel = new QLabel(this);
    m_contentLabel = new QLabel(this);

    m_closeButton = new TransparentToolButton(icon(FluentIcon::Close), this);
    m_closeButton->setFixedSize(36, 36);
    m_closeButton->setIconSize(QSize(12, 12));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setVisible(m_isClosable);
    FluentStyleSheet::setRole(m_closeButton, QStringLiteral("InfoBarCloseButton"));

    if (m_orient == Qt::Horizontal) {
        m_textLayout = new QHBoxLayout;
        m_widgetHLayout = new QHBoxLayout;
    } else {
        m_textVLayout = new QVBoxLayout;
        m_widgetVLayout = new QVBoxLayout;
    }

    applyQss();
    initLayout();

    connect(m_closeButton, &QToolButton::clicked, this, &InfoBar::close);
}

void InfoBar::initLayout()
{
    m_hBoxLayout->setContentsMargins(6, 6, 6, 6);
    m_hBoxLayout->setSpacing(0);

    // Add icon
    m_hBoxLayout->addWidget(m_iconWidget, 0, Qt::AlignTop | Qt::AlignLeft);

    // Text layout
    if (m_orient == Qt::Horizontal) {
        m_textLayout->setContentsMargins(1, 8, 0, 8);
        m_textLayout->setSpacing(5);
        m_textLayout->setAlignment(Qt::AlignTop);

        m_textLayout->addWidget(m_titleLabel, 1, Qt::AlignTop);
        m_titleLabel->setVisible(!m_title.isEmpty());

        m_textLayout->addSpacing(7);
        m_textLayout->addWidget(m_contentLabel, 1, Qt::AlignTop);
        m_contentLabel->setVisible(!m_content.isEmpty());

        m_hBoxLayout->addLayout(m_textLayout);

        // Widget layout
        m_widgetHLayout->setSpacing(10);
        m_hBoxLayout->addLayout(m_widgetHLayout);
    } else {
        m_textVLayout->setContentsMargins(1, 8, 0, 8);
        m_textVLayout->setSpacing(5);
        m_textVLayout->setAlignment(Qt::AlignTop);

        m_textVLayout->addWidget(m_titleLabel, 1, Qt::AlignTop);
        m_titleLabel->setVisible(!m_title.isEmpty());

        m_textVLayout->addWidget(m_contentLabel, 1, Qt::AlignTop);
        m_contentLabel->setVisible(!m_content.isEmpty());

        m_textVLayout->addLayout(m_widgetVLayout);

        m_hBoxLayout->addLayout(m_textVLayout);
    }

    m_hBoxLayout->addSpacing(12);
    m_hBoxLayout->addWidget(m_closeButton, 0, Qt::AlignTop | Qt::AlignLeft);

    adjustText();
}

void InfoBar::applyQss()
{
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));

    setProperty("type", typeNameFromInfoBarIcon(m_icon));

    FluentStyleSheet::setRole(this, QStringLiteral("InfoBar"));
}

void InfoBar::addWidget(QWidget *widget, int stretch)
{
    if (m_orient == Qt::Horizontal) {
        m_widgetHLayout->addSpacing(6);
        m_widgetHLayout->addWidget(widget, stretch, Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        m_widgetVLayout->addSpacing(6);
        m_widgetVLayout->addWidget(widget, stretch, Qt::AlignLeft | Qt::AlignTop);
    }
}

void InfoBar::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

void InfoBar::fadeOut()
{
    m_opacityAni->setDuration(200);
    m_opacityAni->setStartValue(1.0);
    m_opacityAni->setEndValue(0.0);
    connect(m_opacityAni, &QPropertyAnimation::finished, this, &InfoBar::close);
    m_opacityAni->start();
}

void InfoBar::adjustText()
{
    const int widthHint = parentWidget() ? parentWidget()->width() - 50 : 900;
    const int titleChars = qBound(30, widthHint / 10, 120);
    const int contentChars = qBound(30, widthHint / 9, 120);

    if (m_titleLabel) {
        m_titleLabel->setText(wrapInfoBarText(m_title, titleChars));
        m_titleLabel->setWordWrap(false);
        m_titleLabel->adjustSize();
    }
    if (m_contentLabel) {
        m_contentLabel->setText(wrapInfoBarText(m_content, contentChars));
        m_contentLabel->setWordWrap(false);
        m_contentLabel->adjustSize();
    }
    adjustSize();
    updateGeometry();
}

bool InfoBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget()) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::WindowStateChange) {
            adjustText();
        }
    }
    return QFrame::eventFilter(watched, event);
}

void InfoBar::showEvent(QShowEvent *event)
{
    adjustText();
    QFrame::showEvent(event);

    if (m_duration >= 0) {
        QTimer::singleShot(m_duration, this, &InfoBar::fadeOut);
    }

    if (m_position != InfoBarPosition::None) {
        InfoBarManager *manager = InfoBarManager::make(m_position);
        if (manager) {
            manager->add(this);
        }
    }

    if (parentWidget()) {
        parentWidget()->installEventFilter(this);
    }
}

void InfoBar::closeEvent(QCloseEvent *event)
{
    emit closedSignal();
    deleteLater();
    event->ignore();
}

void InfoBar::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    if (!m_lightBackgroundColor.isValid() && !m_darkBackgroundColor.isValid()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setBrush(dark ? m_darkBackgroundColor : m_lightBackgroundColor);

    const QRect r = rect().adjusted(1, 1, -1, -1);
    painter.drawRoundedRect(r, 6, 6);
}

} // namespace FluentQt
