#include <FluentQtWidgets/Widgets/TeachingTip.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Label.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

QString tailPositionName(TeachingTipTailPosition position)
{
    switch (position) {
    case TeachingTipTailPosition::Top:
        return QStringLiteral("Top");
    case TeachingTipTailPosition::Bottom:
        return QStringLiteral("Bottom");
    case TeachingTipTailPosition::Left:
        return QStringLiteral("Left");
    case TeachingTipTailPosition::Right:
        return QStringLiteral("Right");
    case TeachingTipTailPosition::TopLeft:
        return QStringLiteral("TopLeft");
    case TeachingTipTailPosition::TopRight:
        return QStringLiteral("TopRight");
    case TeachingTipTailPosition::BottomLeft:
        return QStringLiteral("BottomLeft");
    case TeachingTipTailPosition::BottomRight:
        return QStringLiteral("BottomRight");
    case TeachingTipTailPosition::LeftTop:
        return QStringLiteral("LeftTop");
    case TeachingTipTailPosition::LeftBottom:
        return QStringLiteral("LeftBottom");
    case TeachingTipTailPosition::RightTop:
        return QStringLiteral("RightTop");
    case TeachingTipTailPosition::RightBottom:
        return QStringLiteral("RightBottom");
    case TeachingTipTailPosition::None:
        return QStringLiteral("None");
    }
    return QStringLiteral("Bottom");
}

bool isDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}

int maxTipImageWidth()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    const int screenWidth = screen ? screen->availableGeometry().width() : 1100;
    return qMax(220, qMin(900, screenWidth - 200));
}

int maxTipImageHeight()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    const int screenHeight = screen ? screen->availableGeometry().height() : 700;
    return qMax(120, qMin(520, screenHeight - 200));
}

QColor bubbleBackgroundColor()
{
    return isDarkTheme() ? QColor(40, 40, 40) : QColor(248, 248, 248);
}

QColor bubbleBorderColor()
{
    return isDarkTheme() ? QColor(23, 23, 23) : QColor(0, 0, 0, 17);
}

QPainterPath roundedTailPath(const QRectF &rect, TeachingTipTailPosition pos, const QPolygonF &tail)
{
    constexpr qreal radius = 8.0;
    const qreal left = rect.left();
    const qreal top = rect.top();
    const qreal right = rect.right();
    const qreal bottom = rect.bottom();
    QPainterPath path;

    switch (pos) {
    case TeachingTipTailPosition::Top:
    case TeachingTipTailPosition::TopLeft:
    case TeachingTipTailPosition::TopRight:
        path.moveTo(left + radius, top);
        path.lineTo(tail.at(0));
        path.lineTo(tail.at(1));
        path.lineTo(tail.at(2));
        path.lineTo(right - radius, top);
        path.quadTo(right, top, right, top + radius);
        path.lineTo(right, bottom - radius);
        path.quadTo(right, bottom, right - radius, bottom);
        path.lineTo(left + radius, bottom);
        path.quadTo(left, bottom, left, bottom - radius);
        path.lineTo(left, top + radius);
        path.quadTo(left, top, left + radius, top);
        break;
    case TeachingTipTailPosition::Bottom:
    case TeachingTipTailPosition::BottomLeft:
    case TeachingTipTailPosition::BottomRight:
        path.moveTo(left + radius, top);
        path.lineTo(right - radius, top);
        path.quadTo(right, top, right, top + radius);
        path.lineTo(right, bottom - radius);
        path.quadTo(right, bottom, right - radius, bottom);
        path.lineTo(tail.at(2));
        path.lineTo(tail.at(1));
        path.lineTo(tail.at(0));
        path.lineTo(left + radius, bottom);
        path.quadTo(left, bottom, left, bottom - radius);
        path.lineTo(left, top + radius);
        path.quadTo(left, top, left + radius, top);
        break;
    case TeachingTipTailPosition::Left:
    case TeachingTipTailPosition::LeftTop:
    case TeachingTipTailPosition::LeftBottom:
        path.moveTo(left + radius, top);
        path.lineTo(right - radius, top);
        path.quadTo(right, top, right, top + radius);
        path.lineTo(right, bottom - radius);
        path.quadTo(right, bottom, right - radius, bottom);
        path.lineTo(left + radius, bottom);
        path.quadTo(left, bottom, left, bottom - radius);
        path.lineTo(tail.at(2));
        path.lineTo(tail.at(1));
        path.lineTo(tail.at(0));
        path.lineTo(left, top + radius);
        path.quadTo(left, top, left + radius, top);
        break;
    case TeachingTipTailPosition::Right:
    case TeachingTipTailPosition::RightTop:
    case TeachingTipTailPosition::RightBottom:
        path.moveTo(left + radius, top);
        path.lineTo(right - radius, top);
        path.quadTo(right, top, right, top + radius);
        path.lineTo(tail.at(0));
        path.lineTo(tail.at(1));
        path.lineTo(tail.at(2));
        path.lineTo(right, bottom - radius);
        path.quadTo(right, bottom, right - radius, bottom);
        path.lineTo(left + radius, bottom);
        path.quadTo(left, bottom, left, bottom - radius);
        path.lineTo(left, top + radius);
        path.quadTo(left, top, left + radius, top);
        break;
    case TeachingTipTailPosition::None:
    default:
        path.addRoundedRect(rect, radius, radius);
        return path;
    }

    path.closeSubpath();
    return path;
}

