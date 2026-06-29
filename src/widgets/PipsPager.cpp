#include <FluentQtWidgets/Widgets/PipsPager.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtGui/QPainter>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLayoutItem>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QToolButton>

#include <utility>

namespace FluentQt {

namespace {
int boundedVisibleNumber(int visibleNumber) { return qMax(1, visibleNumber); }

class PipsScrollButton final : public QToolButton
{
  public:
    explicit PipsScrollButton(FluentIcon icon, QWidget *parent = nullptr)
        : QToolButton(parent)
        , m_icon(icon)
    {
        setFixedSize(12, 12);
        setIconSize(QSize(8, 8));
        setCursor(Qt::PointingHandCursor);
        FluentStyleSheet::setRole(this, QStringLiteral("PipsPagerScrollButton"));
    }

    void setFluentIcon(FluentIcon icon)
    {
        if (m_icon == icon) {
            return;
        }

        m_icon = icon;
        update();
    }

  protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        const bool active = underMouse() || isDown();
        const QColor tint = dark ? QColor(255, 255, 255) : QColor(0, 0, 0);
        painter.setOpacity(dark ? (active ? 0.773 : 0.541) : (active ? 0.616 : 0.45));

        const QRectF rect = isDown() ? QRectF(3, 3, 6, 6) : QRectF(2, 2, 8, 8);
        FluentQt::icon(m_icon, dark ? Theme::Dark : Theme::Light, tint).paint(&painter, rect.toRect());
    }

  private:
    FluentIcon m_icon;
};
} // namespace

PipsPager::PipsPager(QWidget *parent) : QWidget(parent) { init(); }

PipsPager::PipsPager(Qt::Orientation orientation, QWidget *parent) : QWidget(parent), m_orientation(orientation)
{
    init();
}

int PipsPager::pageNumber() const { return m_pageNumber; }

int PipsPager::getPageNumber() const { return pageNumber(); }

int PipsPager::visibleNumber() const { return m_visibleNumber; }

int PipsPager::getVisibleNumber() const { return visibleNumber(); }

int PipsPager::currentIndex() const { return m_currentIndex; }

Qt::Orientation PipsPager::orientation() const { return m_orientation; }

bool PipsPager::isHorizontal() const { return m_orientation == Qt::Horizontal; }

PipsScrollButtonDisplayMode PipsPager::previousButtonDisplayMode() const { return m_previousButtonDisplayMode; }

PipsScrollButtonDisplayMode PipsPager::nextButtonDisplayMode() const { return m_nextButtonDisplayMode; }

QToolButton *PipsPager::previousButton() const { return m_previousButton; }

QToolButton *PipsPager::preButton() const { return m_previousButton; }

QToolButton *PipsPager::nextButton() const { return m_nextButton; }

QWidget *PipsPager::pipContainer() const { return m_pipContainer; }

QList<QToolButton *> PipsPager::pipButtons() const { return m_pipButtons; }

QPropertyAnimation *PipsPager::scrollAnimation() const { return m_scrollAnimation; }

qreal PipsPager::pipScrollOffset() const { return m_pipScrollOffset; }

bool PipsPager::isHover() const { return m_hovered; }

QSize PipsPager::sizeHint() const
{
    const int pipExtent = m_visibleNumber * 12;
    const int buttonExtent = 30;
    return isHorizontal() ? QSize(pipExtent + buttonExtent, 12) : QSize(12, pipExtent + buttonExtent);
}

void PipsPager::setPageNumber(int pageNumber)
{
    const int boundedPageNumber = qMax(0, pageNumber);
    if (m_pageNumber == boundedPageNumber) {
        return;
    }

    m_pageNumber = boundedPageNumber;
    if (m_pageNumber == 0) {
        m_currentIndex = -1;
    } else if (m_currentIndex < 0) {
        m_currentIndex = 0;
    } else if (m_currentIndex >= m_pageNumber) {
        m_currentIndex = m_pageNumber - 1;
    }

    rebuildPips();
    emit pageNumberChanged(m_pageNumber);
    emit currentIndexChanged(m_currentIndex);
}

void PipsPager::setVisibleNumber(int visibleNumber)
{
    const int boundedNumber = boundedVisibleNumber(visibleNumber);
    if (m_visibleNumber == boundedNumber) {
        return;
    }

    m_visibleNumber = boundedNumber;
    updateVisiblePips();
    updateGeometry();
    emit visibleNumberChanged(m_visibleNumber);
}

