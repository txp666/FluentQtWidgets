#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>

#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QListView>
#include <QtWidgets/QScrollBar>

#include <cmath>
#include <deque>

namespace FluentQt {

namespace {

class SmoothScrollEngine : public QObject
{
  public:
    SmoothScrollEngine(QAbstractScrollArea *area, Qt::Orientation orientation, QObject *parent = nullptr)
        : QObject(parent), m_area(area), m_orientation(orientation)
    {
        m_timer.setInterval(1000 / 60);
        connect(&m_timer, &QTimer::timeout, this, [this]() { onTick(); });
    }

    void setSmoothMode(SmoothMode mode) { m_smoothMode = mode; }
    SmoothMode smoothMode() const { return m_smoothMode; }

    void wheelEvent(QWheelEvent *event)
    {
        const int delta = event->angleDelta().y() != 0 ? event->angleDelta().y() : event->angleDelta().x();
        if (delta == 0) {
            return;
        }

        if (m_smoothMode == SmoothMode::NoSmooth || qAbs(delta) % 120 != 0) {
            QScrollBar *bar = m_orientation == Qt::Vertical ? m_area->verticalScrollBar()
                                                             : m_area->horizontalScrollBar();
            if (bar) {
                QApplication::sendEvent(bar, event);
            }
            return;
        }

        m_lastEventPos = event->position().toPoint();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_lastEventPos = event->pos();
#endif
        m_lastGlobalPos = event->globalPosition().toPoint();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_lastGlobalPos = event->globalPos();
#endif

        const qreal scaledDelta = delta * m_stepRatio;
        const int stepsTotal = qRound(m_fps * m_duration / 1000.0);
        m_stepsLeftQueue.push_back({scaledDelta, stepsTotal});
        m_stepsTotal = stepsTotal;

        if (!m_timer.isActive()) {
            m_timer.start();
        }
    }

