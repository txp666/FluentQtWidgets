#include <FluentQtWidgets/Widgets/Flyout.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/AcrylicLabel.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/IconWidget.h>
#include <FluentQtWidgets/Widgets/Label.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QEvent>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QCloseEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

bool isDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}

int maxPopupImageWidth()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    const int screenWidth = screen ? screen->availableGeometry().width() : 1100;
    return qMax(220, qMin(900, screenWidth - 200));
}

// ==========================================
// FlyoutAnimationManager hierarchy
// ==========================================

class FlyoutAnimationManager : public QObject
{
public:
    explicit FlyoutAnimationManager(Flyout *flyout, QObject *parent = nullptr)
        : QObject(parent), m_flyout(flyout)
    {
        m_slideAni = new QPropertyAnimation(flyout, "pos", this);
        m_slideAni->setDuration(187);
        m_slideAni->setEasingCurve(QEasingCurve::OutQuad);

        m_opacityAni = new QPropertyAnimation(flyout, "windowOpacity", this);
        m_opacityAni->setDuration(187);
        m_opacityAni->setEasingCurve(QEasingCurve::OutQuad);
        m_opacityAni->setStartValue(0.0);
        m_opacityAni->setEndValue(1.0);

        m_group = new QParallelAnimationGroup(this);
        m_group->addAnimation(m_slideAni);
        m_group->addAnimation(m_opacityAni);
    }

    virtual QPoint position(QWidget *target) const
    {
        if (!m_flyout || !target) {
            return QPoint();
        }
        const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, 0));
        const QSize targetSize = target->size();
        const QSize flyoutSize = m_flyout->sizeHint();
        return QPoint(globalTopLeft.x() + (targetSize.width() - flyoutSize.width()) / 2,
                      globalTopLeft.y() + targetSize.height() + 8);
    }

    virtual void exec(const QPoint &pos)
    {
        if (!m_flyout) {
            return;
        }
        const QPoint adjusted = adjustedPosition(pos);
        m_slideAni->setStartValue(adjusted - QPoint(0, 8));
        m_slideAni->setEndValue(adjusted);
        m_flyout->setWindowOpacity(0.0);
        m_flyout->move(adjusted - QPoint(0, 8));
        m_group->start();
    }

    QPoint adjustedPosition(const QPoint &pos) const
    {
        if (!m_flyout) {
            return pos;
        }
        QScreen *screen = QGuiApplication::screenAt(pos);
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }
        if (!screen) {
            return pos;
        }
        const QRect available = screen->availableGeometry();
        const int width = m_flyout->sizeHint().width() + 5;
        const int height = m_flyout->sizeHint().height();
        const int x = qBound(available.left(), pos.x(), qMax(available.left(), available.right() - width));
        const int y = qBound(available.top(), pos.y() - 4, qMax(available.top(), available.bottom() - height + 5));
        return QPoint(x, y);
    }

    Flyout *m_flyout = nullptr;
    QPropertyAnimation *m_slideAni = nullptr;
    QPropertyAnimation *m_opacityAni = nullptr;
    QParallelAnimationGroup *m_group = nullptr;
};

class PullUpFlyoutAnimationManager : public FlyoutAnimationManager
{
public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget *target) const override
    {
        if (!m_flyout || !target) {
            return QPoint();
        }
        const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, 0));
        const QSize targetSize = target->size();
        const QSize flyoutSize = m_flyout->sizeHint();
        const int bottomMargin = m_flyout->layout() ? m_flyout->layout()->contentsMargins().bottom() : 20;
        return QPoint(globalTopLeft.x() + targetSize.width() / 2 - flyoutSize.width() / 2,
                      globalTopLeft.y() - flyoutSize.height() + bottomMargin);
    }

    void exec(const QPoint &pos) override
    {
        if (!m_flyout) {
            return;
        }
        const QPoint adjusted = adjustedPosition(pos);
        m_slideAni->setStartValue(adjusted + QPoint(0, 8));
        m_slideAni->setEndValue(adjusted);
        m_flyout->setWindowOpacity(0.0);
        m_flyout->move(adjusted + QPoint(0, 8));
        m_group->start();
    }
};

