#include <FluentQtWidgets/Widgets/StackedWidget.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPauseAnimation>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSequentialAnimationGroup>
#include <QtGui/QPixmap>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>

namespace FluentQt {

namespace {

inline QEasingCurve cubicBezierEasing(qreal x1, qreal y1, qreal x2, qreal y2)
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(x1, y1), QPointF(x2, y2), QPointF(1.0, 1.0));
    return curve;
}

QPropertyAnimation *opacityAnimation(QObject *target, int duration, qreal start, qreal end, QObject *parent)
{
    auto *animation = new QPropertyAnimation(target, "opacity", parent);
    animation->setDuration(duration);
    animation->setStartValue(start);
    animation->setEndValue(end);
    return animation;
}

QPropertyAnimation *posAnimation(QWidget *target, int duration, const QPoint &start, const QPoint &end,
                                 const QEasingCurve &easing, QObject *parent)
{
    auto *animation = new QPropertyAnimation(target, "pos", parent);
    animation->setDuration(duration);
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setEasingCurve(easing);
    return animation;
}

QPropertyAnimation *geometryAnimation(QWidget *target, int duration, const QRect &start, const QRect &end,
                                      const QEasingCurve &easing, QObject *parent)
{
    auto *animation = new QPropertyAnimation(target, "geometry", parent);
    animation->setDuration(duration);
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setEasingCurve(easing);
    return animation;
}

} // namespace

OpacityAniStackedWidget::OpacityAniStackedWidget(QWidget *parent) : QStackedWidget(parent) {}

int OpacityAniStackedWidget::addWidget(QWidget *widget) { return QStackedWidget::addWidget(widget); }

void OpacityAniStackedWidget::insertWidget(int index, QWidget *widget) { QStackedWidget::insertWidget(index, widget); }

void OpacityAniStackedWidget::removeWidget(QWidget *widget)
{
    if (widget && widget == m_animatedWidget) {
        finishAnimation();
    }
    QStackedWidget::removeWidget(widget);
}