  private:
    void onTick()
    {
        if (m_stepsLeftQueue.empty()) {
            m_timer.stop();
            return;
        }

        qreal totalDelta = 0.0;
        for (auto &entry : m_stepsLeftQueue) {
            totalDelta += subDelta(entry.first, entry.second);
            --entry.second;
        }

        while (!m_stepsLeftQueue.empty() && m_stepsLeftQueue.front().second <= 0) {
            m_stepsLeftQueue.pop_front();
        }

        QScrollBar *bar =
            m_orientation == Qt::Vertical ? m_area->verticalScrollBar() : m_area->horizontalScrollBar();
        if (!bar) {
            m_timer.stop();
            return;
        }

        const QPoint angleDelta = m_orientation == Qt::Vertical ? QPoint(0, qRound(totalDelta))
                                                                  : QPoint(qRound(totalDelta), 0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QWheelEvent wheelEvent(m_lastEventPos, m_lastGlobalPos, QPoint(), angleDelta, Qt::NoButton, Qt::NoModifier,
                               Qt::NoScrollPhase, false);
#else
        QWheelEvent wheelEvent(m_lastEventPos, m_lastGlobalPos, angleDelta, angleDelta.y(), Qt::Vertical,
                               Qt::NoButton, Qt::NoModifier);
#endif
        QApplication::sendEvent(bar, &wheelEvent);

        if (m_stepsLeftQueue.empty()) {
            m_timer.stop();
        }
    }

    qreal subDelta(qreal delta, int stepsLeft)
    {
        const qreal m = m_stepsTotal / 2.0;
        const qreal x = std::abs(m_stepsTotal - stepsLeft - m);

        switch (m_smoothMode) {
        case SmoothMode::NoSmooth:
            return 0.0;
        case SmoothMode::Constant:
            return delta / m_stepsTotal;
        case SmoothMode::Linear:
            return 2.0 * delta / m_stepsTotal * (m - x) / m;
        case SmoothMode::Quadrati:
            return 3.0 / 4.0 / m * (1.0 - x * x / m / m) * delta;
        case SmoothMode::Cosine:
            return (std::cos(x * M_PI / m) + 1.0) / (2.0 * m) * delta;
        }
        return 0.0;
    }

    QAbstractScrollArea *m_area = nullptr;
    Qt::Orientation m_orientation = Qt::Vertical;
    SmoothMode m_smoothMode = SmoothMode::Linear;
    QTimer m_timer;
    std::deque<std::pair<qreal, int>> m_stepsLeftQueue;
    QPoint m_lastEventPos;
    QPoint m_lastGlobalPos;
    int m_fps = 60;
    int m_duration = 400;
    int m_stepsTotal = 0;
    qreal m_stepRatio = 1.5;
};

} // namespace

SmoothScrollDelegate::SmoothScrollDelegate(QAbstractScrollArea *parent, bool useAnimation)
    : QObject(parent), m_area(parent), m_useAnimation(useAnimation)
{
    Q_ASSERT(m_area);

    m_verticalScrollBar = new SmoothScrollBar(Qt::Vertical, m_area);
    m_horizontalScrollBar = new SmoothScrollBar(Qt::Horizontal, m_area);
    m_verticalScrollBar->attachToScrollArea(m_area, Qt::Vertical);
    m_horizontalScrollBar->attachToScrollArea(m_area, Qt::Horizontal);

    auto *vEngine = new SmoothScrollEngine(m_area, Qt::Vertical, this);
    auto *hEngine = new SmoothScrollEngine(m_area, Qt::Horizontal, this);
    vEngine->setSmoothMode(m_smoothMode);
    hEngine->setSmoothMode(m_smoothMode);
    m_verticalSmoothScroll = vEngine;
    m_horizontalSmoothScroll = hEngine;

    setupScrollArea();
    setupItemView();

    if (m_area->viewport()) {
        m_area->viewport()->installEventFilter(this);
    }
}

SmoothScrollBar *SmoothScrollDelegate::verticalScrollBar() const { return m_verticalScrollBar; }

SmoothScrollBar *SmoothScrollDelegate::horizontalScrollBar() const { return m_horizontalScrollBar; }

SmoothScrollBar *SmoothScrollDelegate::vScrollBar() const { return m_verticalScrollBar; }

SmoothScrollBar *SmoothScrollDelegate::hScrollBar() const { return m_horizontalScrollBar; }

bool SmoothScrollDelegate::usesAnimation() const { return m_useAnimation; }

SmoothMode SmoothScrollDelegate::smoothMode() const { return m_smoothMode; }

void SmoothScrollDelegate::setSmoothMode(SmoothMode mode)
{
    if (m_smoothMode == mode) {
        return;
    }
    m_smoothMode = mode;
    auto *vEngine = static_cast<SmoothScrollEngine *>(m_verticalSmoothScroll);
    auto *hEngine = static_cast<SmoothScrollEngine *>(m_horizontalSmoothScroll);
    if (vEngine) {
        vEngine->setSmoothMode(mode);
    }
    if (hEngine) {
        hEngine->setSmoothMode(mode);
    }
}

void SmoothScrollDelegate::setupScrollArea()
{
    m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void SmoothScrollDelegate::setupItemView()
{
    auto *itemView = qobject_cast<QAbstractItemView *>(m_area);
    if (!itemView) {
        return;
    }

    itemView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    itemView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    if (qobject_cast<QListView *>(itemView)) {
        itemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        if (itemView->horizontalScrollBar()) {
            itemView->horizontalScrollBar()->setStyleSheet(QStringLiteral("QScrollBar:horizontal{height:0px;}"));
        }
    }
}

bool SmoothScrollDelegate::handleWheelEvent(QWheelEvent *event)
{
    SmoothScrollBar *verticalBar = m_verticalScrollBar;
    SmoothScrollBar *horizontalBar = m_horizontalScrollBar;
    if (!verticalBar || !horizontalBar) {
        return false;
    }

    const int verticalDelta = event->angleDelta().y();
    const int horizontalDelta = event->angleDelta().x();

    const bool verticalAtEnd =
        (verticalDelta < 0 && verticalBar->value() == verticalBar->maximum()) ||
        (verticalDelta > 0 && verticalBar->value() == verticalBar->minimum());
    const bool horizontalAtEnd =
        (horizontalDelta < 0 && horizontalBar->value() == horizontalBar->maximum()) ||
        (horizontalDelta > 0 && horizontalBar->value() == horizontalBar->minimum());

    if (verticalAtEnd || horizontalAtEnd) {
        return false;
    }

    if (verticalDelta != 0) {
        if (m_useAnimation) {
            verticalBar->scrollValue(-verticalDelta);
        } else {
            static_cast<SmoothScrollEngine *>(m_verticalSmoothScroll)->wheelEvent(event);
        }
    } else if (horizontalDelta != 0) {
        if (m_useAnimation) {
            horizontalBar->scrollValue(-horizontalDelta);
        } else {
            static_cast<SmoothScrollEngine *>(m_horizontalSmoothScroll)->wheelEvent(event);
        }
    }

    event->accept();
    return true;
}

bool SmoothScrollDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_area->viewport() && event->type() == QEvent::Wheel) {
        return handleWheelEvent(static_cast<QWheelEvent *>(event));
    }

    return QObject::eventFilter(watched, event);
}

} // namespace FluentQt
