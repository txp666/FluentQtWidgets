#include <FluentQtWidgets/Dialogs/Dialog.h>

#include "../core/TextWrap_p.h"

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Label.h>

#include "DialogShadow_p.h"

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QStringList>
#include <QtGui/QColor>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
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

// As in qfluentwidgets, a dialog with a parent is a child overlay of the parent's window, so its
// mask and centre widget always line up with the window and stay inside it.
bool overlaysAsChild(QWidget *target) { return target != nullptr; }

// qfluentwidgets resizes its Dialog to 240 x 192; the layout's minimum size only ever grows it.
constexpr QSize kDialogMinimumSize(240, 192);

int messageWrapCharacterCount(QWidget *dialog, QLabel *titleLabel)
{
    if (!dialog) {
        return 100;
    }

    if (dialog->isWindow()) {
        if (QWidget *parent = dialog->parentWidget()) {
            const int width = qMax(titleLabel ? titleLabel->width() : 0, parent->width());
            return qMax(qMin(width / 9, 140), 30);
        }
        return 100;
    }

    const int width = qMax(titleLabel ? titleLabel->width() : 0, dialog->window()->width());
    return qMax(qMin(width / 9, 100), 30);
}

} // namespace

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("DialogPopup"));

    setWindowOpacity(0.0);

    // The view stands in for qfluentwidgets' frameless window. A native window gets its shadow and
    // frame from the system; here the margins around the view hold the whole of the view's shadow
    // and the 1 px frame paintEvent() draws, so neither is cropped.
    m_view = new QFrame(this);
    FluentStyleSheet::setRole(m_view, QStringLiteral("Dialog"));
    auto *shadow = new QGraphicsDropShadowEffect(m_view);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 30));
    m_view->setGraphicsEffect(shadow);

    const QRect viewRect(0, 0, 100, 100);
    const QRect shadowRect = shadow->boundingRectFor(viewRect).toAlignedRect().adjusted(-1, -1, 1, 1);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(viewRect.left() - shadowRect.left(), viewRect.top() - shadowRect.top(),
                               shadowRect.right() - viewRect.right(), shadowRect.bottom() - viewRect.bottom());
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setSpacing(0);
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

    m_titleLabel = new QLabel(m_view);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_textLayout->addWidget(m_titleLabel, 0, Qt::AlignTop);

    m_messageLabel = new BodyLabel(m_view);
    m_messageLabel->setObjectName(QStringLiteral("contentLabel"));
    m_messageLabel->setWordWrap(false);
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
    m_acceptButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_acceptButton->setAttribute(Qt::WA_MacShowFocusRect, false);
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

void Dialog::adjustDialogSize()
{
    if (!layout() || !m_view) {
        return;
    }
    // Activating the layouts makes the view's minimum size that of its content, like the
    // SetMinimumSize constraint does for qfluentwidgets' window.
    m_vBoxLayout->activate();
    layout()->activate();
    const QMargins margins = layout()->contentsMargins();
    const QSize viewSize = m_view->minimumSize().expandedTo(kDialogMinimumSize);
    resize(viewSize.grownBy(margins).expandedTo(minimumSize()));
}

void Dialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
    if (!m_view || !m_view->isVisible()) {
        return;
    }

    QPainter painter(this);
    const QRectF viewRect(m_view->geometry());
    // The window frame sits just outside the view, as a native frame does, so the view keeps
    // qfluentwidgets' window size and its children their offsets.
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(dark ? QColor(255, 255, 255, 26) : QColor(0, 0, 0, 36), 1));
    painter.drawRoundedRect(viewRect.adjusted(-0.5, -0.5, 0.5, 0.5), 7.5, 7.5);
}

