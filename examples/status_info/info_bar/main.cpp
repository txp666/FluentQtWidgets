#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *button1 = new PushButton(QStringLiteral("Information"), this);
        auto *button2 = new PushButton(QStringLiteral("Success"), this);
        auto *button3 = new PushButton(QStringLiteral("Warning"), this);
        auto *button4 = new PushButton(QStringLiteral("Error"), this);
        auto *button5 = new PushButton(QStringLiteral("Custom"), this);
        auto *button6 = new PushButton(QStringLiteral("Desktop"), this);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(30, 0, 30, 0);
        layout->addWidget(button1);
        layout->addWidget(button2);
        layout->addWidget(button3);
        layout->addWidget(button4);
        layout->addWidget(button5);
        layout->addWidget(button6);
        resize(700, 700);

        connect(button1, &QPushButton::clicked, this, &Demo::createInfoInfoBar);
        connect(button2, &QPushButton::clicked, this, &Demo::createSuccessInfoBar);
        connect(button3, &QPushButton::clicked, this, &Demo::createWarningInfoBar);
        connect(button4, &QPushButton::clicked, this, &Demo::createErrorInfoBar);
        connect(button5, &QPushButton::clicked, this, &Demo::createCustomInfoBar);
        connect(button6, &QPushButton::clicked, this, &Demo::createDeskTopBottomRightInfoBar);
    }

  private:
    void createInfoInfoBar()
    {
        const QString content =
            QStringLiteral("My name is kira yoshikake, 33 years old. Living in the villa area northeast of "
                           "duwangting, unmarried. I work in Guiyou chain store. Every day I have to work "
                           "overtime until 8 p.m. to go home. I don't smoke. The wine is only for a taste. "
                           "Sleep at 11 p.m. for 8 hours a day. Before I go to bed, I must drink a cup of "
                           "warm milk, then do 20 minutes of soft exercise, get on the bed, and immediately "
                           "fall asleep. Never leave fatigue and stress until the next day. Doctors say I'm normal.");
        auto *bar = new InfoBar(InfoBarIcon::Information, QStringLiteral("Title"), content,
                                Qt::Vertical, true, 2000, InfoBarPosition::TopRight, this);
        bar->addWidget(new PushButton(QStringLiteral("Action"), bar));
        bar->show();
    }

    void createSuccessInfoBar()
    {
        InfoBar::success(QStringLiteral("Lesson 4"),
                         QStringLiteral("With respect, let's advance towards a new stage of the spin."),
                         Qt::Horizontal, true, 2000, InfoBarPosition::Top, this);
    }

    void createWarningInfoBar()
    {
        InfoBar::warning(QStringLiteral("Lesson 3"),
                         QStringLiteral("Believe in the spin, just keep believing!"),
                         Qt::Horizontal, false, 2000, InfoBarPosition::TopLeft, this);
    }

    void createErrorInfoBar()
    {
        InfoBar::error(QStringLiteral("Lesson 5"), QStringLiteral("迂回路を行けば最短ルート。"),
                       Qt::Horizontal, true, -1, InfoBarPosition::BottomRight, this);
    }

    void createCustomInfoBar()
    {
        auto *bar = InfoBar::newInfoBar(FluentIcon::GitHub, QStringLiteral("Zeppeli"),
                                        QStringLiteral("人間讃歌は「勇気」の讃歌ッ！！"),
                                        Qt::Horizontal, true, 2000, InfoBarPosition::Bottom, this);
        bar->setCustomBackgroundColor(Qt::white, QColor(QStringLiteral("#202020")));
    }

    void createDeskTopBottomRightInfoBar()
    {
        InfoBar::warning(QStringLiteral("Plugged Out Notify"), QStringLiteral("Battery is 64%"),
                         Qt::Vertical, true, 2000, InfoBarPosition::BottomRight, nullptr);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo w;
    w.show();
    return QApplication::exec();
}
