#include <FluentQtWidgets/Widgets/StateToolTip.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>

namespace FluentQt {

// --- StateCloseButton ---

StateCloseButton::StateCloseButton(QWidget *parent)
    : QToolButton(parent)
{
    setFixedSize(12, 12);
}

bool StateCloseButton::isPressed() const { return m_isPressed; }

bool StateCloseButton::isEnter() const { return m_isEnter; }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void StateCloseButton::enterEvent(QEnterEvent * /*event*/)
#else
void StateCloseButton::enterEvent(QEvent * /*event*/)
#endif
{
    m_isEnter = true;
    update();
}

void StateCloseButton::leaveEvent(QEvent * /*event*/)
{
    m_isEnter = false;
    m_isPressed = false;
    update();
}

void StateCloseButton::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    update();
    QToolButton::mousePressEvent(event);
}

void StateCloseButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    update();
    QToolButton::mouseReleaseEvent(event);
}

void StateCloseButton::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (m_isPressed) {
        painter.setOpacity(0.6);
    } else if (m_isEnter) {
        painter.setOpacity(0.8);
    }

    FluentQt::icon(FluentIcon::Close).paint(&painter, rect());
}

// --- StateToolTip ---

StateToolTip::StateToolTip(const QString &title, const QString &content, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_animation = new QPropertyAnimation(m_opacityEffect, "opacity", this);

    m_titleLabel = new QLabel(title, this);
    m_contentLabel = new QLabel(content, this);
    m_closeButton = new StateCloseButton(this);

    initWidget();
    applyQss();
    initLayout();

    m_rotateTimer.start();
}

StateToolTip *StateToolTip::showIn(QWidget *parent, const QString &title, const QString &content)
{
    auto *toolTip = new StateToolTip(title, content, parent);
    if (parent) {
        toolTip->move(toolTip->suitablePosition(parent));
    }
    toolTip->show();
    toolTip->raise();
    return toolTip;
}

QString StateToolTip::title() const
{
    return m_titleLabel->text();
}

QString StateToolTip::content() const
{
    return m_contentLabel->text();
}

bool StateToolTip::isDone() const
{
    return m_done;
}

int StateToolTip::rotateAngle() const { return m_rotateAngle; }

int StateToolTip::deltaAngle() const { return m_deltaAngle; }

QLabel *StateToolTip::titleLabel() const { return m_titleLabel; }

QLabel *StateToolTip::contentLabel() const { return m_contentLabel; }

StateCloseButton *StateToolTip::closeButton() const { return m_closeButton; }

QPoint StateToolTip::suitablePosition(QWidget *parent) const
{
    if (!parent) {
        return QPoint(0, 0);
    }

    const int margin = 24;
    const int spacing = 16;
    const int startY = 50;

    const auto siblings = parent->findChildren<StateToolTip *>(QString(), Qt::FindDirectChildrenOnly);
    for (int i = 0; i < 10; ++i) {
        const int dy = i * (height() + spacing);
        const QPoint pos(parent->width() - width() - margin, startY + dy);
        const QPoint checkPt = pos + QPoint(2, 2);

        bool collision = false;
        for (StateToolTip *sibling : siblings) {
            if (sibling == this || sibling->isHidden()) {
                continue;
            }
            if (sibling->geometry().contains(checkPt)) {
                collision = true;
                break;
            }
        }

        if (!collision) {
            return pos;
        }
    }

    return QPoint(parent->width() - width() - margin, startY);
}

void StateToolTip::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
    m_titleLabel->adjustSize();
    initLayout();
}

void StateToolTip::setContent(const QString &content)
{
    m_contentLabel->setText(content);
    m_contentLabel->adjustSize();
    initLayout();
}

void StateToolTip::setState(bool done)
{
    setDone(done);
}

void StateToolTip::setDone(bool done)
{
    if (m_done == done) {
        return;
    }

    m_done = done;
    update();
    emit doneChanged(m_done);

    if (m_done) {
        QTimer::singleShot(1000, this, &StateToolTip::fadeOut);
    } else {
        m_rotateTimer.start();
    }
}

void StateToolTip::setRotateAngle(int angle)
{
    m_rotateAngle = ((angle % 360) + 360) % 360;
    update();
}

void StateToolTip::setDeltaAngle(int angle) { m_deltaAngle = angle; }

void StateToolTip::initWidget()
{
    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(1.0);

    m_rotateTimer.setInterval(50);

    m_contentLabel->setMinimumWidth(200);

    connect(m_closeButton, &QToolButton::clicked, this, [this]() {
        emit closedSignal();
        hide();
    });

    connect(&m_rotateTimer, &QTimer::timeout, this, &StateToolTip::rotateTick);
}

void StateToolTip::initLayout()
{
    setFixedSize(qMax(m_titleLabel->width(), m_contentLabel->width()) + 56, 51);

    m_titleLabel->move(32, 9);
    m_contentLabel->move(12, 27);
    m_closeButton->move(width() - 24, 19);
}

void StateToolTip::applyQss()
{
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));

    FluentStyleSheet::setRole(this, QStringLiteral("StateToolTip"));

    m_titleLabel->adjustSize();
    m_contentLabel->adjustSize();
}

void StateToolTip::fadeOut()
{
    m_rotateTimer.stop();
    m_animation->setDuration(200);
    m_animation->setStartValue(1.0);
    m_animation->setEndValue(0.0);
    connect(m_animation, &QPropertyAnimation::finished, this, &StateToolTip::deleteLater);
    m_animation->start();
}

void StateToolTip::rotateTick()
{
    setRotateAngle(m_rotateAngle + m_deltaAngle);
}

void StateToolTip::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (!m_done) {
        painter.save();
        painter.translate(19, 18);
        painter.rotate(m_rotateAngle);
        const QRectF targetRect(-8, -8, 16, 16);
        icon(FluentIcon::Sync).paint(&painter, targetRect.toRect());
        painter.restore();
    } else {
        const QRectF targetRect(11, 10, 16, 16);
        icon(FluentIcon::Completed).paint(&painter, targetRect.toRect());
    }
}

} // namespace FluentQt
