#include "../../components/GalleryComponents.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtGui/QBrush>
#include <QtGui/QDesktopServices>
#include <QtGui/QLinearGradient>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
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

QIcon galleryIcon(const QString &path)
{
    return QIcon(QStringLiteral(":/gallery/images/%1").arg(path));
}

struct HomeSample
{
    const char *iconPath;
    const char *title;
    const char *content;
    const char *routeKey;
    int index;
};

class LinkCard : public QFrame
{
    Q_OBJECT

  public:
    LinkCard(const QIcon &cardIcon, const QString &title, const QString &content, const QUrl &url, QWidget *parent)
        : QFrame(parent), m_url(url)
    {
        setFixedSize(198, 220);
        setAttribute(Qt::WA_StyledBackground, true);
        setCursor(Qt::PointingHandCursor);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 0, 13);
        layout->setSpacing(0);

        auto *iconWidget = new IconWidget(cardIcon, this);
        iconWidget->setFixedSize(54, 54);
        iconWidget->setIconSize(QSize(54, 54));
        auto *titleLabel = new QLabel(title, this);
        titleLabel->setObjectName(QStringLiteral("titleLabel"));
        auto *contentLabel = new QLabel(content, this);
        contentLabel->setObjectName(QStringLiteral("contentLabel"));
        contentLabel->setWordWrap(true);

        layout->addWidget(iconWidget);
        layout->addSpacing(16);
        layout->addWidget(titleLabel);
        layout->addSpacing(8);
        layout->addWidget(contentLabel);
        layout->addStretch(1);

        auto *urlWidget = new IconWidget(FluentIcon::Link, this);
        urlWidget->setFixedSize(16, 16);
        urlWidget->move(170, 192);
    }

  protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        QFrame::mouseReleaseEvent(event);
        if (event->button() == Qt::LeftButton) {
            QDesktopServices::openUrl(m_url);
        }
    }

  private:
    QUrl m_url;
};

class LinkCardView : public SingleDirectionScrollArea
{
  public:
    explicit LinkCardView(QWidget *parent = nullptr) : SingleDirectionScrollArea(Qt::Horizontal, parent)
    {
        auto *view = new QWidget(this);
        view->setObjectName(QStringLiteral("view"));
        m_layout = new QHBoxLayout(view);
        m_layout->setContentsMargins(36, 0, 0, 0);
        m_layout->setSpacing(12);
        m_layout->setAlignment(Qt::AlignLeft);
        setWidget(view);
        setWidgetResizable(true);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        if (horizontalFluentScrollBar()) {
            horizontalFluentScrollBar()->setForceHidden(true);
        }
    }

    void addCard(const QIcon &cardIcon, const QString &title, const QString &content, const QUrl &url)
    {
        m_layout->addWidget(new LinkCard(cardIcon, title, content, url, widget()), 0, Qt::AlignLeft);
    }

  private:
    QHBoxLayout *m_layout = nullptr;
};

class SampleCard : public CardWidget
{
    Q_OBJECT

  public:
    SampleCard(const QIcon &cardIcon, const QString &title, const QString &content, const QString &routeKey,
               int index, QWidget *parent)
        : CardWidget(parent), m_routeKey(routeKey), m_index(index)
    {
        setFixedSize(360, 90);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        auto *iconWidget = new IconWidget(cardIcon, this);
        iconWidget->setFixedSize(48, 48);
        iconWidget->setIconSize(QSize(48, 48));
        auto *titleLabel = new QLabel(title, this);
        titleLabel->setObjectName(QStringLiteral("titleLabel"));
        auto *contentLabel = new QLabel(content, this);
        contentLabel->setObjectName(QStringLiteral("contentLabel"));
        contentLabel->setWordWrap(true);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(20, 0, 0, 0);
        layout->setSpacing(28);
        auto *textLayout = new QVBoxLayout;
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(2);
        textLayout->addStretch(1);
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(contentLabel);
        textLayout->addStretch(1);
        layout->addWidget(iconWidget);
        layout->addLayout(textLayout);

        setClickEnabled(true);
        connect(this, &CardWidget::clicked, this, [this]() {
            emit activated(m_routeKey, m_index);
        });
    }

    QString routeKey() const { return m_routeKey; }
    int index() const { return m_index; }

  signals:
    void activated(const QString &routeKey, int index);

  private:
    QString m_routeKey;
    int m_index;
};

class SampleCardView : public QWidget
{
    Q_OBJECT

  public:
    explicit SampleCardView(const QString &title, QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(36, 0, 36, 0);
        layout->setSpacing(10);

        auto *titleLabel = new QLabel(title, this);
        titleLabel->setObjectName(QStringLiteral("viewTitleLabel"));
        m_flowLayout = new FlowLayout(0, 12, 12);

        layout->addWidget(titleLabel);
        layout->addLayout(m_flowLayout, 1);
    }

