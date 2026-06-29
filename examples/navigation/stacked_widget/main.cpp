#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

const QString kLorem =
    QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
                   "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
                   "laboris nisi ut aliquip ex ea commodo consequat.");

class ColorBlock : public QFrame
{
  public:
    explicit ColorBlock(const QColor &color, QWidget *parent = nullptr) : QFrame(parent), m_color(color) {}

  protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.fillRect(rect(), m_color);
    }

  private:
    QColor m_color;
};

QWidget *createSamplePage1()
{
    auto *page = new QWidget;
    auto *layout = new QGridLayout(page);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    auto *accent = new ColorBlock(ThemeManager::instance()->accentColor(), page);
    accent->setMinimumWidth(200);
    layout->addWidget(accent, 0, 0, 2, 1);

    QList<ColorBlock *> blocks = {
        new ColorBlock(QColor(128, 128, 128), page),
        new ColorBlock(QColor(192, 192, 192), page),
        new ColorBlock(QColor(192, 192, 192), page),
        new ColorBlock(QColor(160, 160, 160), page),
    };
    for (auto *block : blocks) {
        block->setMinimumHeight(120);
    }

    layout->addWidget(blocks.at(0), 0, 1);
    layout->addWidget(blocks.at(1), 0, 2);
    layout->addWidget(blocks.at(2), 1, 1);
    layout->addWidget(blocks.at(3), 1, 2);

    auto *body = new BodyLabel(kLorem, page);
    body->setWordWrap(true);
    layout->addWidget(body, 2, 0, 1, 3);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 1);
    return page;
}

QWidget *createSamplePage2()
{
    auto *page = new QWidget;
    auto *layout = new QHBoxLayout(page);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(16);
    layout->setAlignment(Qt::AlignTop);

    auto *accent = new ColorBlock(ThemeManager::instance()->accentColor(), page);
    accent->setFixedSize(140, 180);
    layout->addWidget(accent, 0, Qt::AlignTop);

    auto *textLayout = new QVBoxLayout;
    textLayout->setSpacing(8);
    textLayout->setContentsMargins(0, 0, 0, 0);
    auto *title = new TitleLabel(QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit"), page);
    title->setWordWrap(true);
    auto *body = new BodyLabel(kLorem, page);
    body->setWordWrap(true);
    textLayout->addWidget(title);
    textLayout->addWidget(body);
    textLayout->addStretch(1);
    layout->addLayout(textLayout, 1);
    return page;
}

} // namespace

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_stackedWidget = new QStackedWidget(this);
        m_entranceStackedWidget = new EntranceTransitionStackedWidget;
        m_drillInStackedWidget = new DrillInTransitionStackedWidget;

        m_entranceStackedWidget->setMinimumHeight(500);
        m_entranceStackedWidget->addWidget(createSamplePage1());
        m_entranceStackedWidget->addWidget(createSamplePage2());
        m_stackedWidget->addWidget(m_entranceStackedWidget);

        m_drillInStackedWidget->setMinimumHeight(500);
        m_drillInStackedWidget->addWidget(createSamplePage1());
        m_drillInStackedWidget->addWidget(createSamplePage2());
        m_stackedWidget->addWidget(m_drillInStackedWidget);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(m_stackedWidget, 1);
        layout->addWidget(createControlPanel(), 0);
        resize(800, 700);
    }

  private:
    QWidget *createControlPanel()
    {
        auto *panel = new QWidget(this);
        panel->setFixedWidth(260);
        auto *buttonGroup = new QButtonGroup(panel);
        auto *layout = new QVBoxLayout(panel);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        layout->addWidget(new SubtitleLabel(QStringLiteral("Transition modes"), panel));

        auto *entranceButton = new RadioButton(QStringLiteral("Entrance"), panel);
        auto *drillInButton = new RadioButton(QStringLiteral("DrillIn"), panel);
        entranceButton->setChecked(true);
        buttonGroup->addButton(entranceButton, 0);
        buttonGroup->addButton(drillInButton, 1);
        layout->addWidget(entranceButton);
        layout->addWidget(drillInButton);
        connect(buttonGroup, &QButtonGroup::idClicked, m_stackedWidget, [this](int index) {
            m_backStack.clear();
            m_stackedWidget->setCurrentIndex(index);
        });

        layout->addSpacing(16);
        layout->addWidget(new SubtitleLabel(QStringLiteral("Navigate"), panel));
        auto *forwardButton = new PushButton(QStringLiteral("Navigate Forward"), panel);
        auto *backButton = new PushButton(QStringLiteral("Navigate Backward"), panel);
        layout->addWidget(forwardButton);
        layout->addWidget(backButton);
        layout->addStretch(1);

        connect(forwardButton, &QPushButton::clicked, this, [this]() {
            auto *stack = qobject_cast<TransitionStackedWidget *>(m_stackedWidget->currentWidget());
            if (!stack) {
                return;
            }
            m_backStack.append(stack->currentIndex());
            stack->setCurrentIndex((stack->currentIndex() + 1) % stack->count());
        });
        connect(backButton, &QPushButton::clicked, this, [this]() {
            if (m_backStack.isEmpty()) {
                return;
            }
            auto *stack = qobject_cast<TransitionStackedWidget *>(m_stackedWidget->currentWidget());
            if (stack) {
                stack->setCurrentIndex(m_backStack.takeLast(), -1, true);
            }
        });

        return panel;
    }

    QStackedWidget *m_stackedWidget = nullptr;
    EntranceTransitionStackedWidget *m_entranceStackedWidget = nullptr;
    DrillInTransitionStackedWidget *m_drillInStackedWidget = nullptr;
    QList<int> m_backStack;
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
