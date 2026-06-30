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
    switch (icon) {
    case FluentIcon::Up:
        return QStringLiteral("Up");
    case FluentIcon::Add:
        return QStringLiteral("Add");
    case FluentIcon::Bus:
        return QStringLiteral("Bus");
    case FluentIcon::Car:
        return QStringLiteral("Car");
    case FluentIcon::Cut:
        return QStringLiteral("Cut");
    case FluentIcon::IOT:
        return QStringLiteral("IOT");
    case FluentIcon::Pin:
        return QStringLiteral("Pin");
    case FluentIcon::Tag:
        return QStringLiteral("Tag");
    case FluentIcon::VPN:
        return QStringLiteral("VPN");
    case FluentIcon::Cafe:
        return QStringLiteral("Cafe");
    case FluentIcon::Chat:
        return QStringLiteral("Chat");
    case FluentIcon::Copy:
        return QStringLiteral("Copy");
    case FluentIcon::Code:
        return QStringLiteral("Code");
    case FluentIcon::Down:
        return QStringLiteral("Down");
    case FluentIcon::Edit:
        return QStringLiteral("Edit");
    case FluentIcon::Flag:
        return QStringLiteral("Flag");
    case FluentIcon::Font:
        return QStringLiteral("Font");
    case FluentIcon::Game:
        return QStringLiteral("Game");
    case FluentIcon::Help:
        return QStringLiteral("Help");
    case FluentIcon::Hide:
        return QStringLiteral("Hide");
    case FluentIcon::Home:
        return QStringLiteral("Home");
    case FluentIcon::Info:
        return QStringLiteral("Info");
    case FluentIcon::Leaf:
        return QStringLiteral("Leaf");
    case FluentIcon::Link:
        return QStringLiteral("Link");
    case FluentIcon::Mail:
        return QStringLiteral("Mail");
    case FluentIcon::Menu:
        return QStringLiteral("Menu");
    case FluentIcon::Mute:
        return QStringLiteral("Mute");
    case FluentIcon::More:
        return QStringLiteral("More");
    case FluentIcon::Move:
        return QStringLiteral("Move");
    case FluentIcon::Play:
        return QStringLiteral("Play");
    case FluentIcon::Save:
        return QStringLiteral("Save");
    case FluentIcon::Send:
        return QStringLiteral("Send");
    case FluentIcon::Sync:
        return QStringLiteral("Sync");
    case FluentIcon::Unit:
        return QStringLiteral("Unit");
    case FluentIcon::View:
        return QStringLiteral("View");
    case FluentIcon::Wifi:
        return QStringLiteral("Wifi");
    case FluentIcon::Zoom:
        return QStringLiteral("Zoom");
    case FluentIcon::Album:
        return QStringLiteral("Album");
    case FluentIcon::Brush:
        return QStringLiteral("Brush");
    case FluentIcon::Broom:
        return QStringLiteral("Broom");
    case FluentIcon::Close:
        return QStringLiteral("Close");
    case FluentIcon::Cloud:
        return QStringLiteral("Cloud");
    case FluentIcon::Embed:
        return QStringLiteral("Embed");
    case FluentIcon::Globe:
        return QStringLiteral("Globe");
    case FluentIcon::Heart:
        return QStringLiteral("Heart");
    case FluentIcon::Label:
        return QStringLiteral("Label");
    case FluentIcon::Media:
        return QStringLiteral("Media");
    case FluentIcon::Movie:
        return QStringLiteral("Movie");
    case FluentIcon::Music:
        return QStringLiteral("Music");
    case FluentIcon::Robot:
        return QStringLiteral("Robot");
    case FluentIcon::Pause:
        return QStringLiteral("Pause");
    case FluentIcon::Paste:
        return QStringLiteral("Paste");
    case FluentIcon::Photo:
        return QStringLiteral("Photo");
    case FluentIcon::Phone:
        return QStringLiteral("Phone");
    case FluentIcon::Print:
        return QStringLiteral("Print");
    case FluentIcon::Share:
        return QStringLiteral("Share");
    case FluentIcon::Tiles:
        return QStringLiteral("Tiles");
    case FluentIcon::Unpin:
        return QStringLiteral("Unpin");
    case FluentIcon::Video:
        return QStringLiteral("Video");
    case FluentIcon::Train:
        return QStringLiteral("Train");
    case FluentIcon::AddTo:
        return QStringLiteral("AddTo");
    case FluentIcon::Accept:
        return QStringLiteral("Accept");
    case FluentIcon::Camera:
        return QStringLiteral("Camera");
    case FluentIcon::Cancel:
        return QStringLiteral("Cancel");
    case FluentIcon::Delete:
        return QStringLiteral("Delete");
    case FluentIcon::Folder:
        return QStringLiteral("Folder");
    case FluentIcon::Filter:
        return QStringLiteral("Filter");
    case FluentIcon::Market:
        return QStringLiteral("Market");
    case FluentIcon::Scroll:
        return QStringLiteral("Scroll");
    case FluentIcon::Layout:
        return QStringLiteral("Layout");
    case FluentIcon::GitHub:
        return QStringLiteral("GitHub");
    case FluentIcon::Update:
        return QStringLiteral("Update");
    case FluentIcon::Remove:
        return QStringLiteral("Remove");
    case FluentIcon::Return:
        return QStringLiteral("Return");
    case FluentIcon::People:
        return QStringLiteral("People");
    case FluentIcon::QRCode:
        return QStringLiteral("QRCode");
    case FluentIcon::Ringer:
        return QStringLiteral("Ringer");
    case FluentIcon::Rotate:
        return QStringLiteral("Rotate");
    case FluentIcon::Search:
        return QStringLiteral("Search");
    case FluentIcon::Volume:
        return QStringLiteral("Volume");
    case FluentIcon::Frigid:
        return QStringLiteral("Frigid");
    case FluentIcon::SaveAs:
        return QStringLiteral("SaveAs");
    case FluentIcon::ZoomIn:
        return QStringLiteral("ZoomIn");
    case FluentIcon::Connect:
        return QStringLiteral("Connect");
    case FluentIcon::History:
        return QStringLiteral("History");
    case FluentIcon::Setting:
        return QStringLiteral("Setting");
    case FluentIcon::Palette:
        return QStringLiteral("Palette");
    case FluentIcon::Message:
        return QStringLiteral("Message");
    case FluentIcon::FitPage:
        return QStringLiteral("FitPage");
    case FluentIcon::ZoomOut:
        return QStringLiteral("ZoomOut");
    case FluentIcon::Airplane:
        return QStringLiteral("Airplane");
    case FluentIcon::Asterisk:
        return QStringLiteral("Asterisk");
    case FluentIcon::Calories:
        return QStringLiteral("Calories");
    case FluentIcon::Calendar:
        return QStringLiteral("Calendar");
    case FluentIcon::Feedback:
        return QStringLiteral("Feedback");
    case FluentIcon::Library:
        return QStringLiteral("Library");
    case FluentIcon::Minimize:
        return QStringLiteral("Minimize");
    case FluentIcon::Checkbox:
        return QStringLiteral("Checkbox");
    case FluentIcon::Document:
        return QStringLiteral("Document");
    case FluentIcon::Language:
        return QStringLiteral("Language");
    case FluentIcon::Download:
        return QStringLiteral("Download");
    case FluentIcon::Question:
        return QStringLiteral("Question");
    case FluentIcon::Speakers:
        return QStringLiteral("Speakers");
    case FluentIcon::DateTime:
        return QStringLiteral("DateTime");
    case FluentIcon::FontSize:
        return QStringLiteral("FontSize");
    case FluentIcon::HomeFill:
        return QStringLiteral("HomeFill");
    case FluentIcon::PageLeft:
        return QStringLiteral("PageLeft");
    case FluentIcon::SaveCopy:
        return QStringLiteral("SaveCopy");
    case FluentIcon::SendFill:
        return QStringLiteral("SendFill");
    case FluentIcon::SkipBack:
        return QStringLiteral("SkipBack");
    case FluentIcon::SpeedOff:
        return QStringLiteral("SpeedOff");
    case FluentIcon::Alignment:
        return QStringLiteral("Alignment");
    case FluentIcon::Bluetooth:
        return QStringLiteral("Bluetooth");
    case FluentIcon::Completed:
        return QStringLiteral("Completed");
    case FluentIcon::Constract:
        return QStringLiteral("Constract");
    case FluentIcon::Headphone:
        return QStringLiteral("Headphone");
    case FluentIcon::Megaphone:
        return QStringLiteral("Megaphone");
    case FluentIcon::Projector:
        return QStringLiteral("Projector");
    case FluentIcon::Education:
        return QStringLiteral("Education");
    case FluentIcon::LeftArrow:
        return QStringLiteral("LeftArrow");
    case FluentIcon::EraseTool:
        return QStringLiteral("EraseTool");
    case FluentIcon::PageRight:
        return QStringLiteral("PageRight");
    case FluentIcon::PlaySolid:
        return QStringLiteral("PlaySolid");
    case FluentIcon::BookShelf:
        return QStringLiteral("BookShelf");
    case FluentIcon::Hightlight:
        return QStringLiteral("Hightlight");
    case FluentIcon::FolderAdd:
        return QStringLiteral("FolderAdd");
    case FluentIcon::PauseBold:
        return QStringLiteral("PauseBold");
    case FluentIcon::PencilInk:
        return QStringLiteral("PencilInk");
    case FluentIcon::PieSingle:
        return QStringLiteral("PieSingle");
    case FluentIcon::QuickNote:
        return QStringLiteral("QuickNote");
    case FluentIcon::SpeedHigh:
        return QStringLiteral("SpeedHigh");
    case FluentIcon::StopWatch:
        return QStringLiteral("StopWatch");
    case FluentIcon::ZipFolder:
        return QStringLiteral("ZipFolder");
    case FluentIcon::Basketball:
        return QStringLiteral("Basketball");
    case FluentIcon::Brightness:
        return QStringLiteral("Brightness");
    case FluentIcon::Dictionary:
        return QStringLiteral("Dictionary");
    case FluentIcon::Microphone:
        return QStringLiteral("Microphone");
    case FluentIcon::ArrowDown:
        return QStringLiteral("ArrowDown");
    case FluentIcon::FullScreen:
        return QStringLiteral("FullScreen");
    case FluentIcon::MixVolumes:
        return QStringLiteral("MixVolumes");
    case FluentIcon::RemoveFrom:
        return QStringLiteral("RemoveFrom");
    case FluentIcon::RightArrow:
        return QStringLiteral("RightArrow");
    case FluentIcon::QuietHours:
        return QStringLiteral("QuietHours");
    case FluentIcon::Fingerprint:
        return QStringLiteral("Fingerprint");
    case FluentIcon::Application:
        return QStringLiteral("Application");
    case FluentIcon::Certificate:
        return QStringLiteral("Certificate");
    case FluentIcon::Transparent:
        return QStringLiteral("Transparent");
    case FluentIcon::ImageExport:
        return QStringLiteral("ImageExport");
    case FluentIcon::SpeedMedium:
        return QStringLiteral("SpeedMedium");
    case FluentIcon::LibraryFill:
        return QStringLiteral("LibraryFill");
    case FluentIcon::MusicFolder:
        return QStringLiteral("MusicFolder");
    case FluentIcon::PowerButton:
        return QStringLiteral("PowerButton");
    case FluentIcon::SkipForward:
        return QStringLiteral("SkipForward");
    case FluentIcon::CareUpSolid:
        return QStringLiteral("CareUpSolid");
    case FluentIcon::AcceptMedium:
        return QStringLiteral("AcceptMedium");
    case FluentIcon::CancelMedium:
        return QStringLiteral("CancelMedium");
    case FluentIcon::ChevronRight:
        return QStringLiteral("ChevronRight");
    case FluentIcon::ClippingTool:
        return QStringLiteral("ClippingTool");
    case FluentIcon::SearchMirror:
        return QStringLiteral("SearchMirror");
    case FluentIcon::ShoppingCart:
        return QStringLiteral("ShoppingCart");
    case FluentIcon::FontIncrease:
        return QStringLiteral("FontIncrease");
    case FluentIcon::BackToWindow:
        return QStringLiteral("BackToWindow");
    case FluentIcon::CommandPrompt:
        return QStringLiteral("CommandPrompt");
    case FluentIcon::CloudDownload:
        return QStringLiteral("CloudDownload");
    case FluentIcon::DictionaryAdd:
        return QStringLiteral("DictionaryAdd");
    case FluentIcon::CareDownSolid:
        return QStringLiteral("CareDownSolid");
    case FluentIcon::CareLeftSolid:
        return QStringLiteral("CareLeftSolid");
    case FluentIcon::ClearSelection:
        return QStringLiteral("ClearSelection");
    case FluentIcon::DeveloperTools:
        return QStringLiteral("DeveloperTools");
    case FluentIcon::BackgroundFill:
        return QStringLiteral("BackgroundFill");
    case FluentIcon::CareRightSolid:
        return QStringLiteral("CareRightSolid");
    case FluentIcon::ChevronDownMed:
        return QStringLiteral("ChevronDownMed");
    case FluentIcon::ChevronRightMed:
        return QStringLiteral("ChevronRightMed");
    case FluentIcon::EmojiTabSymbols:
        return QStringLiteral("EmojiTabSymbols");
    case FluentIcon::ExpressiveInputEntry:
        return QStringLiteral("ExpressiveInputEntry");
    }
    return QString();
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
        setProperty("isSelected", false);

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
        const QString text = m_nameLabel->fontMetrics().elidedText(iconResourceName(icon), Qt::ElideRight, 90);
        m_nameLabel->setText(text);
        layout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);

        setProperty("iconName", iconResourceName(icon));
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
        setProperty("isSelected", selected);

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
        const QString name = iconResourceName(icon);
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

