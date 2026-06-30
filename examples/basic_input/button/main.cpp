#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

RoundMenu *createMailMenu(QWidget *parent)
{
    auto *menu = new RoundMenu(parent);
    menu->addAction(FluentIcon::Send, QStringLiteral("Send"));
    menu->addAction(FluentIcon::Save, QStringLiteral("Save"));
    return menu;
}

RoundMenu *createActionMenu(QWidget *parent)
{
    auto *menu = new RoundMenu(parent);
    menu->addAction(FluentIcon::Basketball, QStringLiteral("Basketball"));
    menu->addAction(FluentIcon::Album, QStringLiteral("Sing"));
    menu->addAction(FluentIcon::Music, QStringLiteral("Music"));
    return menu;
}

class ButtonView : public QWidget
{
  public:
    explicit ButtonView(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);
    }
};

class ToolButtonDemo : public ButtonView
{
  public:
    explicit ToolButtonDemo(QWidget *parent = nullptr) : ButtonView(parent)
    {
        auto *menu = createMailMenu(this);

        auto *toolButton = new ToolButton(icon(FluentIcon::Setting), this);
        auto *dropDownToolButton = new DropDownToolButton(icon(FluentIcon::Mail), this);
        dropDownToolButton->setMenu(menu);

        auto *splitToolButton = new SplitToolButton(icon(FluentIcon::GitHub), this);
        splitToolButton->setFlyout(menu);

        auto *primaryToolButton = new PrimaryToolButton(icon(FluentIcon::Setting), this);

        auto *primaryDropDownToolButton = new PrimaryDropDownToolButton(icon(FluentIcon::Mail), this);
        primaryDropDownToolButton->setMenu(menu);

        auto *primarySplitToolButton = new PrimarySplitToolButton(icon(FluentIcon::GitHub), this);
        primarySplitToolButton->setFlyout(menu);

        auto *toggleToolButton = new ToggleToolButton(icon(FluentIcon::Setting), this);
        connect(toggleToolButton, &QToolButton::toggled, [](bool) { qDebug() << "Toggled"; });
        toggleToolButton->toggle();

        auto *transparentToggleToolButton = new TransparentToggleToolButton(icon(FluentIcon::GitHub), this);
        auto *transparentToolButton = new TransparentToolButton(icon(FluentIcon::Mail), this);

        auto *transparentDropDownToolButton = new TransparentDropDownToolButton(icon(FluentIcon::Mail), this);
        transparentDropDownToolButton->setMenu(menu);

        auto *pillToolButton1 = new PillToolButton(icon(FluentIcon::Calendar), this);
        auto *pillToolButton2 = new PillToolButton(icon(FluentIcon::Calendar), this);
        auto *pillToolButton3 = new PillToolButton(icon(FluentIcon::Calendar), this);
        pillToolButton2->setDisabled(true);
        pillToolButton3->setChecked(true);
        pillToolButton3->setDisabled(true);

        auto *layout = new QGridLayout(this);
        layout->addWidget(toolButton, 0, 0);
        layout->addWidget(dropDownToolButton, 0, 1);
        layout->addWidget(splitToolButton, 0, 2);
        layout->addWidget(primaryToolButton, 1, 0);
        layout->addWidget(primaryDropDownToolButton, 1, 1);
        layout->addWidget(primarySplitToolButton, 1, 2);
        layout->addWidget(toggleToolButton, 2, 0);
        layout->addWidget(transparentToggleToolButton, 2, 1);
        layout->addWidget(transparentToolButton, 3, 0);
        layout->addWidget(transparentDropDownToolButton, 3, 1);
        layout->addWidget(pillToolButton1, 4, 0);
        layout->addWidget(pillToolButton2, 4, 1);
        layout->addWidget(pillToolButton3, 4, 2);

        resize(300, 300);
    }
};