class DropDownFlyoutAnimationManager : public FlyoutAnimationManager
{
public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget *target) const override
    {
        if (!m_flyout || !target) {
            return QPoint();
        }
        const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, target->height()));
        const QSize targetSize = target->size();
        const QSize flyoutSize = m_flyout->sizeHint();
        const int topMargin = m_flyout->layout() ? m_flyout->layout()->contentsMargins().top() : 8;
        return QPoint(globalTopLeft.x() + targetSize.width() / 2 - flyoutSize.width() / 2,
                      globalTopLeft.y() - topMargin + 8);
    }

    void exec(const QPoint &pos) override
    {
        if (!m_flyout) {
            return;
        }
        const QPoint adjusted = adjustedPosition(pos);
        m_slideAni->setStartValue(adjusted - QPoint(0, 8));
        m_slideAni->setEndValue(adjusted);
        m_flyout->setWindowOpacity(0.0);
        m_flyout->move(adjusted - QPoint(0, 8));
        m_group->start();
    }
};

class SlideLeftFlyoutAnimationManager : public FlyoutAnimationManager
{
public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget *target) const override
    {
        if (!m_flyout || !target) {
            return QPoint();
        }
        const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, 0));
        const QSize targetSize = target->size();
        const QSize flyoutSize = m_flyout->sizeHint();
        const int topMargin = m_flyout->layout() ? m_flyout->layout()->contentsMargins().top() : 0;
        return QPoint(globalTopLeft.x() - flyoutSize.width() + 8,
                      globalTopLeft.y() - flyoutSize.height() / 2 + targetSize.height() / 2 + topMargin);
    }

    void exec(const QPoint &pos) override
    {
        if (!m_flyout) {
            return;
        }
        const QPoint adjusted = adjustedPosition(pos);
        m_slideAni->setStartValue(adjusted + QPoint(8, 0));
        m_slideAni->setEndValue(adjusted);
        m_flyout->setWindowOpacity(0.0);
        m_flyout->move(adjusted + QPoint(8, 0));
        m_group->start();
    }
};

class SlideRightFlyoutAnimationManager : public FlyoutAnimationManager
{
public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget *target) const override
    {
        if (!m_flyout || !target) {
            return QPoint();
        }
        const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, 0));
        const QSize targetSize = target->size();
        const QSize flyoutSize = m_flyout->sizeHint();
        const int topMargin = m_flyout->layout() ? m_flyout->layout()->contentsMargins().top() : 0;
        return QPoint(globalTopLeft.x() + targetSize.width() - 8,
                      globalTopLeft.y() - flyoutSize.height() / 2 + targetSize.height() / 2 + topMargin);
    }

    void exec(const QPoint &pos) override
    {
        if (!m_flyout) {
            return;
        }
        const QPoint adjusted = adjustedPosition(pos);
        m_slideAni->setStartValue(adjusted - QPoint(8, 0));
        m_slideAni->setEndValue(adjusted);
        m_flyout->setWindowOpacity(0.0);
        m_flyout->move(adjusted - QPoint(8, 0));
        m_group->start();
    }
};

class FadeInFlyoutAnimationManager : public PullUpFlyoutAnimationManager
{
public:
    using PullUpFlyoutAnimationManager::PullUpFlyoutAnimationManager;

    void exec(const QPoint &pos) override
    {
        if (!m_flyout) {
            return;
        }
        const QPoint adjusted = adjustedPosition(pos);
        m_flyout->move(adjusted);
        m_group->removeAnimation(m_slideAni);
        m_flyout->setWindowOpacity(0.0);
        m_group->start();
    }
};

class DummyFlyoutAnimationManager : public FlyoutAnimationManager
{
public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    void exec(const QPoint &pos) override
    {
        if (m_flyout) {
            m_flyout->move(adjustedPosition(pos));
            m_flyout->setWindowOpacity(1.0);
        }
    }
};

