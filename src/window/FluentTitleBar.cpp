#include <FluentQtWidgets/Window/FluentTitleBar.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Label.h>

#include <QtCore/QtGlobal>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QWindow>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>

namespace FluentQt {

namespace {

enum class TitleBarSymbol
{
    Minimize,
    Maximize,
    Restore,
    Close
};

class TitleBarToolButton : public TransparentToolButton
{
  public:
    explicit TitleBarToolButton(TitleBarSymbol symbol, QWidget *parent = nullptr)
        : TransparentToolButton(parent), m_symbol(symbol)
    {
        setIcon(QIcon());
        setText(QString());
        setIconSize(QSize(10, 10));
        setCursor(Qt::ArrowCursor);
        setFocusPolicy(Qt::NoFocus);
        setAutoRaise(true);
        setAttribute(Qt::WA_StyledBackground, false);
        setProperty("fqw", QStringLiteral("TitleBarButton"));
        setStyleSheet(QString());
    }

    TitleBarSymbol symbol() const { return m_symbol; }

    QSize sizeHint() const override { return QSize(46, 32); }

    QSize minimumSizeHint() const override { return QSize(46, 32); }

    void setSymbol(TitleBarSymbol symbol)
    {
        if (m_symbol == symbol) {
            return;
        }
        m_symbol = symbol;
        update();
    }

  protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        const bool hovered = underMouse();
        const bool pressed = isDown();

        QColor background(Qt::transparent);
        if (m_symbol == TitleBarSymbol::Close && hovered) {
            background = pressed ? QColor(241, 112, 122) : QColor(232, 17, 35);
        } else if (hovered) {
            if (dark) {
                background = pressed ? QColor(255, 255, 255, 51) : QColor(255, 255, 255, 26);
            } else {
                background = pressed ? QColor(0, 0, 0, 51) : QColor(0, 0, 0, 26);
            }
        }

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.fillRect(rect(), background);

        QColor glyph = dark ? QColor(Qt::white) : QColor(Qt::black);
        if (m_symbol == TitleBarSymbol::Close && hovered) {
            glyph = Qt::white;
        }

        const QSize glyphSize = iconSize();
        const QRect glyphRect((width() - glyphSize.width()) / 2,
                              (height() - glyphSize.height()) / 2,
                              glyphSize.width(),
                              glyphSize.height());
        FluentQt::icon(iconForSymbol(m_symbol), glyph).paint(&painter, glyphRect);
    }

  private:
    static FluentIcon iconForSymbol(TitleBarSymbol symbol)
    {
        switch (symbol) {
        case TitleBarSymbol::Minimize:
            return FluentIcon::Minimize;
        case TitleBarSymbol::Maximize:
            return FluentIcon::FullScreen;
        case TitleBarSymbol::Restore:
            return FluentIcon::BackToWindow;
        case TitleBarSymbol::Close:
            return FluentIcon::Close;
        }
        return FluentIcon::Close;
    }

    TitleBarSymbol m_symbol = TitleBarSymbol::Minimize;
};

TitleBarToolButton *asTitleBarButton(TransparentToolButton *button)
{
    return static_cast<TitleBarToolButton *>(button);
}

} // namespace

FluentTitleBar::FluentTitleBar(QWidget *parent) : QFrame(parent)
{
    setFixedHeight(48);
    setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(this, QStringLiteral("FluentTitleBar"));
    initLayout();
    syncWithWindow();
}

CaptionLabel *FluentTitleBar::titleLabel() const { return m_titleLabel; }

QLabel *FluentTitleBar::iconLabel() const { return m_iconLabel; }

TransparentToolButton *FluentTitleBar::minimizeButton() const { return m_minButton; }

TransparentToolButton *FluentTitleBar::maximizeButton() const { return m_maxButton; }

TransparentToolButton *FluentTitleBar::closeButton() const { return m_closeButton; }

void FluentTitleBar::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
    m_titleLabel->adjustSize();
}

void FluentTitleBar::setWindowIcon(const QIcon &windowIcon)
{
    m_iconLabel->setPixmap(windowIcon.pixmap(QSize(18, 18), devicePixelRatioF()));
}

void FluentTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isCaptionArea(event->pos())) {
        toggleMaximized();
        event->accept();
        return;
    }

    QFrame::mouseDoubleClickEvent(event);
}

void FluentTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        QFrame::mouseMoveEvent(event);
        return;
    }

    QWidget *targetWindow = window();
    if (!targetWindow || targetWindow->isMaximized() || targetWindow->isFullScreen()) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPoint globalPos = event->globalPosition().toPoint();
#else
    const QPoint globalPos = event->globalPos();
#endif
    targetWindow->move(globalPos - m_dragPosition);
    event->accept();
}

void FluentTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isCaptionArea(event->pos())) {
        QWidget *targetWindow = window();
        if (targetWindow && !targetWindow->isMaximized() && !targetWindow->isFullScreen()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QPoint globalPos = event->globalPosition().toPoint();
#else
            const QPoint globalPos = event->globalPos();
#endif
            m_dragPosition = globalPos - targetWindow->frameGeometry().topLeft();
            m_dragging = true;

            if (QWindow *handle = targetWindow->windowHandle()) {
                if (handle->startSystemMove()) {
                    m_dragging = false;
                }
            }

            event->accept();
            return;
        }
    }

    QFrame::mousePressEvent(event);
}

void FluentTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QFrame::mouseReleaseEvent(event);
}

void FluentTitleBar::initLayout()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 0, 0);
    layout->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(18, 18);
    m_titleLabel = new CaptionLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));

    m_minButton = new TitleBarToolButton(TitleBarSymbol::Minimize, this);
    m_minButton->setObjectName(QStringLiteral("MinimizeButton"));
    m_minButton->setFixedSize(46, 32);
    m_minButton->setToolTip(QStringLiteral("Minimize"));

    m_maxButton = new TitleBarToolButton(TitleBarSymbol::Maximize, this);
    m_maxButton->setObjectName(QStringLiteral("MaximizeButton"));
    m_maxButton->setFixedSize(46, 32);
    m_maxButton->setToolTip(QStringLiteral("Maximize"));

    m_closeButton = new TitleBarToolButton(TitleBarSymbol::Close, this);
    m_closeButton->setObjectName(QStringLiteral("CloseButton"));
    m_closeButton->setFixedSize(46, 32);
    m_closeButton->setToolTip(QStringLiteral("Close"));

    layout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);
    layout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    layout->addStretch(1);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(m_minButton);
    buttonLayout->addWidget(m_maxButton);
    buttonLayout->addWidget(m_closeButton);
    layout->addLayout(buttonLayout);

    connect(m_minButton, &TransparentToolButton::clicked, this, [this]() {
        if (QWidget *window = this->window()) {
            window->showMinimized();
        }
    });
    connect(m_maxButton, &TransparentToolButton::clicked, this, [this]() {
        toggleMaximized();
    });
    connect(m_closeButton, &TransparentToolButton::clicked, this, [this]() {
        if (QWidget *window = this->window()) {
            window->close();
        }
    });
}

void FluentTitleBar::syncWithWindow()
{
    QWidget *window = this->window();
    if (!window) {
        return;
    }

    setTitle(window->windowTitle());
    setWindowIcon(window->windowIcon());

    connect(window, &QWidget::windowTitleChanged, this, &FluentTitleBar::setTitle);
    connect(window, &QWidget::windowIconChanged, this, &FluentTitleBar::setWindowIcon);
}

bool FluentTitleBar::isCaptionArea(const QPoint &pos) const
{
    QWidget *child = childAt(pos);
    while (child && child != this) {
        if (child == m_minButton || child == m_maxButton || child == m_closeButton) {
            return false;
        }
        child = child->parentWidget();
    }
    return rect().contains(pos);
}

void FluentTitleBar::toggleMaximized()
{
    if (QWidget *targetWindow = window()) {
        targetWindow->isMaximized() ? targetWindow->showNormal() : targetWindow->showMaximized();
        asTitleBarButton(m_maxButton)->setSymbol(targetWindow->isMaximized() ? TitleBarSymbol::Restore
                                                                             : TitleBarSymbol::Maximize);
    }
}

} // namespace FluentQt