QPolygonF tailPolygon(TeachingTipTailPosition pos, int w, int h, int inset)
{
    switch (pos) {
    // Top variants
    case TeachingTipTailPosition::Top:
        return QPolygonF({QPointF(w / 2.0 - 7, inset), QPointF(w / 2.0, 1), QPointF(w / 2.0 + 7, inset)});
    case TeachingTipTailPosition::TopLeft:
        return QPolygonF({QPointF(20, inset), QPointF(27, 1), QPointF(34, inset)});
    case TeachingTipTailPosition::TopRight:
        return QPolygonF({QPointF(w - 20.0, inset), QPointF(w - 27.0, 1), QPointF(w - 34.0, inset)});

    // Bottom variants
    case TeachingTipTailPosition::Bottom:
        return QPolygonF(
            {QPointF(w / 2.0 - 7, h - inset), QPointF(w / 2.0, h - 1), QPointF(w / 2.0 + 7, h - inset)});
    case TeachingTipTailPosition::BottomLeft:
        return QPolygonF({QPointF(20, h - inset), QPointF(27, h - 1), QPointF(34, h - inset)});
    case TeachingTipTailPosition::BottomRight:
        return QPolygonF(
            {QPointF(w - 20.0, h - inset), QPointF(w - 27.0, h - 1), QPointF(w - 34.0, h - inset)});

    // Left variants
    case TeachingTipTailPosition::Left:
        return QPolygonF({QPointF(inset, h / 2.0 - 7), QPointF(1, h / 2.0), QPointF(inset, h / 2.0 + 7)});
    case TeachingTipTailPosition::LeftTop:
        return QPolygonF({QPointF(inset, 10), QPointF(1, 17), QPointF(inset, 24)});
    case TeachingTipTailPosition::LeftBottom:
        return QPolygonF({QPointF(inset, h - 10.0), QPointF(1, h - 17.0), QPointF(inset, h - 24.0)});

    // Right variants
    case TeachingTipTailPosition::Right:
        return QPolygonF(
            {QPointF(w - inset, h / 2.0 - 7), QPointF(w - 1, h / 2.0), QPointF(w - inset, h / 2.0 + 7)});
    case TeachingTipTailPosition::RightTop:
        return QPolygonF({QPointF(w - inset, 10), QPointF(w - 1, 17), QPointF(w - inset, 24)});
    case TeachingTipTailPosition::RightBottom:
        return QPolygonF({QPointF(w - inset, h - 10.0), QPointF(w - 1, h - 17.0), QPointF(w - inset, h - 24.0)});

    default:
        return QPolygonF();
    }
}

class TeachingTipBubble : public QWidget
{
public:
    TeachingTipBubble(FlyoutViewBase *view, TeachingTipTailPosition tailPosition, QWidget *parent = nullptr)
        : QWidget(parent), m_view(view), m_tailPosition(tailPosition)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setProperty("fqw", QStringLiteral("TeachingTipBubble"));