FlyoutAnimationManager *makeFlyoutAnimationManager(FlyoutAnimationType type, Flyout *flyout)
{
    switch (type) {
    case FlyoutAnimationType::PullUp:
        return new PullUpFlyoutAnimationManager(flyout, flyout);
    case FlyoutAnimationType::DropDown:
        return new DropDownFlyoutAnimationManager(flyout, flyout);
    case FlyoutAnimationType::SlideLeft:
        return new SlideLeftFlyoutAnimationManager(flyout, flyout);
    case FlyoutAnimationType::SlideRight:
        return new SlideRightFlyoutAnimationManager(flyout, flyout);
    case FlyoutAnimationType::FadeIn:
        return new FadeInFlyoutAnimationManager(flyout, flyout);
    case FlyoutAnimationType::None:
    default:
        return new DummyFlyoutAnimationManager(flyout, flyout);
    }
}

void grabAcrylicFlyoutBackground(Flyout *flyout, const QPoint &pos)
{
    if (!flyout || !flyout->view()) {
        return;
    }

    const QSize size = flyout->layout() ? flyout->layout()->sizeHint() : flyout->sizeHint();
    const QRect rect(pos, size);

    if (auto *view = qobject_cast<AcrylicFlyoutView *>(flyout->view())) {
        view->acrylicBrush()->grabImage(rect);
    } else if (auto *viewBase = qobject_cast<AcrylicFlyoutViewBase *>(flyout->view())) {
        viewBase->acrylicBrush()->grabImage(rect);
    }
}

} // namespace

// ==========================================
// FlyoutViewBase
// ==========================================

FlyoutViewBase::FlyoutViewBase(QWidget *parent) : QFrame(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("FlyoutViewBase"));
}

void FlyoutViewBase::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    Q_UNUSED(widget)
    Q_UNUSED(stretch)
    Q_UNUSED(alignment)
}

// ==========================================
// FlyoutView
// ==========================================

FlyoutView::FlyoutView(QWidget *parent) : FlyoutViewBase(parent) {}

FlyoutView::FlyoutView(const QString &title, const QString &content, QWidget *parent)
    : FlyoutViewBase(parent)
{
    initWidgets();
    setTitle(title);
    setContent(content);
    initLayout();
}

FlyoutView::FlyoutView(const QString &title, const QString &content, const QIcon &icon,
                       const QImage &image, bool isClosable, QWidget *parent)
    : FlyoutViewBase(parent)
{
    m_closable = isClosable;
    initWidgets();
    setTitle(title);
    setContent(content);
    setIcon(icon);
    if (!image.isNull()) {
        setImage(image);
    }
    initLayout();
}

FlyoutView::FlyoutView(const QString &title, const QString &content, const QIcon &icon,
                       const QPixmap &image, bool isClosable, QWidget *parent)
    : FlyoutViewBase(parent)
{
    m_closable = isClosable;
    initWidgets();
    setTitle(title);
    setContent(content);
    setIcon(icon);
    if (!image.isNull()) {
        setImage(image);
    }
    initLayout();
}

FlyoutView::FlyoutView(const QString &title, const QString &content, const QIcon &icon,
                       const QString &imagePath, bool isClosable, QWidget *parent)
    : FlyoutViewBase(parent)
{
    m_closable = isClosable;
    initWidgets();
    setTitle(title);
    setContent(content);
    setIcon(icon);
    if (!imagePath.isEmpty()) {
        setImagePath(imagePath);
    }
    initLayout();
}

void FlyoutView::initWidgets()
{
    m_vBoxLayout = new QVBoxLayout(this);
    m_viewLayout = new QHBoxLayout;
    m_widgetLayout = new QVBoxLayout;
    m_bodyLayout = new QVBoxLayout;

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_titleLabel->setVisible(!title().isEmpty());

    m_contentLabel = new QLabel(this);
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setVisible(!content().isEmpty());

    m_iconWidget = new IconWidget(this);
    m_iconWidget->hide();

    m_imageLabel = new ImageLabel(this);
    m_imageLabel->hide();

    m_closeButton = new TransparentToolButton(icon(FluentIcon::Close), this);
    m_closeButton->setFixedSize(32, 32);
    m_closeButton->setIconSize(QSize(12, 12));
    m_closeButton->setVisible(m_closable);
    connect(m_closeButton, &QToolButton::clicked, this, [this]() { emit closed(); });

    FluentStyleSheet::setRole(this, QStringLiteral("FlyoutView"));
}