// The opacity effect lives only for the transition and only on the page being faded: a page's own
// graphics effect would be deleted by setGraphicsEffect(), and OpenGL, video or web views do not
// render under an effect at all. Pages that already have an effect switch without fading.
void OpacityAniStackedWidget::setCurrentIndex(int index)
{
    const int current = currentIndex();
    if (index < 0 || index >= count() || index == current) {
        return;
    }

    finishAnimation();

    // Forward, the next page fades in; backward, the current page fades out over it.
    const bool forward = index > current;
    QWidget *animated = forward ? widget(index) : widget(current);
    if (!animated || animated->graphicsEffect()) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    auto *effect = new QGraphicsOpacityEffect(animated);
    effect->setOpacity(forward ? 0.0 : 1.0);
    animated->setGraphicsEffect(effect);

    m_animatedWidget = animated;
    m_nextIndex = index;
    m_animation = new QPropertyAnimation(effect, "opacity", this);
    m_animation->setDuration(220);
    m_animation->setStartValue(forward ? 0.0 : 1.0);
    m_animation->setEndValue(forward ? 1.0 : 0.0);
    connect(m_animation, &QPropertyAnimation::finished, this, &OpacityAniStackedWidget::onAnimationFinished);

    if (forward) {
        QStackedWidget::setCurrentIndex(index);
    } else if (QWidget *next = widget(index)) {
        // Keep the current page on top while it fades.
        next->show();
        animated->raise();
    }
    m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void OpacityAniStackedWidget::setCurrentWidget(QWidget *widget)
{
    setCurrentIndex(indexOf(widget));
}

void OpacityAniStackedWidget::onAnimationFinished()
{
    if (m_animatedWidget) {
        m_animatedWidget->setGraphicsEffect(nullptr);
    }
    m_animatedWidget = nullptr;
    m_animation = nullptr;
    if (m_nextIndex >= 0 && m_nextIndex < count()) {
        QStackedWidget::setCurrentIndex(m_nextIndex);
    }
    m_nextIndex = -1;
}

// Jumps a running transition to its end.
void OpacityAniStackedWidget::finishAnimation()
{
    if (!m_animation) {
        return;
    }
    QPointer<QPropertyAnimation> animation = m_animation;
    disconnect(animation, nullptr, this, nullptr);
    animation->stop();
    onAnimationFinished();
}

PopUpAniStackedWidget::PopUpAniStackedWidget(QWidget *parent) : QStackedWidget(parent)
{
    connect(this, &QStackedWidget::widgetRemoved, this, &PopUpAniStackedWidget::onWidgetRemoved);
}

PopUpAniStackedWidget::~PopUpAniStackedWidget()
{
    disconnect(this, &QStackedWidget::widgetRemoved, this, &PopUpAniStackedWidget::onWidgetRemoved);
}

int PopUpAniStackedWidget::addWidget(QWidget *widget, int deltaX, int deltaY)
{
    const int index = QStackedWidget::addWidget(widget);
    if (index < 0) {
        return index;
    }

    PopUpAniInfo &info = ensureAnimationInfo(widget);
    info.deltaX = deltaX;
    info.deltaY = deltaY;
    return index;
}

void PopUpAniStackedWidget::removeWidget(QWidget *widget)
{
    // onWidgetRemoved() drops the page's animation data.
    QStackedWidget::removeWidget(widget);
}

bool PopUpAniStackedWidget::isAnimationEnabled() const
{
    return m_animationEnabled;
}

QVector<PopUpAniInfo> PopUpAniStackedWidget::animationInfos() const
{
    QVector<PopUpAniInfo> infos;
    infos.reserve(count());
    for (int i = 0; i < count(); ++i) {
        const int infoIndex = animationInfoIndex(widget(i));
        if (infoIndex >= 0) {
            infos.append(m_animationInfos.at(infoIndex));
        } else {
            PopUpAniInfo info;
            info.widget = widget(i);
            infos.append(info);
        }
    }
    return infos;
}

int PopUpAniStackedWidget::animationInfoIndex(const QWidget *widget) const
{
    for (int i = 0; i < m_animationInfos.size(); ++i) {
        if (m_animationInfos.at(i).widget == widget) {
            return i;
        }
    }
    return -1;
}

PopUpAniInfo &PopUpAniStackedWidget::ensureAnimationInfo(QWidget *widget)
{
    const int infoIndex = animationInfoIndex(widget);
    if (infoIndex >= 0) {
        return m_animationInfos[infoIndex];
    }

    PopUpAniInfo info;
    info.widget = widget;
    info.animation = new QPropertyAnimation(widget, "pos", this);
    m_animationInfos.append(info);
    return m_animationInfos.last();
}

void PopUpAniStackedWidget::onWidgetRemoved(int index)
{
    // The removed page may already be mid-destruction: only compare its pointer.
    for (int i = static_cast<int>(m_animationInfos.size()) - 1; i >= 0; --i) {
        const PopUpAniInfo info = m_animationInfos.at(i);
        if (indexOf(info.widget) >= 0) {
            continue;
        }

        if (m_currentAnimation == info.animation) {
            m_currentAnimation->stop();
            m_currentAnimation = nullptr;
            m_nextIndex = -1;
        }
        delete info.animation;
        m_animationInfos.removeAt(i);
    }

    if (m_nextIndex == index) {
        m_nextIndex = -1;
    } else if (m_nextIndex > index) {
        --m_nextIndex;
    }
}

void PopUpAniStackedWidget::setAnimationEnabled(bool enabled)
{
    m_animationEnabled = enabled;
}

void PopUpAniStackedWidget::setCurrentIndex(int index, bool needPopOut, bool showNextWidgetDirectly, int duration,
                                            QEasingCurve::Type easingCurve)
{
    if (index < 0 || index >= count() || index == currentIndex()) {
        return;
    }

    if (!m_animationEnabled) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    if (m_currentAnimation && m_currentAnimation->state() == QAbstractAnimation::Running) {
        m_currentAnimation->stop();
        onAnimationFinished();
    }

    m_nextIndex = index;
    // Copies: ensureAnimationInfo() may append and invalidate references.
    const PopUpAniInfo nextInfo = ensureAnimationInfo(widget(index));
    const PopUpAniInfo currentInfo = ensureAnimationInfo(currentWidget());
    QWidget *current = currentWidget();
    QWidget *next = nextInfo.widget;

    QPropertyAnimation *animation = needPopOut ? currentInfo.animation : nextInfo.animation;
    m_currentAnimation = animation;

    if (needPopOut) {
        const QPoint endPos = current->pos() + QPoint(currentInfo.deltaX, currentInfo.deltaY);
        setupAnimation(animation, current->pos(), endPos, duration, easingCurve);
        next->setVisible(showNextWidgetDirectly);
    } else {
        const QPoint startPos = next->pos() + QPoint(nextInfo.deltaX, nextInfo.deltaY);
        setupAnimation(animation, startPos, QPoint(next->x(), 0), duration, easingCurve);
        QStackedWidget::setCurrentIndex(index);
    }

    disconnect(animation, nullptr, this, nullptr);
    connect(animation, &QPropertyAnimation::finished, this, &PopUpAniStackedWidget::onAnimationFinished);
    animation->start();
    emit aniStart();
}

void PopUpAniStackedWidget::setCurrentWidget(QWidget *widget, bool needPopOut, bool showNextWidgetDirectly,
                                             int duration, QEasingCurve::Type easingCurve)
{
    setCurrentIndex(indexOf(widget), needPopOut, showNextWidgetDirectly, duration, easingCurve);
}

void PopUpAniStackedWidget::onAnimationFinished()
{
    if (m_currentAnimation) {
        disconnect(m_currentAnimation, nullptr, this, nullptr);
    }
    if (m_nextIndex >= 0 && m_nextIndex < count()) {
        QStackedWidget::setCurrentIndex(m_nextIndex);
    }
    m_currentAnimation = nullptr;
    m_nextIndex = -1;
    emit aniFinished();
}

void PopUpAniStackedWidget::setupAnimation(QPropertyAnimation *animation, const QPoint &startValue,
                                           const QPoint &endValue, int duration, QEasingCurve::Type easingCurve)
{
    animation->setEasingCurve(easingCurve);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setDuration(duration);
}

TransitionStackedWidget::TransitionStackedWidget(QWidget *parent) : QStackedWidget(parent)
{
    m_animationGroup = new QParallelAnimationGroup(this);
    m_currentSnapshot = createSnapshotLabel();
    m_nextSnapshot = createSnapshotLabel();
    connect(m_animationGroup, &QParallelAnimationGroup::finished, this, &TransitionStackedWidget::onAnimationFinished);
}

bool TransitionStackedWidget::isAnimationEnabled() const
{
    return m_animationEnabled;
}

int TransitionStackedWidget::addWidget(QWidget *widget)
{
    if (widget) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }
    return QStackedWidget::addWidget(widget);
}