        m_layout = new QHBoxLayout(this);
        m_layout->setSpacing(0);
        if (m_view) {
            m_view->setParent(this);
            m_layout->addWidget(m_view);
        }
        applyTailMargins();
    }

    FlyoutViewBase *view() const { return m_view; }

    TeachingTipTailPosition tailPosition() const { return m_tailPosition; }

    void setTailPosition(TeachingTipTailPosition tailPosition)
    {
        if (m_tailPosition == tailPosition) {
            return;
        }
        m_tailPosition = tailPosition;
        applyTailMargins();
        updateGeometry();
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const int w = width();
        const int h = height();
        const int inset = m_tailInset;
        QRectF bodyRect;

        switch (m_tailPosition) {
        case TeachingTipTailPosition::Top:
        case TeachingTipTailPosition::TopLeft:
        case TeachingTipTailPosition::TopRight:
            bodyRect = QRectF(1, inset, w - 2, h - inset - 1);
            break;
        case TeachingTipTailPosition::Bottom:
        case TeachingTipTailPosition::BottomLeft:
        case TeachingTipTailPosition::BottomRight:
            bodyRect = QRectF(1, 1, w - 2, h - inset - 1);
            break;
        case TeachingTipTailPosition::Left:
        case TeachingTipTailPosition::LeftTop:
        case TeachingTipTailPosition::LeftBottom:
            bodyRect = QRectF(inset, 1, w - inset - 2, h - 2);
            break;
        case TeachingTipTailPosition::Right:
        case TeachingTipTailPosition::RightTop:
        case TeachingTipTailPosition::RightBottom:
            bodyRect = QRectF(1, 1, w - inset - 1, h - 2);
            break;
        case TeachingTipTailPosition::None:
        default:
            bodyRect = QRectF(1, 1, w - 2, h - 2);
            break;
        }

        const QPainterPath path = roundedTailPath(bodyRect, m_tailPosition, tailPolygon(m_tailPosition, w, h, inset));
        painter.setPen(Qt::NoPen);
        painter.setBrush(bubbleBackgroundColor());
        painter.drawPath(path);

        QPen borderPen(bubbleBorderColor());
        borderPen.setWidthF(1.0);
        borderPen.setJoinStyle(Qt::RoundJoin);
        borderPen.setCapStyle(Qt::RoundCap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(borderPen);
        painter.drawPath(path);
    }

private:
    void applyTailMargins()
    {
        if (!m_layout) {
            return;
        }

        switch (m_tailPosition) {
        case TeachingTipTailPosition::Top:
        case TeachingTipTailPosition::TopLeft:
        case TeachingTipTailPosition::TopRight:
            m_layout->setContentsMargins(0, m_tailInset, 0, 0);
            break;
        case TeachingTipTailPosition::Bottom:
        case TeachingTipTailPosition::BottomLeft:
        case TeachingTipTailPosition::BottomRight:
            m_layout->setContentsMargins(0, 0, 0, m_tailInset);
            break;
        case TeachingTipTailPosition::Left:
        case TeachingTipTailPosition::LeftTop:
        case TeachingTipTailPosition::LeftBottom:
            m_layout->setContentsMargins(m_tailInset, 0, 0, 0);
            break;
        case TeachingTipTailPosition::Right:
        case TeachingTipTailPosition::RightTop:
        case TeachingTipTailPosition::RightBottom:
            m_layout->setContentsMargins(0, 0, m_tailInset, 0);
            break;
        case TeachingTipTailPosition::None:
        default:
            m_layout->setContentsMargins(0, 0, 0, 0);
            break;
        }
    }

    FlyoutViewBase *m_view = nullptr;
    TeachingTipTailPosition m_tailPosition = TeachingTipTailPosition::Bottom;
    QHBoxLayout *m_layout = nullptr;
    int m_tailInset = 8;
};

} // namespace

// ==========================================
// TeachingTipView
// ==========================================

