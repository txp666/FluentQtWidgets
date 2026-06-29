#include <FluentQtWidgets/Widgets/StackedWidget.h>

#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPauseAnimation>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSequentialAnimationGroup>
#include <QtGui/QPixmap>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>

namespace FluentQt {

namespace {

QGraphicsOpacityEffect *opacityEffect(QWidget *widget)
{
    return widget ? qobject_cast<QGraphicsOpacityEffect *>(widget->graphicsEffect()) : nullptr;
}

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

int OpacityAniStackedWidget::addWidget(QWidget *widget)
{
    const int index = QStackedWidget::addWidget(widget);
    ensureAnimationForWidget(widget, index);
    return index;
}

void OpacityAniStackedWidget::insertWidget(int index, QWidget *widget)
{
    const int insertedIndex = QStackedWidget::insertWidget(index, widget);
    ensureAnimationForWidget(widget, insertedIndex);
}

void OpacityAniStackedWidget::removeWidget(QWidget *widget)
{
    const int index = indexOf(widget);
    if (index >= 0 && index < m_animations.size()) {
        delete m_animations.takeAt(index);
        m_effects.removeAt(index);
    }
    QStackedWidget::removeWidget(widget);
}

void OpacityAniStackedWidget::setCurrentIndex(int index)
{
    const int current = currentIndex();
    if (index < 0 || index >= count() || index == current) {
        return;
    }

    QPropertyAnimation *animation = nullptr;
    if (index > current) {
        animation = m_animations.value(index);
        if (auto *effect = m_effects.value(index, nullptr)) {
            effect->setOpacity(0.0);
        }
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        QStackedWidget::setCurrentIndex(index);
    } else {
        animation = m_animations.value(current);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
    }

    if (!animation) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    if (QWidget *oldWidget = widget(current)) {
        oldWidget->show();
    }
    m_nextIndex = index;
    animation->stop();
    animation->start();
}

void OpacityAniStackedWidget::setCurrentWidget(QWidget *widget)
{
    setCurrentIndex(indexOf(widget));
}

void OpacityAniStackedWidget::onAnimationFinished()
{
    QStackedWidget::setCurrentIndex(m_nextIndex);
}

void OpacityAniStackedWidget::ensureAnimationForWidget(QWidget *widget, int index)
{
    if (!widget || index < 0) {
        return;
    }

    auto *effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(1.0);
    widget->setGraphicsEffect(effect);

    auto *animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(220);
    connect(animation, &QPropertyAnimation::finished, this, &OpacityAniStackedWidget::onAnimationFinished);

    m_effects.insert(index, effect);
    m_animations.insert(index, animation);
}

PopUpAniStackedWidget::PopUpAniStackedWidget(QWidget *parent) : QStackedWidget(parent) {}

int PopUpAniStackedWidget::addWidget(QWidget *widget, int deltaX, int deltaY)
{
    const int index = QStackedWidget::addWidget(widget);
    PopUpAniInfo info;
    info.widget = widget;
    info.deltaX = deltaX;
    info.deltaY = deltaY;
    info.animation = new QPropertyAnimation(widget, "pos", this);
    m_animationInfos.insert(index, info);
    return index;
}

void PopUpAniStackedWidget::removeWidget(QWidget *widget)
{
    const int index = indexOf(widget);
    if (index >= 0 && index < m_animationInfos.size()) {
        delete m_animationInfos.at(index).animation;
        m_animationInfos.removeAt(index);
    }
    QStackedWidget::removeWidget(widget);
}

bool PopUpAniStackedWidget::isAnimationEnabled() const
{
    return m_animationEnabled;
}

QVector<PopUpAniInfo> PopUpAniStackedWidget::animationInfos() const
{
    return m_animationInfos;
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
    PopUpAniInfo nextInfo = m_animationInfos.at(index);
    PopUpAniInfo currentInfo = m_animationInfos.at(currentIndex());
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
    QStackedWidget::setCurrentIndex(m_nextIndex);
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