void Dialog::showEvent(QShowEvent *event)
{
    adjustDialogSize();
    if (QWidget *parentWindow = parentWidget() ? parentWidget()->window() : nullptr) {
        const QPoint targetCenter = parentWindow->frameGeometry().center();
        const QPoint dialogCenter = m_view ? m_view->geometry().center() : rect().center();
        move(targetCenter - dialogCenter);
    }

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

void Dialog::setMessage(const QString &message)
{
    m_messageLabel->setText(Private::wrapText(message, messageWrapCharacterCount(this, m_titleLabel)));
}

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
    if (QWidget *target = dialogOverlayTarget(parentWidget()); overlaysAsChild(target)) {
        setParent(target, Qt::Widget | Qt::FramelessWindowHint);
    } else {
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    }
    setAttribute(Qt::WA_TranslucentBackground);

    m_windowMask = new QWidget(this);
    m_windowMask->setObjectName(QStringLiteral("windowMask"));
    m_windowMask->installEventFilter(this);
    updateMaskStyle();

    // The layout keeps the style's margins, as qfluentwidgets' does: a centre widget that only
    // has a maximum size (ColorDialog) shrinks to the window less those margins.
    m_hBoxLayout = new QHBoxLayout(this);

    m_widget = new QFrame(this);
    m_widget->setObjectName(QStringLiteral("centerWidget"));
    m_widget->installEventFilter(this);
    m_hBoxLayout->addWidget(m_widget);

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
    // The first accept()/reject() decides; later ones during its fade-out (a second click,
    // Escape, a mask click) would otherwise finish the dialog again with a different result.
    if (m_finishing) {
        return;
    }
    if (!isVisible()) {
        QDialog::done(result);
        return;
    }
    installFadeOutAnimation(result);
}

