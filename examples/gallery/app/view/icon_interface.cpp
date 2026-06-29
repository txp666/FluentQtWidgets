#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtCore/QFile>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QStyle>

#include <functional>

using namespace FluentQt;

namespace {

QString galleryStyleContent(const QString &styleName, Theme theme)
{
    const QString themeFolder = theme == Theme::Dark ? QStringLiteral("dark") : QStringLiteral("light");
    QFile styleFile(QStringLiteral(":/gallery/qss/%1/%2.qss").arg(themeFolder, styleName));
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(styleFile.readAll());
}

void applyIconGalleryStyle(QWidget *widget)
{
    if (widget) {
        FluentStyleSheet::setCustomStyleSheet(widget, galleryStyleContent(QStringLiteral("icon_interface"), Theme::Light),
                                              galleryStyleContent(QStringLiteral("icon_interface"), Theme::Dark));
    }
}

void repolish(QWidget *widget)
{
    if (!widget) {
        return;
    }
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

QString iconEnumMemberName(FluentIcon icon)
{
    if (icon == FluentIcon::GitHub) {
        return QStringLiteral("GitHub");
    }

    const QString name = iconName(icon);
    QString memberName;
    memberName.reserve(name.size());
    bool upperNext = true;
    for (const QChar ch : name) {
        if (ch == QLatin1Char('_')) {
            upperNext = true;
            continue;
        }
        memberName.append(upperNext ? ch.toUpper() : ch);
        upperNext = false;
    }
    return memberName;
}

QString iconEnumLabel(FluentIcon icon)
{
    return QStringLiteral("FluentIcon::%1").arg(iconEnumMemberName(icon));
}

class IconCard : public QFrame
{
  public:
    explicit IconCard(FluentIcon icon, QWidget *parent = nullptr) : QFrame(parent), m_icon(icon)
    {
        setObjectName(QStringLiteral("iconCard"));
        setFixedSize(96, 96);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);
        setProperty("selected", false);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(8, 28, 8, 0);
        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignTop);

        m_iconWidget = new IconWidget(icon, this);
        m_iconWidget->setFixedSize(28, 28);
        m_iconWidget->setIconSize(QSize(28, 28));
        layout->addWidget(m_iconWidget, 0, Qt::AlignHCenter);
        layout->addSpacing(14);

        m_nameLabel = new QLabel(this);
        m_nameLabel->setAlignment(Qt::AlignHCenter);
        const QString text = m_nameLabel->fontMetrics().elidedText(iconName(icon), Qt::ElideRight, 90);
        m_nameLabel->setText(text);
        layout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);

        setProperty("iconName", iconName(icon));
        setProperty("iconEnumName", iconEnumLabel(icon));
        setProperty("iconValue", QVariant::fromValue(static_cast<int>(icon)));
    }

    FluentIcon iconValue() const { return m_icon; }

    void setSelected(bool selected, bool force = false)
    {
        if (m_selected == selected && !force) {
            return;
        }

        m_selected = selected;
        setProperty("selected", selected);

        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        m_iconWidget->setIcon(selected ? FluentQt::icon(m_icon, dark ? Theme::Light : Theme::Dark)
                                       : FluentQt::icon(m_icon));

        repolish(this);
        repolish(m_nameLabel);
    }

    std::function<void(IconCard *)> clicked;

  protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && rect().contains(event->pos()) && clicked) {
            clicked(this);
            event->accept();
            return;
        }
        QFrame::mouseReleaseEvent(event);
    }

  private:
    FluentIcon m_icon;
    IconWidget *m_iconWidget = nullptr;
    QLabel *m_nameLabel = nullptr;
    bool m_selected = false;
};

