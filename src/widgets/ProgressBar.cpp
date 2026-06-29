#include <FluentQtWidgets/Widgets/ProgressBar.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QLocale>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QSequentialAnimationGroup>
#include <QtCore/QRectF>
#include <QtCore/QEasingCurve>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <cmath>

namespace FluentQt {

// --- ProgressBar ---

ProgressBar::ProgressBar(QWidget *parent, bool useAni) : QProgressBar(parent), m_useAni(useAni)
{
    setTextVisible(false);
    setFixedHeight(4);
    m_animation = new QPropertyAnimation(this, "val", this);
    FluentStyleSheet::setRole(this, QStringLiteral("ProgressBar"));
    connect(this, QOverload<int>::of(&QProgressBar::valueChanged), this, &ProgressBar::onValueChanged);
    setValue(0);
}

QPropertyAnimation *ProgressBar::ani() const { return m_animation; }

qreal ProgressBar::val() const { return m_val; }

bool ProgressBar::useAni() const { return m_useAni; }

bool ProgressBar::isPaused() const { return m_isPaused; }

bool ProgressBar::isError() const { return m_isError; }

QColor ProgressBar::lightBarColor() const { return m_lightBarColor.isValid() ? m_lightBarColor : themeColor(); }

QColor ProgressBar::darkBarColor() const { return m_darkBarColor.isValid() ? m_darkBarColor : themeColor(); }

QColor ProgressBar::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor ProgressBar::darkBackgroundColor() const { return m_darkBackgroundColor; }

void ProgressBar::setVal(qreal v)
{
    m_val = v;
    update();
}

void ProgressBar::setUseAni(bool use) { m_useAni = use; }

void ProgressBar::setLightBarColor(const QColor &color)
{
    m_lightBarColor = color;
    update();
}

void ProgressBar::setDarkBarColor(const QColor &color)
{
    m_darkBarColor = color;
    update();
}

void ProgressBar::setLightBackgroundColor(const QColor &color)
{
    m_lightBackgroundColor = color;
    update();
}

void ProgressBar::setDarkBackgroundColor(const QColor &color)
{
    m_darkBackgroundColor = color;
    update();
}

void ProgressBar::onValueChanged(int value)
{
    if (!m_useAni) {
        m_val = value;
        return;
    }
    m_animation->stop();
    m_animation->setEndValue(static_cast<qreal>(value));
    m_animation->setDuration(150);
    m_animation->start();
    QProgressBar::setValue(value);
}

void ProgressBar::setCustomBarColor(const QColor &light, const QColor &dark)
{
    m_lightBarColor = light;
    m_darkBarColor = dark;
    update();
}

void ProgressBar::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

void ProgressBar::pause()
{
    m_isPaused = true;
    update();
}

void ProgressBar::resume()
{
    m_isPaused = false;
    m_isError = false;
    update();
}

void ProgressBar::error()
{
    m_isError = true;
    update();
}

void ProgressBar::setPaused(bool paused)
{
    m_isPaused = paused;
    update();
}

void ProgressBar::setError(bool isError)
{
    if (isError) {
        error();
    } else {
        resume();
    }
}

QColor ProgressBar::barColor() const
{
    if (m_isPaused) {
        const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        return isDark ? QColor(252, 225, 0) : QColor(157, 93, 0);
    }
    if (m_isError) {
        const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        return isDark ? QColor(255, 153, 164) : QColor(196, 43, 28);
    }
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return isDark ? darkBarColor() : lightBarColor();
}

QColor ProgressBar::backgroundColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return isDark ? m_darkBackgroundColor : m_lightBackgroundColor;
}

QString ProgressBar::valText() const
{
    if (maximum() <= minimum()) return {};
    const int total = maximum() - minimum();
    QString result = format();
    QLocale locale = this->locale();
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    result.replace(QStringLiteral("%m"), locale.toString(total));
    result.replace(QStringLiteral("%v"), locale.toString(static_cast<int>(m_val)));
    if (total == 0)
        result.replace(QStringLiteral("%p"), locale.toString(100));
    else {
        int progress = static_cast<int>((m_val - minimum()) * 100 / total);
        result.replace(QStringLiteral("%p"), locale.toString(progress));
    }
    return result;
}

void ProgressBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setPen(backgroundColor());
    int y = static_cast<int>(std::floor(height() / 2.0));
    painter.drawLine(0, y, width(), y);

    if (minimum() >= maximum()) return;

    painter.setPen(Qt::NoPen);
    painter.setBrush(barColor());
    int w = static_cast<int>(m_val / (maximum() - minimum()) * width());
    qreal r = height() / 2.0;
    painter.drawRoundedRect(0, 0, w, height(), r, r);
}

// --- IndeterminateProgressBar ---

IndeterminateProgressBar::IndeterminateProgressBar(QWidget *parent, bool start) : QProgressBar(parent)
{
    setFixedHeight(4);
    setRange(0, 0);

    m_shortBarAni = new QPropertyAnimation(this, "shortPos", this);
    m_longBarAni = new QPropertyAnimation(this, "longPos", this);
    m_aniGroup = new QParallelAnimationGroup(this);
    m_longBarAniGroup = new QSequentialAnimationGroup(this);

    m_shortBarAni->setDuration(833);
    m_longBarAni->setDuration(1167);
    m_shortBarAni->setStartValue(0.0);
    m_longBarAni->setStartValue(0.0);
    m_shortBarAni->setEndValue(1.45);
    m_longBarAni->setEndValue(1.75);
    m_longBarAni->setEasingCurve(QEasingCurve::OutQuad);

    m_aniGroup->addAnimation(m_shortBarAni);
    m_longBarAniGroup->addPause(785);
    m_longBarAniGroup->addAnimation(m_longBarAni);
    m_aniGroup->addAnimation(m_longBarAniGroup);
    m_aniGroup->setLoopCount(-1);

    FluentStyleSheet::setRole(this, QStringLiteral("IndeterminateProgressBar"));

    if (start) {
        this->start();
    }
}

QPropertyAnimation *IndeterminateProgressBar::shortBarAni() const { return m_shortBarAni; }

QPropertyAnimation *IndeterminateProgressBar::longBarAni() const { return m_longBarAni; }

QParallelAnimationGroup *IndeterminateProgressBar::aniGroup() const { return m_aniGroup; }

QSequentialAnimationGroup *IndeterminateProgressBar::longBarAniGroup() const { return m_longBarAniGroup; }

qreal IndeterminateProgressBar::shortPos() const { return m_shortPos; }
qreal IndeterminateProgressBar::longPos() const { return m_longPos; }
bool IndeterminateProgressBar::isStarted() const { return m_aniGroup->state() == QAbstractAnimation::Running; }
bool IndeterminateProgressBar::isPaused() const { return m_aniGroup->state() == QAbstractAnimation::Paused; }
bool IndeterminateProgressBar::isError() const { return m_isError; }
QColor IndeterminateProgressBar::lightBarColor() const
{
    return m_lightBarColor.isValid() ? m_lightBarColor : themeColor();
}
QColor IndeterminateProgressBar::darkBarColor() const
{
    return m_darkBarColor.isValid() ? m_darkBarColor : themeColor();
}

void IndeterminateProgressBar::setShortPos(qreal p)
{
    m_shortPos = p;
    update();
}

void IndeterminateProgressBar::setLongPos(qreal p)
{
    m_longPos = p;
    update();
}

void IndeterminateProgressBar::setLightBarColor(const QColor &color)
{
    m_lightBarColor = color;
    update();
}

void IndeterminateProgressBar::setDarkBarColor(const QColor &color)
{
    m_darkBarColor = color;
    update();
}

void IndeterminateProgressBar::start()
{
    m_shortPos = 0;
    m_longPos = 0;
    m_isError = false;
    m_aniGroup->start();
    update();
}

void IndeterminateProgressBar::stop()
{
    m_aniGroup->stop();
    m_shortPos = 0;
    m_longPos = 0;
    update();
}

void IndeterminateProgressBar::pause()
{
    m_aniGroup->pause();
    update();
}

void IndeterminateProgressBar::resume()
{
    m_aniGroup->resume();
    update();
}

void IndeterminateProgressBar::error()
{
    m_isError = true;
    m_aniGroup->stop();
    update();
}

void IndeterminateProgressBar::setPaused(bool paused)
{
    m_aniGroup->setPaused(paused);
    update();
}

void IndeterminateProgressBar::setError(bool isError)
{
    if (isError) {
        error();
    } else {
        start();
    }
}

