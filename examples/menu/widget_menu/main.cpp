#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class ProfileCard : public QWidget
{
  public:
    explicit ProfileCard(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *avatar = new AvatarWidget(QStringLiteral(":/widget_menu/shoko.png"), this);
        auto *nameLabel = new BodyLabel(QStringLiteral("硝子酱"), this);
        auto *emailLabel = new CaptionLabel(QStringLiteral("shokokawaii@outlook.com"), this);
        auto *logoutButton = new HyperlinkButton(QStringLiteral(FQW_REPOSITORY_URL),
                                                 QStringLiteral("注销"), this);

        emailLabel->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));
        nameLabel->setTextColor(QColor(0, 0, 0), QColor(255, 255, 255));
        auto font = logoutButton->font();
        font.setPixelSize(13);
        logoutButton->setFont(font);

        setFixedSize(307, 82);
        avatar->setRadius(24);
        avatar->move(2, 6);
        nameLabel->move(64, 13);
        emailLabel->move(64, 32);
        logoutButton->move(52, 48);
    }
};

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *label = new BodyLabel(QStringLiteral("Right-click your mouse"), this);
        label->setAlignment(Qt::AlignCenter);
        auto font = label->font();
        font.setPixelSize(18);
        label->setFont(font);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(label);
        resize(400, 400);
        setStyleSheet(QStringLiteral("Demo{background:white}"));
    }

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override
    {
        RoundMenu menu(this);
        menu.addWidget(new ProfileCard(&menu), false);
        menu.addSeparator();
        menu.addAction(FluentIcon::People, QStringLiteral("管理账户和设置"));
        menu.addAction(FluentIcon::ShoppingCart, QStringLiteral("支付方式"));
        menu.addAction(FluentIcon::Code, QStringLiteral("兑换代码和礼品卡"));
        menu.addSeparator();
        menu.addAction(FluentIcon::Settings, QStringLiteral("设置"));
        menu.exec(event->globalPos(), true);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo window;
    window.show();
    return QApplication::exec();
}