void PipsPager::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_pageNumber || m_currentIndex == index) {
        return;
    }

    const int targetStart = visibleStartIndex(index);
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
        m_scrollAnimation->setStartValue(m_pipScrollOffset);
        m_scrollAnimation->setEndValue(static_cast<qreal>(targetStart));
        m_scrollAnimation->start();
    } else {
        setPipScrollOffset(static_cast<qreal>(targetStart));
    }

    m_currentIndex = index;
    updateCurrentPip();
    updateScrollButtonVisibility();
    emit currentIndexChanged(m_currentIndex);
}

void PipsPager::setPipScrollOffset(qreal offset)
{
    if (qFuzzyCompare(m_pipScrollOffset, offset)) {
        return;
    }

    m_pipScrollOffset = offset;
    updatePipContainerOffset();
    updateVisiblePips();
}

void PipsPager::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }

    m_orientation = orientation;
    updateLayoutDirection();
    updateGeometry();
    emit orientationChanged(m_orientation);
}

void PipsPager::setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode mode)
{
    if (m_previousButtonDisplayMode == mode) {
        return;
    }

    m_previousButtonDisplayMode = mode;
    m_previousButton->setProperty("displayMode", displayModeName(mode));
    updateScrollButtonVisibility();
}

void PipsPager::setNextButtonDisplayMode(PipsScrollButtonDisplayMode mode)
{
    if (m_nextButtonDisplayMode == mode) {
        return;
    }

    m_nextButtonDisplayMode = mode;
    m_nextButton->setProperty("displayMode", displayModeName(mode));
    updateScrollButtonVisibility();
}

void PipsPager::scrollNext() { setCurrentIndex(m_currentIndex + 1); }

void PipsPager::scrollPrevious() { setCurrentIndex(m_currentIndex - 1); }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void PipsPager::enterEvent(QEnterEvent *event)
#else
void PipsPager::enterEvent(QEvent *event)
#endif
{
    m_hovered = true;
    updateScrollButtonVisibility();
    QWidget::enterEvent(event);
}

void PipsPager::leaveEvent(QEvent *event)
{
    m_hovered = false;
    updateScrollButtonVisibility();
    QWidget::leaveEvent(event);
}

void PipsPager::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

void PipsPager::init()
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_previousButton = new PipsScrollButton(FluentIcon::Back, this);
    m_previousButton->setToolTip(tr("Previous Page"));
    m_previousButton->setProperty("displayMode", displayModeName(m_previousButtonDisplayMode));
    QSizePolicy previousButtonSizePolicy = m_previousButton->sizePolicy();
    previousButtonSizePolicy.setRetainSizeWhenHidden(true);
    m_previousButton->setSizePolicy(previousButtonSizePolicy);
    connect(m_previousButton, &QToolButton::clicked, this, &PipsPager::scrollPrevious);

    m_nextButton = new PipsScrollButton(FluentIcon::Forward, this);
    m_nextButton->setToolTip(tr("Next Page"));
    m_nextButton->setProperty("displayMode", displayModeName(m_nextButtonDisplayMode));
    QSizePolicy nextButtonSizePolicy = m_nextButton->sizePolicy();
    nextButtonSizePolicy.setRetainSizeWhenHidden(true);
    m_nextButton->setSizePolicy(nextButtonSizePolicy);
    connect(m_nextButton, &QToolButton::clicked, this, &PipsPager::scrollNext);

    m_pipContainer = new QWidget(this);
    m_pipContainer->setFixedSize(m_visibleNumber * 12, 12);

    m_scrollAnimation = new QPropertyAnimation(this, "pipScrollOffset", this);
    m_scrollAnimation->setDuration(500);
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_layout->addWidget(m_previousButton, 0, Qt::AlignCenter);
    m_layout->addWidget(m_pipContainer, 0, Qt::AlignCenter);
    m_layout->addWidget(m_nextButton, 0, Qt::AlignCenter);

    FluentStyleSheet::setRole(this, QStringLiteral("PipsPager"));

    updateLayoutDirection();
    updateScrollButtonVisibility();
}

void PipsPager::rebuildPips()
{
    for (QToolButton *button : std::as_const(m_pipButtons)) {
        button->deleteLater();
    }
    m_pipButtons.clear();

    for (int i = 0; i < m_pageNumber; ++i) {
        auto *button = new QToolButton(m_pipContainer);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setFixedSize(12, 12);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(tr("Page %1").arg(i + 1));
        button->setProperty("pageIndex", i);
        FluentStyleSheet::setRole(button, QStringLiteral("PipsPagerDot"));
        connect(button, &QToolButton::clicked, this, [this, i]() { setCurrentIndex(i); });
        m_pipButtons.append(button);
    }

    if (m_pipContainer) {
        const int extent = qMax(12, m_pageNumber * 12);
        if (m_orientation == Qt::Horizontal) {
            m_pipContainer->setFixedSize(qMin(m_visibleNumber * 12, extent), 12);
        } else {
            m_pipContainer->setFixedSize(12, qMin(m_visibleNumber * 12, extent));
        }
    }

    setPipScrollOffset(static_cast<qreal>(visibleStartIndex(m_currentIndex)));
    updateLayoutDirection();
    updateVisiblePips();
    updateCurrentPip();
    updateScrollButtonVisibility();
    updateGeometry();
}

