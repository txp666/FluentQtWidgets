#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QSize>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);

        auto *row1 = new QHBoxLayout;
        row1->setSpacing(20);
        row1->setSizeConstraint(QLayout::SetMinimumSize);
        row1->addStretch(1);
        row1->addWidget(InfoBadge::info(1));
        row1->addWidget(InfoBadge::success(10));
        row1->addWidget(InfoBadge::attension(100));
        row1->addWidget(InfoBadge::warning(1000));
        row1->addWidget(InfoBadge::error(10000));
        row1->addWidget(InfoBadge::custom(QStringLiteral("1w+"), QColor(QStringLiteral("#005fb8")),
                                          QColor(QStringLiteral("#60cdff"))));
        row1->addStretch(1);
        layout->addLayout(row1);

        auto *row2 = new QHBoxLayout;
        row2->setSpacing(20);
        row2->setSizeConstraint(QLayout::SetMinimumSize);
        row2->addStretch(1);
        row2->addWidget(DotInfoBadge::info());
        row2->addWidget(DotInfoBadge::success());
        row2->addWidget(DotInfoBadge::attension());
        row2->addWidget(DotInfoBadge::warning());
        row2->addWidget(DotInfoBadge::error());
        row2->addWidget(DotInfoBadge::custom(QColor(QStringLiteral("#005fb8")),
                                             QColor(QStringLiteral("#60cdff"))));
        row2->addStretch(1);
        layout->addLayout(row2);

        auto *row3 = new QHBoxLayout;
        row3->setSpacing(20);
        row3->setSizeConstraint(QLayout::SetMinimumSize);
        row3->addStretch(1);
        row3->addWidget(IconInfoBadge::info(FluentIcon::Check));
        row3->addWidget(IconInfoBadge::success(FluentIcon::Check));
        row3->addWidget(IconInfoBadge::attension(FluentIcon::Check));
        row3->addWidget(IconInfoBadge::warning(FluentIcon::Close));
        row3->addWidget(IconInfoBadge::error(FluentIcon::Close));
        auto *badge = IconInfoBadge::custom(FluentIcon::Ringer, QColor(QStringLiteral("#005fb8")),
                                            QColor(QStringLiteral("#60cdff")));
        badge->setFixedSize(32, 32);
        badge->setIconSize(QSize(16, 16));
        row3->addWidget(badge);
        row3->addStretch(1);
        layout->addLayout(row3);

        auto *button = new ToolButton(icon(FluentIcon::Basketball), this);
        layout->addWidget(button, 0, Qt::AlignHCenter);
        InfoBadge::success(1, this, button, InfoBadgePosition::TopRight);
        resize(450, 400);
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