int TransitionStackedWidget::insertWidget(int index, QWidget *widget)
{
    if (widget) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }
    return QStackedWidget::insertWidget(index, widget);
}

QLabel *TransitionStackedWidget::currentSnapshot() const
{
    return m_currentSnapshot;
}

QLabel *TransitionStackedWidget::nextSnapshot() const
{
    return m_nextSnapshot;
}

void TransitionStackedWidget::setAnimationEnabled(bool enabled)
{
    m_animationEnabled = enabled;
}

void TransitionStackedWidget::setCurrentIndex(int index, int duration, bool isBack)
{
    if (index < 0 || index >= count() || index == currentIndex()) {
        return;
    }

    if (!m_animationEnabled || count() <= 1) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    stopAnimation();
    m_nextIndex = index;
    setupTransitionAnimation(index, duration, isBack);
    m_animationGroup->start();
    emit aniStart();
}

void TransitionStackedWidget::setCurrentWidget(QWidget *widget, int duration, bool isBack)
{
    setCurrentIndex(indexOf(widget), duration, isBack);
}

void TransitionStackedWidget::setupTransitionAnimation(int nextIndex, int /*duration*/, bool /*isBack*/)
{
    QStackedWidget::setCurrentIndex(nextIndex);
}

void TransitionStackedWidget::stopAnimation()
{
    if (m_animationGroup->state() != QAbstractAnimation::Running) {
        return;
    }

    m_animationGroup->stop();
    onAnimationFinished();
}

void TransitionStackedWidget::hideSnapshots()
{
    m_currentSnapshot->hide();
    m_nextSnapshot->hide();
}

QLabel *TransitionStackedWidget::createSnapshotLabel()
{
    auto *label = new QLabel(this);
    label->setAttribute(Qt::WA_TranslucentBackground);
    auto *effect = new QGraphicsOpacityEffect(label);
    effect->setOpacity(1.0);
    label->setGraphicsEffect(effect);
    label->hide();
    return label;
}

void TransitionStackedWidget::renderSnapshot(QWidget *widget, QLabel *label)
{
    if (!widget || !label) {
        return;
    }

    widget->resize(size());
    // A hidden page may still have the style of the previous theme.
    StyleSheetManager::instance()->ensureUpdated(widget);
    QPixmap pixmap = widget->grab();
    if (pixmap.isNull() || pixmap.size().isEmpty()) {
        pixmap = QPixmap(widget->size());
        pixmap.fill(Qt::transparent);
        widget->render(&pixmap);
    }

    label->setPixmap(pixmap);
    label->setGeometry(rect());
    label->show();
    label->raise();
}

