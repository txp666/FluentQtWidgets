#include "../../components/GalleryComponents.h"

#include "GalleryViewHelpers.h"

#include <FluentQtWidgets/Config.h>

#include <QtCore/QFile>
#include <QtCore/QEvent>
#include <QtCore/QUrl>
#include <QtGui/QBrush>
#include <QtGui/QDesktopServices>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>

#include <initializer_list>

using namespace FluentQt;

namespace {

QString galleryStyleContent(const QString &styleName, Theme theme)
{
    const QString themeFolder =
        theme == Theme::Dark ? QStringLiteral("dark") : QStringLiteral("light");
    QFile styleFile(QStringLiteral(":/gallery/qss/%1/%2.qss").arg(themeFolder, styleName));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString::fromUtf8(styleFile.readAll());
    }
    return {};
}

void applyGalleryStyle(QWidget *widget, const QString &styleName)
{
    if (!widget) {
        return;
    }

    FluentStyleSheet::setCustomStyleSheet(widget, galleryStyleContent(styleName, Theme::Light),
                                          galleryStyleContent(styleName, Theme::Dark));
}

class GalleryToolBar : public QWidget
{
  public:
    GalleryToolBar(const QString &title, const QString &subtitle, QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedHeight(138);
        setAttribute(Qt::WA_StyledBackground, true);
        setObjectName(QStringLiteral("galleryToolBar"));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(36, 22, 36, 12);
        layout->setSpacing(4);

        m_titleLabel = new TitleLabel(title, this);
        m_subtitleLabel = new CaptionLabel(subtitle, this);

        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(4);
        buttonLayout->setContentsMargins(0, 0, 0, 0);

        m_documentButton = new PushButton(icon(FluentIcon::Help), tx("ToolBar", "Documentation"), this);
        m_sourceButton = new PushButton(icon(FluentIcon::GitHub), tx("ToolBar", "Source"), this);
        m_themeButton = new ToolButton(icon(FluentIcon::Constract), this);
        m_separator = new VerticalSeparator(this);
        m_separator->setFixedSize(6, 16);
        m_supportButton = new ToolButton(icon(FluentIcon::Heart), this);
        m_feedbackButton = new ToolButton(icon(FluentIcon::Feedback), this);

        m_themeButton->setToolTip(tx("ToolBar", "Toggle theme"));
        m_supportButton->setToolTip(tx("ToolBar", "Support me"));
        m_feedbackButton->setToolTip(tx("ToolBar", "Send feedback"));

        buttonLayout->addWidget(m_documentButton, 0, Qt::AlignLeft);
        buttonLayout->addWidget(m_sourceButton, 0, Qt::AlignLeft);
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(m_themeButton, 0, Qt::AlignRight);
        buttonLayout->addWidget(m_separator, 0, Qt::AlignRight);
        buttonLayout->addWidget(m_supportButton, 0, Qt::AlignRight);
        buttonLayout->addWidget(m_feedbackButton, 0, Qt::AlignRight);

        layout->addWidget(m_titleLabel);
        layout->addWidget(m_subtitleLabel);
        layout->addLayout(buttonLayout, 1);

        connect(m_themeButton, &ToolButton::clicked, this, []() {
            const Theme current = ThemeManager::instance()->effectiveTheme();
            const Theme next = current == Theme::Dark ? Theme::Light : Theme::Dark;
            FluentConfig::instance()->setThemeMode(next);
            FluentConfig::instance()->save();
            ThemeManager::instance()->setTheme(next);
        });
        connect(m_documentButton, &PushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral(FQW_REPOSITORY_URL "/blob/main/docs/quick-start.md")));
        });
        connect(m_sourceButton, &PushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral(FQW_REPOSITORY_URL)));
        });
        connect(m_feedbackButton, &ToolButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues")));
        });
    }

    TitleLabel *titleLabel() const { return m_titleLabel; }
    CaptionLabel *subtitleLabel() const { return m_subtitleLabel; }

  private:
    TitleLabel *m_titleLabel = nullptr;
    CaptionLabel *m_subtitleLabel = nullptr;
    PushButton *m_documentButton = nullptr;
    PushButton *m_sourceButton = nullptr;
    ToolButton *m_themeButton = nullptr;
    VerticalSeparator *m_separator = nullptr;
    ToolButton *m_supportButton = nullptr;
    ToolButton *m_feedbackButton = nullptr;
};

class ExampleCard : public QWidget
{
  public:
    ExampleCard(const QString &title, QWidget *widget, const QString &sourcePath, int stretch, QWidget *parent)
        : QWidget(parent), m_sourcePath(sourcePath)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setSizeConstraint(QLayout::SetMinimumSize);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(12);
        layout->setAlignment(Qt::AlignTop);

