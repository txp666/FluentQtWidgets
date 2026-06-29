#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *tree = new TreeWidget(this);
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 8, 0, 0);
        layout->addWidget(tree);

        auto *item1 = new QTreeWidgetItem(QStringList{tr("JoJo 1 - Phantom Blood")});
        item1->addChildren({
            new QTreeWidgetItem(QStringList{tr("Jonathan Joestar")}),
            new QTreeWidgetItem(QStringList{tr("Dio Brando")}),
            new QTreeWidgetItem(QStringList{tr("Will A. Zeppeli")}),
        });
        tree->addTopLevelItem(item1);

        auto *item2 = new QTreeWidgetItem(QStringList{tr("JoJo 3 - Stardust Crusaders")});
        for (int branch = 0; branch < 3; ++branch) {
            auto *jotaro = new QTreeWidgetItem(QStringList{tr("Jotaro Kujo")});
            for (int i = 0; i < 10; ++i) {
                jotaro->addChildren({
                    new QTreeWidgetItem(QStringList{QStringLiteral("空条承太郎")}),
                    new QTreeWidgetItem(QStringList{QStringLiteral("空条蕉太狼")}),
                    new QTreeWidgetItem(QStringList{QStringLiteral("阿强")}),
                    new QTreeWidgetItem(QStringList{QStringLiteral("卖鱼强")}),
                    new QTreeWidgetItem(QStringList{QStringLiteral("那个无敌的男人")}),
                });
            }
            item2->addChild(jotaro);
        }
        tree->addTopLevelItem(item2);
        tree->expandAll();
        tree->setHeaderHidden(true);
        resize(400, 500);
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
