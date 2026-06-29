#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *tableView = new TableWidget(this);
        tableView->setBorderVisible(true);
        tableView->setBorderRadius(8);
        tableView->setWordWrap(false);
        tableView->setRowCount(60);
        tableView->setColumnCount(5);

        const QList<QStringList> songs = {
            {QStringLiteral("かばん"), QStringLiteral("aiko"), QStringLiteral("かばん"), QStringLiteral("2004"),
             QStringLiteral("5:04")},
            {QStringLiteral("爱你"), QStringLiteral("王心凌"), QStringLiteral("爱你"), QStringLiteral("2004"),
             QStringLiteral("3:39")},
            {QStringLiteral("星のない世界"), QStringLiteral("aiko"), QStringLiteral("星のない世界/横顔"),
             QStringLiteral("2007"), QStringLiteral("5:30")},
            {QStringLiteral("横顔"), QStringLiteral("aiko"), QStringLiteral("星のない世界/横顔"),
             QStringLiteral("2007"), QStringLiteral("5:06")},
            {QStringLiteral("秘密"), QStringLiteral("aiko"), QStringLiteral("秘密"), QStringLiteral("2008"),
             QStringLiteral("6:27")},
            {QStringLiteral("シアワセ"), QStringLiteral("aiko"), QStringLiteral("秘密"), QStringLiteral("2008"),
             QStringLiteral("5:25")},
            {QStringLiteral("二人"), QStringLiteral("aiko"), QStringLiteral("二人"), QStringLiteral("2008"),
             QStringLiteral("5:00")},
            {QStringLiteral("スパークル"), QStringLiteral("RADWIMPS"), QStringLiteral("君の名は。"),
             QStringLiteral("2016"), QStringLiteral("8:54")},
            {QStringLiteral("なんでもないや"), QStringLiteral("RADWIMPS"), QStringLiteral("君の名は。"),
             QStringLiteral("2016"), QStringLiteral("3:16")},
            {QStringLiteral("前前前世"), QStringLiteral("RADWIMPS"), QStringLiteral("人間開花"),
             QStringLiteral("2016"), QStringLiteral("4:35")},
            {QStringLiteral("恋をしたのは"), QStringLiteral("aiko"), QStringLiteral("恋をしたのは"),
             QStringLiteral("2016"), QStringLiteral("6:02")},
            {QStringLiteral("夏バテ"), QStringLiteral("aiko"), QStringLiteral("恋をしたのは"),
             QStringLiteral("2016"), QStringLiteral("4:41")},
            {QStringLiteral("もっと"), QStringLiteral("aiko"), QStringLiteral("もっと"), QStringLiteral("2016"),
             QStringLiteral("4:50")},
            {QStringLiteral("問題集"), QStringLiteral("aiko"), QStringLiteral("もっと"), QStringLiteral("2016"),
             QStringLiteral("4:18")},
            {QStringLiteral("半袖"), QStringLiteral("aiko"), QStringLiteral("もっと"), QStringLiteral("2016"),
             QStringLiteral("5:50")},
            {QStringLiteral("ひねくれ"), QStringLiteral("鎖那"), QStringLiteral("Hush a by little girl"),
             QStringLiteral("2017"), QStringLiteral("3:54")},
            {QStringLiteral("シュテルン"), QStringLiteral("鎖那"), QStringLiteral("Hush a by little girl"),
             QStringLiteral("2017"), QStringLiteral("3:16")},
            {QStringLiteral("愛は勝手"), QStringLiteral("aiko"), QStringLiteral("湿った夏の始まり"),
             QStringLiteral("2018"), QStringLiteral("5:31")},
            {QStringLiteral("ドライブモード"), QStringLiteral("aiko"), QStringLiteral("湿った夏の始まり"),
             QStringLiteral("2018"), QStringLiteral("3:37")},
            {QStringLiteral("うん。"), QStringLiteral("aiko"), QStringLiteral("湿った夏の始まり"),
             QStringLiteral("2018"), QStringLiteral("5:48")},
            {QStringLiteral("キラキラ"), QStringLiteral("aiko"), QStringLiteral("aikoの詩。"),
             QStringLiteral("2019"), QStringLiteral("5:08")},
            {QStringLiteral("恋のスーパーボール"), QStringLiteral("aiko"), QStringLiteral("aikoの詩。"),
             QStringLiteral("2019"), QStringLiteral("4:31")},
            {QStringLiteral("磁石"), QStringLiteral("aiko"), QStringLiteral("どうしたって伝えられないから"),
             QStringLiteral("2021"), QStringLiteral("4:24")},
            {QStringLiteral("食べた愛"), QStringLiteral("aiko"), QStringLiteral("食べた愛/あたしたち"),
             QStringLiteral("2021"), QStringLiteral("5:17")},
            {QStringLiteral("列車"), QStringLiteral("aiko"), QStringLiteral("食べた愛/あたしたち"),
             QStringLiteral("2021"), QStringLiteral("4:18")},
            {QStringLiteral("花の塔"), QStringLiteral("さユり"), QStringLiteral("花の塔"), QStringLiteral("2022"),
             QStringLiteral("4:35")},
            {QStringLiteral("夏恋のライフ"), QStringLiteral("aiko"), QStringLiteral("夏恋のライフ"),
             QStringLiteral("2022"), QStringLiteral("5:03")},
            {QStringLiteral("あかときリロード"), QStringLiteral("aiko"), QStringLiteral("あかときリロード"),
             QStringLiteral("2023"), QStringLiteral("4:04")},
            {QStringLiteral("荒れた唇は恋を失くす"), QStringLiteral("aiko"),
             QStringLiteral("今の二人をお互いが見てる"), QStringLiteral("2023"), QStringLiteral("4:07")},
            {QStringLiteral("ワンツースリー"), QStringLiteral("aiko"), QStringLiteral("今の二人をお互いが見てる"),
             QStringLiteral("2023"), QStringLiteral("4:47")},
        };

        for (int row = 0; row < tableView->rowCount(); ++row) {
            const QStringList song = songs.at(row % songs.size());
            for (int column = 0; column < song.size(); ++column) {
                tableView->setItem(row, column, new QTableWidgetItem(song.at(column)));
            }
        }

        tableView->verticalHeader()->hide();
        tableView->setHorizontalHeaderLabels({QStringLiteral("Title"), QStringLiteral("Artist"),
                                              QStringLiteral("Album"), QStringLiteral("Year"),
                                              QStringLiteral("Duration")});
        tableView->resizeColumnsToContents();

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(50, 30, 50, 30);
        layout->addWidget(tableView);
        setStyleSheet(QStringLiteral("Demo{background: rgb(255, 255, 255)}"));
        resize(735, 760);
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