TeachingTipView::TeachingTipView(const QString &title, const QString &content, QWidget *parent) : FlyoutViewBase(parent)
{
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(1, 1, 1, 1);
    m_rootLayout->setSpacing(0);

    m_imageLabel = new ImageLabel(this);
    m_imageLabel->hide();

    m_contentLayout = new QVBoxLayout;
    m_contentLayout->setContentsMargins(6, 8, 6, 8);
    m_contentLayout->setSpacing(4);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(36, 36);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->hide();
    headerLayout->addWidget(m_iconLabel, 0, Qt::AlignTop);

    m_titleLabel = new StrongBodyLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("TeachingTipTitle"));
    headerLayout->addWidget(m_titleLabel, 1);

    m_closeButton = new TransparentToolButton(icon(FluentIcon::Close), this);
    m_closeButton->setFixedSize(32, 32);
    m_closeButton->setIconSize(QSize(12, 12));
    m_closeButton->setToolTip(tr("Close"));
    headerLayout->addWidget(m_closeButton, 0, Qt::AlignTop);
    connect(m_closeButton, &QToolButton::clicked, this, [this]() { emit closed(); });

    m_contentLayout->addLayout(headerLayout);

    m_contentLabel = new BodyLabel(this);
    m_contentLabel->setObjectName(QStringLiteral("TeachingTipContent"));
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setMinimumWidth(220);
    m_contentLayout->addWidget(m_contentLabel);

    m_bodyLayout = new QVBoxLayout;
    m_bodyLayout->setContentsMargins(0, 0, 0, 0);
    m_bodyLayout->setSpacing(8);
    m_contentLayout->addLayout(m_bodyLayout);

    m_rootLayout->addWidget(m_imageLabel);
    m_rootLayout->addLayout(m_contentLayout);

    setTitle(title);
    setContent(content);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("TeachingTipView"));
}

TeachingTipView::TeachingTipView(const QString &title, const QString &content, const QIcon &iconValue,
                                 const QPixmap &image, bool isClosable, TeachingTipTailPosition tail, QWidget *parent)
    : TeachingTipView(title, content, parent)
{
    setIcon(iconValue);
    setClosable(isClosable);
    setTailHint(tail);
    if (!image.isNull()) {
        setImage(image);
    }
}

TeachingTipView::TeachingTipView(const QString &title, const QString &content, const QIcon &iconValue,
                                 const QString &imagePath, bool isClosable, TeachingTipTailPosition tail,
                                 QWidget *parent)
    : TeachingTipView(title, content, parent)
{
    setIcon(iconValue);
    setClosable(isClosable);
    setTailHint(tail);
    if (!imagePath.isEmpty()) {
        setImagePath(imagePath);
    }
}

QString TeachingTipView::title() const { return m_titleLabel->text(); }

QString TeachingTipView::content() const { return m_contentLabel->text(); }

bool TeachingTipView::isClosable() const { return m_closable; }

void TeachingTipView::setTitle(const QString &title) { m_titleLabel->setText(title); }

void TeachingTipView::setContent(const QString &content) { m_contentLabel->setText(content); }

void TeachingTipView::setIcon(const QIcon &iconValue)
{
    m_iconLabel->setVisible(!iconValue.isNull());
    m_iconLabel->setPixmap(iconValue.pixmap(QSize(18, 18), devicePixelRatioF()));
}

void TeachingTipView::setClosable(bool closable)
{
    m_closable = closable;
    m_closeButton->setVisible(m_closable);
}

void TeachingTipView::setImage(const QPixmap &pixmap) { setImage(pixmap.toImage()); }

void TeachingTipView::setImage(const QImage &image)
{
    m_imageLabel->setImage(image);
    m_imageLabel->setVisible(!m_imageLabel->isNull());
    updateImageLayout();
    adjustSize();
}

bool TeachingTipView::setImagePath(const QString &path)
{
    if (!m_imageLabel->setImagePath(path)) {
        return false;
    }
    m_imageLabel->setVisible(!m_imageLabel->isNull());
    updateImageLayout();
    adjustSize();
    return true;
}

QVBoxLayout *TeachingTipView::bodyLayout() const { return m_bodyLayout; }

void TeachingTipView::setTailHint(TeachingTipTailPosition tail)
{
    m_tailHint = tail;
    updateImageLayout();
}

void TeachingTipView::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    if (!widget || !m_bodyLayout) {
        return;
    }
    m_bodyLayout->addWidget(widget, stretch, alignment);
    adjustSize();
}

void TeachingTipView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

void TeachingTipView::showEvent(QShowEvent *event)
{
    FlyoutViewBase::showEvent(event);
    updateImageLayout();
}

