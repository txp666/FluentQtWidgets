#include <FluentQtWidgets/Widgets/AudioWaveformWidget.h>

#include <FluentQtWidgets/Theme.h>

#include <QtCore/QtMath>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtWidgets/QSizePolicy>

#include <algorithm>
#include <cmath>

namespace FluentQt {

namespace {

qreal boundedLevel(qreal value)
{
    if (!std::isfinite(value)) {
        return 0;
    }
    return qBound<qreal>(0, qAbs(value), 1);
}

qreal eventX(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position().x();
#else
    return event->pos().x();
#endif
}

} // namespace

AudioWaveformWidget::AudioWaveformWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_StyledBackground, true);

    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) { update(); });
}

QSize AudioWaveformWidget::sizeHint() const { return QSize(640, 180); }

QSize AudioWaveformWidget::minimumSizeHint() const { return QSize(180, 72); }

QVector<qreal> AudioWaveformWidget::samples() const { return m_samples; }

QVariantList AudioWaveformWidget::sampleLevels() const
{
    QVariantList result;
    result.reserve(m_samples.size());
    for (qreal sample : m_samples) {
        result.append(sample);
    }
    return result;
}

qreal AudioWaveformWidget::progress() const { return m_progress; }

qreal AudioWaveformWidget::barWidth() const { return m_barWidth; }

qreal AudioWaveformWidget::barGap() const { return m_barGap; }

qreal AudioWaveformWidget::minBarHeight() const { return m_minBarHeight; }

QColor AudioWaveformWidget::lightPlayedColor() const { return m_lightPlayedColor; }

QColor AudioWaveformWidget::darkPlayedColor() const { return m_darkPlayedColor; }

QColor AudioWaveformWidget::lightRemainingColor() const { return m_lightRemainingColor; }

QColor AudioWaveformWidget::darkRemainingColor() const { return m_darkRemainingColor; }

QColor AudioWaveformWidget::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor AudioWaveformWidget::darkBackgroundColor() const { return m_darkBackgroundColor; }

void AudioWaveformWidget::setSamples(const QVector<qreal> &samples)
{
    QVector<qreal> normalized;
    normalized.reserve(samples.size());
    for (qreal sample : samples) {
        normalized.append(boundedLevel(sample));
    }

    if (m_samples == normalized) {
        return;
    }

    m_samples = normalized;
    update();
    emit samplesChanged();
}

void AudioWaveformWidget::setSampleLevels(const QVariantList &samples)
{
    QVector<qreal> normalized;
    normalized.reserve(samples.size());
    for (const QVariant &sample : samples) {
        normalized.append(sample.toReal());
    }
    setSamples(normalized);
}

void AudioWaveformWidget::setProgress(qreal progress)
{
    const qreal boundedProgress = qBound<qreal>(0, progress, 1);
    if (qFuzzyCompare(m_progress + 1, boundedProgress + 1)) {
        return;
    }

    m_progress = boundedProgress;
    update();
    emit progressChanged(m_progress);
}

void AudioWaveformWidget::setBarWidth(qreal width)
{
    const qreal boundedWidth = qMax<qreal>(1, width);
    if (qFuzzyCompare(m_barWidth + 1, boundedWidth + 1)) {
        return;
    }

    m_barWidth = boundedWidth;
    update();
    emit metricsChanged();
}

void AudioWaveformWidget::setBarGap(qreal gap)
{
    const qreal boundedGap = qMax<qreal>(0, gap);
    if (qFuzzyCompare(m_barGap + 1, boundedGap + 1)) {
        return;
    }

    m_barGap = boundedGap;
    update();
    emit metricsChanged();
}

void AudioWaveformWidget::setMinBarHeight(qreal height)
{
    const qreal boundedHeight = qMax<qreal>(1, height);
    if (qFuzzyCompare(m_minBarHeight + 1, boundedHeight + 1)) {
        return;
    }

    m_minBarHeight = boundedHeight;
    update();
    emit metricsChanged();
}