void FlyoutView::initLayout()
{
    m_vBoxLayout->setContentsMargins(1, 1, 1, 1);
    m_widgetLayout->setContentsMargins(0, 8, 0, 8);
    m_viewLayout->setSpacing(4);
    m_widgetLayout->setSpacing(0);
    m_vBoxLayout->setSpacing(0);

    if (title().isEmpty() && content().isEmpty()) {
        m_iconWidget->setFixedHeight(36);
    }

    m_vBoxLayout->addLayout(m_viewLayout);
    m_viewLayout->addWidget(m_iconWidget, 0, Qt::AlignTop);

    adjustText();
    m_widgetLayout->addWidget(m_titleLabel);
    m_widgetLayout->addWidget(m_contentLabel);
    m_viewLayout->addLayout(m_widgetLayout);

    m_closeButton->setVisible(m_closable);
    m_viewLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);

    // Adjust content margins
    QMargins margins(6, 5, 6, 5);
    margins.setLeft(20);
    if (!m_iconWidget->icon().isNull()) {
        margins.setLeft(5);
    }
    margins.setRight(m_closable ? 6 : 20);
    m_viewLayout->setContentsMargins(margins);

    // Add body layout
    m_widgetLayout->addLayout(m_bodyLayout);

    // Add image
    adjustImage();
    m_imageLabel->setBorderRadius(8, 8, 0, 0);
    m_imageLabel->setHidden(m_imageLabel->isNull());
    m_vBoxLayout->insertWidget(0, m_imageLabel);
}

void FlyoutView::adjustText()
{
    if (m_titleLabel) {
        m_titleLabel->setWordWrap(true);
    }
    if (m_contentLabel) {
        m_contentLabel->setWordWrap(true);
    }
}

void FlyoutView::adjustImage()
{
    if (m_imageLabel->isNull()) {
        return;
    }
    const int w = qMax(1, qMin(m_vBoxLayout->sizeHint().width() - 2, maxPopupImageWidth()));
    m_imageLabel->scaledToWidth(w);
}

void FlyoutView::showEvent(QShowEvent *event)
{
    FlyoutViewBase::showEvent(event);
    adjustImage();
    adjustSize();
}

QString FlyoutView::title() const { return m_titleLabel ? m_titleLabel->text() : QString(); }

QString FlyoutView::content() const { return m_contentLabel ? m_contentLabel->text() : QString(); }

QIcon FlyoutView::viewIcon() const { return m_iconWidget ? m_iconWidget->icon() : QIcon(); }

bool FlyoutView::isClosable() const { return m_closable; }

QLabel *FlyoutView::titleLabel() const { return m_titleLabel; }

QLabel *FlyoutView::contentLabel() const { return m_contentLabel; }

IconWidget *FlyoutView::iconWidget() const { return m_iconWidget; }

ImageLabel *FlyoutView::imageLabel() const { return m_imageLabel; }

QToolButton *FlyoutView::closeButton() const { return m_closeButton; }

void FlyoutView::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
        m_titleLabel->setVisible(!title.isEmpty());
    }
}

void FlyoutView::setContent(const QString &content)
{
    if (m_contentLabel) {
        m_contentLabel->setText(content);
        m_contentLabel->setVisible(!content.isEmpty());
    }
}

void FlyoutView::setIcon(const QIcon &icon)
{
    if (m_iconWidget) {
        m_iconWidget->setIcon(icon);
        m_iconWidget->setHidden(icon.isNull());
    }
}

void FlyoutView::setClosable(bool closable)
{
    m_closable = closable;
    if (m_closeButton) {
        m_closeButton->setVisible(closable);
    }
}

void FlyoutView::setImage(const QPixmap &pixmap) { setImage(pixmap.toImage()); }

void FlyoutView::setImage(const QImage &image)
{
    if (m_imageLabel) {
        m_imageLabel->setImage(image);
        m_imageLabel->setVisible(!m_imageLabel->isNull());
        adjustImage();
        adjustSize();
    }
}