void IndeterminateProgressBar::setCustomBarColor(const QColor &light, const QColor &dark)
{
    m_lightBarColor = light;
    m_darkBarColor = dark;
    update();
}

QColor IndeterminateProgressBar::barColor() const
{
    if (m_isError) {
        const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        return isDark ? QColor(255, 153, 164) : QColor(196, 43, 28);
    }
    if (isPaused()) {
        const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        return isDark ? QColor(252, 225, 0) : QColor(157, 93, 0);
    }
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return isDark ? darkBarColor() : lightBarColor();
}

void IndeterminateProgressBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(barColor());

    qreal r = height() / 2.0;
    int x = static_cast<int>((m_shortPos - 0.4) * width());
    int w = static_cast<int>(0.4 * width());
    painter.drawRoundedRect(x, 0, w, height(), r, r);

    x = static_cast<int>((m_longPos - 0.6) * width());
    w = static_cast<int>(0.6 * width());
    painter.drawRoundedRect(x, 0, w, height(), r, r);
}

// --- ProgressRing ---

ProgressRing::ProgressRing(QWidget *parent, bool useAni) : ProgressBar(parent, useAni)
{
    setCustomBackgroundColor(QColor(0, 0, 0, 34), QColor(255, 255, 255, 34));
    setTextVisible(false);
    setFixedSize(100, 100);
    FluentStyleSheet::setRole(this, QStringLiteral("ProgressRing"));
}

int ProgressRing::strokeWidth() const { return m_strokeWidth; }

QSize ProgressRing::sizeHint() const { return QSize(100, 100); }

void ProgressRing::setStrokeWidth(int width)
{
    if (m_strokeWidth == width) {
        return;
    }
    m_strokeWidth = qMax(1, width);
    update();
}

void ProgressRing::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const qreal cw = m_strokeWidth;
    const qreal side = qMin(height(), width()) - cw;
    QRectF ringRect(cw / 2.0, height() / 2.0 - side / 2.0, side, side);

    QPen pen(backgroundColor(), cw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawArc(ringRect, 0, 360 * 16);

    if (maximum() <= minimum()) {
        return;
    }

    pen.setColor(barColor());
    painter.setPen(pen);
    const int degree = static_cast<int>(val() / (maximum() - minimum()) * 360);
    painter.drawArc(ringRect, 90 * 16, -degree * 16);

    if (isTextVisible()) {
        drawText(&painter, valText());
    }
}

void ProgressRing::drawText(QPainter *painter, const QString &text) const
{
    painter->setFont(font());
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter->setPen(isDark ? Qt::white : Qt::black);
    painter->drawText(rect(), Qt::AlignCenter, text);
}

// --- IndeterminateProgressRing ---

IndeterminateProgressRing::IndeterminateProgressRing(QWidget *parent, bool start) : QProgressBar(parent)
{
    setTextVisible(false);
    setFixedSize(80, 80);

    m_startAngleAni1 = new QPropertyAnimation(this, "startAngle", this);
    m_startAngleAni2 = new QPropertyAnimation(this, "startAngle", this);
    m_spanAngleAni1 = new QPropertyAnimation(this, "spanAngle", this);
    m_spanAngleAni2 = new QPropertyAnimation(this, "spanAngle", this);
    m_startAngleAniGroup = new QSequentialAnimationGroup(this);
    m_spanAngleAniGroup = new QSequentialAnimationGroup(this);
    m_aniGroup = new QParallelAnimationGroup(this);

    m_startAngleAni1->setDuration(1000);
    m_startAngleAni1->setStartValue(0);
    m_startAngleAni1->setEndValue(450);
    m_startAngleAni2->setDuration(1000);
    m_startAngleAni2->setStartValue(450);
    m_startAngleAni2->setEndValue(1080);
    m_startAngleAniGroup->addAnimation(m_startAngleAni1);
    m_startAngleAniGroup->addAnimation(m_startAngleAni2);

    m_spanAngleAni1->setDuration(1000);
    m_spanAngleAni1->setStartValue(0);
    m_spanAngleAni1->setEndValue(180);
    m_spanAngleAni2->setDuration(1000);
    m_spanAngleAni2->setStartValue(180);
    m_spanAngleAni2->setEndValue(0);
    m_spanAngleAniGroup->addAnimation(m_spanAngleAni1);
    m_spanAngleAniGroup->addAnimation(m_spanAngleAni2);

    m_aniGroup->addAnimation(m_startAngleAniGroup);
    m_aniGroup->addAnimation(m_spanAngleAniGroup);
    m_aniGroup->setLoopCount(-1);

    FluentStyleSheet::setRole(this, QStringLiteral("IndeterminateProgressRing"));
    if (start) {
        this->start();
    }
}

