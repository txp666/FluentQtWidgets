#include <FluentQtWidgets/Dialogs/Dialog.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Label.h>

#include "DialogShadow_p.h"

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QColor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

QColor defaultMaskColor()
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return dark ? QColor(0, 0, 0, 153) : QColor(255, 255, 255, 153);
}

QWidget *dialogOverlayTarget(QWidget *parent)
{
    return parent ? parent->window() : nullptr;
}

} // namespace

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("DialogPopup"));

    setWindowOpacity(0.0);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 20);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setSpacing(12);

    m_view = new QFrame(this);
    m_view->setMinimumWidth(360);
    FluentStyleSheet::setRole(m_view, QStringLiteral("Dialog"));
    auto *shadow = new QGraphicsDropShadowEffect(m_view);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 30));
    m_view->setGraphicsEffect(shadow);
    layout->addWidget(m_view);

    m_vBoxLayout = new QVBoxLayout(m_view);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_vBoxLayout->setSpacing(0);
    m_vBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);

    m_windowTitleLabel = new QLabel(m_view);
    m_windowTitleLabel->setObjectName(QStringLiteral("windowTitleLabel"));
    m_vBoxLayout->addWidget(m_windowTitleLabel, 0, Qt::AlignTop);

    m_textLayout = new QVBoxLayout;
    m_textLayout->setContentsMargins(24, 24, 24, 24);
    m_textLayout->setSpacing(12);
    m_vBoxLayout->addLayout(m_textLayout, 1);

    m_titleLabel = new SubtitleLabel(m_view);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_textLayout->addWidget(m_titleLabel, 0, Qt::AlignTop);

    m_messageLabel = new BodyLabel(m_view);
    m_messageLabel->setObjectName(QStringLiteral("contentLabel"));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    m_textLayout->addWidget(m_messageLabel, 0, Qt::AlignTop);

    m_buttonGroup = new QFrame(m_view);
    m_buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
    m_buttonGroup->setFixedHeight(81);
    m_vBoxLayout->addWidget(m_buttonGroup, 0, Qt::AlignBottom);

    m_buttonLayout = new QHBoxLayout(m_buttonGroup);
    m_buttonLayout->setContentsMargins(24, 24, 24, 24);
    m_buttonLayout->setSpacing(12);

    m_cancelButton = new PushButton(tr("Cancel"), m_buttonGroup);
    m_cancelButton->setObjectName(QStringLiteral("cancelButton"));
    m_acceptButton = new PrimaryPushButton(tr("OK"), m_buttonGroup);
    m_cancelButton->setMinimumWidth(96);
    m_acceptButton->setMinimumWidth(96);
    m_acceptButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_acceptButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_acceptButton->setAutoDefault(false);
    m_acceptButton->setDefault(false);
    m_cancelButton->setAutoDefault(false);
    m_cancelButton->setDefault(false);
    m_acceptButton->setFocus();
    m_buttonLayout->addWidget(m_acceptButton, 1, Qt::AlignVCenter);
    m_buttonLayout->addWidget(m_cancelButton, 1, Qt::AlignVCenter);

    connect(m_acceptButton, &QPushButton::clicked, this, [this]() {
        if (validate()) {
            accept();
            emit yesSignal();
        }
    });
    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        reject();
        emit cancelSignal();
    });
}

void Dialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    installFadeInAnimation();
}

