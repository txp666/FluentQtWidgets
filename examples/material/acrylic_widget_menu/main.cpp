#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPoint>
#include <QtCore/Qt>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

QString resourcePath(const QString &fileName)
{
#ifdef FQW_EXAMPLE_RESOURCE_DIR
    return QStringLiteral(FQW_EXAMPLE_RESOURCE_DIR) + QLatin1Char('/') + fileName;
#else
    return QStringLiteral("resource/") + fileName;
#endif
}

class ProfileCard : public QWidget
{
  public:
    ProfileCard(const QString &avatarPath, const QString &name, const QString &email, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        auto *avatar = new AvatarWidget(avatarPath, this);
        auto *nameLabel = new BodyLabel(name, this);
        auto *emailLabel = new CaptionLabel(email, this);
        auto *logoutButton = new HyperlinkButton(QStringLiteral(FQW_REPOSITORY_URL),
                                                 QStringLiteral("Logout"), this);

        nameLabel->setTextColor(QColor(0, 0, 0), QColor(255, 255, 255));
        emailLabel->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));
        QFont linkFont = logoutButton->font();
        linkFont.setPixelSize(13);
        logoutButton->setFont(linkFont);

        setFixedSize(307, 82);
        avatar->setRadius(24);
        avatar->setFixedSize(48, 48);
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
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *layout = new QHBoxLayout(this);
        auto *label = new BodyLabel(QStringLiteral("Right-click your mouse"), this);
        QFont labelFont = label->font();
        labelFont.setPixelSize(18);
        label->setFont(labelFont);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        resize(400, 400);
    }

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override
    {
        auto *menu = new AcrylicMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        auto *card = new ProfileCard(resourcePath(QStringLiteral("shoko.png")), QStringLiteral("Shoko"),
                                     QStringLiteral("shokokawaii@outlook.com"), menu);
        menu->addWidget(card, false);
        menu->addSeparator();
        menu->addAction(FluentIcon::People, QStringLiteral("Manage account and settings"));
        menu->addAction(FluentIcon::ShoppingCart, QStringLiteral("Payment method"));
        menu->addAction(FluentIcon::Code, QStringLiteral("Redeem code or gift card"));
        menu->addSeparator();
        menu->addAction(FluentIcon::Setting, QStringLiteral("Settings"));
        menu->exec(event->globalPos(), true, MenuAnimationType::DropDown);
    }
};

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Demo window;
    window.show();
    return app.exec();
}
