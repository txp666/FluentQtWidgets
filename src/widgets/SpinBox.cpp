#include <FluentQtWidgets/Widgets/SpinBox.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>

#include <QtGui/QContextMenuEvent>
#include <QtGui/QFont>
#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QStyle>

namespace FluentQt {

// ============================================================================
// Internal helpers
// ============================================================================

static bool isDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}

static QString spinIconPath(const QString &name)
{
    const QString color = isDarkTheme() ? QStringLiteral("white") : QStringLiteral("black");
    return QStringLiteral(":/qfluentwidgets/images/spin_box/%1_%2.svg").arg(name, color);
}

static QPixmap loadSpinIcon(const QString &name, int size = 11)
{
    QPixmap pix(spinIconPath(name));
    if (pix.isNull()) {
        return QPixmap(size, size);
    }
    return pix.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

class SpinFlyoutButton final : public TransparentToolButton
{
public:
    explicit SpinFlyoutButton(SpinIconType icon, QWidget *parent = nullptr) : TransparentToolButton(parent), m_icon(icon)
    {
        setCursor(Qt::PointingHandCursor);
        setIcon(QIcon());
        setIconSize(QSize(13, 13));
        FluentStyleSheet::setRole(this, QStringLiteral("TransparentToolButton"));
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        TransparentToolButton::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        if (!isEnabled()) {
            painter.setOpacity(0.36);
        } else if (isDown()) {
            painter.setOpacity(0.7);
        }

        const int iconExtent = iconSize().isValid() ? qMin(iconSize().width(), iconSize().height()) : 13;
        const QString iconName = (m_icon == SpinIconType::Up) ? QStringLiteral("Up") : QStringLiteral("Down");
        const QPixmap pix = loadSpinIcon(iconName, iconExtent);
        const QRectF rect((width() - iconExtent) / 2.0, (height() - iconExtent) / 2.0, iconExtent, iconExtent);
        painter.drawPixmap(rect, pix, QRectF(QPointF(0, 0), QSizeF(pix.size())));
    }

private:
    SpinIconType m_icon = SpinIconType::Up;
};

// ============================================================================
// SpinButton
// ============================================================================

SpinButton::SpinButton(SpinIconType icon, QWidget *parent) : QToolButton(parent), m_icon(icon)
{
    setFixedSize(31, 23);
    setIconSize(QSize(10, 10));
    FluentStyleSheet::setRole(this, QStringLiteral("SpinButton"));
}

SpinIconType SpinButton::iconType() const
{
    return m_icon;
}

bool SpinButton::isPressed() const
{
    return m_isPressed;
}

void SpinButton::setIconType(SpinIconType icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    update();
}

void SpinButton::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    QToolButton::mousePressEvent(event);
}

void SpinButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    QToolButton::mouseReleaseEvent(event);
}

void SpinButton::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }

    const QString iconName = (m_icon == SpinIconType::Up) ? QStringLiteral("Up") : QStringLiteral("Down");
    const QPixmap pix = loadSpinIcon(iconName);
    painter.drawPixmap(QRectF(10, 6.5, 11, 11), pix, QRectF(QPointF(0, 0), QSizeF(pix.size())));
}

// ============================================================================
// CompactSpinButton
// ============================================================================

CompactSpinButton::CompactSpinButton(QWidget *parent) : QToolButton(parent)
{
    setFixedSize(26, 33);
    setCursor(Qt::IBeamCursor);
    FluentStyleSheet::setRole(this, QStringLiteral("CompactSpinButton"));
}

void CompactSpinButton::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const qreal x = (width() - 10) / 2.0;
    const qreal s = 9;

    const QPixmap upPix = loadSpinIcon(QStringLiteral("Up"), 9);
    const QPixmap downPix = loadSpinIcon(QStringLiteral("Down"), 9);

    const qreal upY = height() / 2.0 - s + 1;
    const qreal downY = height() / 2.0;

    painter.drawPixmap(QRectF(x, upY, s, s), upPix, QRectF(QPointF(0, 0), QSizeF(upPix.size())));
    painter.drawPixmap(QRectF(x, downY, s, s), downPix, QRectF(QPointF(0, 0), QSizeF(downPix.size())));
}

