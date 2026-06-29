#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>

using namespace FluentQt;

namespace {

class DemoPage : public QWidget
{
  public:
    explicit DemoPage(const QString &text, QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName(text.toLower().replace(QLatin1Char(' '), QLatin1Char('-')));

        auto *label = new SubtitleLabel(text, this);
        QFont font = label->font();
        font.setPixelSize(24);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label, 1, Qt::AlignCenter);
    }
};

class DemoWindow : public FluentWidget
{
  public:
    explicit DemoWindow(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        setWindowTitle(QStringLiteral("Navigation Bar Demo"));
        setWindowIcon(icon(FluentIcon::Home));

        m_navigationBar = new NavigationBar(this);
        m_stack = new QStackedWidget(this);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 48, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_navigationBar);
        layout->addWidget(m_stack, 1);

        addPage(QStringLiteral("home"), icon(FluentIcon::Home), QStringLiteral("Home"),
                new DemoPage(QStringLiteral("Home Interface"), this));
        addPage(QStringLiteral("application"), icon(FluentIcon::Code), QStringLiteral("Apps"),
                new DemoPage(QStringLiteral("Application Interface"), this));
        addPage(QStringLiteral("video"), icon(FluentIcon::Video), QStringLiteral("Video"),
                new DemoPage(QStringLiteral("Video Interface"), this));
        addPage(QStringLiteral("library"), icon(FluentIcon::Folder), QStringLiteral("Library"),
                new DemoPage(QStringLiteral("Library Interface"), this), NavigationItemPosition::Bottom);

        auto *help = m_navigationBar->addItem(QStringLiteral("help"), icon(FluentIcon::Info),
                                              QStringLiteral("Help"), false, QIcon(),
                                              NavigationItemPosition::Bottom);
        connect(help, &NavigationWidget::clicked, this, [this]() {
            MessageBox box(QStringLiteral("Navigation Bar"),
                           QStringLiteral("This item is not selectable, but it can still trigger commands."), this);
            box.exec();
        });

        connect(m_navigationBar, &NavigationBar::itemClicked, this, [this](const QString &routeKey) {
            const int index = m_routes.indexOf(routeKey);
            if (index >= 0) {
                m_stack->setCurrentIndex(index);
            }
        });
        connect(m_stack, &QStackedWidget::currentChanged, this, [this](int index) {
            if (index >= 0 && index < m_routes.size()) {
                m_navigationBar->setCurrentItem(m_routes.at(index));
            }
        });

        m_navigationBar->setCurrentItem(QStringLiteral("home"));
        resize(900, 700);
    }

  private:
    void addPage(const QString &routeKey, const QIcon &pageIcon, const QString &text, QWidget *page,
                 NavigationItemPosition position = NavigationItemPosition::Top)
    {
        m_routes.append(routeKey);
        m_stack->addWidget(page);
        m_navigationBar->addItem(routeKey, pageIcon, text, true, QIcon(), position);
    }

    NavigationBar *m_navigationBar = nullptr;
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
