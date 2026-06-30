#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

QWidget *GalleryWindow::createNavigationPage()
{
    auto *page = new GalleryInterface(navTx("Navigation"),
                                      QStringLiteral("FluentQtWidgets::Navigation"), this);
    const QString breadcrumbSource = exampleSourceUrl("navigation/breadcrumb_bar");
    const QString pivotSource = exampleSourceUrl("navigation/pivot");
    const QString segmentedWidgetSource = exampleSourceUrl("navigation/segmented_widget");
    const QString segmentedToolWidgetSource = exampleSourceUrl("navigation/segmented_tool_widget");
    const QString tabViewSource = exampleSourceUrl("navigation/tab_view");

    auto *breadcrumb = new BreadcrumbBar;
    const QStringList items = {
        QStringLiteral("Home"),          QStringLiteral("Documents"), QStringLiteral("Study"),
        QStringLiteral("Janpanese Sensei"), QStringLiteral("Action Film"), QStringLiteral("G Cup"),
        QStringLiteral("Mikami Yua"),    QStringLiteral("Folder1"),   QStringLiteral("Folder2"),
    };
    for (const QString &item : items) {
        breadcrumb->addItem(item, item);
    }
    page->addExampleCard(tx("NavigationViewInterface", "Breadcrumb bar"), breadcrumb, breadcrumbSource, 1);

    const auto createNavInterface = [](Pivot *nav) {
        auto *widget = new QWidget;
        widget->setFixedSize(300, 140);
        auto *stack = new QStackedWidget(widget);
        auto *layout = new QVBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(nav, 0, Qt::AlignLeft);
        layout->addWidget(stack);

        const QList<QPair<QString, QString>> tabs = {
            {QStringLiteral("songInterface"), tx("PivotInterface", "Song")},
            {QStringLiteral("albumInterface"), tx("PivotInterface", "Album")},
            {QStringLiteral("artistInterface"), tx("PivotInterface", "Artist")}};
        for (const auto &tab : tabs) {
            auto *label = new QLabel(tx("NavigationViewInterface", "%1 Interface").arg(tab.second), widget);
            label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            stack->addWidget(label);
            nav->addItem(tab.first, tab.second);
        }

        QObject::connect(nav, &Pivot::currentItemChanged, stack, [stack, tabs](const QString &routeKey) {
            for (int i = 0; i < tabs.size(); ++i) {
                if (tabs.at(i).first == routeKey) {
                    stack->setCurrentIndex(i);
                    return;
                }
            }
        });
        nav->setCurrentItem(QStringLiteral("songInterface"));
        return widget;
    };

    page->addExampleCard(tx("NavigationViewInterface", "A basic pivot"), createNavInterface(new Pivot), pivotSource);
    page->addExampleCard(tx("NavigationViewInterface", "A segmented control"), createNavInterface(new SegmentedWidget),
                         segmentedWidgetSource);

    auto *segmentedTools = new SegmentedToggleToolWidget;
    segmentedTools->addItem(QStringLiteral("k1"), icon(FluentIcon::Search));
    segmentedTools->addItem(QStringLiteral("k2"), icon(FluentIcon::Accept));
    segmentedTools->addItem(QStringLiteral("k3"), icon(FluentIcon::Setting));
    segmentedTools->setCurrentItem(QStringLiteral("k1"));
    page->addExampleCard(tx("NavigationViewInterface", "Another segmented control"), segmentedTools, segmentedToolWidgetSource);

    auto *tabInterface = new QWidget;
    tabInterface->setFixedHeight(280);
    auto *tabMainLayout = new QHBoxLayout(tabInterface);
    tabMainLayout->setContentsMargins(0, 0, 0, 0);

    auto *tabView = new QWidget(tabInterface);
    auto *tabViewLayout = new QVBoxLayout(tabView);
    tabViewLayout->setContentsMargins(0, 0, 0, 0);
    auto *tabBar = new TabBar(tabView);
    tabBar->setTabMaximumWidth(200);
    auto *tabStack = new QStackedWidget(tabView);
    tabViewLayout->addWidget(tabBar);
    tabViewLayout->addWidget(tabStack);

    auto *controlPanel = new QFrame(tabInterface);
    controlPanel->setObjectName(QStringLiteral("controlPanel"));
    controlPanel->setFixedWidth(220);
    auto *panelLayout = new QVBoxLayout(controlPanel);
    panelLayout->setSpacing(8);
    panelLayout->setContentsMargins(14, 16, 14, 14);
    panelLayout->setAlignment(Qt::AlignTop);

    auto *movableCheckBox = new CheckBox(tx("TabInterface", "IsTabMovable"), controlPanel);
    auto *scrollableCheckBox = new CheckBox(tx("TabInterface", "IsTabScrollable"), controlPanel);
    auto *shadowEnabledCheckBox = new CheckBox(tx("TabInterface", "IsTabShadowEnabled"), controlPanel);
    shadowEnabledCheckBox->setChecked(true);
    auto *tabMaxWidthLabel = new BodyLabel(tx("TabInterface", "TabMaximumWidth"), controlPanel);
    auto *tabMaxWidthSpinBox = new SpinBox(controlPanel);
    tabMaxWidthSpinBox->setRange(60, 400);
    tabMaxWidthSpinBox->setValue(tabBar->tabMaximumWidth());
    auto *closeDisplayModeLabel = new BodyLabel(tx("TabInterface", "TabCloseButtonDisplayMode"), controlPanel);
    auto *closeDisplayModeComboBox = new ComboBox(controlPanel);
    closeDisplayModeComboBox->addItems({tx("TabInterface", "Always"), tx("TabInterface", "OnHover"), tx("TabInterface", "Never")});

    panelLayout->addWidget(movableCheckBox);
    panelLayout->addWidget(scrollableCheckBox);
    panelLayout->addWidget(shadowEnabledCheckBox);
    panelLayout->addSpacing(4);
    panelLayout->addWidget(tabMaxWidthLabel);
    panelLayout->addWidget(tabMaxWidthSpinBox);
    panelLayout->addSpacing(4);
    panelLayout->addWidget(closeDisplayModeLabel);
    panelLayout->addWidget(closeDisplayModeComboBox);

    tabMainLayout->addWidget(tabView, 1);
    tabMainLayout->addWidget(controlPanel, 0, Qt::AlignRight);

    const auto addTabPage = [tabBar, tabStack](const QString &routeKey, const QString &text, const QString &tabIconPath) {
        auto *label = new QLabel(tx("NavigationViewInterface", "%1 Interface").arg(text), tabStack);
        label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        tabStack->addWidget(label);
        return tabBar->addTab(routeKey, text, QIcon(tabIconPath));
    };
    addTabPage(QStringLiteral("tabSongInterface"), tx("TabInterface", "Song"), QStringLiteral(":/gallery/images/MusicNote.png"));
    addTabPage(QStringLiteral("tabAlbumInterface"), tx("TabInterface", "Album"), QStringLiteral(":/gallery/images/Dvd.png"));
    addTabPage(QStringLiteral("tabArtistInterface"), tx("TabInterface", "Artist"), QStringLiteral(":/gallery/images/Singer.png"));
    tabBar->setCurrentTab(QStringLiteral("tabSongInterface"));

    connect(movableCheckBox, &QAbstractButton::toggled, tabBar, &TabBar::setMovable);
    connect(scrollableCheckBox, &QAbstractButton::toggled, tabBar, &TabBar::setScrollable);
    connect(shadowEnabledCheckBox, &QAbstractButton::toggled, tabBar, &TabBar::setTabShadowEnabled);
    connect(tabMaxWidthSpinBox, QOverload<int>::of(&SpinBox::valueChanged), tabBar, &TabBar::setTabMaximumWidth);
    connect(closeDisplayModeComboBox, &ComboBox::currentIndexChanged, tabBar, [tabBar](int index) {
        const auto mode = index == 1   ? TabCloseButtonDisplayMode::OnHover
                          : index == 2 ? TabCloseButtonDisplayMode::Never
                                       : TabCloseButtonDisplayMode::Always;
        tabBar->setCloseButtonDisplayMode(mode);
    });
    connect(tabBar, &TabBar::currentChanged, tabStack, [tabStack](int index) {
        if (index >= 0 && index < tabStack->count()) {
            tabStack->setCurrentIndex(index);
        }
    });
    connect(tabBar, &TabBar::tabAddRequested, tabInterface, [tabBar, tabStack, addTabPage, tabCount = 1]() mutable {
        const QString text = tx("TabInterface", "Shoko is super kawaii x%1").arg(tabCount++);
        const int index = addTabPage(text, text, QStringLiteral(":/gallery/images/Smiling_with_heart.png"));
        tabBar->setCurrentIndex(index);
        tabStack->setCurrentIndex(index);
    });
    connect(tabBar, &TabBar::tabCloseRequested, tabInterface, [tabBar, tabStack](int index) {
        if (tabBar->count() <= 1 || index < 0 || index >= tabStack->count()) {
            return;
        }
        QWidget *widget = tabStack->widget(index);
        tabStack->removeWidget(widget);
        tabBar->removeTab(index);
        widget->deleteLater();
    });

    page->addExampleCard(tx("NavigationViewInterface", "A tab bar"), tabInterface, tabViewSource, 1);

    return page;
}