QPropertyAnimation *IndeterminateProgressRing::startAngleAni1() const { return m_startAngleAni1; }

QPropertyAnimation *IndeterminateProgressRing::startAngleAni2() const { return m_startAngleAni2; }

QPropertyAnimation *IndeterminateProgressRing::spanAngleAni1() const { return m_spanAngleAni1; }

QPropertyAnimation *IndeterminateProgressRing::spanAngleAni2() const { return m_spanAngleAni2; }

QSequentialAnimationGroup *IndeterminateProgressRing::startAngleAniGroup() const { return m_startAngleAniGroup; }

QSequentialAnimationGroup *IndeterminateProgressRing::spanAngleAniGroup() const { return m_spanAngleAniGroup; }

QParallelAnimationGroup *IndeterminateProgressRing::aniGroup() const { return m_aniGroup; }

int IndeterminateProgressRing::startAngle() const { return m_startAngle; }

int IndeterminateProgressRing::spanAngle() const { return m_spanAngle; }

int IndeterminateProgressRing::strokeWidth() const { return m_strokeWidth; }

bool IndeterminateProgressRing::isStarted() const { return m_aniGroup->state() == QAbstractAnimation::Running; }

QColor IndeterminateProgressRing::lightBarColor() const
{
    return m_lightBarColor.isValid() ? m_lightBarColor : themeColor();
}

QColor IndeterminateProgressRing::darkBarColor() const
{
    return m_darkBarColor.isValid() ? m_darkBarColor : themeColor();
}

QColor IndeterminateProgressRing::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor IndeterminateProgressRing::darkBackgroundColor() const { return m_darkBackgroundColor; }

void IndeterminateProgressRing::setStartAngle(int angle)
{
    m_startAngle = angle;
    update();
}

void IndeterminateProgressRing::setSpanAngle(int angle)
{
    m_spanAngle = angle;
    update();
}

void IndeterminateProgressRing::setStrokeWidth(int width)
{
    if (m_strokeWidth == width) {
        return;
    }
    m_strokeWidth = qMax(1, width);
    update();
}

void IndeterminateProgressRing::setLightBarColor(const QColor &color)
{
    m_lightBarColor = color;
    update();
}

void IndeterminateProgressRing::setDarkBarColor(const QColor &color)
{
    m_darkBarColor = color;
    update();
}

void IndeterminateProgressRing::setLightBackgroundColor(const QColor &color)
{
    m_lightBackgroundColor = color;
    update();
}

void IndeterminateProgressRing::setDarkBackgroundColor(const QColor &color)
{
    m_darkBackgroundColor = color;
    update();
}

void IndeterminateProgressRing::start()
{
    m_startAngle = 0;
    m_spanAngle = 0;
    m_aniGroup->start();
    update();
}

void IndeterminateProgressRing::stop()
{
    m_aniGroup->stop();
    setStartAngle(0);
    setSpanAngle(0);
}

void IndeterminateProgressRing::setCustomBarColor(const QColor &light, const QColor &dark)
{
    m_lightBarColor = light;
    m_darkBarColor = dark;
    update();
}

void IndeterminateProgressRing::setCustomBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

QColor IndeterminateProgressRing::barColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return isDark ? darkBarColor() : lightBarColor();
}

QColor IndeterminateProgressRing::backgroundColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return isDark ? m_darkBackgroundColor : m_lightBackgroundColor;
}

void IndeterminateProgressRing::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const qreal cw = m_strokeWidth;
    const qreal side = qMin(height(), width()) - cw;
    QRectF ringRect(cw / 2.0, height() / 2.0 - side / 2.0, side, side);

    QPen pen(backgroundColor(), cw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawArc(ringRect, 0, 360 * 16);

    pen.setColor(barColor());
    painter.setPen(pen);
    const int startAngle = -m_startAngle + 180;
    painter.drawArc(ringRect, (startAngle % 360) * 16, -m_spanAngle * 16);
}

} // namespace FluentQt
