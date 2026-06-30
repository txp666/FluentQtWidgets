#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

QString resourcePath(const QString &name)
{
    return QStringLiteral(FQW_EXAMPLE_RESOURCE_DIR) + QLatin1Char('/') + name;
}

class CustomFlyoutView : public FlyoutViewBase
{
  public:
    explicit CustomFlyoutView(QWidget *parent = nullptr) : FlyoutViewBase(parent)
    {
        auto *label = new BodyLabel(
            QStringLiteral("这是一场「试炼」，我认为这就是一场为了战胜过去的「试炼」，\n"
                           "只有战胜了那些幼稚的过去，人才能有所成长。"),
            this);
        auto *button = new PrimaryPushButton(QStringLiteral("Action"), this);
        button->setFixedWidth(140);

        auto *layout = new QVBoxLayout(this);
        layout->setSpacing(12);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->addWidget(label);
        layout->addWidget(button);
    }

    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::AlignLeft) override
    {
        if (auto *box = qobject_cast<QVBoxLayout *>(layout())) {
            box->addWidget(widget, stretch, alignment);
        }
    }
};

} // namespace

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *button1 = new PushButton(QStringLiteral("Top"), this);
        auto *button2 = new PushButton(QStringLiteral("Bottom"), this);
        auto *button3 = new PushButton(QStringLiteral("Custom"), this);
        button1->setFixedWidth(150);
        button2->setFixedWidth(150);
        button3->setFixedWidth(150);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(button2, 0, Qt::AlignHCenter);
        layout->addWidget(button1, 0, Qt::AlignHCenter);
        layout->addWidget(button3, 0, Qt::AlignHCenter);
        resize(700, 500);

        connect(button1, &QPushButton::clicked, this, [this, button1]() {
            const auto position = TeachingTipTailPosition::Bottom;
            auto *view = new TeachingTipView(
                QStringLiteral("Lesson 5"),
                QStringLiteral("最短的捷径就是绕远路，绕远路才是我的最短捷径。"),
                QIcon(), resourcePath(QStringLiteral("Gyro.jpg")), true, position);

            auto *button = new PushButton(QStringLiteral("Action"));
            button->setFixedWidth(120);
            view->addWidget(button, 0, Qt::AlignRight);

            auto *tip = TeachingTip::make(view, button1, position, -1, this);
            connect(view, &FlyoutViewBase::closed, tip, &TeachingTip::close);
        });

        connect(button2, &QPushButton::clicked, this, [this, button2]() {
            TeachingTip::create(QStringLiteral("Lesson 4"),
                                QStringLiteral("表达敬意吧，表达出敬意，然后迈向回旋的另一个全新阶段！"),
                                icon(FluentIcon::Accept), QPixmap(), true, button2,
                                TeachingTipTailPosition::Top, 2000, this);
        });

        connect(button3, &QPushButton::clicked, this, [this, button3]() {
            PopupTeachingTip::make(new CustomFlyoutView, button3, TeachingTipTailPosition::Right, 2000, this);
        });
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