void AudioWaveformWidget::setLightPlayedColor(const QColor &color)
{
    if (m_lightPlayedColor == color) {
        return;
    }
    m_lightPlayedColor = color;
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::setDarkPlayedColor(const QColor &color)
{
    if (m_darkPlayedColor == color) {
        return;
    }
    m_darkPlayedColor = color;
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::setLightRemainingColor(const QColor &color)
{
    if (m_lightRemainingColor == color) {
        return;
    }
    m_lightRemainingColor = color;
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::setDarkRemainingColor(const QColor &color)
{
    if (m_darkRemainingColor == color) {
        return;
    }
    m_darkRemainingColor = color;
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::setLightBackgroundColor(const QColor &color)
{
    if (m_lightBackgroundColor == color) {
        return;
    }
    m_lightBackgroundColor = color;
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::setDarkBackgroundColor(const QColor &color)
{
    if (m_darkBackgroundColor == color) {
        return;
    }
    m_darkBackgroundColor = color;
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::setCustomWaveformColor(const QColor &lightPlayed, const QColor &darkPlayed,
                                                 const QColor &lightRemaining, const QColor &darkRemaining)
{
    m_lightPlayedColor = lightPlayed;
    m_darkPlayedColor = darkPlayed;
    if (lightRemaining.isValid()) {
        m_lightRemainingColor = lightRemaining;
    }
    if (darkRemaining.isValid()) {
        m_darkRemainingColor = darkRemaining;
    }
    update();
    emit colorsChanged();
}

void AudioWaveformWidget::clear()
{
    if (m_samples.isEmpty()) {
        return;
    }
    m_samples.clear();
    update();
    emit samplesChanged();
}

QColor AudioWaveformWidget::playedColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? m_darkPlayedColor : m_lightPlayedColor;
}

QColor AudioWaveformWidget::remainingColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? m_darkRemainingColor : m_lightRemainingColor;
}

QColor AudioWaveformWidget::backgroundColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? m_darkBackgroundColor : m_lightBackgroundColor;
}

QVector<qreal> AudioWaveformWidget::displaySamples(int maximumBars) const
{
    if (maximumBars <= 0 || m_samples.isEmpty()) {
        return {};
    }
    if (m_samples.size() <= maximumBars) {
        return m_samples;
    }

    QVector<qreal> result;
    result.reserve(maximumBars);
    const int total = m_samples.size();
    for (int i = 0; i < maximumBars; ++i) {
        const int begin = static_cast<int>(std::floor(static_cast<qreal>(i) * total / maximumBars));
        const int end = qMax(begin + 1, static_cast<int>(std::floor(static_cast<qreal>(i + 1) * total / maximumBars)));
        qreal level = 0;
        for (int j = begin; j < qMin(end, total); ++j) {
            level = qMax(level, m_samples.at(j));
        }
        result.append(level);
    }
    return result;
}

qreal AudioWaveformWidget::progressForPosition(qreal x) const
{
    const QRectF content = rect().adjusted(16, 16, -16, -16);
    if (content.width() <= 0) {
        return 0;
    }
    return qBound<qreal>(0, (x - content.left()) / content.width(), 1);
}

void AudioWaveformWidget::updateProgressFromPosition(qreal x, bool moving)
{
    const qreal nextProgress = progressForPosition(x);
    setProgress(nextProgress);
    if (moving) {
        emit waveformMoved(nextProgress);
    } else {
        emit waveformClicked(nextProgress);
    }
}

void AudioWaveformWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawRoundedRect(rect(), 8, 8);

    const QRectF content = rect().adjusted(16, 16, -16, -16);
    if (content.width() <= 0 || content.height() <= 0) {
        return;
    }

    const qreal step = m_barWidth + m_barGap;
    const int maximumBars = qMax(1, static_cast<int>(std::floor((content.width() + m_barGap) / step)));
    const QVector<qreal> bars = displaySamples(maximumBars);
    const qreal centerY = content.center().y();
    const qreal progressX = content.left() + content.width() * m_progress;

    if (bars.isEmpty()) {
        QPen pen(remainingColor(), qMax<qreal>(1, m_barWidth), Qt::DotLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(QPointF(content.left(), centerY), QPointF(content.right(), centerY));
        return;
    }

    QPen pen(playedColor(), m_barWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);

    const qreal maxBarHeight = content.height();
    for (int i = 0; i < bars.size(); ++i) {
        const qreal x = content.left() + i * step + m_barWidth / 2.0;
        if (x > content.right()) {
            break;
        }
        const qreal barHeight = qBound<qreal>(m_minBarHeight, m_minBarHeight + bars.at(i) * (maxBarHeight - m_minBarHeight),
                                              maxBarHeight);
        pen.setColor(x <= progressX ? playedColor() : remainingColor());
        painter.setPen(pen);
        painter.drawLine(QPointF(x, centerY - barHeight / 2.0), QPointF(x, centerY + barHeight / 2.0));
    }
}

void AudioWaveformWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    m_dragging = true;
    updateProgressFromPosition(eventX(event), false);
    event->accept();
}

void AudioWaveformWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    updateProgressFromPosition(eventX(event), true);
    event->accept();
}

void AudioWaveformWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    m_dragging = false;
    updateProgressFromPosition(eventX(event), true);
    event->accept();
}

} // namespace FluentQt