void Dialog::installFadeInAnimation()
{
    setGraphicsEffect(nullptr);
    setWindowOpacity(0.0);
    auto *ani = new QPropertyAnimation(this, "windowOpacity", this);
    ani->setDuration(150);
    ani->setStartValue(0.0);
    ani->setEndValue(1.0);
    ani->setEasingCurve(QEasingCurve::OutCubic);
    connect(ani, &QPropertyAnimation::finished, this, [this]() {
        setWindowOpacity(1.0);
    });
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

Dialog::Dialog(const QString &title, const QString &message, QWidget *parent) : Dialog(parent)
{
    setTitle(title);
    setMessage(message);
}

void Dialog::setTitle(const QString &title)
{
    setWindowTitle(title);
    m_windowTitleLabel->setText(title);
    m_titleLabel->setText(title);
}

void Dialog::setMessage(const QString &message) { m_messageLabel->setText(message); }

void Dialog::setContentCopyable(bool copyable)
{
    m_messageLabel->setTextInteractionFlags(copyable ? Qt::TextSelectableByMouse : Qt::NoTextInteraction);
}

void Dialog::setTitleBarVisible(bool visible)
{
    if (m_windowTitleLabel) {
        m_windowTitleLabel->setVisible(visible);
    }
}

void Dialog::hideYesButton()
{
    if (m_acceptButton) {
        m_acceptButton->hide();
    }
    addButtonStretchOnce();
}

void Dialog::hideCancelButton()
{
    if (m_cancelButton) {
        m_cancelButton->hide();
    }
    addButtonStretchOnce();
}

QFrame *Dialog::view() const { return m_view; }

QFrame *Dialog::widget() const { return m_view; }

QLabel *Dialog::windowTitleLabel() const { return m_windowTitleLabel; }

QLabel *Dialog::titleLabel() const { return m_titleLabel; }

QLabel *Dialog::contentLabel() const { return m_messageLabel; }

QLabel *Dialog::messageLabel() const { return m_messageLabel; }

QFrame *Dialog::buttonGroup() const { return m_buttonGroup; }

QPushButton *Dialog::yesButton() const { return m_acceptButton; }

QPushButton *Dialog::acceptButton() const { return m_acceptButton; }

QPushButton *Dialog::cancelButton() const { return m_cancelButton; }

QVBoxLayout *Dialog::vBoxLayout() const { return m_vBoxLayout; }

QVBoxLayout *Dialog::viewLayout() const { return m_textLayout; }

QVBoxLayout *Dialog::textLayout() const { return m_textLayout; }

QHBoxLayout *Dialog::buttonLayout() const { return m_buttonLayout; }

bool Dialog::validate() const { return true; }

void Dialog::addButtonStretchOnce()
{
    if (m_buttonLayout && !m_buttonStretchInserted) {
        m_buttonLayout->insertStretch(0, 1);
        m_buttonStretchInserted = true;
    }
}

// ============================================================================
// MaskDialogBase
// ============================================================================

MaskDialogBase::MaskDialogBase(QWidget *parent)
    : QDialog(parent)
    , m_maskColor(defaultMaskColor())
{
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_windowMask = new QWidget(this);
    m_windowMask->setObjectName(QStringLiteral("windowMask"));
    m_windowMask->installEventFilter(this);
    updateMaskStyle();

    m_hBoxLayout = new QHBoxLayout(this);
    m_hBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_hBoxLayout->setSpacing(0);

    m_widget = new QFrame(this);
    m_widget->setObjectName(QStringLiteral("centerWidget"));
    m_widget->setMinimumWidth(360);
    m_widget->installEventFilter(this);
    m_hBoxLayout->addWidget(m_widget, 1, Qt::AlignCenter);

    setShadowEffect();
    syncGeometryToParent();

    if (QWidget *target = dialogOverlayTarget(parentWidget())) {
        target->installEventFilter(this);
    }
}

QFrame *MaskDialogBase::widget() const { return m_widget; }

QWidget *MaskDialogBase::windowMask() const { return m_windowMask; }

QHBoxLayout *MaskDialogBase::hBoxLayout() const { return m_hBoxLayout; }

bool MaskDialogBase::isClosableOnMaskClicked() const { return m_closableOnMaskClicked; }

void MaskDialogBase::setClosableOnMaskClicked(bool closable) { m_closableOnMaskClicked = closable; }

bool MaskDialogBase::isDraggable() const { return m_draggable; }

void MaskDialogBase::setDraggable(bool draggable) { m_draggable = draggable; }

QColor MaskDialogBase::maskColor() const { return m_maskColor; }

void MaskDialogBase::setMaskColor(const QColor &color)
{
    m_maskColor = color;
    updateMaskStyle();
}

void MaskDialogBase::setShadowEffect(int blurRadius, const QPoint &offset, const QColor &color)
{
    if (!m_widget) {
        return;
    }
    m_shadowBlurRadius = qMax(0, blurRadius);
    m_shadowOffset = offset;
    m_shadowColor = color;
    m_widget->setGraphicsEffect(nullptr);
    update();
}

void MaskDialogBase::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
    if (!m_widget) {
        return;
    }

    QPainter painter(this);
    Private::paintFluentWindowShadow(&painter, QRectF(m_widget->geometry()), m_shadowBlurRadius,
                                     m_shadowOffset, m_shadowColor);
}

void MaskDialogBase::showEvent(QShowEvent *event)
{
    syncGeometryToParent();
    QDialog::showEvent(event);
    installFadeInAnimation();
}

void MaskDialogBase::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (m_windowMask) {
        m_windowMask->resize(size());
        m_windowMask->lower();
    }
    if (m_widget) {
        m_widget->raise();
    }
}

void MaskDialogBase::done(int result)
{
    if (m_finishing || !isVisible()) {
        QDialog::done(result);
        return;
    }
    installFadeOutAnimation(result);
}