class IconInfoPanel : public QFrame
{
  public:
    explicit IconInfoPanel(FluentIcon icon, QWidget *parent = nullptr) : QFrame(parent)
    {
        setObjectName(QStringLiteral("iconInfoPanel"));
        setAttribute(Qt::WA_StyledBackground, true);
        setFixedWidth(216);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 20, 16, 20);
        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignTop);

        m_nameLabel = new QLabel(this);
        m_nameLabel->setObjectName(QStringLiteral("nameLabel"));
        layout->addWidget(m_nameLabel);
        layout->addSpacing(16);

        m_iconWidget = new IconWidget(icon, this);
        m_iconWidget->setFixedSize(48, 48);
        m_iconWidget->setIconSize(QSize(48, 48));
        layout->addWidget(m_iconWidget);
        layout->addSpacing(45);

        auto *iconNameTitleLabel = new QLabel(QCoreApplication::translate("IconInterface", "Icon name"), this);
        iconNameTitleLabel->setObjectName(QStringLiteral("subTitleLabel"));
        layout->addWidget(iconNameTitleLabel);
        layout->addSpacing(5);

        m_iconNameLabel = new QLabel(this);
        layout->addWidget(m_iconNameLabel);
        layout->addSpacing(34);

        auto *enumTitleLabel = new QLabel(QCoreApplication::translate("IconInterface", "Enum member"), this);
        enumTitleLabel->setObjectName(QStringLiteral("subTitleLabel"));
        layout->addWidget(enumTitleLabel);
        layout->addSpacing(5);

        m_enumNameLabel = new QLabel(this);
        layout->addWidget(m_enumNameLabel);

        setIcon(icon);
    }

    void setIcon(FluentIcon icon)
    {
        const QString name = iconName(icon);
        m_iconWidget->setIcon(icon);
        m_nameLabel->setText(name);
        m_iconNameLabel->setText(name);
        m_enumNameLabel->setText(iconEnumLabel(icon));
    }

  private:
    QLabel *m_nameLabel = nullptr;
    IconWidget *m_iconWidget = nullptr;
    QLabel *m_iconNameLabel = nullptr;
    QLabel *m_enumNameLabel = nullptr;
};

} // namespace