bool FlyoutView::setImagePath(const QString &path)
{
    if (!m_imageLabel || !m_imageLabel->setImagePath(path)) {
        return false;
    }
    m_imageLabel->setVisible(!m_imageLabel->isNull());
    adjustImage();
    adjustSize();
    return true;
}

void FlyoutView::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    if (!widget || !m_widgetLayout) {
        return;
    }

    m_widgetLayout->addSpacing(8);
    m_widgetLayout->addWidget(widget, stretch, alignment);
    adjustSize();
}

QVBoxLayout *FlyoutView::widgetLayout() const { return m_widgetLayout; }

QVBoxLayout *FlyoutView::bodyLayout() const { return m_bodyLayout; }

// ==========================================
// AcrylicFlyoutViewBase
// ==========================================

AcrylicFlyoutViewBase::AcrylicFlyoutViewBase(QWidget *parent)
    : FlyoutViewBase(parent)
    , m_acrylicBrush(new AcrylicBrush(this, 30))
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyoutViewBase"));
}

AcrylicFlyoutViewBase::~AcrylicFlyoutViewBase()
{
    delete m_acrylicBrush;
    m_acrylicBrush = nullptr;
}

AcrylicBrush *AcrylicFlyoutViewBase::acrylicBrush() const { return m_acrylicBrush; }

QPainterPath AcrylicFlyoutViewBase::acrylicClipPath() const
{
    QPainterPath path;
    path.addRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
    return path;
}

void AcrylicFlyoutViewBase::updateAcrylicColor()
{
    if (isDarkTheme()) {
        m_acrylicBrush->setTintColor(QColor(32, 32, 32, 200));
        m_acrylicBrush->setLuminosityColor(QColor(0, 0, 0, 0));
    } else {
        m_acrylicBrush->setTintColor(QColor(255, 255, 255, 180));
        m_acrylicBrush->setLuminosityColor(QColor(255, 255, 255, 0));
    }
}

QColor AcrylicFlyoutViewBase::acrylicBorderColor() const
{
    return isDarkTheme() ? QColor(QStringLiteral("#474747")) : QColor(QStringLiteral("#d8d8d8"));
}

void AcrylicFlyoutViewBase::drawAcrylic(QPainter *painter)
{
    updateAcrylicColor();
    m_acrylicBrush->setClipPath(acrylicClipPath());
    m_acrylicBrush->paint(painter);
}

void AcrylicFlyoutViewBase::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    drawAcrylic(&painter);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(acrylicBorderColor());
    painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
}

// ==========================================
// AcrylicFlyoutView
// ==========================================

AcrylicFlyoutView::AcrylicFlyoutView(QWidget *parent)
    : FlyoutView(parent)
    , m_acrylicBrush(new AcrylicBrush(this, 30))
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyoutView"));
}

AcrylicFlyoutView::AcrylicFlyoutView(const QString &title, const QString &content, QWidget *parent)
    : FlyoutView(title, content, parent)
    , m_acrylicBrush(new AcrylicBrush(this, 30))
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyoutView"));
}

AcrylicFlyoutView::AcrylicFlyoutView(const QString &title, const QString &content, const QIcon &icon,
                                     const QImage &image, bool isClosable, QWidget *parent)
    : FlyoutView(title, content, icon, image, isClosable, parent)
    , m_acrylicBrush(new AcrylicBrush(this, 30))
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyoutView"));
}

AcrylicFlyoutView::AcrylicFlyoutView(const QString &title, const QString &content, const QIcon &icon,
                                     const QPixmap &image, bool isClosable, QWidget *parent)
    : FlyoutView(title, content, icon, image, isClosable, parent)
    , m_acrylicBrush(new AcrylicBrush(this, 30))
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyoutView"));
}

AcrylicFlyoutView::AcrylicFlyoutView(const QString &title, const QString &content, const QIcon &icon,
                                     const QString &imagePath, bool isClosable, QWidget *parent)
    : FlyoutView(title, content, icon, imagePath, isClosable, parent)
    , m_acrylicBrush(new AcrylicBrush(this, 30))
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyoutView"));
}

