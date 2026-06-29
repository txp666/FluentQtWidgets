#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *listWidget = new ListWidget(this);
        const QStringList stands = {
            QStringLiteral("白金之星"), QStringLiteral("绿色法皇"), QStringLiteral("天堂制造"),
            QStringLiteral("绯红之王"), QStringLiteral("银色战车"), QStringLiteral("疯狂钻石"),
            QStringLiteral("壮烈成仁"), QStringLiteral("败者食尘"), QStringLiteral("黑蚊子多"),
            QStringLiteral("杀手皇后"), QStringLiteral("金属制品"), QStringLiteral("石之自由"),
            QStringLiteral("砸瓦鲁多"), QStringLiteral("钢链手指"), QStringLiteral("臭氧宝宝"),
            QStringLiteral("华丽挚爱"), QStringLiteral("隐者之紫"), QStringLiteral("黄金体验"),
            QStringLiteral("虚无之王"), QStringLiteral("纸月之王"), QStringLiteral("骇人恶兽"),
            QStringLiteral("男子领域"), QStringLiteral("20世纪男孩"), QStringLiteral("牙 Act 4"),
            QStringLiteral("铁球破坏者"), QStringLiteral("性感手枪"), QStringLiteral("D4C • 爱之列车"),
            QStringLiteral("天生完美"), QStringLiteral("软又湿"), QStringLiteral("佩斯利公园"),
            QStringLiteral("奇迹于你"), QStringLiteral("行走的心"), QStringLiteral("护霜旅行者"),
            QStringLiteral("十一月雨"), QStringLiteral("调情圣手"), QStringLiteral("片刻静候"),
        };
        listWidget->addItems(stands);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(listWidget);
        setStyleSheet(QStringLiteral("Demo{background: rgb(249, 249, 249)}"));
        resize(300, 400);
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