QParallelAnimationGroup *TransitionStackedWidget::animationGroup() const
{
    return m_animationGroup;
}

int TransitionStackedWidget::nextIndex() const
{
    return m_nextIndex;
}

void TransitionStackedWidget::onAnimationFinished()
{
    hideSnapshots();
    m_animationGroup->clear();
    if (m_nextIndex >= 0 && m_nextIndex < count()) {
        QStackedWidget::setCurrentIndex(m_nextIndex);
        if (QWidget *w = currentWidget()) {
            w->show();
            w->setGeometry(rect());
        }
    }
    emit aniFinished();
}

EntranceTransitionStackedWidget::EntranceTransitionStackedWidget(QWidget *parent) : TransitionStackedWidget(parent) {}

void EntranceTransitionStackedWidget::setupTransitionAnimation(int nextIndex, int duration, bool isBack)
{
    const int offset = 140;
    const int outDuration = 150;
    const int inDuration = duration > 0 ? duration : 300;
    QWidget *current = currentWidget();
    QWidget *next = widget(nextIndex);

    if (current) {
        renderSnapshot(current, currentSnapshot());
        current->hide();
        animationGroup()->addAnimation(opacityAnimation(currentSnapshot()->graphicsEffect(), outDuration, 1.0, 0.0, this));
        if (isBack) {
            animationGroup()->addAnimation(posAnimation(currentSnapshot(), outDuration, QPoint(0, 0),
                                                       QPoint(0, offset), cubicBezierEasing(0.1, 0.9, 0.2, 1.0), this));
        }
    }

    next->hide();
    auto *nextGroup = new QSequentialAnimationGroup(this);
    QAbstractAnimation *pause = nextGroup->addPause(outDuration);
    connect(pause, &QAbstractAnimation::finished, next, [next]() { next->show(); });
    animationGroup()->addAnimation(nextGroup);

    if (!isBack) {
        next->setGeometry(0, offset, width(), height());
        nextGroup->addAnimation(posAnimation(next, inDuration, QPoint(0, offset), QPoint(0, 0),
                                             cubicBezierEasing(0.7, 0.0, 1.0, 0.5), this));
    } else {
        next->setGeometry(rect());
    }
}

DrillInTransitionStackedWidget::DrillInTransitionStackedWidget(QWidget *parent) : TransitionStackedWidget(parent) {}

void DrillInTransitionStackedWidget::setupTransitionAnimation(int nextIndex, int duration, bool isBack)
{
    const qreal inScale = isBack ? 1.06 : 0.94;
    const qreal outScale = isBack ? 0.96 : 1.04;
    const int inDuration = duration > 0 ? duration : 333;
    const int outDuration = 100;
    const QRect r = rect();
    QWidget *current = currentWidget();
    QWidget *next = widget(nextIndex);

    if (current) {
        renderSnapshot(current, currentSnapshot());
        currentSnapshot()->setScaledContents(true);
        current->hide();

        const int outW = qRound(r.width() * outScale);
        const int outH = qRound(r.height() * outScale);
        const QRect outRect((r.width() - outW) / 2, (r.height() - outH) / 2, outW, outH);
        animationGroup()->addAnimation(geometryAnimation(currentSnapshot(), outDuration, r, outRect,
                                                        cubicBezierEasing(0.7, 0.0, 1.0, 0.5), this));
        animationGroup()->addAnimation(opacityAnimation(currentSnapshot()->graphicsEffect(), outDuration, 1.0, 0.0, this));
    }

    renderSnapshot(next, nextSnapshot());
    nextSnapshot()->setScaledContents(true);
    next->hide();

    const int inW = qRound(r.width() * inScale);
    const int inH = qRound(r.height() * inScale);
    const QRect inRect((r.width() - inW) / 2, (r.height() - inH) / 2, inW, inH);
    nextSnapshot()->setGeometry(inRect);
    animationGroup()->addAnimation(geometryAnimation(nextSnapshot(), inDuration, inRect, r,
                                                    cubicBezierEasing(0.7, 0.0, 1.0, 0.5), this));
    animationGroup()->addAnimation(opacityAnimation(nextSnapshot()->graphicsEffect(), inDuration, 0.0, 1.0, this));
}

} // namespace FluentQt
