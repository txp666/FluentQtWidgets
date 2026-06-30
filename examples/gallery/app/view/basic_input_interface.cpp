#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtCore/QCoreApplication>

using namespace FluentQt;

QWidget *GalleryWindow::createBasicInputPage()
{
#define B_TR(s) QCoreApplication::translate("BasicInputInterface", s)

    auto *page = new GalleryInterface(navTx("Basic input"),
                                      QStringLiteral("qfluentwidgets.components.widgets"), this);
    const QString buttonSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/basic_input/button/main.cpp");
    const QString checkBoxSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/basic_input/check_box/main.cpp");
    const QString comboBoxSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/basic_input/combo_box/main.cpp");
    const QString radioButtonSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/basic_input/radio_button/main.cpp");
    const QString sliderSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/basic_input/slider/main.cpp");
    const QString switchButtonSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/basic_input/switch_button/main.cpp");

    // -- Push buttons --
    page->addExampleCard(B_TR("A simple button with text content"),
                         new PushButton(B_TR("Standard push button")), buttonSource);

    auto *toolBtn = new ToolButton(QStringLiteral(":/gallery/images/kunkun.png"));
    toolBtn->setIconSize(QSize(40, 40));
    toolBtn->resize(70, 70);
    page->addExampleCard(B_TR("A button with graphical content"), toolBtn, buttonSource);

    page->addExampleCard(B_TR("Accent style applied to push button"),
                         new PrimaryPushButton(B_TR("Accent style button")), buttonSource);

    page->addExampleCard(B_TR("Accent style applied to tool button"),
                         new PrimaryToolButton(icon(FluentIcon::Basketball)), buttonSource);

    // -- Pill buttons --
    page->addExampleCard(B_TR("Pill push button"),
                         new PillPushButton(icon(FluentIcon::Tag), B_TR("Tag")), buttonSource);

    page->addExampleCard(B_TR("Pill tool button"),
                         new PillToolButton(icon(FluentIcon::Basketball)), buttonSource);

    // -- Transparent buttons --
    page->addExampleCard(B_TR("A transparent push button"),
                         new TransparentPushButton(icon(FluentIcon::BookShelf),
                                                   B_TR("Transparent push button")), buttonSource);

    page->addExampleCard(B_TR("A transparent tool button"),
                         new TransparentToolButton(icon(FluentIcon::BookShelf)), buttonSource);

    // -- Check boxes --
    page->addExampleCard(B_TR("A 2-state CheckBox"),
                         new CheckBox(B_TR("Two-state CheckBox")), checkBoxSource);

    auto *triCheck = new CheckBox(B_TR("Three-state CheckBox"));
    triCheck->setTristate(true);
    page->addExampleCard(B_TR("A 3-state CheckBox"), triCheck, checkBoxSource);

    // -- ComboBox --
    auto *combo1 = new ComboBox;
    combo1->addItems({QStringLiteral("shoko 🥰"), QStringLiteral("西宫硝子 😊"),
                      QStringLiteral("一级棒卡哇伊的硝子酱 😘")});
    combo1->setCurrentIndex(0);
    combo1->setMinimumWidth(210);
    page->addExampleCard(B_TR("A ComboBox with items"), combo1, comboBoxSource);

    auto *editableCombo = new EditableComboBox;
    editableCombo->addItems({B_TR("Star Platinum"), B_TR("Crazy Diamond"),
                             B_TR("Gold Experience"), B_TR("Sticky Fingers")});
    editableCombo->setPlaceholderText(B_TR("Choose your stand"));
    editableCombo->setMinimumWidth(210);
    page->addExampleCard(B_TR("An editable ComboBox"), editableCombo, comboBoxSource);

    // -- Drop down buttons --
    {
        auto *menu = new RoundMenu(page);
        menu->addAction(FluentIcon::Send, B_TR("Send"));
        menu->addAction(FluentIcon::Save, B_TR("Save"));

        auto *ddPush = new DropDownPushButton(icon(FluentIcon::Mail), B_TR("Email"));
        ddPush->setDropDownMenu(menu);
        page->addExampleCard(B_TR("A push button with drop down menu"), ddPush, buttonSource);

        auto *ddTool = new DropDownToolButton(icon(FluentIcon::Mail));
        ddTool->setDropDownMenu(menu);
        page->addExampleCard(B_TR("A tool button with drop down menu"), ddTool, buttonSource);

        auto *primaryDDPush = new PrimaryDropDownPushButton(icon(FluentIcon::Mail), B_TR("Email"));
        primaryDDPush->setDropDownMenu(menu);
        page->addExampleCard(B_TR("A primary color push button with drop down menu"), primaryDDPush,
                             buttonSource);

        auto *primaryDDTool = new PrimaryDropDownToolButton(icon(FluentIcon::Mail));
        primaryDDTool->setDropDownMenu(menu);
        page->addExampleCard(B_TR("A primary color tool button with drop down menu"), primaryDDTool,
                             buttonSource);

        auto *transDDPush = new TransparentDropDownPushButton(icon(FluentIcon::Mail), B_TR("Email"));
        transDDPush->setDropDownMenu(menu);
        page->addExampleCard(B_TR("A transparent push button with drop down menu"), transDDPush,
                             buttonSource);

        auto *transDDTool = new TransparentDropDownToolButton(icon(FluentIcon::Mail));
        transDDTool->setDropDownMenu(menu);
        page->addExampleCard(B_TR("A transparent tool button with drop down menu"), transDDTool,
                             buttonSource);
    }

    // -- Hyperlink button --
    page->addExampleCard(B_TR("A hyperlink button that navigates to a URI"),
                         new HyperlinkButton(icon(FluentIcon::Link),
                                             QStringLiteral(FQW_REPOSITORY_URL),
                                             QStringLiteral("GitHub")),
                         buttonSource);

    // -- Radio button group --
    {
        auto *radioWidget = new QWidget;
        auto *radioLayout = new QVBoxLayout(radioWidget);
        radioLayout->setContentsMargins(2, 0, 0, 0);
        radioLayout->setSpacing(15);
        auto *radio1 = new RadioButton(B_TR("Star Platinum"), radioWidget);
        auto *radio2 = new RadioButton(B_TR("Crazy Diamond"), radioWidget);
        auto *radio3 = new RadioButton(B_TR("Soft and Wet"), radioWidget);
        auto *btnGroup = new QButtonGroup(radioWidget);
        btnGroup->addButton(radio1);
        btnGroup->addButton(radio2);
        btnGroup->addButton(radio3);
        radioLayout->addWidget(radio1);
        radioLayout->addWidget(radio2);
        radioLayout->addWidget(radio3);
        radio1->click();
        page->addExampleCard(B_TR("A group of RadioButton controls in a button group"), radioWidget,
                             radioButtonSource);
    }

    // -- Horizontal slider --
    {
        auto *slider = new Slider(Qt::Horizontal);
        slider->setRange(0, 100);
        slider->setValue(30);
        slider->setMinimumWidth(200);
        page->addExampleCard(B_TR("A simple horizontal slider"), slider, sliderSource);
    }

    // -- Split buttons --
    {
        auto *splitPush = new SplitPushButton(icon(FluentIcon::Basketball), B_TR("Choose your stand"));
        auto *standMenu = new RoundMenu(page);
        auto addStandAction = [standMenu, splitPush](const QString &text) {
            QAction *action = standMenu->addAction(text);
            QObject::connect(action, &QAction::triggered, splitPush, [splitPush, text]() { splitPush->setText(text); });
        };
        addStandAction(B_TR("Star Platinum"));
        addStandAction(B_TR("Crazy Diamond"));
        addStandAction(B_TR("Gold Experience"));
        addStandAction(B_TR("Sticky Fingers"));
        splitPush->setDropDownMenu(standMenu);
        page->addExampleCard(B_TR("A split push button with drop down menu"), splitPush, buttonSource);

        auto *ikunMenu = new RoundMenu(page);
        ikunMenu->addAction(B_TR("Sing"));
        ikunMenu->addAction(B_TR("Jump"));
        ikunMenu->addAction(B_TR("Rap"));
        ikunMenu->addAction(B_TR("Music"));

        auto *splitTool = new SplitToolButton(QStringLiteral(":/gallery/images/kunkun.png"));
        splitTool->setIconSize(QSize(30, 30));
        splitTool->setDropDownMenu(ikunMenu);
        page->addExampleCard(B_TR("A split tool button with drop down menu"), splitTool, buttonSource);

        auto *primarySplitPush = new PrimarySplitPushButton(icon(FluentIcon::Basketball), B_TR("Choose your stand"));
        auto *primaryStandMenu = new RoundMenu(page);
        auto addPrimaryStandAction = [primaryStandMenu, primarySplitPush](const QString &text) {
            QAction *action = primaryStandMenu->addAction(text);
            QObject::connect(action, &QAction::triggered, primarySplitPush,
                             [primarySplitPush, text]() { primarySplitPush->setText(text); });
        };
        addPrimaryStandAction(B_TR("Star Platinum"));
        addPrimaryStandAction(B_TR("Crazy Diamond"));
        addPrimaryStandAction(B_TR("Gold Experience"));
        addPrimaryStandAction(B_TR("Sticky Fingers"));
        primarySplitPush->setDropDownMenu(primaryStandMenu);
        page->addExampleCard(B_TR("A primary color split push button with drop down menu"), primarySplitPush,
                             buttonSource);

        auto *primarySplitTool = new PrimarySplitToolButton(icon(FluentIcon::Basketball));
        primarySplitTool->setDropDownMenu(ikunMenu);
        page->addExampleCard(B_TR("A primary color split tool button with drop down menu"), primarySplitTool,
                             buttonSource);
    }

    // -- SwitchButton --
    {
        auto *switchBtn = new SwitchButton(B_TR("Off"));
        connect(switchBtn, &SwitchButton::checkedChanged, page,
                [switchBtn](bool checked) { switchBtn->setText(checked ? B_TR("On") : B_TR("Off")); });
        page->addExampleCard(B_TR("A simple switch button"), switchBtn, switchButtonSource);
    }

    // -- Toggle buttons --
    page->addExampleCard(B_TR("A simple toggle push button"),
                         new ToggleButton(icon(FluentIcon::Basketball), B_TR("Start practicing")),
                         buttonSource);

    page->addExampleCard(B_TR("A simple toggle tool button"),
                         new ToggleToolButton(icon(FluentIcon::Basketball)), buttonSource);

    page->addExampleCard(B_TR("A transparent toggle push button"),
                         new TransparentTogglePushButton(icon(FluentIcon::Basketball), B_TR("Start practicing")),
                         buttonSource);

    page->addExampleCard(B_TR("A transparent toggle tool button"),
                         new TransparentToggleToolButton(icon(FluentIcon::Basketball)), buttonSource);

#undef B_TR
    return page;
}