AcrylicFlyoutView::~AcrylicFlyoutView()
{
    delete m_acrylicBrush;
    m_acrylicBrush = nullptr;
}

AcrylicBrush *AcrylicFlyoutView::acrylicBrush() const { return m_acrylicBrush; }

QPainterPath AcrylicFlyoutView::acrylicClipPath() const
{
    QPainterPath path;
    path.addRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
    return path;
}

void AcrylicFlyoutView::updateAcrylicColor()
{
    if (isDarkTheme()) {
        m_acrylicBrush->setTintColor(QColor(32, 32, 32, 200));
        m_acrylicBrush->setLuminosityColor(QColor(0, 0, 0, 0));
    } else {
        m_acrylicBrush->setTintColor(QColor(255, 255, 255, 180));
        m_acrylicBrush->setLuminosityColor(QColor(255, 255, 255, 0));
    }
}

QColor AcrylicFlyoutView::acrylicBorderColor() const
{
    return isDarkTheme() ? QColor(QStringLiteral("#474747")) : QColor(QStringLiteral("#d8d8d8"));
}

void AcrylicFlyoutView::drawAcrylic(QPainter *painter)
{
    updateAcrylicColor();
    m_acrylicBrush->setClipPath(acrylicClipPath());
    m_acrylicBrush->paint(painter);
}

void AcrylicFlyoutView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    drawAcrylic(&painter);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(acrylicBorderColor());
    painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
}

// ==========================================
// Flyout
// ==========================================

Flyout::Flyout(QWidget *parent) : QFrame(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("Flyout"));
}

Flyout::Flyout(FlyoutViewBase *view, QWidget *parent, bool isDeleteOnClose) : Flyout(parent)
{
    m_isDeleteOnClose = isDeleteOnClose;
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(15, 8, 15, 20);
    m_layout->setSpacing(0);

    m_view = view;
    if (m_view) {
        m_view->setParent(this);
        m_layout->addWidget(m_view);
        connect(m_view, &FlyoutViewBase::closed, this, &Flyout::fadeOut);
    }

    setShadowEffect();
}

void Flyout::setShadowEffect(int blurRadius, const QPoint &offset)
{
    const QColor color = isDarkTheme() ? QColor(0, 0, 0, 80) : QColor(0, 0, 0, 30);
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(blurRadius);
    m_shadowEffect->setOffset(offset);
    m_shadowEffect->setColor(color);
    if (m_view) {
        m_view->setGraphicsEffect(nullptr);
        m_view->setGraphicsEffect(m_shadowEffect);
    }
}

Flyout *Flyout::create(FlyoutViewBase *view, QWidget *target, FlyoutAnimationType animation)
{
    auto *flyout = new Flyout(view);
    flyout->setAnimationType(animation);
    flyout->exec(target);
    return flyout;
}

Flyout *Flyout::create(FlyoutViewBase *view, const QPoint &pos, FlyoutAnimationType animation)
{
    auto *flyout = new Flyout(view);
    flyout->exec(pos, animation);
    return flyout;
}

Flyout *Flyout::make(FlyoutViewBase *view, QWidget *target, QWidget *parent, FlyoutAnimationType animation,
                     bool isDeleteOnClose)
{
    auto *flyout = new Flyout(view, parent, isDeleteOnClose);
    flyout->setAnimationType(animation);
    if (target) {
        flyout->exec(target);
    }
    return flyout;
}

Flyout *Flyout::make(FlyoutViewBase *view, const QPoint &pos, QWidget *parent, FlyoutAnimationType animation,
                     bool isDeleteOnClose)
{
    auto *flyout = new Flyout(view, parent, isDeleteOnClose);
    flyout->exec(pos, animation);
    return flyout;
}

Flyout *Flyout::make(const QString &title, const QString &content, QWidget *target, FlyoutAnimationType animation)
{
    return create(new FlyoutView(title, content), target, animation);
}