    SampleCard *addSampleCard(const QIcon &cardIcon, const QString &title, const QString &content,
                              const QString &routeKey, int index)
    {
        auto *card = new SampleCard(cardIcon, title, content, routeKey, index, this);
        m_flowLayout->addWidget(card);
        return card;
    }

  private:
    FlowLayout *m_flowLayout = nullptr;
};

class BannerWidget : public QWidget
{
  public:
    explicit BannerWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedHeight(336);
        setAttribute(Qt::WA_StyledBackground, true);
        m_banner = QPixmap(QStringLiteral(":/gallery/images/header1.png"));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 20, 0, 0);
        layout->setSpacing(0);

        auto *galleryLabel = new QLabel(QCoreApplication::translate("HomeInterface", "Fluent Gallery"), this);
        galleryLabel->setObjectName(QStringLiteral("galleryLabel"));
        m_linkCardView = new LinkCardView(this);

        layout->addWidget(galleryLabel);
        layout->addWidget(m_linkCardView, 1, Qt::AlignBottom);
        layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        m_linkCardView->addCard(galleryIcon(QStringLiteral("logo.png")),
                                QCoreApplication::translate("HomeInterface", "Getting started"),
                                QCoreApplication::translate("HomeInterface",
                                                            "An overview of app development options and samples."),
                                QUrl(QStringLiteral(FQW_REPOSITORY_URL "/blob/main/docs/quick-start.md")));
        m_linkCardView->addCard(icon(FluentIcon::GitHub),
                                QCoreApplication::translate("HomeInterface", "GitHub repo"),
                                QCoreApplication::translate("HomeInterface",
                                                            "The latest fluent design controls and styles for your "
                                                            "applications."),
                                QUrl(QStringLiteral(FQW_REPOSITORY_URL)));
        m_linkCardView->addCard(icon(FluentIcon::Code),
                                QCoreApplication::translate("HomeInterface", "Code samples"),
                                QCoreApplication::translate("HomeInterface",
                                                            "Find samples that demonstrate specific tasks, features and "
                                                            "APIs."),
                                QUrl(QStringLiteral(FQW_REPOSITORY_URL "/tree/main/examples")));
        m_linkCardView->addCard(icon(FluentIcon::Feedback),
                                QCoreApplication::translate("HomeInterface", "Send feedback"),
                                QCoreApplication::translate("HomeInterface",
                                                            "Help us improve FluentQtWidgets by providing "
                                                            "feedback."),
                                QUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues")));
    }

  protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.setPen(Qt::NoPen);

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        const int w = width();
        const int h = height();
        path.addRoundedRect(QRectF(0, 0, w, h), 10, 10);
        path.addRect(QRectF(0, h - 50, 50, 50));
        path.addRect(QRectF(w - 50, 0, 50, 50));
        path.addRect(QRectF(w - 50, h - 50, 50, 50));
        path = path.simplified();

        QLinearGradient gradient(0, 0, 0, h);
        if (ThemeManager::instance()->effectiveTheme() == Theme::Dark) {
            gradient.setColorAt(0, QColor(0, 0, 0, 255));
            gradient.setColorAt(1, QColor(0, 0, 0, 0));
        } else {
            gradient.setColorAt(0, QColor(207, 216, 228, 255));
            gradient.setColorAt(1, QColor(207, 216, 228, 0));
        }
        painter.fillPath(path, gradient);

        if (!m_banner.isNull()) {
            painter.fillPath(path, QBrush(m_banner.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        }
    }

  private:
    LinkCardView *m_linkCardView = nullptr;
    QPixmap m_banner;
};

} // namespace