class PushButtonDemo : public ButtonView
{
  public:
    explicit PushButtonDemo(QWidget *parent = nullptr) : ButtonView(parent)
    {
        auto *menu = createActionMenu(this);

        auto *pushButton1 = new PushButton(QStringLiteral("Standard push button"), this);
        auto *pushButton2 =
            new PushButton(icon(FluentIcon::Folder), QStringLiteral("Standard push button with icon"), this);

        auto *primaryButton1 = new PrimaryPushButton(QStringLiteral("Accent style button"), this);
        auto *primaryButton2 =
            new PrimaryPushButton(icon(FluentIcon::Update), QStringLiteral("Accent style button with icon"), this);

        auto *transparentPushButton1 = new TransparentPushButton(QStringLiteral("Transparent push button"), this);
        auto *transparentPushButton2 =
            new TransparentPushButton(icon(FluentIcon::BookShelf), QStringLiteral("Transparent push button"), this);

        auto *toggleButton1 = new TogglePushButton(QStringLiteral("Toggle push button"), this);
        auto *toggleButton2 = new TogglePushButton(icon(FluentIcon::Send), QStringLiteral("Toggle push button"), this);

        auto *transparentTogglePushButton1 =
            new TransparentTogglePushButton(QStringLiteral("Transparent toggle button"), this);
        auto *transparentTogglePushButton2 =
            new TransparentTogglePushButton(icon(FluentIcon::BookShelf), QStringLiteral("Transparent toggle button"), this);

        auto *dropDownPushButton1 = new DropDownPushButton(QStringLiteral("Email"), this);
        auto *dropDownPushButton2 = new DropDownPushButton(icon(FluentIcon::Mail), QStringLiteral("Email"), this);
        dropDownPushButton1->setMenu(menu);
        dropDownPushButton2->setMenu(menu);

        auto *primaryDropDownPushButton1 = new PrimaryDropDownPushButton(QStringLiteral("Email"), this);
        auto *primaryDropDownPushButton2 =
            new PrimaryDropDownPushButton(icon(FluentIcon::Mail), QStringLiteral("Email"), this);
        primaryDropDownPushButton1->setMenu(menu);
        primaryDropDownPushButton2->setMenu(menu);

        auto *transparentDropDownPushButton1 = new TransparentDropDownPushButton(QStringLiteral("Email"), this);
        auto *transparentDropDownPushButton2 =
            new TransparentDropDownPushButton(icon(FluentIcon::Mail), QStringLiteral("Email"), this);
        transparentDropDownPushButton1->setMenu(menu);
        transparentDropDownPushButton2->setMenu(menu);

        auto *splitPushButton1 = new SplitPushButton(QStringLiteral("Split push button"), this);
        auto *splitPushButton2 =
            new SplitPushButton(icon(FluentIcon::GitHub), QStringLiteral("Split push button"), this);
        splitPushButton1->setFlyout(menu);
        splitPushButton2->setFlyout(menu);

        auto *primarySplitPushButton1 = new PrimarySplitPushButton(QStringLiteral("Split push button"), this);
        auto *primarySplitPushButton2 =
            new PrimarySplitPushButton(icon(FluentIcon::GitHub), QStringLiteral("Split push button"), this);
        primarySplitPushButton1->setFlyout(menu);
        primarySplitPushButton2->setFlyout(menu);

        auto *hyperlinkButton1 = new HyperlinkButton(QStringLiteral(FQW_REPOSITORY_URL),
                                                     QStringLiteral("Hyper link button"), this);
        auto *hyperlinkButton2 = new HyperlinkButton(icon(FluentIcon::Link),
                                                     QStringLiteral(FQW_REPOSITORY_URL),
                                                     QStringLiteral("Hyper link button"), this);

        auto *pillPushButton1 = new PillPushButton(QStringLiteral("Pill Push Button"), this);
        auto *pillPushButton2 =
            new PillPushButton(icon(FluentIcon::Calendar), QStringLiteral("Pill Push Button"), this);

        auto *layout = new QGridLayout(this);
        layout->addWidget(pushButton1, 0, 0);
        layout->addWidget(pushButton2, 0, 1);
        layout->addWidget(primaryButton1, 1, 0);
        layout->addWidget(primaryButton2, 1, 1);
        layout->addWidget(transparentPushButton1, 2, 0);
        layout->addWidget(transparentPushButton2, 2, 1);

        layout->addWidget(toggleButton1, 3, 0);
        layout->addWidget(toggleButton2, 3, 1);
        layout->addWidget(transparentTogglePushButton1, 4, 0);
        layout->addWidget(transparentTogglePushButton2, 4, 1);

        layout->addWidget(splitPushButton1, 5, 0);
        layout->addWidget(splitPushButton2, 5, 1);
        layout->addWidget(primarySplitPushButton1, 6, 0);
        layout->addWidget(primarySplitPushButton2, 6, 1);

        layout->addWidget(dropDownPushButton1, 7, 0, Qt::AlignLeft);
        layout->addWidget(dropDownPushButton2, 7, 1, Qt::AlignLeft);
        layout->addWidget(primaryDropDownPushButton1, 8, 0, Qt::AlignLeft);
        layout->addWidget(primaryDropDownPushButton2, 8, 1, Qt::AlignLeft);
        layout->addWidget(transparentDropDownPushButton1, 9, 0, Qt::AlignLeft);
        layout->addWidget(transparentDropDownPushButton2, 9, 1, Qt::AlignLeft);

        layout->addWidget(pillPushButton1, 10, 0, Qt::AlignLeft);
        layout->addWidget(pillPushButton2, 10, 1, Qt::AlignLeft);

        layout->addWidget(hyperlinkButton1, 11, 0, Qt::AlignLeft);
        layout->addWidget(hyperlinkButton2, 11, 1, Qt::AlignLeft);

        resize(600, 700);
    }
};

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    ToolButtonDemo toolDemo;
    toolDemo.show();

    PushButtonDemo pushDemo;
    pushDemo.show();

    return QApplication::exec();
}