Flyout *Flyout::create(const QString &title, const QString &content, const QIcon &icon, const QPixmap &image,
                       bool isClosable, QWidget *target, QWidget *parent, FlyoutAnimationType animation,
                       bool isDeleteOnClose)
{
    auto *view = new FlyoutView(title, content, icon, image, isClosable);
    auto *flyout = make(view, target, parent, animation, isDeleteOnClose);
    return flyout;
}

Flyout *Flyout::create(const QString &title, const QString &content, const QIcon &icon, const QString &imagePath,
                       bool isClosable, QWidget *target, QWidget *parent, FlyoutAnimationType animation,
                       bool isDeleteOnClose)
{
    auto *view = new FlyoutView(title, content, icon, imagePath, isClosable);
    auto *flyout = make(view, target, parent, animation, isDeleteOnClose);
    return flyout;
}

Flyout *Flyout::create(const QString &title, const QString &content, const QIcon &icon, const QPixmap &image,
                       bool isClosable, const QPoint &pos, QWidget *parent, FlyoutAnimationType animation,
                       bool isDeleteOnClose)
{
    auto *view = new FlyoutView(title, content, icon, image, isClosable);
    auto *flyout = make(view, pos, parent, animation, isDeleteOnClose);
    return flyout;
}

Flyout *Flyout::create(const QString &title, const QString &content, const QIcon &icon, const QString &imagePath,
                       bool isClosable, const QPoint &pos, QWidget *parent, FlyoutAnimationType animation,
                       bool isDeleteOnClose)
{
    auto *view = new FlyoutView(title, content, icon, imagePath, isClosable);
    auto *flyout = make(view, pos, parent, animation, isDeleteOnClose);
    return flyout;
}

FlyoutAnimationType Flyout::animationType() const { return m_animationType; }

void Flyout::setAnimationType(FlyoutAnimationType type) { m_animationType = type; }

FlyoutViewBase *Flyout::view() const { return m_view; }

bool Flyout::isDeleteOnClose() const { return m_isDeleteOnClose; }

void Flyout::setIsDeleteOnClose(bool value) { m_isDeleteOnClose = value; }

bool Flyout::isPopupMode() const { return m_popupMode; }

void Flyout::setPopupMode(bool value)
{
    if (m_popupMode == value) {
        return;
    }

    const bool wasVisible = isVisible();
    if (wasVisible) {
        hide();
    }

    m_popupMode = value;
    setWindowFlags((m_popupMode ? Qt::Popup : Qt::Tool) | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating, !m_popupMode);
    setAttribute(Qt::WA_TranslucentBackground);

    if (wasVisible) {
        show();
    }
}

void Flyout::exec(QWidget *target)
{
    if (!target) {
        show();
        return;
    }

    adjustSize();
    FlyoutAnimationManager *manager = makeFlyoutAnimationManager(m_animationType, this);
    const QPoint pos = manager->position(target);
    grabAcrylicFlyoutBackground(this, manager->adjustedPosition(pos));
    show();
    raise();
    manager->exec(pos);
}

void Flyout::exec(const QPoint &pos, FlyoutAnimationType animationType)
{
    setAnimationType(animationType);
    adjustSize();
    FlyoutAnimationManager *manager = makeFlyoutAnimationManager(animationType, this);
    grabAcrylicFlyoutBackground(this, manager->adjustedPosition(pos));
    show();
    raise();
    manager->exec(pos);
}

void Flyout::fadeOut()
{
    auto *anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(120);
    anim->setStartValue(windowOpacity());
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, &Flyout::close);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

bool Flyout::eventFilter(QObject *watched, QEvent *event)
{
    if (isVisible() && event && event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (!rect().contains(mapFromGlobal(mouseEvent->globalPosition().toPoint()))) {
            close();
        }
    }

    return QFrame::eventFilter(watched, event);
}

void Flyout::showEvent(QShowEvent *event)
{
    if (!m_eventFilterInstalled && qApp) {
        qApp->installEventFilter(this);
        m_eventFilterInstalled = true;
    }
    activateWindow();
    QFrame::showEvent(event);
}