// ============================================================================
// SpinFlyoutView
// ============================================================================

SpinFlyoutView::SpinFlyoutView(QWidget *parent) : FlyoutViewBase(parent)
{
    upButton = new SpinFlyoutButton(SpinIconType::Up, this);
    downButton = new SpinFlyoutButton(SpinIconType::Down, this);
    m_layout = new QVBoxLayout(this);

    upButton->setFixedSize(36, 36);
    downButton->setFixedSize(36, 36);
    upButton->setIconSize(QSize(13, 13));
    downButton->setIconSize(QSize(13, 13));

    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->setSpacing(0);
    m_layout->addWidget(upButton);
    m_layout->addWidget(downButton);
}

TransparentToolButton *SpinFlyoutView::upButtonWidget() const
{
    return upButton;
}

TransparentToolButton *SpinFlyoutView::downButtonWidget() const
{
    return downButton;
}

QVBoxLayout *SpinFlyoutView::vBoxLayout() const
{
    return m_layout;
}

void SpinFlyoutView::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setBrush(isDarkTheme() ? QColor(46, 46, 46) : QColor(249, 249, 249));
    painter.setPen(isDarkTheme() ? QColor(0, 0, 0, 51) : QColor(0, 0, 0, 15));

    const QRect r = rect().adjusted(1, 1, -1, -1);
    painter.drawRoundedRect(r, 8, 8);
}

// ============================================================================
// SpinBoxHelper
// ============================================================================

SpinBoxHelper::SpinBoxHelper() = default;

SpinBoxHelper::~SpinBoxHelper() = default;

void SpinBoxHelper::init(QAbstractSpinBox *spinBox, Mode mode)
{
    m_spinBox = spinBox;
    m_mode = mode;
    m_symbolVisible = true;

    m_spinBox->setProperty("transparent", true);
    m_spinBox->setFixedHeight(33);
    m_spinBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    QFont font = m_spinBox->font();
    font.setPixelSize(14);
    m_spinBox->setFont(font);
    FluentStyleSheet::apply(m_spinBox, FluentStyleSheetSource::SpinBox);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);

    m_layout = new QHBoxLayout(m_spinBox);

    if (mode == Inline) {
        m_upButton = new SpinButton(SpinIconType::Up, m_spinBox);
        m_downButton = new SpinButton(SpinIconType::Down, m_spinBox);

        m_layout->setContentsMargins(0, 4, 4, 4);
        m_layout->setSpacing(5);
        m_layout->addStretch();
        m_layout->addWidget(m_upButton, 0, Qt::AlignRight | Qt::AlignVCenter);
        m_layout->addWidget(m_downButton, 0, Qt::AlignRight | Qt::AlignVCenter);

        QObject::connect(m_upButton, &QToolButton::clicked, m_spinBox, [this]() { m_spinBox->stepUp(); });
        QObject::connect(m_downButton, &QToolButton::clicked, m_spinBox, [this]() { m_spinBox->stepDown(); });
    } else {
        m_compactButton = new CompactSpinButton(m_spinBox);
        m_flyoutView = new SpinFlyoutView(nullptr);
        m_flyout = new Flyout(m_flyoutView, m_spinBox);
        m_flyout->setIsDeleteOnClose(false);
        m_flyout->setPopupMode(true);
        m_flyout->hide();

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->addStretch();
        m_layout->addWidget(m_compactButton, 0, Qt::AlignRight | Qt::AlignVCenter);

        QObject::connect(m_compactButton, &QToolButton::clicked, m_spinBox, [this]() { onFocusIn(); });
        QObject::connect(m_flyoutView->upButton, &QToolButton::clicked, m_spinBox,
                         [this]() { m_spinBox->stepUp(); });
        QObject::connect(m_flyoutView->downButton, &QToolButton::clicked, m_spinBox,
                         [this]() { m_spinBox->stepDown(); });
    }

    // Context menu
    m_spinBox->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(m_spinBox, &QWidget::customContextMenuRequested, m_spinBox,
                     [this](const QPoint &pos) { showContextMenu(pos); });
}

