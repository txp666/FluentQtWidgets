#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

class TabInterface : public QWidget
{
  public:
    TabInterface(const QString &text, const QIcon &icon, QWidget *parent = nullptr) : QWidget(parent)
    {
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
        m_tabWidget = new TabWidget(this);
        m_tabWidget->setMovable(true);

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(m_tabWidget);

        const QIcon heart(QStringLiteral(":/navigation/tab_widget/resource/Heart.png"));
        m_tabWidget->addTab(new TabInterface(QStringLiteral("Heart"), heart, m_tabWidget),
                            QStringLiteral("As long as you love me"), heart, QStringLiteral("heart"));
        connect(m_tabWidget, &TabWidget::tabCloseRequested, m_tabWidget, &TabWidget::removeTab);
        connect(m_tabWidget, &TabWidget::tabAddRequested, this, &Demo::addNewPage);
        resize(1100, 750);
    }

  private:
    void addNewPage()
    {
        const QString text = QStringLiteral("Shoko makinohara %1").arg(m_tabCount++);
        const QIcon icon(QStringLiteral(":/navigation/tab_widget/resource/Smiling_with_heart.png"));
        m_tabWidget->addTab(new TabInterface(text, icon, m_tabWidget), text, icon);
    }

    TabWidget *m_tabWidget = nullptr;
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