HomeInterface::HomeInterface(QWidget *parent) : ScrollArea(parent)
{
    setObjectName(QStringLiteral("homeInterface"));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (horizontalFluentScrollBar()) {
        horizontalFluentScrollBar()->setForceHidden(true);
    }

    m_view = new QWidget(this);
    m_view->setObjectName(QStringLiteral("view"));
    m_viewLayout = new QVBoxLayout(m_view);
    m_viewLayout->setContentsMargins(0, 0, 0, 36);
    m_viewLayout->setSpacing(40);
    m_viewLayout->setAlignment(Qt::AlignTop);

    m_banner = new BannerWidget(m_view);
    m_viewLayout->addWidget(m_banner);

    const auto addSection = [this](SampleCardView *view, std::initializer_list<HomeSample> samples) {
        for (const HomeSample &sample : samples) {
            auto *card = view->addSampleCard(galleryIcon(QString::fromUtf8(sample.iconPath)),
                                             QCoreApplication::translate("HomeInterface", sample.title),
                                             QCoreApplication::translate("HomeInterface", sample.content),
                                             QString::fromUtf8(sample.routeKey),
                                             sample.index);
            connect(card, &SampleCard::activated, this, &HomeInterface::sampleCardClicked);
        }
        m_viewLayout->addWidget(view);
        applyGalleryStyle(view, QStringLiteral("sample_card"));
    };

    auto *basicInputView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Basic input samples"), m_view);
    addSection(basicInputView, {
                                   {"controls/Button.png", "Button",
                                    "A control that responds to user input and emit clicked signal.",
                                    "basicInputInterface", 0},
                                   {"controls/Checkbox.png", "CheckBox",
                                    "A control that a user can select or clear.",
                                    "basicInputInterface", 8},
                                   {"controls/ComboBox.png", "ComboBox",
                                    "A drop-down list of items a user can select from.",
                                    "basicInputInterface", 10},
                                   {"controls/DropDownButton.png", "DropDownButton",
                                    "A button that displays a flyout of choices when clicked.",
                                    "basicInputInterface", 12},
                                   {"controls/HyperlinkButton.png", "HyperlinkButton",
                                    "A button that appears as hyperlink text, and can navigate to a URI or handle a "
                                    "Click event.",
                                    "basicInputInterface", 18},
                                   {"controls/RadioButton.png", "RadioButton",
                                    "A control that allows a user to select a single option from a group of options.",
                                    "basicInputInterface", 19},
                                   {"controls/Slider.png", "Slider",
                                    "A control that lets the user select from a range of values by moving a Thumb "
                                    "control along a track.",
                                    "basicInputInterface", 20},
                                   {"controls/SplitButton.png", "SplitButton",
                                    "A two-part button that displays a flyout when its secondary part is clicked.",
                                    "basicInputInterface", 21},
                                   {"controls/ToggleSwitch.png", "SwitchButton",
                                    "A switch that can be toggled between 2 states.",
                                    "basicInputInterface", 25},
                                   {"controls/ToggleButton.png", "ToggleButton",
                                    "A button that can be switched between two states like a CheckBox.",
                                    "basicInputInterface", 26},
                               });

    auto *dateTimeView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Date & time samples"), m_view);
    addSection(dateTimeView, {
                                 {"controls/CalendarDatePicker.png", "CalendarPicker",
                                  "A control that lets a user pick a date value using a calendar.",
                                  "dateTimeInterface", 0},
                                 {"controls/DatePicker.png", "DatePicker",
                                  "A control that lets a user pick a date value.",
                                  "dateTimeInterface", 2},
                                 {"controls/TimePicker.png", "TimePicker",
                                  "A configurable control that lets a user pick a time value.",
                                  "dateTimeInterface", 4},
                             });

    auto *dialogView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Dialog samples"), m_view);
    addSection(dialogView, {
                               {"controls/Flyout.png", "Dialog",
                                "A frameless message dialog.",
                                "dialogInterface", 0},
                               {"controls/ContentDialog.png", "MessageBox",
                                "A message dialog with mask.",
                                "dialogInterface", 1},
                               {"controls/ColorPicker.png", "ColorDialog",
                                "A dialog that allows user to select color.",
                                "dialogInterface", 2},
                               {"controls/Flyout.png", "Flyout",
                                "Shows contextual information and enables user interaction.",
                                "dialogInterface", 3},
                               {"controls/TeachingTip.png", "TeachingTip",
                                "A content-rich flyout for guiding users and enabling teaching moments.",
                                "dialogInterface", 5},
                           });

    auto *layoutView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Layout samples"), m_view);
    addSection(layoutView, {
                               {"controls/Grid.png", "FlowLayout",
                                "A layout arranges components in a left-to-right flow, wrapping to the next row when "
                                "the current row is full.",
                                "layoutInterface", 0},
                           });

    auto *materialView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Material samples"), m_view);
    addSection(materialView, {
                                 {"controls/Acrylic.png", "AcrylicLabel",
                                  "A translucent material recommended for panel background.",
                                  "materialInterface", 0},
                             });

    auto *menuView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Menu & toolbars samples"), m_view);
    addSection(menuView, {
                             {"controls/MenuFlyout.png", "RoundMenu",
                              "Shows a contextual list of simple commands or options.",
                              "menuInterface", 0},
                             {"controls/CommandBar.png", "CommandBar",
                              "Shows a contextual list of simple commands or options.",
                              "menuInterface", 2},
                             {"controls/CommandBarFlyout.png", "CommandBarFlyout",
                              "A mini-toolbar displaying proactive commands, and an optional menu of commands.",
                              "menuInterface", 3},
                         });

    auto *navigationView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Navigation"), m_view);
    addSection(navigationView, {
                                   {"controls/BreadcrumbBar.png", "BreadcrumbBar",
                                    "Shows the trail of navigation taken to the current location.",
                                    "navigationViewInterface", 0},
                                   {"controls/Pivot.png", "Pivot",
                                    "Presents information from different sources in a tabbed view.",
                                    "navigationViewInterface", 1},
                                   {"controls/TabView.png", "TabView",
                                    "Presents information from different sources in a tabbed view.",
                                    "navigationViewInterface", 3},
                               });

    auto *scrollView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Scrolling samples"), m_view);
    addSection(scrollView, {
                               {"controls/ScrollViewer.png", "ScrollArea",
                                "A container control that lets the user pan and zoom its content smoothly.",
                                "scrollInterface", 0},
                               {"controls/PipsPager.png", "PipsPager",
                                "A control to let the user navigate through a paginated collection when the page "
                                "numbers do not need to be visually known.",
                                "scrollInterface", 3},
                           });

    auto *statusInfoView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Status & info samples"), m_view);
    addSection(statusInfoView, {
                                   {"controls/ProgressRing.png", "StateToolTip",
                                    "Shows the apps progress on a task, or that the app is performing ongoing work "
                                    "that does block user interaction.",
                                    "statusInfoInterface", 0},
                                   {"controls/InfoBadge.png", "InfoBadge",
                                    "An non-intrusive Ul to display notifications or bring focus to an area.",
                                    "statusInfoInterface", 3},
                                   {"controls/InfoBar.png", "InfoBar",
                                    "An inline message to display app-wide status change information.",
                                    "statusInfoInterface", 4},
                                   {"controls/ProgressBar.png", "ProgressBar",
                                    "Shows the apps progress on a task, or that the app is performing ongoing work "
                                    "that doesn't block user interaction.",
                                    "statusInfoInterface", 8},
                                   {"controls/ProgressRing.png", "ProgressRing",
                                    "Shows the apps progress on a task, or that the app is performing ongoing work "
                                    "that doesn't block user interaction.",
                                    "statusInfoInterface", 10},
                                   {"controls/ToolTip.png", "ToolTip",
                                    "Displays information for an element in a pop-up window.",
                                    "statusInfoInterface", 1},
                               });

    auto *textView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "Text samples"), m_view);
    addSection(textView, {
                             {"controls/TextBox.png", "LineEdit",
                              "A single-line plain text field.",
                              "textInterface", 0},
                             {"controls/PasswordBox.png", "PasswordLineEdit",
                              "A control for entering passwords.",
                              "textInterface", 2},
                             {"controls/NumberBox.png", "SpinBox",
                              "A text control used for numeric input and evaluation of algebraic equations.",
                              "textInterface", 3},
                             {"controls/RichEditBox.png", "TextEdit",
                              "A rich text editing control that supports formatted text, hyperlinks, and other rich "
                              "content.",
                              "textInterface", 8},
                         });

    auto *viewsView = new SampleCardView(
        QCoreApplication::translate("HomeInterface", "View samples"), m_view);
    addSection(viewsView, {
                              {"controls/ListView.png", "ListView",
                               "A control that presents a collection of items in a vertical list.",
                               "viewInterface", 0},
                              {"controls/DataGrid.png", "TableView",
                               "The DataGrid control provides a flexible way to display a collection of data in rows "
                               "and columns.",
                               "viewInterface", 1},
                              {"controls/TreeView.png", "TreeView",
                               "The TreeView control is a hierarchical list pattern with expanding and collapsing "
                               "nodes that contain nested items.",
                               "viewInterface", 2},
                              {"controls/FlipView.png", "FlipView",
                               "Presents a collection of items that the user can flip through,one item at a time.",
                               "viewInterface", 4},
                          });

    setWidget(m_view);
    setWidgetResizable(true);
    applyGalleryStyle(this, QStringLiteral("home_interface"));
    applyGalleryStyle(m_banner, QStringLiteral("link_card"));
    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) {
        applyGalleryStyle(this, QStringLiteral("home_interface"));
        applyGalleryStyle(m_banner, QStringLiteral("link_card"));
        for (auto *view : m_view->findChildren<SampleCardView *>(QString(), Qt::FindDirectChildrenOnly)) {
            applyGalleryStyle(view, QStringLiteral("sample_card"));
        }
        if (m_banner) {
            m_banner->update();
        }
    });
    connect(ThemeManager::instance(), &ThemeManager::accentColorChanged, this, [this](const QColor &) {
        applyGalleryStyle(this, QStringLiteral("home_interface"));
        applyGalleryStyle(m_banner, QStringLiteral("link_card"));
        for (auto *view : m_view->findChildren<SampleCardView *>(QString(), Qt::FindDirectChildrenOnly)) {
            applyGalleryStyle(view, QStringLiteral("sample_card"));
        }
    });
}

void HomeInterface::resizeEvent(QResizeEvent *event)
{
    ScrollArea::resizeEvent(event);
    if (m_banner) {
        m_banner->setFixedWidth(width());
    }
}

#include "home_interface.moc"