bool SpinBoxHelper::isError() const
{
    return m_isError;
}

bool SpinBoxHelper::isSymbolVisible() const
{
    return m_symbolVisible;
}

void SpinBoxHelper::setError(bool error)
{
    if (m_isError == error) {
        return;
    }
    m_isError = error;
    m_spinBox->style()->unpolish(m_spinBox);
    m_spinBox->style()->polish(m_spinBox);
    m_spinBox->update();
}

void SpinBoxHelper::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_lightFocusedBorderColor = light;
    m_darkFocusedBorderColor = dark;
    m_spinBox->update();
}

void SpinBoxHelper::setSymbolVisible(bool visible)
{
    if (m_symbolVisible == visible) {
        return;
    }
    m_symbolVisible = visible;
    m_spinBox->setProperty("symbolVisible", visible);
    m_spinBox->style()->unpolish(m_spinBox);
    m_spinBox->style()->polish(m_spinBox);
    m_spinBox->update();

    if (m_mode == Inline) {
        if (m_upButton)
            m_upButton->setVisible(visible);
        if (m_downButton)
            m_downButton->setVisible(visible);
    } else {
        if (!visible && m_flyout && m_flyout->isVisible()) {
            m_flyout->close();
        }
        if (m_compactButton)
            m_compactButton->setVisible(visible);
    }
}

void SpinBoxHelper::setAccelerated(bool on)
{
    m_spinBox->setAccelerated(on);

    if (m_mode == Inline) {
        if (m_upButton)
            m_upButton->setAutoRepeat(on);
        if (m_downButton)
            m_downButton->setAutoRepeat(on);
    } else {
        if (m_flyoutView) {
            m_flyoutView->upButton->setAutoRepeat(on);
            m_flyoutView->downButton->setAutoRepeat(on);
        }
    }
}

void SpinBoxHelper::setReadOnly(bool readOnly)
{
    m_spinBox->setReadOnly(readOnly);
    setSymbolVisible(!readOnly);
}

void SpinBoxHelper::drawBorderBottom(QPainter &painter)
{
    if (!m_spinBox->hasFocus()) {
        return;
    }

    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    const qreal w = m_spinBox->width();
    const qreal h = m_spinBox->height();
    path.addRoundedRect(QRectF(0, h - 10, w, 10), 5, 5);

    QPainterPath rectPath;
    rectPath.addRect(QRectF(0, h - 10, w, 8));
    path = path.subtracted(rectPath);

    painter.fillPath(path, focusedBorderColor());
}

QColor SpinBoxHelper::focusedBorderColor() const
{
    if (m_isError) {
        return FluentQt::color(FluentSystemColor::CriticalForeground);
    }
    return autoFallbackThemeColor(m_lightFocusedBorderColor, m_darkFocusedBorderColor);
}

void SpinBoxHelper::onFocusIn()
{
    if (m_mode != Compact || !m_flyout) {
        return;
    }

    if (m_flyout->isVisible() || m_spinBox->isReadOnly()) {
        return;
    }

    const int y = static_cast<int>(m_compactButton->height() / 2.0 - 46);
    const QPoint pos = m_compactButton->mapToGlobal(QPoint(-12, y));

    m_flyout->exec(pos, FlyoutAnimationType::FadeIn);
}

void SpinBoxHelper::showContextMenu(const QPoint &pos)
{
    QLineEdit *lineEdit = m_spinBox->findChild<QLineEdit *>();
    if (!lineEdit) {
        return;
    }

    auto *menu = new LineEditMenu(lineEdit);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(m_spinBox->mapToGlobal(pos), true, MenuAnimationType::FadeInDropDown);
}

SpinButton *SpinBoxHelper::upButton() const
{
    return m_upButton;
}

SpinButton *SpinBoxHelper::downButton() const
{
    return m_downButton;
}

CompactSpinButton *SpinBoxHelper::compactSpinButton() const
{
    return m_compactButton;
}

SpinFlyoutView *SpinBoxHelper::spinFlyoutView() const
{
    return m_flyoutView;
}

Flyout *SpinBoxHelper::spinFlyout() const
{
    return m_flyout;
}