QWidget *GalleryWindow::createIconPage()
{
    auto *page = new GalleryInterface(navTx("Icons"),
                                      tx("IconInterface", "Browse all icons in the FluentIcon enum"), this);
    const QString sourceUrl = exampleSourceUrl("text/font_icon");

    // All FluentIcon enum values
    static const FluentIcon allIcons[] = {
        FluentIcon::Add,       FluentIcon::Remove,       FluentIcon::Cut,       FluentIcon::Copy,
        FluentIcon::Paste,     FluentIcon::Search,       FluentIcon::Settings,  FluentIcon::Home,
        FluentIcon::Back,      FluentIcon::Forward,      FluentIcon::Check,     FluentIcon::Close,
        FluentIcon::Cancel,    FluentIcon::Constract,    FluentIcon::Info,      FluentIcon::Warning,
        FluentIcon::Play,      FluentIcon::Pause,
        FluentIcon::Video,     FluentIcon::Album,        FluentIcon::Volume,    FluentIcon::Music,
        FluentIcon::Folder,    FluentIcon::Calendar,     FluentIcon::Ringer,    FluentIcon::StopWatch,
        FluentIcon::PowerButton, FluentIcon::Pin,        FluentIcon::More,      FluentIcon::Edit,
        FluentIcon::Flag,      FluentIcon::Download,     FluentIcon::Upload,    FluentIcon::Heart,
        FluentIcon::People,    FluentIcon::ShoppingCart, FluentIcon::Basketball, FluentIcon::Tag,
        FluentIcon::BookShelf, FluentIcon::Mail,         FluentIcon::Send,       FluentIcon::Save,
        FluentIcon::Star,      FluentIcon::ArrowDown,
        FluentIcon::View,      FluentIcon::Sync,         FluentIcon::Completed, FluentIcon::Minimize,
        FluentIcon::Maximize,  FluentIcon::Restore,      FluentIcon::Brush,     FluentIcon::Transparent,
        FluentIcon::Palette,   FluentIcon::Zoom,         FluentIcon::Language,  FluentIcon::Help,
        FluentIcon::Update,    FluentIcon::Font,         FluentIcon::PencilInk, FluentIcon::Highlight,
        FluentIcon::Alignment, FluentIcon::Link,         FluentIcon::Code,      FluentIcon::Feedback,
        FluentIcon::GitHub,    FluentIcon::Scroll,
    };
    static const int iconCount = sizeof(allIcons) / sizeof(allIcons[0]);

    auto *container = new QWidget(page);
    container->setObjectName(QStringLiteral("iconCardView"));
    container->setAttribute(Qt::WA_StyledBackground, true);
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(12);

    auto *titleLabel = new SubtitleLabel(tx("IconCardView", "Fluent Icons Library"));
    containerLayout->addWidget(titleLabel);

    auto *searchEdit = new SearchLineEdit(page);
    searchEdit->setPlaceholderText(tx("LineEdit", "Search icons"));
    searchEdit->setFixedWidth(304);
    containerLayout->addWidget(searchEdit);

    auto *viewArea = new QWidget(page);
    viewArea->setObjectName(QStringLiteral("iconView"));
    viewArea->setAttribute(Qt::WA_StyledBackground, true);
    auto *viewLayout = new QHBoxLayout(viewArea);
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(0);

    // ScrollArea with icon grid
    auto *scrollArea = new SingleDirectionScrollArea(Qt::Vertical);
    auto *scrollWidget = new QWidget(scrollArea);
    scrollWidget->setObjectName(QStringLiteral("scrollWidget"));
    scrollWidget->setAttribute(Qt::WA_StyledBackground, true);
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *flowLayout = new FlowLayout(0, 8, 8);
    flowLayout->setContentsMargins(8, 3, 8, 8);
    scrollWidget->setLayout(flowLayout);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->enableTransparentBackground();
    viewLayout->addWidget(scrollArea);

    // Info panel on the right
    auto *infoPanel = new IconInfoPanel(allIcons[0], page);

    viewLayout->addWidget(infoPanel, 0, Qt::AlignRight);
    containerLayout->addWidget(viewArea);

    // Build the icon grid
    QVector<IconCard *> iconCards;
    auto currentCard = std::make_shared<QPointer<IconCard>>();
    for (int i = 0; i < iconCount; ++i) {
        FluentIcon fic = allIcons[i];
        auto *card = new IconCard(fic, scrollWidget);
        card->clicked = [card, currentCard, infoPanel](IconCard *) {
            if (currentCard->data() == card) {
                return;
            }
            if (currentCard->data()) {
                currentCard->data()->setSelected(false);
            }
            *currentCard = card;
            card->setSelected(true);
            infoPanel->setIcon(card->iconValue());
        };
        if (i == 0) {
            *currentCard = card;
            card->setSelected(true);
        }

        iconCards.append(card);
        flowLayout->addWidget(card);
    }

    // Helper lambda to refresh the layout
    auto refreshFlowLayout = [flowLayout, iconCards]() {
        while (flowLayout->count() > 0) {
            auto *item = flowLayout->takeAt(0);
            delete item;
        }
        for (auto *card : iconCards) {
            if (card->isVisible()) {
                flowLayout->addWidget(card);
            }
        }
    };

    // Search filtering
    connect(searchEdit, &SearchLineEdit::searchSignal, page, [iconCards, refreshFlowLayout](const QString &text) {
        for (auto *card : iconCards) {
            const QString name = card->property("iconName").toString();
            card->setVisible(text.isEmpty() || name.contains(text, Qt::CaseInsensitive));
        }
        refreshFlowLayout();
    });
    connect(searchEdit, &SearchLineEdit::clearSignal, page, [iconCards, refreshFlowLayout]() {
        for (auto *card : iconCards) {
            card->show();
        }
        refreshFlowLayout();
    });

    applyIconGalleryStyle(container);
    QObject::connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, container,
                     [container, currentCard](Theme) {
                         applyIconGalleryStyle(container);
                         if (currentCard->data()) {
                             currentCard->data()->setSelected(true, true);
                         }
                     });
    QObject::connect(ThemeManager::instance(), &ThemeManager::accentColorChanged, container,
                     [container, currentCard](const QColor &) {
                         applyIconGalleryStyle(container);
                         if (currentCard->data()) {
                             currentCard->data()->setSelected(true, true);
                         }
                     });

    page->addExampleCard(tx("IconCardView", "Fluent Icons Library"), container, sourceUrl, 1);

    return page;
}