class IconScrollArea : public SingleDirectionScrollArea
{
  public:
    explicit IconScrollArea(QWidget *parent = nullptr) : SingleDirectionScrollArea(Qt::Vertical, parent) {}

    using SingleDirectionScrollArea::setViewportMargins;
};

} // namespace

QWidget *GalleryWindow::createIconPage()
{
    auto *page = new GalleryInterface(navTx("Icons"),
                                      tx("IconInterface", "Browse all icons in the FluentIcon enum"), this);

    // All FluentIcon enum values
    static const FluentIcon allIcons[] = {
        FluentIcon::Up, FluentIcon::Add, FluentIcon::Bus, FluentIcon::Car,
        FluentIcon::Cut, FluentIcon::IOT, FluentIcon::Pin, FluentIcon::Tag,
        FluentIcon::VPN, FluentIcon::Cafe, FluentIcon::Chat, FluentIcon::Copy,
        FluentIcon::Code, FluentIcon::Down, FluentIcon::Edit, FluentIcon::Flag,
        FluentIcon::Font, FluentIcon::Game, FluentIcon::Help, FluentIcon::Hide,
        FluentIcon::Home, FluentIcon::Info, FluentIcon::Leaf, FluentIcon::Link,
        FluentIcon::Mail, FluentIcon::Menu, FluentIcon::Mute, FluentIcon::More,
        FluentIcon::Move, FluentIcon::Play, FluentIcon::Save, FluentIcon::Send,
        FluentIcon::Sync, FluentIcon::Unit, FluentIcon::View, FluentIcon::Wifi,
        FluentIcon::Zoom, FluentIcon::Album, FluentIcon::Brush, FluentIcon::Broom,
        FluentIcon::Close, FluentIcon::Cloud, FluentIcon::Embed, FluentIcon::Globe,
        FluentIcon::Heart, FluentIcon::Label, FluentIcon::Media, FluentIcon::Movie,
        FluentIcon::Music, FluentIcon::Robot, FluentIcon::Pause, FluentIcon::Paste,
        FluentIcon::Photo, FluentIcon::Phone, FluentIcon::Print, FluentIcon::Share,
        FluentIcon::Tiles, FluentIcon::Unpin, FluentIcon::Video, FluentIcon::Train,
        FluentIcon::AddTo, FluentIcon::Accept, FluentIcon::Camera, FluentIcon::Cancel,
        FluentIcon::Delete, FluentIcon::Folder, FluentIcon::Filter, FluentIcon::Market,
        FluentIcon::Scroll, FluentIcon::Layout, FluentIcon::GitHub, FluentIcon::Update,
        FluentIcon::Remove, FluentIcon::Return, FluentIcon::People, FluentIcon::QRCode,
        FluentIcon::Ringer, FluentIcon::Rotate, FluentIcon::Search, FluentIcon::Volume,
        FluentIcon::Frigid, FluentIcon::SaveAs, FluentIcon::ZoomIn, FluentIcon::Connect,
        FluentIcon::History, FluentIcon::Setting, FluentIcon::Palette, FluentIcon::Message,
        FluentIcon::FitPage, FluentIcon::ZoomOut, FluentIcon::Airplane, FluentIcon::Asterisk,
        FluentIcon::Calories, FluentIcon::Calendar, FluentIcon::Feedback, FluentIcon::Library,
        FluentIcon::Minimize, FluentIcon::Checkbox, FluentIcon::Document, FluentIcon::Language,
        FluentIcon::Download, FluentIcon::Question, FluentIcon::Speakers, FluentIcon::DateTime,
        FluentIcon::FontSize, FluentIcon::HomeFill, FluentIcon::PageLeft, FluentIcon::SaveCopy,
        FluentIcon::SendFill, FluentIcon::SkipBack, FluentIcon::SpeedOff, FluentIcon::Alignment,
        FluentIcon::Bluetooth, FluentIcon::Completed, FluentIcon::Constract, FluentIcon::Headphone,
        FluentIcon::Megaphone, FluentIcon::Projector, FluentIcon::Education, FluentIcon::LeftArrow,
        FluentIcon::EraseTool, FluentIcon::PageRight, FluentIcon::PlaySolid, FluentIcon::BookShelf,
        FluentIcon::Hightlight, FluentIcon::FolderAdd, FluentIcon::PauseBold, FluentIcon::PencilInk,
        FluentIcon::PieSingle, FluentIcon::QuickNote, FluentIcon::SpeedHigh, FluentIcon::StopWatch,
        FluentIcon::ZipFolder, FluentIcon::Basketball, FluentIcon::Brightness, FluentIcon::Dictionary,
        FluentIcon::Microphone, FluentIcon::ArrowDown, FluentIcon::FullScreen, FluentIcon::MixVolumes,
        FluentIcon::RemoveFrom, FluentIcon::RightArrow, FluentIcon::QuietHours, FluentIcon::Fingerprint,
        FluentIcon::Application, FluentIcon::Certificate, FluentIcon::Transparent, FluentIcon::ImageExport,
        FluentIcon::SpeedMedium, FluentIcon::LibraryFill, FluentIcon::MusicFolder, FluentIcon::PowerButton,
        FluentIcon::SkipForward, FluentIcon::CareUpSolid, FluentIcon::AcceptMedium, FluentIcon::CancelMedium,
        FluentIcon::ChevronRight, FluentIcon::ClippingTool, FluentIcon::SearchMirror, FluentIcon::ShoppingCart,
        FluentIcon::FontIncrease, FluentIcon::BackToWindow, FluentIcon::CommandPrompt, FluentIcon::CloudDownload,
        FluentIcon::DictionaryAdd, FluentIcon::CareDownSolid, FluentIcon::CareLeftSolid, FluentIcon::ClearSelection,
        FluentIcon::DeveloperTools, FluentIcon::BackgroundFill, FluentIcon::CareRightSolid, FluentIcon::ChevronDownMed,
        FluentIcon::ChevronRightMed, FluentIcon::EmojiTabSymbols, FluentIcon::ExpressiveInputEntry,
    };
    static const int iconCount = sizeof(allIcons) / sizeof(allIcons[0]);

    auto *container = new QWidget(page);
    container->setObjectName(QStringLiteral("iconCardView"));
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(12);

    auto *titleLabel = new StrongBodyLabel(tx("IconCardView", "Fluent Icons Library"));
    containerLayout->addWidget(titleLabel);

    auto *searchEdit = new SearchLineEdit(page);
    searchEdit->setPlaceholderText(tx("LineEdit", "Search icons"));
    searchEdit->setFixedWidth(304);
    containerLayout->addWidget(searchEdit);

    auto *viewArea = new QFrame(page);
    viewArea->setObjectName(QStringLiteral("iconView"));
    viewArea->setAttribute(Qt::WA_StyledBackground, true);
    viewArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *viewLayout = new QHBoxLayout(viewArea);
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(0);

    // ScrollArea with icon grid
    auto *scrollArea = new IconScrollArea;
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *scrollWidget = new QWidget(scrollArea);
    scrollWidget->setObjectName(QStringLiteral("scrollWidget"));
    scrollWidget->setAttribute(Qt::WA_StyledBackground, true);
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *flowLayout = new FlowLayout(0, 8, 8, scrollWidget, false, true);
    flowLayout->setContentsMargins(8, 3, 8, 8);
    scrollWidget->setLayout(flowLayout);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setViewportMargins(0, 5, 0, 5);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->enableTransparentBackground();
    viewLayout->addWidget(scrollArea);

    // Info panel on the right
    auto *infoPanel = new IconInfoPanel(allIcons[0], page);

    viewLayout->addWidget(infoPanel, 0, Qt::AlignRight);
    containerLayout->addWidget(viewArea, 1);

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
        const QString key = text.toLower();
        for (auto *card : iconCards) {
            const QString name = card->property("iconName").toString().toLower();
            card->setVisible(key.isEmpty() || name.startsWith(key));
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

    page->contentLayout()->addWidget(container, 1);

    return page;
}