QHBoxLayout *SpinBoxHelper::hBoxLayout() const
{
    return m_layout;
}

// ============================================================================
// SpinBox (Inline)
// ============================================================================

SpinBox::SpinBox(QWidget *parent) : QSpinBox(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Inline);
    FluentStyleSheet::setRole(this, QStringLiteral("SpinBox"));
}

SpinBox::~SpinBox() { delete m_helper; }

bool SpinBox::isError() const
{
    return m_helper->isError();
}

bool SpinBox::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *SpinBox::hBoxLayout() const { return m_helper->hBoxLayout(); }

SpinButton *SpinBox::upButton() const { return m_helper->upButton(); }

SpinButton *SpinBox::downButton() const { return m_helper->downButton(); }

void SpinBox::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void SpinBox::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void SpinBox::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void SpinBox::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void SpinBox::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void SpinBox::paintEvent(QPaintEvent *event)
{
    QSpinBox::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void SpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

// ============================================================================
// DoubleSpinBox (Inline)
// ============================================================================

DoubleSpinBox::DoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Inline);
    FluentStyleSheet::setRole(this, QStringLiteral("DoubleSpinBox"));
}

DoubleSpinBox::~DoubleSpinBox() { delete m_helper; }

bool DoubleSpinBox::isError() const
{
    return m_helper->isError();
}

bool DoubleSpinBox::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *DoubleSpinBox::hBoxLayout() const { return m_helper->hBoxLayout(); }

SpinButton *DoubleSpinBox::upButton() const { return m_helper->upButton(); }

SpinButton *DoubleSpinBox::downButton() const { return m_helper->downButton(); }

void DoubleSpinBox::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void DoubleSpinBox::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void DoubleSpinBox::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void DoubleSpinBox::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void DoubleSpinBox::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void DoubleSpinBox::paintEvent(QPaintEvent *event)
{
    QDoubleSpinBox::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void DoubleSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

// ============================================================================
// TimeEdit (Inline)
// ============================================================================

TimeEdit::TimeEdit(QWidget *parent) : QTimeEdit(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Inline);
    FluentStyleSheet::setRole(this, QStringLiteral("TimeEdit"));
}

TimeEdit::~TimeEdit() { delete m_helper; }

bool TimeEdit::isError() const
{
    return m_helper->isError();
}

bool TimeEdit::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *TimeEdit::hBoxLayout() const { return m_helper->hBoxLayout(); }

SpinButton *TimeEdit::upButton() const { return m_helper->upButton(); }

SpinButton *TimeEdit::downButton() const { return m_helper->downButton(); }

void TimeEdit::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void TimeEdit::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void TimeEdit::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void TimeEdit::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void TimeEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void TimeEdit::paintEvent(QPaintEvent *event)
{
    QTimeEdit::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void TimeEdit::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

// ============================================================================
// DateEdit (Inline)
// ============================================================================

DateEdit::DateEdit(QWidget *parent) : QDateEdit(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Inline);
    FluentStyleSheet::setRole(this, QStringLiteral("DateEdit"));
}

DateEdit::~DateEdit() { delete m_helper; }

bool DateEdit::isError() const
{
    return m_helper->isError();
}

bool DateEdit::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *DateEdit::hBoxLayout() const { return m_helper->hBoxLayout(); }

SpinButton *DateEdit::upButton() const { return m_helper->upButton(); }

SpinButton *DateEdit::downButton() const { return m_helper->downButton(); }

void DateEdit::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void DateEdit::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void DateEdit::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void DateEdit::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void DateEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void DateEdit::paintEvent(QPaintEvent *event)
{
    QDateEdit::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void DateEdit::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

// ============================================================================
// DateTimeEdit (Inline)
// ============================================================================

DateTimeEdit::DateTimeEdit(QWidget *parent) : QDateTimeEdit(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Inline);
    FluentStyleSheet::setRole(this, QStringLiteral("DateTimeEdit"));
}

DateTimeEdit::~DateTimeEdit() { delete m_helper; }

bool DateTimeEdit::isError() const
{
    return m_helper->isError();
}

