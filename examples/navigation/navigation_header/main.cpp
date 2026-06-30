#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>

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

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_navigation);

        initNavigation();
    }

  private:
    void initNavigation()
    {
        m_navigation->addPage(createPage(QStringLiteral("Home"), this), QStringLiteral("Home"), icon(FluentIcon::Home),
                              QStringLiteral("home"), NavigationItemPosition::Top);
        m_navigation->addSeparator();

        m_navigation->addItemHeader(QStringLiteral("Basic Input"));
        m_navigation->addPage(createPage(QStringLiteral("Button"), this), QStringLiteral("Button"),
                              icon(FluentIcon::Accept), QStringLiteral("button"));
        m_navigation->addPage(createPage(QStringLiteral("Input"), this), QStringLiteral("Input"),
                              icon(FluentIcon::Edit), QStringLiteral("input"));

        m_navigation->addItemHeader(QStringLiteral("Data"));
        m_navigation->addPage(createPage(QStringLiteral("Table"), this), QStringLiteral("Table"),
                              icon(FluentIcon::Scroll), QStringLiteral("table"));
        m_navigation->addPage(createPage(QStringLiteral("List"), this), QStringLiteral("List"),
                              icon(FluentIcon::Folder), QStringLiteral("list"));

        m_navigation->addPage(createPage(QStringLiteral("Settings"), this), QStringLiteral("Settings"),
                              icon(FluentIcon::Setting), QStringLiteral("settings"),
                              NavigationItemPosition::Bottom);
    }

    NavigationInterface *m_navigation = nullptr;
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