void PipsPager::updateVisiblePips()
{
    if (m_pipButtons.isEmpty()) {
        return;
    }

    const int visibleCount = qMin(m_visibleNumber, m_pipButtons.size());
    const int start = qRound(m_pipScrollOffset);
    const int end = start + visibleCount;

    for (int i = 0; i < m_pipButtons.size(); ++i) {
        m_pipButtons.at(i)->setVisible(i >= start && i < end);
    }
}

int PipsPager::visibleStartIndex() const { return visibleStartIndex(m_currentIndex); }

int PipsPager::visibleStartIndex(int index) const
{
    if (m_pipButtons.isEmpty()) {
        return 0;
    }

    const int visibleCount = qMin(m_visibleNumber, m_pipButtons.size());
    int start = 0;
    if (index >= 0) {
        start = index - visibleCount / 2;
    }
    return qBound(0, start, m_pipButtons.size() - visibleCount);
}

void PipsPager::updatePipContainerOffset()
{
    if (!m_pipContainer || m_pipButtons.isEmpty()) {
        return;
    }

    const int pixelOffset = qRound(m_pipScrollOffset * 12.0);
    for (int i = 0; i < m_pipButtons.size(); ++i) {
        QToolButton *button = m_pipButtons.at(i);
        if (m_orientation == Qt::Horizontal) {
            button->move(-pixelOffset + i * 12, 0);
        } else {
            button->move(0, -pixelOffset + i * 12);
        }
    }
}

void PipsPager::updateCurrentPip()
{
    for (int i = 0; i < m_pipButtons.size(); ++i) {
        QToolButton *button = m_pipButtons.at(i);
        const bool current = i == m_currentIndex;
        button->setChecked(current);
        button->setProperty("current", current);
        FluentStyleSheet::polish(button);
    }
}

void PipsPager::updateScrollButtonVisibility()
{
    m_previousButton->setVisible(isPreviousButtonVisible());
    m_nextButton->setVisible(isNextButtonVisible());
}

void PipsPager::updateLayoutDirection()
{
    if (!m_layout || !m_previousButton || !m_nextButton || !m_pipContainer) {
        return;
    }

    if (auto *button = dynamic_cast<PipsScrollButton *>(m_previousButton)) {
        button->setFluentIcon(isHorizontal() ? FluentIcon::Back : FluentIcon::Upload);
    }
    if (auto *button = dynamic_cast<PipsScrollButton *>(m_nextButton)) {
        button->setFluentIcon(isHorizontal() ? FluentIcon::Forward : FluentIcon::Download);
    }
    m_layout->setDirection(isHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);

    const int extent = qMax(12, m_pageNumber * 12);
    if (isHorizontal()) {
        m_pipContainer->setFixedSize(qMin(m_visibleNumber * 12, extent), 12);
    } else {
        m_pipContainer->setFixedSize(12, qMin(m_visibleNumber * 12, extent));
    }

    updatePipContainerOffset();
}

bool PipsPager::isPreviousButtonVisible() const
{
    if (m_currentIndex <= 0 || m_previousButtonDisplayMode == PipsScrollButtonDisplayMode::Never) {
        return false;
    }
    return m_previousButtonDisplayMode == PipsScrollButtonDisplayMode::Always || m_hovered;
}

bool PipsPager::isNextButtonVisible() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_pageNumber - 1 ||
        m_nextButtonDisplayMode == PipsScrollButtonDisplayMode::Never) {
        return false;
    }
    return m_nextButtonDisplayMode == PipsScrollButtonDisplayMode::Always || m_hovered;
}

QString PipsPager::displayModeName(PipsScrollButtonDisplayMode mode) const
{
    switch (mode) {
    case PipsScrollButtonDisplayMode::Always:
        return QStringLiteral("Always");
    case PipsScrollButtonDisplayMode::OnHover:
        return QStringLiteral("OnHover");
    case PipsScrollButtonDisplayMode::Never:
        return QStringLiteral("Never");
    }
    return QStringLiteral("Never");
}

HorizontalPipsPager::HorizontalPipsPager(QWidget *parent) : PipsPager(Qt::Horizontal, parent) {}

VerticalPipsPager::VerticalPipsPager(QWidget *parent) : PipsPager(Qt::Vertical, parent) {}

} // namespace FluentQt