bool DateTimeEdit::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *DateTimeEdit::hBoxLayout() const { return m_helper->hBoxLayout(); }

SpinButton *DateTimeEdit::upButton() const { return m_helper->upButton(); }

SpinButton *DateTimeEdit::downButton() const { return m_helper->downButton(); }

void DateTimeEdit::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void DateTimeEdit::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void DateTimeEdit::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void DateTimeEdit::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void DateTimeEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void DateTimeEdit::paintEvent(QPaintEvent *event)
{
    QDateTimeEdit::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void DateTimeEdit::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

// ============================================================================
// CompactSpinBox
// ============================================================================

CompactSpinBox::CompactSpinBox(QWidget *parent) : QSpinBox(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Compact);
    FluentStyleSheet::setRole(this, QStringLiteral("CompactSpinBox"));
}

CompactSpinBox::~CompactSpinBox() { delete m_helper; }

bool CompactSpinBox::isError() const
{
    return m_helper->isError();
}

bool CompactSpinBox::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *CompactSpinBox::hBoxLayout() const { return m_helper->hBoxLayout(); }

CompactSpinButton *CompactSpinBox::compactSpinButton() const { return m_helper->compactSpinButton(); }

SpinFlyoutView *CompactSpinBox::spinFlyoutView() const { return m_helper->spinFlyoutView(); }

Flyout *CompactSpinBox::spinFlyout() const { return m_helper->spinFlyout(); }

void CompactSpinBox::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void CompactSpinBox::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void CompactSpinBox::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void CompactSpinBox::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void CompactSpinBox::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void CompactSpinBox::paintEvent(QPaintEvent *event)
{
    QSpinBox::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void CompactSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

void CompactSpinBox::focusInEvent(QFocusEvent *event)
{
    QSpinBox::focusInEvent(event);
    m_helper->onFocusIn();
}

// ============================================================================
// CompactDoubleSpinBox
// ============================================================================

CompactDoubleSpinBox::CompactDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Compact);
    FluentStyleSheet::setRole(this, QStringLiteral("CompactDoubleSpinBox"));
}

CompactDoubleSpinBox::~CompactDoubleSpinBox() { delete m_helper; }

bool CompactDoubleSpinBox::isError() const
{
    return m_helper->isError();
}

bool CompactDoubleSpinBox::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *CompactDoubleSpinBox::hBoxLayout() const { return m_helper->hBoxLayout(); }

CompactSpinButton *CompactDoubleSpinBox::compactSpinButton() const { return m_helper->compactSpinButton(); }

SpinFlyoutView *CompactDoubleSpinBox::spinFlyoutView() const { return m_helper->spinFlyoutView(); }

Flyout *CompactDoubleSpinBox::spinFlyout() const { return m_helper->spinFlyout(); }

void CompactDoubleSpinBox::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void CompactDoubleSpinBox::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void CompactDoubleSpinBox::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void CompactDoubleSpinBox::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void CompactDoubleSpinBox::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void CompactDoubleSpinBox::paintEvent(QPaintEvent *event)
{
    QDoubleSpinBox::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void CompactDoubleSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

void CompactDoubleSpinBox::focusInEvent(QFocusEvent *event)
{
    QDoubleSpinBox::focusInEvent(event);
    m_helper->onFocusIn();
}

// ============================================================================
// CompactTimeEdit
// ============================================================================

CompactTimeEdit::CompactTimeEdit(QWidget *parent) : QTimeEdit(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Compact);
    FluentStyleSheet::setRole(this, QStringLiteral("CompactTimeEdit"));
}

CompactTimeEdit::~CompactTimeEdit() { delete m_helper; }

bool CompactTimeEdit::isError() const
{
    return m_helper->isError();
}

bool CompactTimeEdit::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *CompactTimeEdit::hBoxLayout() const { return m_helper->hBoxLayout(); }

CompactSpinButton *CompactTimeEdit::compactSpinButton() const { return m_helper->compactSpinButton(); }

SpinFlyoutView *CompactTimeEdit::spinFlyoutView() const { return m_helper->spinFlyoutView(); }

Flyout *CompactTimeEdit::spinFlyout() const { return m_helper->spinFlyout(); }

