#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

class DemoPage : public QWidget
{
  public:
    explicit DemoPage(const QString &text, QWidget *parent = nullptr) : QWidget(parent)
    {
        QString objectName = text;
        setObjectName(objectName.replace(QLatin1Char(' '), QLatin1Char('-')));

        auto *label = new SubtitleLabel(text, this);
        QFont font = label->font();
        font.setPixelSize(24);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label, 1, Qt::AlignCenter);
    }
};

class CompactNavigationBar : public QWidget
{
    Q_OBJECT

  public:
    explicit CompactNavigationBar(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(8);

        m_menuButton = new TransparentToolButton(icon(FluentIcon::More), this);
        m_menuButton->setFixedSize(40, 36);
        m_titleLabel = new BodyLabel(this);
        layout->addWidget(m_menuButton);
        layout->addWidget(m_titleLabel, 1, Qt::AlignVCenter);

        m_panel = new NavigationPanel(window());
        m_panel->setExpandWidth(260);
        m_panel->setMenuButtonVisible(false);
        m_panel->setReturnButtonVisible(false);
        m_panel->hide();

        connect(m_menuButton, &TransparentToolButton::clicked, this, [this]() {
            m_panel->setParent(window());
            m_panel->setFixedHeight(qMax(0, window()->height() - height()));
            m_panel->move(0, height());
            m_panel->show();
            m_panel->raise();
            m_panel->expand(false);
        });
        connect(m_panel, &NavigationPanel::itemClicked, this, [this](const QString &routeKey) {
            if (auto *item = m_panel->widget(routeKey)) {
                setTitle(item->text());
            }
            m_panel->collapse(false);
            m_panel->hide();
            emit itemClicked(routeKey);
        });
    }

    NavigationTreeWidget *addItem(const QString &routeKey, const QIcon &itemIcon, const QString &text,
                                  NavigationItemPosition position = NavigationItemPosition::Top)
    {
        return m_panel->addItem(routeKey, itemIcon, text, position);
    }

    QFrame *addSeparator(NavigationItemPosition position = NavigationItemPosition::Top)
    {
        return m_panel->addSeparator(position);
    }

    void setCurrentItem(const QString &routeKey)
    {
        m_panel->setCurrentItem(routeKey);
        if (auto *item = m_panel->widget(routeKey)) {
            setTitle(item->text());
        }
    }

    void setTitle(const QString &title) { m_titleLabel->setText(title); }

  signals:
    void itemClicked(const QString &routeKey);

  protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        if (m_panel && m_panel->isVisible()) {
            m_panel->setFixedHeight(qMax(0, window()->height() - height()));
        }
    }

  private:
    TransparentToolButton *m_menuButton = nullptr;
    BodyLabel *m_titleLabel = nullptr;
    NavigationPanel *m_panel = nullptr;
};

class DemoWindow : public FluentWidget
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(icon(FluentIcon::Home));

        auto *root = new QWidget(this);
        auto *layout = new QVBoxLayout(root);
        layout->setContentsMargins(0, 48, 0, 0);
        layout->setSpacing(0);
        m_navigation = new CompactNavigationBar(root);
        m_stack = new QStackedWidget(root);
        layout->addWidget(m_navigation);
        layout->addWidget(m_stack, 1);

        auto *outerLayout = new QVBoxLayout(this);
        outerLayout->setContentsMargins(0, 0, 0, 0);
        outerLayout->addWidget(root);

        addPage(QStringLiteral("Search-Interface"), icon(FluentIcon::Search), QStringLiteral("Search"),
                new DemoPage(QStringLiteral("Search Interface"), this));
        addPage(QStringLiteral("Music-Interface"), icon(FluentIcon::Music), QStringLiteral("Music library"),
                new DemoPage(QStringLiteral("Music Interface"), this));
        addPage(QStringLiteral("Video-Interface"), icon(FluentIcon::Video), QStringLiteral("Video library"),
                new DemoPage(QStringLiteral("Video Interface"), this));
        m_navigation->addSeparator();
        addPage(QStringLiteral("Folder-Interface"), icon(FluentIcon::Folder), QStringLiteral("Folder library"),
                new DemoPage(QStringLiteral("Folder Interface"), this), NavigationItemPosition::Scroll);
        addPage(QStringLiteral("Setting-Interface"), icon(FluentIcon::Setting), QStringLiteral("Settings"),
                new DemoPage(QStringLiteral("Setting Interface"), this), NavigationItemPosition::Bottom);

        connect(m_navigation, &CompactNavigationBar::itemClicked, this, [this](const QString &routeKey) {
            const int index = m_routes.indexOf(routeKey);
            if (index >= 0) {
                m_stack->setCurrentIndex(index);
            }
        });
        connect(m_stack, &QStackedWidget::currentChanged, this, [this](int index) {
            if (index >= 0 && index < m_routes.size()) {
                m_navigation->setCurrentItem(m_routes.at(index));
            }
        });

        m_stack->setCurrentIndex(1);
        m_navigation->setCurrentItem(QStringLiteral("Music-Interface"));
        resize(500, 600);
    }

  private:
    void addPage(const QString &routeKey, const QIcon &pageIcon, const QString &text, QWidget *page,
                 NavigationItemPosition position = NavigationItemPosition::Top)
    {
        page->setObjectName(routeKey);
        m_routes.append(routeKey);
        m_stack->addWidget(page);
        m_navigation->addItem(routeKey, pageIcon, text, position);
    }

    CompactNavigationBar *m_navigation = nullptr;
    QStackedWidget *m_stack = nullptr;
    QStringList m_routes;
};

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    DemoWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
