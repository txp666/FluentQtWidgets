#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QSize>
#include <QtCore/QtGlobal>
#include <QtGui/QFont>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

namespace {

class StatisticsWidget : public QWidget
{
  public:
    StatisticsWidget(const QString &title, const QString &value, QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *valueLabel = new BodyLabel(value, this);
        auto valueFont = valueLabel->font();
        valueFont.setPixelSize(18);
        valueFont.setWeight(QFont::DemiBold);
        valueLabel->setFont(valueFont);

        auto *titleLabel = new CaptionLabel(title, this);
        titleLabel->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 0, 16, 0);
        layout->addWidget(valueLabel, 0, Qt::AlignTop);
        layout->addWidget(titleLabel, 0, Qt::AlignBottom);
    }
};

class AppInfoCard : public SimpleCardWidget
{
  public:
    explicit AppInfoCard(QWidget *parent = nullptr) : SimpleCardWidget(parent)
    {
        auto *iconLabel = new ImageLabel(QStringLiteral(":/view_card/Python.svg"), this);
        iconLabel->setBorderRadius(8);
        iconLabel->scaledToWidth(120);

        auto *nameLabel = new TitleLabel(QStringLiteral("FluentQtWidgets"), this);
        auto *installButton = new PrimaryPushButton(QStringLiteral("Install"), this);
        installButton->setFixedWidth(160);
        auto *companyLabel = new HyperlinkLabel(QUrl(QStringLiteral(FQW_REPOSITORY_URL)),
                                              QStringLiteral("FluentQtWidgets"), this);

        auto *descriptionLabel = new BodyLabel(
            QStringLiteral("A Fluent Design inspired QWidget library for C++/Qt, aligned with the Python reference "
                           "control structure and standalone examples."),
            this);
        descriptionLabel->setWordWrap(true);

        auto *tagButton = new PillPushButton(QStringLiteral("Widgets"), this);
        tagButton->setCheckable(false);
        tagButton->setFixedSize(80, 32);

        auto *shareButton = new TransparentToolButton(icon(FluentIcon::More), this);
        shareButton->setFixedSize(32, 32);
        shareButton->setIconSize(QSize(14, 14));

        auto *root = new QHBoxLayout(this);
        root->setSpacing(30);
        root->setContentsMargins(34, 24, 24, 24);
        root->addWidget(iconLabel);

        auto *body = new QVBoxLayout;
        body->setContentsMargins(0, 0, 0, 0);
        body->setSpacing(0);
        root->addLayout(body);

        auto *top = new QHBoxLayout;
        body->addLayout(top);
        top->addWidget(nameLabel);
        top->addWidget(installButton, 0, Qt::AlignRight);
        body->addSpacing(3);
        body->addWidget(companyLabel);

        auto *statistics = new QHBoxLayout;
        statistics->setContentsMargins(0, 0, 0, 0);
        statistics->setSpacing(10);
        statistics->addWidget(new StatisticsWidget(QStringLiteral("Average"), QStringLiteral("5.0"), this));
        statistics->addWidget(new VerticalSeparator(this));
        statistics->addWidget(new StatisticsWidget(QStringLiteral("Ratings"), QStringLiteral("3K"), this));
        statistics->setAlignment(Qt::AlignLeft);
        body->addSpacing(20);
        body->addLayout(statistics);
        body->addSpacing(20);
        body->addWidget(descriptionLabel);

        auto *buttons = new QHBoxLayout;
        buttons->setContentsMargins(0, 0, 0, 0);
        buttons->addWidget(tagButton, 0, Qt::AlignLeft);
        buttons->addWidget(shareButton, 0, Qt::AlignRight);
        body->addSpacing(12);
        body->addLayout(buttons);
        setBorderRadius(8);
    }
};

class GalleryCard : public HeaderCardWidget
{
  public:
    explicit GalleryCard(QWidget *parent = nullptr) : HeaderCardWidget(QStringLiteral("Screenshots"), parent)
    {
        auto *expandButton = new TransparentToolButton(icon(FluentIcon::RightArrow), this);
        expandButton->setFixedSize(32, 32);
        expandButton->setIconSize(QSize(12, 12));
        headerLayout()->addWidget(expandButton, 0, Qt::AlignRight);

        auto *flipView = new HorizontalFlipView(this);
        flipView->addImages(QStringList{
            QStringLiteral(":/view_card/shoko1.jpg"),
            QStringLiteral(":/view_card/shoko2.jpg"),
            QStringLiteral(":/view_card/shoko3.jpg"),
            QStringLiteral(":/view_card/shoko4.jpg"),
        });
        flipView->setBorderRadius(8);
        flipView->setSpacing(10);
        viewLayout()->addWidget(flipView);
        setBorderRadius(8);
    }
};

class EmojiCard : public ElevatedCardWidget
{
  public:
    explicit EmojiCard(const QString &path, QWidget *parent = nullptr) : ElevatedCardWidget(parent)
    {
        auto *iconLabel = new ImageLabel(path, this);
        iconLabel->scaledToHeight(68);
        auto *label = new CaptionLabel(path.section('/', -1).section('.', 0, 0), this);

        auto *layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->addStretch(1);
        layout->addWidget(iconLabel, 0, Qt::AlignCenter);
        layout->addStretch(1);
        layout->addWidget(label, 0, Qt::AlignHCenter | Qt::AlignBottom);
        setFixedSize(168, 176);
    }
};

} // namespace

class Demo : public ScrollArea
{
  public:
    explicit Demo(QWidget *parent = nullptr) : ScrollArea(parent)
    {
        auto *content = new QWidget(this);
        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->setSpacing(16);
        layout->addWidget(new AppInfoCard(content));
        layout->addWidget(new GalleryCard(content));

        auto *emojiCard = new HeaderCardWidget(QStringLiteral("Emoji"), content);
        auto *flow = new FlowLayout(0, 12, 12, emojiCard->view());
        emojiCard->view()->setLayout(flow);
        for (const QString &path : {
                 QStringLiteral(":/view_card/Basketball.png"),
                 QStringLiteral(":/view_card/Chicken.png"),
                 QStringLiteral(":/view_card/Clown face.png"),
                 QStringLiteral(":/view_card/Drooling face.png"),
                 QStringLiteral(":/view_card/Face savoring food.png"),
                 QStringLiteral(":/view_card/Grinning face with sweat.png"),
                 QStringLiteral(":/view_card/Hot face.png"),
                 QStringLiteral(":/view_card/Partying face.png"),
                 QStringLiteral(":/view_card/Smiling face with hearts.png"),
             }) {
            flow->addWidget(new EmojiCard(path, emojiCard->view()));
        }
        layout->addWidget(emojiCard);
        layout->addStretch(1);

        setWidget(content);
        resize(880, 760);
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
