#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QStandardPaths>
#include <QtCore/Qt>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

constexpr const char *kHelpUrl = FQW_REPOSITORY_URL;
constexpr const char *kFeedbackUrl = FQW_REPOSITORY_URL "/issues";

class SettingInterface : public ScrollArea
{
  public:
    explicit SettingInterface(QWidget *parent = nullptr) : ScrollArea(parent)
    {
        setObjectName(QStringLiteral("SettingInterface"));
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setWidgetResizable(true);
        setViewportMargins(0, 120, 0, 20);

        auto *scrollWidget = new QWidget(this);
        scrollWidget->setObjectName(QStringLiteral("scrollWidget"));
        setWidget(scrollWidget);

        m_settingLabel = new LargeTitleLabel(tr("Settings"), this);
        m_settingLabel->setObjectName(QStringLiteral("settingLabel"));
        m_settingLabel->move(60, 63);

        auto *layout = new QVBoxLayout(scrollWidget);
        layout->setContentsMargins(60, 10, 60, 0);
        layout->setSpacing(28);

        addMusicGroup(layout, scrollWidget);
        addPersonalGroup(layout, scrollWidget);
        addOnlineMusicGroup(layout, scrollWidget);
        addDesktopLyricGroup(layout, scrollWidget);
        addMainPanelGroup(layout, scrollWidget);
        addUpdateGroup(layout, scrollWidget);
        addAboutGroup(layout, scrollWidget);
        layout->addStretch(1);
    }

  protected:
    void resizeEvent(QResizeEvent *event) override
    {
        ScrollArea::resizeEvent(event);
        if (m_settingLabel) {
            m_settingLabel->move(60, 63);
        }
    }

