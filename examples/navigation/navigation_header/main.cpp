#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>

using namespace FluentQt;

namespace {

QWidget *createPage(const QString &text, QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setObjectName(text.toLower().replace(QLatin1Char(' '), QLatin1Char('-')));

    auto *label = new SubtitleLabel(text, page);
    QFont font = label->font();
    font.setPixelSize(24);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);

    auto *layout = new QHBoxLayout(page);
    layout->addWidget(label, 1, Qt::AlignCenter);
    return page;
}

class DemoWindow : public QWidget
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle(QStringLiteral("Navigation Header Demo"));
        resize(900, 600);

        m_navigation = new NavigationInterface(this);
        m_stackedWidget = new QStackedWidget(this);

        initNavigation();

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_navigation);
        layout->addWidget(m_stackedWidget, 1);
    }

  private:
    void initNavigation()
    {
        addInterface(QStringLiteral("home"), icon(FluentIcon::Home), QStringLiteral("Home"));
        m_navigation->addSeparator();

        m_navigation->addItemHeader(QStringLiteral("Basic Input"), NavigationItemPosition::Scroll);
        addInterface(QStringLiteral("button"), icon(FluentIcon::Checkbox), QStringLiteral("Button"),
                     NavigationItemPosition::Scroll);
        addInterface(QStringLiteral("input"), icon(FluentIcon::Edit), QStringLiteral("Input"),
                     NavigationItemPosition::Scroll);

        m_navigation->addItemHeader(QStringLiteral("Data"), NavigationItemPosition::Scroll);
        addInterface(QStringLiteral("table"), icon(FluentIcon::Document), QStringLiteral("Table"),
                     NavigationItemPosition::Scroll);
        addInterface(QStringLiteral("list"), icon(FluentIcon::Menu), QStringLiteral("List"),
                     NavigationItemPosition::Scroll);

        addInterface(QStringLiteral("settings"), icon(FluentIcon::Setting), QStringLiteral("Settings"),
                     NavigationItemPosition::Bottom);

        // The window keeps the pages; the navigation switches them on a click.
        connect(m_navigation->panel(), &NavigationPanel::itemClicked, this, [this](const QString &routeKey) {
            if (QWidget *page = m_interfaces.value(routeKey)) {
                m_stackedWidget->setCurrentWidget(page);
            }
        });

        m_stackedWidget->setCurrentIndex(0);
        m_navigation->setCurrentItem(QStringLiteral("home"));
    }

    void addInterface(const QString &routeKey, const QIcon &icon, const QString &text,
                      NavigationItemPosition position = NavigationItemPosition::Top)
    {
        QWidget *page = createPage(text, this);
        m_interfaces.insert(routeKey, page);
        m_stackedWidget->addWidget(page);
        m_navigation->addItem(routeKey, icon, text, position);
    }

    NavigationInterface *m_navigation = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    QHash<QString, QWidget *> m_interfaces;
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