void Flyout::closeEvent(QCloseEvent *event)
{
    if (m_eventFilterInstalled && qApp) {
        qApp->removeEventFilter(this);
        m_eventFilterInstalled = false;
    }
    if (m_isDeleteOnClose) {
        deleteLater();
    }
    emit closed();
    QFrame::closeEvent(event);
}

QPoint Flyout::calculatePosition(QWidget *target) const
{
    const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, 0));
    const QSize targetSize = target->size();
    const QSize flyoutSize = sizeHint();
    const int gap = 8;

    return QPoint(globalTopLeft.x() + (targetSize.width() - flyoutSize.width()) / 2,
                  globalTopLeft.y() + targetSize.height() + gap);
}

// ==========================================
// AcrylicFlyout
// ==========================================

AcrylicFlyout::AcrylicFlyout(QWidget *parent) : Flyout(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyout"));
}

AcrylicFlyout::AcrylicFlyout(FlyoutViewBase *view, QWidget *parent, bool isDeleteOnClose)
    : Flyout(view, parent, isDeleteOnClose)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicFlyout"));
}

AcrylicFlyout *AcrylicFlyout::create(FlyoutViewBase *view, QWidget *target, FlyoutAnimationType animation)
{
    auto *flyout = new AcrylicFlyout(view);
    flyout->setAnimationType(animation);
    flyout->exec(target);
    return flyout;
}

AcrylicFlyout *AcrylicFlyout::create(FlyoutViewBase *view, const QPoint &pos, FlyoutAnimationType animation)
{
    auto *flyout = new AcrylicFlyout(view);
    flyout->exec(pos, animation);
    return flyout;
}

AcrylicFlyout *AcrylicFlyout::make(FlyoutViewBase *view, QWidget *target, QWidget *parent,
                                   FlyoutAnimationType animation, bool isDeleteOnClose)
{
    auto *flyout = new AcrylicFlyout(view, parent, isDeleteOnClose);
    flyout->setAnimationType(animation);
    if (target) {
        flyout->exec(target);
    }
    return flyout;
}

AcrylicFlyout *AcrylicFlyout::make(FlyoutViewBase *view, const QPoint &pos, QWidget *parent,
                                   FlyoutAnimationType animation, bool isDeleteOnClose)
{
    auto *flyout = new AcrylicFlyout(view, parent, isDeleteOnClose);
    flyout->exec(pos, animation);
    return flyout;
}

AcrylicFlyout *AcrylicFlyout::create(const QString &title, const QString &content, const QIcon &icon,
                                     const QPixmap &image, bool isClosable, QWidget *target,
                                     QWidget *parent, FlyoutAnimationType animation,
                                     bool isDeleteOnClose)
{
    auto *view = new AcrylicFlyoutView(title, content, icon, image, isClosable);
    return make(view, target, parent, animation, isDeleteOnClose);
}

AcrylicFlyout *AcrylicFlyout::create(const QString &title, const QString &content, const QIcon &icon,
                                     const QString &imagePath, bool isClosable, QWidget *target,
                                     QWidget *parent, FlyoutAnimationType animation,
                                     bool isDeleteOnClose)
{
    auto *view = new AcrylicFlyoutView(title, content, icon, imagePath, isClosable);
    return make(view, target, parent, animation, isDeleteOnClose);
}

AcrylicFlyout *AcrylicFlyout::create(const QString &title, const QString &content, const QIcon &icon,
                                     const QPixmap &image, bool isClosable, const QPoint &pos,
                                     QWidget *parent, FlyoutAnimationType animation,
                                     bool isDeleteOnClose)
{
    auto *view = new AcrylicFlyoutView(title, content, icon, image, isClosable);
    return make(view, pos, parent, animation, isDeleteOnClose);
}

AcrylicFlyout *AcrylicFlyout::create(const QString &title, const QString &content, const QIcon &icon,
                                     const QString &imagePath, bool isClosable, const QPoint &pos,
                                     QWidget *parent, FlyoutAnimationType animation,
                                     bool isDeleteOnClose)
{
    auto *view = new AcrylicFlyoutView(title, content, icon, imagePath, isClosable);
    return make(view, pos, parent, animation, isDeleteOnClose);
}

} // namespace FluentQt