void TeachingTipView::updateImageLayout()
{
    if (m_imageLabel->isNull()) {
        m_imageLabel->hide();
        return;
    }

    const TeachingTipImagePosition imagePosition = imagePositionForTail(m_tailHint);
    const QSize contentHint = m_contentLayout ? m_contentLayout->sizeHint() : QSize(220, 120);
    if (imagePosition == TeachingTipImagePosition::Top || imagePosition == TeachingTipImagePosition::Bottom) {
        const int width = qMax(1, qMin(qMax(220, contentHint.width()) - 2, maxTipImageWidth()));
        m_imageLabel->scaledToWidth(width);
        if (imagePosition == TeachingTipImagePosition::Top) {
            m_imageLabel->setBorderRadius(8, 8, 0, 0);
        } else {
            m_imageLabel->setBorderRadius(0, 0, 8, 8);
        }
    } else {
        const int height = qMax(1, qMin(qMax(120, contentHint.height()) - 2, maxTipImageHeight()));
        m_imageLabel->scaledToHeight(height);
        m_imageLabel->setBorderRadius(8);
    }
    m_imageLabel->show();
}

TeachingTipImagePosition TeachingTipView::imagePositionForTail(TeachingTipTailPosition tail) const
{
    switch (tail) {
    case TeachingTipTailPosition::Top:
    case TeachingTipTailPosition::TopLeft:
    case TeachingTipTailPosition::TopRight:
        return TeachingTipImagePosition::Bottom;
    case TeachingTipTailPosition::Left:
        return TeachingTipImagePosition::Right;
    case TeachingTipTailPosition::Right:
        return TeachingTipImagePosition::Left;
    case TeachingTipTailPosition::LeftTop:
    case TeachingTipTailPosition::RightTop:
        return TeachingTipImagePosition::Bottom;
    case TeachingTipTailPosition::LeftBottom:
    case TeachingTipTailPosition::RightBottom:
        return TeachingTipImagePosition::Top;
    case TeachingTipTailPosition::Bottom:
    case TeachingTipTailPosition::BottomLeft:
    case TeachingTipTailPosition::BottomRight:
    case TeachingTipTailPosition::None:
    default:
        return TeachingTipImagePosition::Top;
    }
}

// ==========================================
// TeachingTip
// ==========================================

TeachingTip::TeachingTip(FlyoutViewBase *view, QWidget *target, int durationMs, TeachingTipTailPosition tail,
                         QWidget *parent)
    : QWidget(parent), m_view(view), m_target(target), m_duration(durationMs), m_tailPosition(tail)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setProperty("tailPosition", tailPositionName(m_tailPosition));

    m_opacityAni = new QPropertyAnimation(this, "windowOpacity", this);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 8, 15, 20);
    layout->setSpacing(0);

    if (m_view) {
        m_bubble = new TeachingTipBubble(m_view, tail, this);
        layout->addWidget(m_bubble);
        if (auto *tipView = qobject_cast<TeachingTipView *>(m_view)) {
            tipView->setTailHint(tail);
        }
        connect(m_view, &FlyoutViewBase::closed, this, &TeachingTip::closeAndDelete);
    }
    setShadowEffect();
    FluentStyleSheet::setRole(this, QStringLiteral("TeachingTip"));
    if (m_target && m_target->window()) {
        m_target->window()->installEventFilter(this);
    }
}

void TeachingTip::setShadowEffect()
{
    if (!m_bubble) {
        return;
    }

    const QColor color = isDarkTheme() ? QColor(0, 0, 0, 80) : QColor(0, 0, 0, 30);
    m_shadowEffect = new QGraphicsDropShadowEffect(m_bubble);
    m_shadowEffect->setBlurRadius(35);
    m_shadowEffect->setOffset(0, 8);
    m_shadowEffect->setColor(color);
    m_bubble->setGraphicsEffect(nullptr);
    m_bubble->setGraphicsEffect(m_shadowEffect);
}

void TeachingTip::resizeToSizeHint()
{
    if (layout()) {
        layout()->activate();
    }

    const QSize hint = sizeHint().expandedTo(minimumSizeHint());
    if (hint.isValid()) {
        resize(hint.boundedTo(maximumSize()).expandedTo(minimumSize()));
    }
}

TeachingTip *TeachingTip::make(FlyoutViewBase *view, QWidget *target, TeachingTipTailPosition tail, int durationMs,
                               QWidget *parent)
{
    auto *tip = new TeachingTip(view, target, durationMs, tail, parent);
    tip->setIsDeleteOnClose(true);
    tip->reposition();
    tip->show();
    tip->raise();
    return tip;
}

TeachingTip *TeachingTip::create(const QString &title, const QString &content, QWidget *target,
                                 TeachingTipTailPosition tail, int durationMs, QWidget *parent)
{
    return make(new TeachingTipView(title, content), target, tail, durationMs, parent);
}

