#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

class TabInterface : public QWidget
{
  public:
    TabInterface(const QString &text, const QIcon &icon, const QString &objectName, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setObjectName(objectName);
        auto *iconWidget = new IconWidget(icon, this);
        auto *label = new SubtitleLabel(text, this);
        iconWidget->setFixedSize(120, 120);
        iconWidget->setIconSize(QSize(120, 120));
        QFont labelFont = label->font();
        labelFont.setPixelSize(24);
        label->setFont(labelFont);

        auto *layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->setSpacing(30);
        layout->addWidget(iconWidget, 0, Qt::AlignCenter);
        layout->addWidget(label, 0, Qt::AlignCenter);
    }
};

} // namespace

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_tabBar = new TabBar(this);
        m_stackedWidget = new QStackedWidget(this);
        m_tabBar->setMovable(true);
        m_tabBar->setTabMaximumWidth(220);
        m_tabBar->setTabShadowEnabled(false);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(0);
        layout->addWidget(m_tabBar);
        layout->addWidget(m_stackedWidget, 1);

        addTab(QStringLiteral("heart"), QStringLiteral("As long as you love me"),
               QIcon(QStringLiteral(":/navigation/tab_view/resource/Heart.png")));
        connect(m_tabBar, &TabBar::tabCloseRequested, this, [this](int index) {
            if (auto *page = m_stackedWidget->widget(index)) {
                m_stackedWidget->removeWidget(page);
                page->deleteLater();
            }
            m_tabBar->removeTab(index);
        });
        connect(m_tabBar, &TabBar::currentChanged, this, [this](int index) {
            if (index >= 0 && index < m_stackedWidget->count()) {
                m_stackedWidget->setCurrentIndex(index);
            }
        });
        connect(m_tabBar, &TabBar::tabAddRequested, this, &Demo::addNewPage);
        resize(1100, 750);
    }

  private:
    void addNewPage()
    {
        const QString routeKey = QStringLiteral("shoko-%1").arg(m_tabCount);
        const QString text = QStringLiteral("Shoko makinohara %1").arg(m_tabCount++);
        addTab(routeKey, text, QIcon(QStringLiteral(":/navigation/tab_view/resource/Smiling_with_heart.png")));
    }

    void addTab(const QString &routeKey, const QString &text, const QIcon &icon)
    {
        m_tabBar->addTab(routeKey, text, icon);
        m_stackedWidget->addWidget(new TabInterface(text, icon, routeKey, m_stackedWidget));
        m_tabBar->setCurrentTab(routeKey);
    }

    TabBar *m_tabBar = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    int m_tabCount = 1;
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    Demo demo;
    demo.show();
    return app.exec();
}