void CompactTimeEdit::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void CompactTimeEdit::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void CompactTimeEdit::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void CompactTimeEdit::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void CompactTimeEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void CompactTimeEdit::paintEvent(QPaintEvent *event)
{
    QTimeEdit::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void CompactTimeEdit::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

void CompactTimeEdit::focusInEvent(QFocusEvent *event)
{
    QTimeEdit::focusInEvent(event);
    m_helper->onFocusIn();
}

// ============================================================================
// CompactDateEdit
// ============================================================================

CompactDateEdit::CompactDateEdit(QWidget *parent) : QDateEdit(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Compact);
    FluentStyleSheet::setRole(this, QStringLiteral("CompactDateEdit"));
}

CompactDateEdit::~CompactDateEdit() { delete m_helper; }

bool CompactDateEdit::isError() const
{
    return m_helper->isError();
}

bool CompactDateEdit::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *CompactDateEdit::hBoxLayout() const { return m_helper->hBoxLayout(); }

CompactSpinButton *CompactDateEdit::compactSpinButton() const { return m_helper->compactSpinButton(); }

SpinFlyoutView *CompactDateEdit::spinFlyoutView() const { return m_helper->spinFlyoutView(); }

Flyout *CompactDateEdit::spinFlyout() const { return m_helper->spinFlyout(); }

void CompactDateEdit::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void CompactDateEdit::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void CompactDateEdit::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void CompactDateEdit::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void CompactDateEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void CompactDateEdit::paintEvent(QPaintEvent *event)
{
    QDateEdit::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void CompactDateEdit::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

void CompactDateEdit::focusInEvent(QFocusEvent *event)
{
    QDateEdit::focusInEvent(event);
    m_helper->onFocusIn();
}

// ============================================================================
// CompactDateTimeEdit
// ============================================================================

CompactDateTimeEdit::CompactDateTimeEdit(QWidget *parent) : QDateTimeEdit(parent)
{
    m_helper = new SpinBoxHelper();
    m_helper->init(this, SpinBoxHelper::Compact);
    FluentStyleSheet::setRole(this, QStringLiteral("CompactDateTimeEdit"));
}

CompactDateTimeEdit::~CompactDateTimeEdit() { delete m_helper; }

bool CompactDateTimeEdit::isError() const
{
    return m_helper->isError();
}

bool CompactDateTimeEdit::isSymbolVisible() const { return m_helper->isSymbolVisible(); }

QHBoxLayout *CompactDateTimeEdit::hBoxLayout() const { return m_helper->hBoxLayout(); }

CompactSpinButton *CompactDateTimeEdit::compactSpinButton() const { return m_helper->compactSpinButton(); }

SpinFlyoutView *CompactDateTimeEdit::spinFlyoutView() const { return m_helper->spinFlyoutView(); }

Flyout *CompactDateTimeEdit::spinFlyout() const { return m_helper->spinFlyout(); }

void CompactDateTimeEdit::setError(bool error)
{
    if (error == m_helper->isError()) {
        return;
    }
    m_helper->setError(error);
    emit errorChanged(error);
}

void CompactDateTimeEdit::setReadOnly(bool readOnly) { m_helper->setReadOnly(readOnly); }

void CompactDateTimeEdit::setSymbolVisible(bool visible) { m_helper->setSymbolVisible(visible); }

void CompactDateTimeEdit::setAccelerated(bool on) { m_helper->setAccelerated(on); }

void CompactDateTimeEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    m_helper->setCustomFocusedBorderColor(light, dark);
}

void CompactDateTimeEdit::paintEvent(QPaintEvent *event)
{
    QDateTimeEdit::paintEvent(event);
    QPainter painter(this);
    m_helper->drawBorderBottom(painter);
}

void CompactDateTimeEdit::contextMenuEvent(QContextMenuEvent *event)
{
    m_helper->showContextMenu(event->pos());
    event->accept();
}

void CompactDateTimeEdit::focusInEvent(QFocusEvent *event)
{
    QDateTimeEdit::focusInEvent(event);
    m_helper->onFocusIn();
}

} // namespace FluentQt