TeachingTip *TeachingTip::create(const QString &title, const QString &content, const QIcon &iconValue,
                                 const QPixmap &image, bool isClosable, QWidget *target,
                                 TeachingTipTailPosition tail, int durationMs, QWidget *parent,
                                 bool isDeleteOnClose)
{
    auto *view = new TeachingTipView(title, content, iconValue, image, isClosable, tail);
    auto *tip = make(view, target, tail, durationMs, parent);
    tip->setIsDeleteOnClose(isDeleteOnClose);
    connect(view, &FlyoutViewBase::closed, tip, &TeachingTip::close);
    return tip;
}

TeachingTip *TeachingTip::create(const QString &title, const QString &content, const QIcon &iconValue,
                                 const QString &imagePath, bool isClosable, QWidget *target,
                                 TeachingTipTailPosition tail, int durationMs, QWidget *parent,
                                 bool isDeleteOnClose)
{
    auto *view = new TeachingTipView(title, content, iconValue, imagePath, isClosable, tail);
    auto *tip = make(view, target, tail, durationMs, parent);
    tip->setIsDeleteOnClose(isDeleteOnClose);
    connect(view, &FlyoutViewBase::closed, tip, &TeachingTip::close);
    return tip;
}

FlyoutViewBase *TeachingTip::view() const { return m_view; }

QWidget *TeachingTip::target() const { return m_target; }

int TeachingTip::duration() const { return m_duration; }

TeachingTipTailPosition TeachingTip::tailPosition() const { return m_tailPosition; }

bool TeachingTip::isDeleteOnClose() const { return m_isDeleteOnClose; }

void TeachingTip::setIsDeleteOnClose(bool value) { m_isDeleteOnClose = value; }

void TeachingTip::setDuration(int durationMs) { m_duration = durationMs; }

void TeachingTip::setTailPosition(TeachingTipTailPosition position)
{
    if (m_tailPosition == position) {
        return;
    }
    m_tailPosition = position;
    setProperty("tailPosition", tailPositionName(m_tailPosition));
    if (auto *tipView = qobject_cast<TeachingTipView *>(m_view)) {
        tipView->setTailHint(position);
    }
    applyTailMargins();
    FluentStyleSheet::polish(this);
    reposition();
    update();
}

void TeachingTip::reposition()
{
    resizeToSizeHint();
    move(calculatePosition());
}

void TeachingTip::fadeOut()
{
    m_opacityAni->setDuration(167);
    m_opacityAni->setStartValue(1.0);
    m_opacityAni->setEndValue(0.0);
    connect(m_opacityAni, &QPropertyAnimation::finished, this, &TeachingTip::close);
    m_opacityAni->start();
}

void TeachingTip::showEvent(QShowEvent *event)
{
    if (m_duration >= 0) {
        QTimer::singleShot(m_duration, this, &TeachingTip::fadeOut);
    }

    reposition();

    m_opacityAni->setDuration(167);
    m_opacityAni->setStartValue(0.0);
    m_opacityAni->setEndValue(1.0);
    m_opacityAni->start();

    QWidget::showEvent(event);
}

void TeachingTip::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void TeachingTip::closeEvent(QCloseEvent *event)
{
    if (m_isDeleteOnClose) {
        deleteLater();
    }
    emit closed();
    QWidget::closeEvent(event);
}

bool TeachingTip::eventFilter(QObject *watched, QEvent *event)
{
    if (m_target && watched == m_target->window()) {
        if (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
            event->type() == QEvent::WindowStateChange) {
            reposition();
        }
    }
    return QWidget::eventFilter(watched, event);
}

