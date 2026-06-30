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

class CustomFlyoutView : public AcrylicFlyoutViewBase
{
public:
    explicit CustomFlyoutView(QWidget *parent = nullptr) : AcrylicFlyoutViewBase(parent)
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
        auto *button1 = new PushButton(QStringLiteral("Click Me"), this);
        auto *button2 = new PushButton(QStringLiteral("Click Me"), this);
        auto *button3 = new PushButton(QStringLiteral("Click Me"), this);
        button1->setFixedWidth(150);
        button2->setFixedWidth(150);
        button3->setFixedWidth(150);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(30, 50, 30, 50);
        layout->addWidget(button1, 0, Qt::AlignBottom);
        layout->addWidget(button2, 0, Qt::AlignBottom);
        layout->addWidget(button3, 0, Qt::AlignBottom);
        resize(750, 550);

        connect(button1, &QPushButton::clicked, this, [this, button1]() {
            AcrylicFlyout::create(QStringLiteral("Lesson 4"),
                                  QStringLiteral("表达敬意吧，表达出敬意，然后迈向回旋的另一个全新阶段！"),
                                  icon(FluentIcon::Accept), QPixmap(), true, button1, this);
        });

        connect(button2, &QPushButton::clicked, this, [this, button2]() {
            auto *view = new AcrylicFlyoutView(
                QStringLiteral("杰洛·齐贝林"),
                QStringLiteral("触网而起的网球会落到哪一侧，谁也无法知晓。\n"
                               "如果那种时刻到来，我希望「女神」是存在的。\n"
                               "这样的话，不管网球落到哪一边，我都会坦然接受的吧。"),
                QIcon(), resourcePath(QStringLiteral("SBR.jpg")), true);

            auto *button = new PushButton(QStringLiteral("Action"));
            button->setFixedWidth(120);
            view->addWidget(button, 0, Qt::AlignRight);
            view->widgetLayout()->insertSpacing(1, 5);
            view->widgetLayout()->addSpacing(5);

            auto *flyout = AcrylicFlyout::make(view, button2, this);
            connect(view, &FlyoutViewBase::closed, flyout, &Flyout::close);
        });

        connect(button3, &QPushButton::clicked, this, [this, button3]() {
            AcrylicFlyout::make(new CustomFlyoutView, button3, this, FlyoutAnimationType::DropDown);
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
