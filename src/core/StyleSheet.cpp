#include <FluentQtWidgets/StyleSheet.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/Theme.h>

#include "FontUtils_p.h"

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtWidgets/QStyle>
#include <QtWidgets/QWidget>

#include <utility>

namespace FluentQt {

namespace {
constexpr const char *kLightCustomQssProperty = "fqw-light-qss";
constexpr const char *kDarkCustomQssProperty = "fqw-dark-qss";
constexpr const char *kDirtyQssProperty = "fqw-dirty-qss";
constexpr const char *kStyleSourceProperty = "fqw-style-source";

FluentStyleSheetSource sourceFromProperty(const QWidget *widget)
{
    if (!widget) {
        return FluentStyleSheetSource::Global;
    }
    return static_cast<FluentStyleSheetSource>(widget->property(kStyleSourceProperty).toInt());
}
} // namespace

QString styleSheetSourceName(FluentStyleSheetSource source)
{
    switch (source) {
    case FluentStyleSheetSource::Button:
        return QStringLiteral("button");
    case FluentStyleSheetSource::Label:
        return QStringLiteral("label");
    case FluentStyleSheetSource::LineEdit:
        return QStringLiteral("line_edit");
    case FluentStyleSheetSource::CardWidget:
        return QStringLiteral("card_widget");
    case FluentStyleSheetSource::Menu:
        return QStringLiteral("menu");
    case FluentStyleSheetSource::ComboBox:
        return QStringLiteral("combo_box");
    case FluentStyleSheetSource::CheckBox:
        return QStringLiteral("check_box");
    case FluentStyleSheetSource::Slider:
        return QStringLiteral("slider");
    case FluentStyleSheetSource::SpinBox:
        return QStringLiteral("spin_box");
    case FluentStyleSheetSource::SwitchButton:
        return QStringLiteral("switch_button");
    case FluentStyleSheetSource::ScrollArea:
        return QStringLiteral("scroll_bar");
    case FluentStyleSheetSource::TabView:
        return QStringLiteral("tab_view");
    case FluentStyleSheetSource::PipsPager:
        return QStringLiteral("pips_pager");
    case FluentStyleSheetSource::ListView:
        return QStringLiteral("list_view");
    case FluentStyleSheetSource::TableView:
        return QStringLiteral("table_view");
    case FluentStyleSheetSource::TreeView:
        return QStringLiteral("tree_view");
    case FluentStyleSheetSource::InfoBar:
        return QStringLiteral("info_bar");
    case FluentStyleSheetSource::InfoBadge:
        return QStringLiteral("info_badge");
    case FluentStyleSheetSource::ToolTip:
        return QStringLiteral("tool_tip");
    case FluentStyleSheetSource::StateToolTip:
        return QStringLiteral("state_tool_tip");
    case FluentStyleSheetSource::Flyout:
        return QStringLiteral("flyout");
    case FluentStyleSheetSource::TeachingTip:
        return QStringLiteral("teaching_tip");
    case FluentStyleSheetSource::SettingCard:
        return QStringLiteral("setting_card");
    case FluentStyleSheetSource::SettingCardGroup:
        return QStringLiteral("setting_card_group");
    case FluentStyleSheetSource::ColorDialog:
        return QStringLiteral("color_dialog");
    case FluentStyleSheetSource::Dialog:
        return QStringLiteral("dialog");
    case FluentStyleSheetSource::NavigationInterface:
        return QStringLiteral("navigation_interface");
    case FluentStyleSheetSource::FluentWindow:
        return QStringLiteral("fluent_window");
    case FluentStyleSheetSource::Pivot:
        return QStringLiteral("pivot");
    case FluentStyleSheetSource::RadioButton:
        return QStringLiteral("radio_button");
    case FluentStyleSheetSource::ProgressBar:
        return QStringLiteral("progress_bar");
    case FluentStyleSheetSource::IconWidget:
        return QStringLiteral("icon_widget");
    case FluentStyleSheetSource::CalendarPicker:
        return QStringLiteral("calendar_picker");
    case FluentStyleSheetSource::DatePicker:
        return QStringLiteral("date_picker");
    case FluentStyleSheetSource::TimePicker:
        return QStringLiteral("time_picker");
    case FluentStyleSheetSource::MessageDialog:
        return QStringLiteral("message_dialog");
    case FluentStyleSheetSource::FolderListDialog:
        return QStringLiteral("folder_list_dialog");
    case FluentStyleSheetSource::FlipView:
        return QStringLiteral("flip_view");
    case FluentStyleSheetSource::MediaPlayer:
        return QStringLiteral("media_player");
    case FluentStyleSheetSource::ExpandSettingCard:
        return QStringLiteral("expand_setting_card");
    case FluentStyleSheetSource::Global:
        return QStringLiteral("global");
    }
    return QStringLiteral("global");
}

FluentStyleSheetSource styleSheetSourceForRole(const QString &role)
{
    static const QHash<QString, FluentStyleSheetSource> mapping = {
        {QStringLiteral("PushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PrimaryPushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TransparentPushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TogglePushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TransparentTogglePushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("DropDownPushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TransparentDropDownPushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PrimaryDropDownPushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("ToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PrimaryToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TransparentToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("ToggleToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TransparentToggleToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("DropDownToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("TransparentDropDownToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PrimaryDropDownToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("HyperlinkButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PillPushButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PillToolButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("FolderRemoveButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("SplitDropButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("PrimarySplitDropButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("CommandBar"), FluentStyleSheetSource::Button},
        {QStringLiteral("CommandBarView"), FluentStyleSheetSource::Button},
        {QStringLiteral("CaptionLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("BodyLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("StrongBodyLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("SubtitleLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("TitleLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("LargeTitleLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("DisplayLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("ImageLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("AcrylicLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("AvatarWidget"), FluentStyleSheetSource::Label},
        {QStringLiteral("HyperlinkLabel"), FluentStyleSheetSource::Label},
        {QStringLiteral("LineEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("AcrylicLineEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("SearchLineEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("AcrylicSearchLineEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("PasswordLineEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("TextEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("PlainTextEdit"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("TextBrowser"), FluentStyleSheetSource::LineEdit},
        {QStringLiteral("CardWidget"), FluentStyleSheetSource::CardWidget},
        {QStringLiteral("SimpleCardWidget"), FluentStyleSheetSource::CardWidget},
        {QStringLiteral("HeaderCardWidget"), FluentStyleSheetSource::CardWidget},
        {QStringLiteral("RoundMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("CheckableMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("SystemTrayMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("MenuActionListWidget"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicCheckableMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicSystemTrayMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicCheckableSystemTrayMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicLineEditMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicCompleterMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("ComboBoxMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("AcrylicComboBoxMenu"), FluentStyleSheetSource::Menu},
        {QStringLiteral("ComboBox"), FluentStyleSheetSource::ComboBox},
        {QStringLiteral("AcrylicComboBox"), FluentStyleSheetSource::ComboBox},
        {QStringLiteral("EditableComboBox"), FluentStyleSheetSource::ComboBox},
        {QStringLiteral("AcrylicEditableComboBox"), FluentStyleSheetSource::ComboBox},
        {QStringLiteral("ModelComboBox"), FluentStyleSheetSource::ComboBox},
        {QStringLiteral("EditableModelComboBox"), FluentStyleSheetSource::ComboBox},
        {QStringLiteral("CheckBox"), FluentStyleSheetSource::CheckBox},
        {QStringLiteral("RadioButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("Slider"), FluentStyleSheetSource::Slider},
        {QStringLiteral("ClickableSlider"), FluentStyleSheetSource::Slider},
        {QStringLiteral("SpinBox"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("DoubleSpinBox"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("TimeEdit"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("DateEdit"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("DateTimeEdit"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("CompactSpinBox"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("CompactDoubleSpinBox"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("CompactTimeEdit"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("CompactDateEdit"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("CompactDateTimeEdit"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("SpinButton"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("CompactSpinButton"), FluentStyleSheetSource::SpinBox},
        {QStringLiteral("SwitchButton"), FluentStyleSheetSource::SwitchButton},
        {QStringLiteral("ScrollBar"), FluentStyleSheetSource::ScrollArea},
        {QStringLiteral("SmoothScrollBar"), FluentStyleSheetSource::ScrollArea},
        {QStringLiteral("ScrollArea"), FluentStyleSheetSource::ScrollArea},
        {QStringLiteral("SingleDirectionScrollArea"), FluentStyleSheetSource::ScrollArea},
        {QStringLiteral("SmoothScrollArea"), FluentStyleSheetSource::ScrollArea},
        {QStringLiteral("TabBar"), FluentStyleSheetSource::TabView},
        {QStringLiteral("TabWidget"), FluentStyleSheetSource::TabView},
        {QStringLiteral("TabItem"), FluentStyleSheetSource::TabView},
        {QStringLiteral("TabCloseButton"), FluentStyleSheetSource::TabView},
        {QStringLiteral("TabAddButton"), FluentStyleSheetSource::TabView},
        {QStringLiteral("TabScrollButton"), FluentStyleSheetSource::TabView},
        {QStringLiteral("PipsPager"), FluentStyleSheetSource::PipsPager},
        {QStringLiteral("HorizontalPipsPager"), FluentStyleSheetSource::PipsPager},
        {QStringLiteral("VerticalPipsPager"), FluentStyleSheetSource::PipsPager},
        {QStringLiteral("PipsPagerScrollButton"), FluentStyleSheetSource::PipsPager},
        {QStringLiteral("PipsPagerDot"), FluentStyleSheetSource::PipsPager},
        {QStringLiteral("ListView"), FluentStyleSheetSource::ListView},
        {QStringLiteral("ListWidget"), FluentStyleSheetSource::ListView},
        {QStringLiteral("TableView"), FluentStyleSheetSource::TableView},
        {QStringLiteral("TableWidget"), FluentStyleSheetSource::TableView},
        {QStringLiteral("TreeView"), FluentStyleSheetSource::TreeView},
        {QStringLiteral("TreeWidget"), FluentStyleSheetSource::TreeView},
        {QStringLiteral("InfoBar"), FluentStyleSheetSource::InfoBar},
        {QStringLiteral("InfoBarCloseButton"), FluentStyleSheetSource::InfoBar},
        {QStringLiteral("InfoBadge"), FluentStyleSheetSource::InfoBadge},
        {QStringLiteral("DotInfoBadge"), FluentStyleSheetSource::InfoBadge},
        {QStringLiteral("IconInfoBadge"), FluentStyleSheetSource::InfoBadge},
        {QStringLiteral("ToolTip"), FluentStyleSheetSource::ToolTip},
        {QStringLiteral("AcrylicToolTip"), FluentStyleSheetSource::ToolTip},
        {QStringLiteral("StateToolTip"), FluentStyleSheetSource::StateToolTip},
        {QStringLiteral("Flyout"), FluentStyleSheetSource::Flyout},
        {QStringLiteral("FlyoutViewBase"), FluentStyleSheetSource::Flyout},
        {QStringLiteral("FlyoutView"), FluentStyleSheetSource::Flyout},
        {QStringLiteral("AcrylicFlyout"), FluentStyleSheetSource::Flyout},
        {QStringLiteral("AcrylicFlyoutViewBase"), FluentStyleSheetSource::Flyout},
        {QStringLiteral("AcrylicFlyoutView"), FluentStyleSheetSource::Flyout},
        {QStringLiteral("TeachingTip"), FluentStyleSheetSource::TeachingTip},
        {QStringLiteral("TeachingTipView"), FluentStyleSheetSource::TeachingTip},
        {QStringLiteral("SettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("PushSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("PrimaryPushSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("HyperlinkCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("SwitchSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("RangeSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("ComboBoxSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("ColorSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("HeaderSettingCard"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("ExpandGroupSettingCard"), FluentStyleSheetSource::ExpandSettingCard},
        {QStringLiteral("SimpleExpandGroupSettingCard"), FluentStyleSheetSource::ExpandSettingCard},
        {QStringLiteral("OptionsSettingCard"), FluentStyleSheetSource::ExpandSettingCard},
        {QStringLiteral("FolderListSettingCard"), FluentStyleSheetSource::ExpandSettingCard},
        {QStringLiteral("FolderPickerDialog"), FluentStyleSheetSource::FolderListDialog},
        {QStringLiteral("CustomColorSettingCard"), FluentStyleSheetSource::ExpandSettingCard},
        {QStringLiteral("SettingCardGroup"), FluentStyleSheetSource::SettingCardGroup},
        {QStringLiteral("ColorDialogPopup"), FluentStyleSheetSource::ColorDialog},
        {QStringLiteral("ColorDialog"), FluentStyleSheetSource::ColorDialog},
        {QStringLiteral("ColorDialogScrollArea"), FluentStyleSheetSource::ColorDialog},
        {QStringLiteral("DialogPopup"), FluentStyleSheetSource::Dialog},
        {QStringLiteral("Dialog"), FluentStyleSheetSource::Dialog},
        {QStringLiteral("MessageBoxPopup"), FluentStyleSheetSource::Dialog},
        {QStringLiteral("MessageBox"), FluentStyleSheetSource::Dialog},
        {QStringLiteral("NavigationInterface"), FluentStyleSheetSource::NavigationInterface},
        {QStringLiteral("NavigationBar"), FluentStyleSheetSource::NavigationInterface},
        {QStringLiteral("NavigationBarPushButton"), FluentStyleSheetSource::NavigationInterface},
        {QStringLiteral("NavigationPanel"), FluentStyleSheetSource::NavigationInterface},
        {QStringLiteral("NavigationScrollWidget"), FluentStyleSheetSource::NavigationInterface},
        {QStringLiteral("NavigationSeparator"), FluentStyleSheetSource::NavigationInterface},
        {QStringLiteral("FluentWidget"), FluentStyleSheetSource::FluentWindow},
        {QStringLiteral("FluentWindow"), FluentStyleSheetSource::FluentWindow},
        {QStringLiteral("MSFluentWindow"), FluentStyleSheetSource::FluentWindow},
        {QStringLiteral("Pivot"), FluentStyleSheetSource::Pivot},
        {QStringLiteral("PivotItem"), FluentStyleSheetSource::Pivot},
        {QStringLiteral("SegmentedWidget"), FluentStyleSheetSource::Pivot},
        {QStringLiteral("SegmentedItem"), FluentStyleSheetSource::Pivot},
        {QStringLiteral("BreadcrumbBar"), FluentStyleSheetSource::Pivot},
        {QStringLiteral("BreadcrumbItem"), FluentStyleSheetSource::Pivot},
        {QStringLiteral("BreadcrumbSeparator"), FluentStyleSheetSource::Label},
        {QStringLiteral("FolderItem"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("GroupSeparator"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("GroupWidget"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("ExpandButton"), FluentStyleSheetSource::Button},
        {QStringLiteral("MessageDialog"), FluentStyleSheetSource::MessageDialog},
        {QStringLiteral("FolderListDialog"), FluentStyleSheetSource::FolderListDialog},
        {QStringLiteral("FlipView"), FluentStyleSheetSource::FlipView},
        {QStringLiteral("FluentTitleBar"), FluentStyleSheetSource::FluentWindow},
        {QStringLiteral("ColorPickerButton"), FluentStyleSheetSource::SettingCard},
        {QStringLiteral("ProgressBar"), FluentStyleSheetSource::ProgressBar},
        {QStringLiteral("IndeterminateProgressBar"), FluentStyleSheetSource::ProgressBar},
        {QStringLiteral("ProgressRing"), FluentStyleSheetSource::ProgressBar},
        {QStringLiteral("IndeterminateProgressRing"), FluentStyleSheetSource::ProgressBar},
        {QStringLiteral("IconWidget"), FluentStyleSheetSource::IconWidget},
        {QStringLiteral("CalendarPicker"), FluentStyleSheetSource::CalendarPicker},
        {QStringLiteral("CalendarPickerPopup"), FluentStyleSheetSource::CalendarPicker},
        {QStringLiteral("CalendarPickerCalendar"), FluentStyleSheetSource::CalendarPicker},
        {QStringLiteral("CalendarView"), FluentStyleSheetSource::CalendarPicker},
        {QStringLiteral("FastCalendarView"), FluentStyleSheetSource::CalendarPicker},
        {QStringLiteral("FastCalendarPage"), FluentStyleSheetSource::CalendarPicker},
        {QStringLiteral("DatePicker"), FluentStyleSheetSource::DatePicker},
        {QStringLiteral("DatePickerPanel"), FluentStyleSheetSource::DatePicker},
        {QStringLiteral("PickerPanel"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("SeparatorWidget"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("ItemMaskWidget"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("ScrollButton"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("CycleListWidget"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("TimePicker"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("TimePickerPanel"), FluentStyleSheetSource::TimePicker},
        {QStringLiteral("MediaPlayer"), FluentStyleSheetSource::MediaPlayer},
        {QStringLiteral("ExpandSettingCard"), FluentStyleSheetSource::ExpandSettingCard},
    };

    return mapping.value(role, FluentStyleSheetSource::Global);
}

StyleSheetManager *StyleSheetManager::instance()
{
    static StyleSheetManager manager;
    return &manager;
}

StyleSheetManager::StyleSheetManager(QObject *parent) : QObject(parent) {}

void StyleSheetManager::registerWidget(QWidget *widget, FluentStyleSheetSource source)
{
    if (!widget) {
        return;
    }

    widget->setProperty(kStyleSourceProperty, static_cast<int>(source));
    if (!isRegistered(widget)) {
        m_widgets.append(QPointer<QWidget>(widget));
        widget->installEventFilter(this);
        connect(widget, &QObject::destroyed, this, [this](QObject *object) {
            deregisterWidget(qobject_cast<QWidget *>(object));
            compact();
        });
    }

    updateWidget(widget);
}

void StyleSheetManager::deregisterWidget(QWidget *widget)
{
    if (!widget) {
        compact();
        return;
    }

    widget->removeEventFilter(this);
    for (int i = m_widgets.size() - 1; i >= 0; --i) {
        if (m_widgets.at(i).isNull() || m_widgets.at(i).data() == widget) {
            m_widgets.removeAt(i);
        }
    }
}

bool StyleSheetManager::isRegistered(QWidget *widget) const
{
    if (!widget) {
        return false;
    }

    for (const QPointer<QWidget> &registeredWidget : m_widgets) {
        if (registeredWidget.data() == widget) {
            return true;
        }
    }
    return false;
}

FluentStyleSheetSource StyleSheetManager::sourceForWidget(QWidget *widget) const
{
    return sourceFromProperty(widget);
}

int StyleSheetManager::registeredWidgetCount() const
{
    int count = 0;
    for (const QPointer<QWidget> &widget : m_widgets) {
        if (!widget.isNull()) {
            ++count;
        }
    }
    return count;
}

void StyleSheetManager::setCustomStyleSheet(QWidget *widget, const QString &lightQss, const QString &darkQss)
{
    if (!widget) {
        return;
    }

    registerWidget(widget, sourceFromProperty(widget));
    widget->setProperty(kLightCustomQssProperty, lightQss);
    widget->setProperty(kDarkCustomQssProperty, darkQss);
    updateWidget(widget);
}

QString StyleSheetManager::customStyleSheet(QWidget *widget, Theme theme) const
{
    if (!widget) {
        return {};
    }

    const Theme resolvedTheme = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
    const QVariant value =
        widget->property(resolvedTheme == Theme::Dark ? kDarkCustomQssProperty : kLightCustomQssProperty);
    return value.toString();
}

void StyleSheetManager::updateWidget(QWidget *widget, Theme theme)
{
    if (!widget) {
        return;
    }

    const FluentStyleSheetSource source = sourceFromProperty(widget);
    const QString composed = FluentStyleSheet::compose(widget, source, theme);
    if (!composed.isEmpty()) {
        widget->setStyleSheet(composed);
    }

    widget->setProperty(kDirtyQssProperty, false);
    FluentStyleSheet::polish(widget);
}

void StyleSheetManager::updateAll(Theme theme, bool lazy)
{
    compact();

    for (const QPointer<QWidget> &widgetPointer : std::as_const(m_widgets)) {
        QWidget *widget = widgetPointer.data();
        if (!widget) {
            continue;
        }

        if (lazy && !widget->isVisible()) {
            widget->setProperty(kDirtyQssProperty, true);
            continue;
        }

        updateWidget(widget, theme);
    }
}

bool StyleSheetManager::eventFilter(QObject *watched, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Paint && widget->property(kDirtyQssProperty).toBool()) {
        updateWidget(widget);
    }

    if (event->type() == QEvent::DynamicPropertyChange) {
        FluentStyleSheet::polish(widget);
    }

    return QObject::eventFilter(watched, event);
}

void StyleSheetManager::compact()
{
    for (int i = m_widgets.size() - 1; i >= 0; --i) {
        if (m_widgets.at(i).isNull()) {
            m_widgets.removeAt(i);
        }
    }
}

QString FluentStyleSheet::render(const QString &qss, Theme theme)
{
    QString rendered = qss;
    const QColor accent = ThemeManager::instance()->accentColor();
    const Theme resolved = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;

    rendered.replace(QStringLiteral("@accentColor"), accent.name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorPrimary"), derivedThemeColor(accent, ThemeColor::Primary, resolved).name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorDark1"), derivedThemeColor(accent, ThemeColor::Dark1, resolved).name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorDark2"), derivedThemeColor(accent, ThemeColor::Dark2, resolved).name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorDark3"), derivedThemeColor(accent, ThemeColor::Dark3, resolved).name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorLight1"), derivedThemeColor(accent, ThemeColor::Light1, resolved).name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorLight2"), derivedThemeColor(accent, ThemeColor::Light2, resolved).name(QColor::HexRgb));
    rendered.replace(QStringLiteral("--ThemeColorLight3"), derivedThemeColor(accent, ThemeColor::Light3, resolved).name(QColor::HexRgb));

    QStringList quotedFamilies;
    for (const QString &family : Private::resolvedFontFamilies(FluentConfig::instance()->fontFamilies())) {
        quotedFamilies.append(QStringLiteral("\"%1\"").arg(family));
    }
    rendered.replace(QStringLiteral("--FontFamilies"), quotedFamilies.join(QStringLiteral(", ")));

    return rendered;
}

QString FluentStyleSheet::load(Theme theme)
{
    return render(loadSource(FluentStyleSheetSource::Global, theme), theme);
}

QString FluentStyleSheet::loadSource(FluentStyleSheetSource source, Theme theme)
{
    const Theme resolvedTheme = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
    const QString path = QStringLiteral(":/qfluentwidgets/qss/%1/%2.qss")
                             .arg(themeName(resolvedTheme), styleSheetSourceName(source));

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll());
}

QString FluentStyleSheet::compose(QWidget *widget, FluentStyleSheetSource source, Theme theme)
{
    QString composed = loadSource(source, theme);
    if (widget) {
        const QString customQss = StyleSheetManager::instance()->customStyleSheet(widget, theme);
        if (!customQss.isEmpty()) {
            if (!composed.isEmpty()) {
                composed.append(QLatin1Char('\n'));
            }
            composed.append(customQss);
        }
    }
    return composed.isEmpty() ? composed : render(composed, theme);
}

void FluentStyleSheet::apply(QWidget *widget, Theme theme)
{
    apply(widget, FluentStyleSheetSource::Global, theme);
}

void FluentStyleSheet::apply(QWidget *widget, FluentStyleSheetSource source, Theme theme)
{
    if (!widget) {
        return;
    }

    StyleSheetManager::instance()->registerWidget(widget, source);
}

void FluentStyleSheet::updateAll(Theme theme, bool lazy) { StyleSheetManager::instance()->updateAll(theme, lazy); }

void FluentStyleSheet::polish(QWidget *widget)
{
    if (!widget || !widget->style()) {
        return;
    }

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

void FluentStyleSheet::setRole(QWidget *widget, const QString &role)
{
    if (!widget) {
        return;
    }

    widget->setProperty("fqw", role);
    StyleSheetManager::instance()->registerWidget(widget, styleSheetSourceForRole(role));
}

void FluentStyleSheet::setCustomStyleSheet(QWidget *widget, const QString &lightQss, const QString &darkQss)
{
    StyleSheetManager::instance()->setCustomStyleSheet(widget, lightQss, darkQss);
}

} // namespace FluentQt