QPoint TeachingTip::calculatePosition() const
{
    if (!m_target) {
        return QPoint(0, 0);
    }

    const QPoint targetTopLeft = m_target->mapToGlobal(QPoint(0, 0));
    const QSize targetSize = m_target->size();
    const QSize tipSize = sizeHint();
    const QSize viewSize = m_view ? m_view->sizeHint() : tipSize;
    const QMargins margins = layout() ? layout()->contentsMargins() : QMargins(15, 8, 15, 20);

    QPoint point;

    switch (m_tailPosition) {
    case TeachingTipTailPosition::Top:
        point = QPoint(targetTopLeft.x() + targetSize.width() / 2 - tipSize.width() / 2,
                       targetTopLeft.y() + targetSize.height() - margins.top());
        break;
    case TeachingTipTailPosition::TopLeft:
        point = QPoint(targetTopLeft.x() - margins.left(), targetTopLeft.y() + targetSize.height() - margins.top());
        break;
    case TeachingTipTailPosition::TopRight:
        point = QPoint(targetTopLeft.x() + targetSize.width() - tipSize.width() + margins.left(),
                       targetTopLeft.y() + targetSize.height() - margins.top());
        break;

    case TeachingTipTailPosition::Bottom:
        point = QPoint(targetTopLeft.x() + targetSize.width() / 2 - tipSize.width() / 2,
                       targetTopLeft.y() - tipSize.height() + margins.bottom());
        break;
    case TeachingTipTailPosition::BottomLeft:
        point = QPoint(targetTopLeft.x() - margins.left(), targetTopLeft.y() - tipSize.height() + margins.bottom());
        break;
    case TeachingTipTailPosition::BottomRight:
        point = QPoint(targetTopLeft.x() + targetSize.width() - tipSize.width() + margins.left(),
                       targetTopLeft.y() - tipSize.height() + margins.bottom());
        break;

    case TeachingTipTailPosition::Left:
        point = QPoint(targetTopLeft.x() + targetSize.width() - margins.left(),
                       targetTopLeft.y() - viewSize.height() / 2 + targetSize.height() / 2 - margins.top());
        break;
    case TeachingTipTailPosition::LeftTop:
        point = QPoint(targetTopLeft.x() + targetSize.width() - margins.left(), targetTopLeft.y() - margins.top());
        break;
    case TeachingTipTailPosition::LeftBottom:
        point = QPoint(targetTopLeft.x() + targetSize.width() - margins.left(),
                       targetTopLeft.y() + targetSize.height() - tipSize.height() + margins.bottom());
        break;

    case TeachingTipTailPosition::Right:
        point = QPoint(targetTopLeft.x() - tipSize.width() + margins.right(),
                       targetTopLeft.y() - viewSize.height() / 2 + targetSize.height() / 2 - margins.top());
        break;
    case TeachingTipTailPosition::RightTop:
        point = QPoint(targetTopLeft.x() - tipSize.width() + margins.right(), targetTopLeft.y() - margins.top());
        break;
    case TeachingTipTailPosition::RightBottom:
        point = QPoint(targetTopLeft.x() - tipSize.width() + margins.right(),
                       targetTopLeft.y() + targetSize.height() - tipSize.height() + margins.bottom());
        break;

    case TeachingTipTailPosition::None:
    default:
        point = QPoint(targetTopLeft.x() + (targetSize.width() - tipSize.width()) / 2,
                       targetTopLeft.y() + targetSize.height() + 8);
        break;
    }

    QScreen *screen = QGuiApplication::screenAt(targetTopLeft);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return point;
    }

    const QRect available = screen->availableGeometry();
    const int maxX = qMax(available.left(), available.x() + available.width() - tipSize.width());
    const int maxY = qMax(available.top(), available.y() + available.height() - tipSize.height());
    point.setX(qBound(available.left(), point.x(), maxX));
    point.setY(qBound(available.top(), point.y(), maxY));
    return point;
}

void TeachingTip::closeAndDelete()
{
    close();
    deleteLater();
}

void TeachingTip::applyTailMargins()
{
    auto *bubble = dynamic_cast<TeachingTipBubble *>(m_bubble);
    if (!bubble) {
        return;
    }
    bubble->setTailPosition(m_tailPosition);
}

PopupTeachingTip::PopupTeachingTip(FlyoutViewBase *view, QWidget *target, int durationMs,
                                   TeachingTipTailPosition tail, QWidget *parent, bool isDeleteOnClose)
    : TeachingTip(view, target, durationMs, tail, parent)
{
    setIsDeleteOnClose(isDeleteOnClose);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
}

PopupTeachingTip *PopupTeachingTip::make(FlyoutViewBase *view, QWidget *target, TeachingTipTailPosition tail,
                                         int durationMs, QWidget *parent, bool isDeleteOnClose)
{
    auto *tip = new PopupTeachingTip(view, target, durationMs, tail, parent, isDeleteOnClose);
    tip->reposition();
    tip->show();
    tip->raise();
    return tip;
}

} // namespace FluentQt