bool MaskDialogBase::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == dialogOverlayTarget(parentWidget())) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            syncGeometryToParent();
        }
    } else if (watched == m_windowMask) {
        if (event->type() == QEvent::MouseButtonRelease && m_closableOnMaskClicked) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                reject();
                return true;
            }
        }
    } else if (watched == m_widget && m_draggable) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton &&
                !m_widget->childrenRegion().contains(mouseEvent->pos())) {
                m_dragPos = mouseEvent->pos();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && !m_dragPos.isNull()) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint pos = m_widget->pos() + mouseEvent->pos() - m_dragPos;
            pos.setX(qBound(0, pos.x(), qMax(0, width() - m_widget->width())));
            pos.setY(qBound(0, pos.y(), qMax(0, height() - m_widget->height())));
            m_widget->move(pos);
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragPos = QPoint();
        }
    }
    return QDialog::eventFilter(watched, event);
}

void MaskDialogBase::syncGeometryToParent()
{
    if (QWidget *target = dialogOverlayTarget(parentWidget())) {
        setGeometry(target->geometry());
    } else if (size().isEmpty()) {
        resize(600, 400);
    }

    if (m_windowMask) {
        m_windowMask->resize(size());
        m_windowMask->lower();
    }
    if (m_widget) {
        m_widget->raise();
    }
}

void MaskDialogBase::setCenterWidgetRole(const QString &role)
{
    if (m_widget) {
        FluentStyleSheet::setRole(m_widget, role);
    }
}

void MaskDialogBase::installFadeInAnimation()
{
    setGraphicsEffect(nullptr);
    setWindowOpacity(0.0);
    auto *ani = new QPropertyAnimation(this, "windowOpacity", this);
    ani->setDuration(200);
    ani->setStartValue(0.0);
    ani->setEndValue(1.0);
    ani->setEasingCurve(QEasingCurve::InSine);
    connect(ani, &QPropertyAnimation::finished, this, [this]() {
        setWindowOpacity(1.0);
    });
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void MaskDialogBase::installFadeOutAnimation(int result)
{
    m_finishing = true;
    if (m_widget) {
        m_widget->setGraphicsEffect(nullptr);
    }

    setGraphicsEffect(nullptr);
    setWindowOpacity(1.0);
    auto *ani = new QPropertyAnimation(this, "windowOpacity", this);
    ani->setDuration(100);
    ani->setStartValue(1.0);
    ani->setEndValue(0.0);
    connect(ani, &QPropertyAnimation::finished, this, [this, result]() {
        setGraphicsEffect(nullptr);
        setWindowOpacity(1.0);
        m_finishing = false;
        QDialog::done(result);
    });
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void MaskDialogBase::updateMaskStyle()
{
    if (!m_windowMask) {
        return;
    }
    m_windowMask->setStyleSheet(QStringLiteral("background: rgba(%1, %2, %3, %4); border: none;")
                                    .arg(m_maskColor.red())
                                    .arg(m_maskColor.green())
                                    .arg(m_maskColor.blue())
                                    .arg(m_maskColor.alpha()));
}

// ============================================================================
// MessageBoxBase
// ============================================================================

MessageBoxBase::MessageBoxBase(QWidget *parent)
    : MaskDialogBase(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("MessageBoxPopup"));
    setCenterWidgetRole(QStringLiteral("MessageBox"));
    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 50));
    setMaskColor(QColor(0, 0, 0, 76));

    m_buttonGroup = new QFrame(m_widget);
    m_buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
    m_buttonGroup->setFixedHeight(81);

    m_acceptButton = new PrimaryPushButton(tr("OK"), m_buttonGroup);
    m_cancelButton = new PushButton(tr("Cancel"), m_buttonGroup);
    m_cancelButton->setObjectName(QStringLiteral("cancelButton"));
    m_acceptButton->setMinimumWidth(96);
    m_cancelButton->setMinimumWidth(96);
    m_acceptButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_acceptButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_acceptButton->setAutoDefault(false);
    m_acceptButton->setDefault(false);
    m_cancelButton->setAutoDefault(false);
    m_cancelButton->setDefault(false);
    m_acceptButton->setFocus();

    m_vBoxLayout = new QVBoxLayout(m_widget);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_vBoxLayout->setSpacing(0);
    m_vBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);

    m_viewLayout = new QVBoxLayout;
    m_viewLayout->setContentsMargins(24, 24, 24, 24);
    m_viewLayout->setSpacing(12);
    m_vBoxLayout->addLayout(m_viewLayout, 1);
    m_vBoxLayout->addWidget(m_buttonGroup, 0, Qt::AlignBottom);

    m_buttonLayout = new QHBoxLayout(m_buttonGroup);
    m_buttonLayout->setContentsMargins(24, 24, 24, 24);
    m_buttonLayout->setSpacing(12);
    m_buttonLayout->addWidget(m_acceptButton, 1, Qt::AlignVCenter);
    m_buttonLayout->addWidget(m_cancelButton, 1, Qt::AlignVCenter);

    connect(m_acceptButton, &QPushButton::clicked, this, [this]() {
        if (validate()) {
            accept();
            emit yesSignal();
        }
    });
    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        reject();
        emit cancelSignal();
    });
}