        auto *titleLabel = new StrongBodyLabel(title, this);
        auto *card = new QFrame(this);
        card->setObjectName(QStringLiteral("card"));
        card->setAttribute(Qt::WA_StyledBackground);

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setSizeConstraint(QLayout::SetMinimumSize);
        cardLayout->setContentsMargins(0, 0, 0, 0);
        cardLayout->setSpacing(0);
        cardLayout->setAlignment(Qt::AlignTop);

        auto *topLayout = new QHBoxLayout;
        topLayout->setSizeConstraint(QLayout::SetMinimumSize);
        topLayout->setContentsMargins(12, 12, 12, 12);
        widget->setParent(card);
        topLayout->addWidget(widget, stretch);
        if (stretch == 0) {
            topLayout->addStretch(1);
        }

        m_sourceWidget = new QFrame(card);
        m_sourceWidget->setObjectName(QStringLiteral("sourceWidget"));
        m_sourceWidget->setCursor(Qt::PointingHandCursor);
        m_sourceWidget->setAttribute(Qt::WA_StyledBackground);
        m_sourceWidget->installEventFilter(this);

        auto *sourceLayout = new QHBoxLayout(m_sourceWidget);
        sourceLayout->setContentsMargins(18, 18, 18, 18);
        auto *sourceLabel = new BodyLabel(tx("ExampleCard", "Source code"), m_sourceWidget);
        sourceLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        sourceLayout->addWidget(sourceLabel, 0, Qt::AlignLeft);
        sourceLayout->addStretch(1);
        auto *linkIcon = new IconWidget(FluentIcon::Link, m_sourceWidget);
        linkIcon->setFixedSize(16, 16);
        linkIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
        sourceLayout->addWidget(linkIcon, 0, Qt::AlignRight);
        sourceLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        cardLayout->addLayout(topLayout);
        cardLayout->addWidget(m_sourceWidget, 0, Qt::AlignBottom);

        layout->addWidget(titleLabel, 0, Qt::AlignTop);
        layout->addWidget(card, 0, Qt::AlignTop);
        widget->show();
    }

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_sourceWidget && event->type() == QEvent::MouseButtonRelease) {
            QDesktopServices::openUrl(QUrl(m_sourcePath));
            return true;
        }
        return QWidget::eventFilter(watched, event);
    }

  private:
    QString m_sourcePath;
    QFrame *m_sourceWidget = nullptr;
};


} // namespace

GalleryInterface::GalleryInterface(const QString &title, const QString &subtitle, QWidget *parent)
    : ScrollArea(parent)
{
    setObjectName(QStringLiteral("galleryInterface"));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (horizontalFluentScrollBar()) {
        horizontalFluentScrollBar()->setForceHidden(true);
    }

    auto *toolBar = new GalleryToolBar(title, subtitle, this);
    m_toolBar = toolBar;
    m_view = new QWidget(this);
    m_view->setObjectName(QStringLiteral("view"));
    m_viewLayout = new QVBoxLayout(m_view);
    m_viewLayout->setContentsMargins(36, 20, 36, 36);
    m_viewLayout->setSpacing(30);
    m_viewLayout->setAlignment(Qt::AlignTop);

    setWidget(m_view);
    setWidgetResizable(true);
    setViewportMargins(0, m_toolBar->height(), 0, 0);
    applyGalleryStyle(this, QStringLiteral("gallery_interface"));
    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) {
        applyGalleryStyle(this, QStringLiteral("gallery_interface"));
    });
    connect(ThemeManager::instance(), &ThemeManager::accentColorChanged, this, [this](const QColor &) {
        applyGalleryStyle(this, QStringLiteral("gallery_interface"));
    });
}

QWidget *GalleryInterface::contentWidget() const { return m_view; }

QVBoxLayout *GalleryInterface::contentLayout() const { return m_viewLayout; }

QWidget *GalleryInterface::addExampleCard(const QString &title, QWidget *widget, const QString &sourcePath, int stretch)
{
    auto *card = new ExampleCard(title, widget, sourcePath, stretch, m_view);
    m_viewLayout->addWidget(card, 0, Qt::AlignTop);
    return card;
}

void GalleryInterface::scrollToCard(int index)
{
    if (!m_viewLayout) {
        return;
    }

    QLayoutItem *item = m_viewLayout->itemAt(index);
    QWidget *card = item ? item->widget() : nullptr;
    if (!card) {
        return;
    }

    verticalScrollBar()->setValue(card->y());
}

void GalleryInterface::resizeEvent(QResizeEvent *event)
{
    ScrollArea::resizeEvent(event);
    if (m_toolBar) {
        m_toolBar->resize(width(), m_toolBar->height());
    }
}