bool MaskDialogBase::isFinishing() const { return m_finishing; }

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
    if (QWidget *target = dialogOverlayTarget(parentWidget()); target && target != this) {
        // A child overlay is placed in its parent's coordinates.
        setGeometry(isWindow() ? target->geometry() : target->rect());
        if (!isWindow()) {
            raise();
        }
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
    setWindowOpacity(1.0);
    setGraphicsEffect(nullptr);

    auto *opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    setGraphicsEffect(opacityEffect);

    auto *ani = new QPropertyAnimation(opacityEffect, "opacity", this);
    ani->setDuration(200);
    ani->setStartValue(0.0);
    ani->setEndValue(1.0);
    ani->setEasingCurve(QEasingCurve::InSine);
    connect(ani, &QPropertyAnimation::finished, this, [this, opacityEffect]() {
        if (graphicsEffect() == opacityEffect) {
            setGraphicsEffect(nullptr);
        }
    });
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void MaskDialogBase::installFadeOutAnimation(int result)
{
    m_finishing = true;
    if (m_widget) {
        m_widget->setGraphicsEffect(nullptr);
    }

    setWindowOpacity(1.0);
    setGraphicsEffect(nullptr);

    auto *opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(1.0);
    setGraphicsEffect(opacityEffect);

    auto *ani = new QPropertyAnimation(opacityEffect, "opacity", this);
    ani->setDuration(100);
    ani->setStartValue(1.0);
    ani->setEndValue(0.0);
    connect(ani, &QPropertyAnimation::finished, this, [this, result, opacityEffect]() {
        if (graphicsEffect() == opacityEffect) {
            setGraphicsEffect(nullptr);
        }
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
    // Only the background, as Python's setMaskColor(): a dialog's own sheet may still give the mask a
    // border, as folder_list_dialog.qss's QWidget rule does in Python.
    m_windowMask->setStyleSheet(QStringLiteral("background: rgba(%1, %2, %3, %4);")
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
    m_acceptButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_acceptButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_acceptButton->setFocus();

    // The centre widget takes its content's size, centred in the mask; a minimum size set on
    // widget() holds, as in qfluentwidgets (no SetMinimumSize constraint overriding it).
    m_hBoxLayout->removeWidget(m_widget);
    m_hBoxLayout->addWidget(m_widget, 1, Qt::AlignCenter);

    m_vBoxLayout = new QVBoxLayout(m_widget);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_vBoxLayout->setSpacing(0);

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

    // The buttons stay clickable during the fade-out; only the first click counts.
    connect(m_acceptButton, &QPushButton::clicked, this, [this]() {
        if (!isFinishing() && validate()) {
            accept();
            emit yesSignal();
        }
    });
    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        if (!isFinishing()) {
            reject();
            emit cancelSignal();
        }
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

    m_titleLabel = new QLabel(m_widget);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_viewLayout->insertWidget(0, m_titleLabel, 0, Qt::AlignTop);

    m_messageLabel = new BodyLabel(m_widget);
    m_messageLabel->setObjectName(QStringLiteral("contentLabel"));
    m_messageLabel->setWordWrap(false);
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

void MessageBox::showEvent(QShowEvent *event)
{
    // Callers may add buttons after construction; size the box for them before it is centred.
    adjustMessageSize();
    MessageBoxBase::showEvent(event);
}

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

    m_messageLabel->setText(Private::wrapText(m_content, messageWrapCharacterCount(this, m_titleLabel)));
    // The style sheet sets the labels' fonts and the frame's border, so polish before measuring.
    // The box is then the natural size of its layout, frame included: qfluentwidgets measures the
    // laid-out labels (contentLabel.y() + height + 105), which comes to the same. Buttons added to
    // buttonLayout() widen it instead of overlapping.
    m_widget->ensurePolished();
    m_titleLabel->ensurePolished();
    m_messageLabel->ensurePolished();
    m_vBoxLayout->invalidate();
    m_widget->setFixedSize(m_widget->minimumSizeHint());
}

MessageDialog::MessageDialog(const QString &title, const QString &content, QWidget *parent) : MaskDialogBase(parent)
{
    setWindowTitle(title);
    FluentStyleSheet::setRole(this, QStringLiteral("MessageDialog"));
    setCenterWidgetRole(QStringLiteral("MessageDialog"));

    m_titleLabel = new QLabel(title, m_widget);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_contentLabel = new QLabel(m_widget);
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));
    m_contentLabel->setText(Private::wrapText(content, 71));
    m_yesButton = new QPushButton(tr("OK"), m_widget);
    m_cancelButton = new QPushButton(tr("Cancel"), m_widget);
    m_windowMask->setObjectName(QStringLiteral("windowMask"));

    m_widget->setMaximumWidth(540);
    m_titleLabel->move(24, 24);
    m_contentLabel->move(24, 56);
    adjustLayout();

    connect(m_yesButton, &QPushButton::clicked, this, [this]() {
        if (!isFinishing()) {
            emit yesSignal();
            accept();
        }
    });
    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        if (!isFinishing()) {
            emit cancelSignal();
            reject();
        }
    });
}

QLabel *MessageDialog::titleLabel() const { return m_titleLabel; }

QLabel *MessageDialog::contentLabel() const { return m_contentLabel; }

QPushButton *MessageDialog::yesButton() const { return m_yesButton; }

QPushButton *MessageDialog::cancelButton() const { return m_cancelButton; }

void MessageDialog::adjustLayout()
{
    // The style sheet sets the label fonts, so polish before measuring.
    m_widget->ensurePolished();
    m_titleLabel->ensurePolished();
    m_contentLabel->ensurePolished();
    m_titleLabel->adjustSize();
    m_contentLabel->adjustSize();
    m_widget->setFixedSize(48 + m_contentLabel->width(), m_contentLabel->y() + m_contentLabel->height() + 92);
    m_yesButton->resize((m_widget->width() - 54) / 2, 32);
    m_cancelButton->resize(m_yesButton->width(), 32);
    m_yesButton->move(24, m_widget->height() - 56);
    m_cancelButton->move(m_widget->width() - 24 - m_cancelButton->width(), m_widget->height() - 56);
}

} // namespace FluentQt