QFrame *MessageBoxBase::view() const { return m_widget; }

QFrame *MessageBoxBase::buttonGroup() const { return m_buttonGroup; }

QPushButton *MessageBoxBase::yesButton() const { return m_acceptButton; }

QPushButton *MessageBoxBase::acceptButton() const { return m_acceptButton; }

QPushButton *MessageBoxBase::cancelButton() const { return m_cancelButton; }

QVBoxLayout *MessageBoxBase::vBoxLayout() const { return m_vBoxLayout; }

QVBoxLayout *MessageBoxBase::viewLayout() const { return m_viewLayout; }

QHBoxLayout *MessageBoxBase::buttonLayout() const { return m_buttonLayout; }

void MessageBoxBase::hideYesButton()
{
    if (m_acceptButton) {
        m_acceptButton->hide();
    }
    addButtonStretchOnce();
}

void MessageBoxBase::hideCancelButton()
{
    if (m_cancelButton) {
        m_cancelButton->hide();
    }
    addButtonStretchOnce();
}

bool MessageBoxBase::validate() const { return true; }

void MessageBoxBase::addButtonStretchOnce()
{
    if (m_buttonLayout && !m_buttonStretchInserted) {
        m_buttonLayout->insertStretch(0, 1);
        m_buttonStretchInserted = true;
    }
}

// ============================================================================
// MessageBox
// ============================================================================

MessageBox::MessageBox(const QString &title, const QString &message, QWidget *parent)
    : MessageBoxBase(parent)
    , m_content(message)
{
    FluentStyleSheet::setRole(this, QStringLiteral("MessageBoxPopup"));
    setCenterWidgetRole(QStringLiteral("MessageBox"));

    m_titleLabel = new SubtitleLabel(m_widget);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_viewLayout->insertWidget(0, m_titleLabel, 0, Qt::AlignTop);

    m_messageLabel = new BodyLabel(m_widget);
    m_messageLabel->setObjectName(QStringLiteral("contentLabel"));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    m_viewLayout->insertWidget(1, m_messageLabel, 0, Qt::AlignTop);

    m_buttonGroup->setMinimumWidth(280);
    setTitle(title);
    setMessage(message);
    adjustMessageSize();
}

void MessageBox::setTitle(const QString &title)
{
    setWindowTitle(title);
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
    adjustMessageSize();
}

void MessageBox::setMessage(const QString &message)
{
    m_content = message;
    if (m_messageLabel) {
        m_messageLabel->setText(message);
    }
    adjustMessageSize();
}

void MessageBox::setContentCopyable(bool copyable)
{
    if (m_messageLabel) {
        m_messageLabel->setTextInteractionFlags(copyable ? Qt::TextSelectableByMouse : Qt::NoTextInteraction);
    }
}

QLabel *MessageBox::titleLabel() const { return m_titleLabel; }

QLabel *MessageBox::contentLabel() const { return m_messageLabel; }

QLabel *MessageBox::messageLabel() const { return m_messageLabel; }

bool MessageBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == dialogOverlayTarget(parentWidget()) && event->type() == QEvent::Resize) {
        adjustMessageSize();
    }
    return MessageBoxBase::eventFilter(watched, event);
}

void MessageBox::adjustMessageSize()
{
    if (!m_widget || !m_titleLabel || !m_messageLabel) {
        return;
    }

    m_titleLabel->adjustSize();
    m_messageLabel->adjustSize();
    const int contentWidth = qMax(m_titleLabel->sizeHint().width(), m_messageLabel->sizeHint().width());
    const int width = qBound(280, contentWidth + 48, 640);
    m_widget->setMinimumWidth(width);
    m_widget->adjustSize();
}

MessageDialog::MessageDialog(const QString &title, const QString &message, QWidget *parent)
    : MessageBox(title, message, parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("MessageDialog"));
    setCenterWidgetRole(QStringLiteral("MessageDialog"));
}

} // namespace FluentQt