  private:
    void addMusicGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("Music on this PC"), parent);
        group->addSettingCard(new FolderListSettingCard({QStandardPaths::writableLocation(QStandardPaths::MusicLocation)},
                                                        icon(FluentIcon::Folder), tr("Local music library"),
                                                        tr("Choose folders that contain local music"),
                                                        QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
                                                        group));
        group->addSettingCard(new PushSettingCard(tr("Choose folder"), icon(FluentIcon::Download),
                                                  tr("Download directory"),
                                                  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                                                  group));
        layout->addWidget(group);
    }

    void addPersonalGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("Personalization"), parent);
        group->addSettingCard(new SwitchSettingCard(icon(FluentIcon::Transparent), tr("Use Acrylic effect"),
                                                    tr("Acrylic effect has better visual experience"), group));
        group->addSettingCard(new OptionsSettingCard({tr("Light"), tr("Dark"), tr("Use system setting")},
                                                     {QStringLiteral("Light"), QStringLiteral("Dark"),
                                                      QStringLiteral("System")},
                                                     icon(FluentIcon::Brush), tr("Application theme"),
                                                     tr("Change the appearance of your application"), group));
        group->addSettingCard(new ColorSettingCard(QColor(0, 153, 188), icon(FluentIcon::Palette), tr("Theme color"),
                                                   tr("Change the theme color of your application"), group));
        group->addSettingCard(new OptionsSettingCard({QStringLiteral("100%"), QStringLiteral("125%"),
                                                      QStringLiteral("150%"), QStringLiteral("175%"),
                                                      QStringLiteral("200%"), tr("Use system setting")},
                                                     {1.0, 1.25, 1.5, 1.75, 2.0, QStringLiteral("Auto")},
                                                     icon(FluentIcon::Zoom), tr("Interface zoom"),
                                                     tr("Change the size of widgets and fonts"), group));
        group->addSettingCard(new ComboBoxSettingCard({QStringLiteral("简体中文"), QStringLiteral("繁體中文"),
                                                       QStringLiteral("English"), tr("Use system setting")},
                                                      icon(FluentIcon::Language), tr("Language"),
                                                      tr("Set your preferred language for UI"), group));
        layout->addWidget(group);
    }

    void addOnlineMusicGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("Online Music"), parent);
        group->addSettingCard(new RangeSettingCard(0, 50, 30, icon(FluentIcon::Search),
                                                   tr("Number of online music displayed on each page"), QString(), group));
        group->addSettingCard(new OptionsSettingCard({tr("Standard quality"), tr("High quality"),
                                                      tr("Super quality"), tr("Lossless quality")},
                                                     {0, 1, 2, 3}, icon(FluentIcon::Music),
                                                     tr("Online music quality"), QString(), group));
        group->addSettingCard(new OptionsSettingCard({QStringLiteral("Full HD"), QStringLiteral("HD"),
                                                      QStringLiteral("SD"), QStringLiteral("LD")},
                                                     {0, 1, 2, 3}, icon(FluentIcon::Video),
                                                     tr("Online MV quality"), QString(), group));
        layout->addWidget(group);
    }

    void addDesktopLyricGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("Desktop Lyric"), parent);
        group->addSettingCard(new PushSettingCard(tr("Choose font"), icon(FluentIcon::Font), tr("Font"), QString(), group));
        group->addSettingCard(new ColorSettingCard(QColor(0, 153, 188), icon(FluentIcon::Palette),
                                                   tr("Foreground color"), QString(), group));
        group->addSettingCard(new ColorSettingCard(Qt::black, icon(FluentIcon::PencilInk), tr("Stroke color"),
                                                   QString(), group));
        group->addSettingCard(new RangeSettingCard(0, 20, 5, icon(FluentIcon::Hightlight), tr("Stroke size"),
                                                   QString(), group));
        group->addSettingCard(new OptionsSettingCard({tr("Center aligned"), tr("Left aligned"), tr("Right aligned")},
                                                     {QStringLiteral("Center"), QStringLiteral("Left"),
                                                      QStringLiteral("Right")},
                                                     icon(FluentIcon::Alignment), tr("Alignment"), QString(), group));
        layout->addWidget(group);
    }

    void addMainPanelGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("Main Panel"), parent);
        auto *card = new SwitchSettingCard(icon(FluentIcon::Minimize), tr("Minimize to tray after closing"),
                                           tr("FluentQtWidgets will continue to run in the background"), group);
        card->setChecked(true);
        group->addSettingCard(card);
        layout->addWidget(group);
    }

    void addUpdateGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("Software update"), parent);
        auto *card = new SwitchSettingCard(icon(FluentIcon::Update), tr("Check for updates when the application starts"),
                                           tr("The new version will be more stable and have more features"), group);
        card->setChecked(true);
        group->addSettingCard(card);
        layout->addWidget(group);
    }

    void addAboutGroup(QVBoxLayout *layout, QWidget *parent)
    {
        auto *group = new SettingCardGroup(tr("About"), parent);
        group->addSettingCard(new HyperlinkCard(QUrl(QString::fromLatin1(kHelpUrl)), tr("Open help page"),
                                                icon(FluentIcon::Help), tr("Help"),
                                                tr("Discover new features and learn useful tips about FluentQtWidgets"),
                                                group));
        auto *feedback = new PrimaryPushSettingCard(tr("Provide feedback"), icon(FluentIcon::Feedback),
                                                    tr("Provide feedback"),
                                                    tr("Help us improve FluentQtWidgets by providing feedback"), group);
        connect(feedback, &PushSettingCard::clicked, feedback, []() {
            QDesktopServices::openUrl(QUrl(QString::fromLatin1(kFeedbackUrl)));
        });
        group->addSettingCard(feedback);
        group->addSettingCard(new PrimaryPushSettingCard(tr("Check update"), icon(FluentIcon::Info), tr("About"),
                                                        tr("Copyright 2026, FluentQtWidgets"), group));
        layout->addWidget(group);
    }

    LargeTitleLabel *m_settingLabel = nullptr;
};

class SettingsWindow : public FluentWidget
{
  public:
    explicit SettingsWindow(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::Setting));

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(new SettingInterface(this));

        titleBar()->raise();
        resize(1080, 784);
    }
};

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    SettingsWindow window;
    window.show();
    return app.exec();
}
