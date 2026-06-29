#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>

class QWidget;
class QEvent;

namespace FluentQt {

enum class FluentStyleSheetSource
{
    Global,
    Button,
    RadioButton,
    Label,
    LineEdit,
    CardWidget,
    Menu,
    ComboBox,
    CheckBox,
    Slider,
    SpinBox,
    SwitchButton,
    ScrollArea,
    TabView,
    PipsPager,
    ListView,
    TableView,
    TreeView,
    InfoBar,
    InfoBadge,
    ToolTip,
    StateToolTip,
    Flyout,
    TeachingTip,
    SettingCard,
    SettingCardGroup,
    ColorDialog,
    Dialog,
    MessageDialog,
    FolderListDialog,
    NavigationInterface,
    FluentWindow,
    Pivot,
    ProgressBar,
    IconWidget,
    CalendarPicker,
    DatePicker,
    TimePicker,
    FlipView,
    MediaPlayer,
    ExpandSettingCard
};

FQW_API QString styleSheetSourceName(FluentStyleSheetSource source);
FQW_API FluentStyleSheetSource styleSheetSourceForRole(const QString &role);

class FQW_API StyleSheetManager : public QObject
{
    Q_OBJECT

  public:
    static StyleSheetManager *instance();

    void registerWidget(QWidget *widget, FluentStyleSheetSource source = FluentStyleSheetSource::Global);
    void deregisterWidget(QWidget *widget);
    bool isRegistered(QWidget *widget) const;
    int registeredWidgetCount() const;
    FluentStyleSheetSource sourceForWidget(QWidget *widget) const;
    void setCustomStyleSheet(QWidget *widget, const QString &lightQss, const QString &darkQss);
    QString customStyleSheet(QWidget *widget, Theme theme = Theme::Auto) const;

  public slots:
    void updateWidget(QWidget *widget, FluentQt::Theme theme = FluentQt::Theme::Auto);
    void updateAll(FluentQt::Theme theme = FluentQt::Theme::Auto, bool lazy = false);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    explicit StyleSheetManager(QObject *parent = nullptr);
    void compact();

    QList<QPointer<QWidget>> m_widgets;
};

class FQW_API FluentStyleSheet final
{
  public:
    static QString render(const QString &qss, Theme theme = Theme::Auto);
    static QString load(Theme theme = Theme::Auto);
    static QString loadSource(FluentStyleSheetSource source, Theme theme = Theme::Auto);
    static QString compose(QWidget *widget, FluentStyleSheetSource source, Theme theme = Theme::Auto);
    static void apply(QWidget *widget, Theme theme = Theme::Auto);
    static void apply(QWidget *widget, FluentStyleSheetSource source, Theme theme = Theme::Auto);
    static void updateAll(Theme theme = Theme::Auto, bool lazy = false);
    static void polish(QWidget *widget);
    static void setRole(QWidget *widget, const QString &role);
    static void setCustomStyleSheet(QWidget *widget, const QString &lightQss, const QString &darkQss);

  private:
    FluentStyleSheet() = default;
};

} // namespace FluentQt
