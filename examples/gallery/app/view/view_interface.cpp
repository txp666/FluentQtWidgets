#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtWidgets/QTreeWidgetItemIterator>

using namespace FluentQt;

QWidget *GalleryWindow::createViewsPage()
{
    auto *page = new GalleryInterface(navTx("View"), QStringLiteral("FluentQtWidgets::Views"), this);
    const QString listViewSource = exampleSourceUrl("view/list_view");
    const QString tableViewSource = exampleSourceUrl("view/table_view");
    const QString treeViewSource = exampleSourceUrl("view/tree_view");
    const QString flipViewSource = exampleSourceUrl("view/flip_view");

    // List view
    auto *listWidget = new ListWidget;
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    const QStringList stands = {
        QStringLiteral("Star Platinum"),       QStringLiteral("Hierophant Green"),
        QStringLiteral("Made in Haven"),        QStringLiteral("King Crimson"),
        QStringLiteral("Silver Chariot"),       QStringLiteral("Crazy diamond"),
        QStringLiteral("Metallica"),            QStringLiteral("Another One Bites The Dust"),
        QStringLiteral("Heaven's Door"),        QStringLiteral("Killer Queen"),
        QStringLiteral("The Grateful Dead"),    QStringLiteral("Stone Free"),
        QStringLiteral("The World"),            QStringLiteral("Sticky Fingers"),
        QStringLiteral("Ozone Baby"),           QStringLiteral("Love Love Deluxe"),
        QStringLiteral("Hermit Purple"),        QStringLiteral("Gold Experience"),
        QStringLiteral("King Nothing"),         QStringLiteral("Paper Moon King"),
        QStringLiteral("Scary Monster"),        QStringLiteral("Mandom"),
        QStringLiteral("20th Century Boy"),     QStringLiteral("Tusk Act 4"),
        QStringLiteral("Ball Breaker"),         QStringLiteral("Sex Pistols"),
        QStringLiteral("D4C • Love Train"),     QStringLiteral("Born This Way"),
        QStringLiteral("SOFT & WET"),           QStringLiteral("Paisley Park"),
        QStringLiteral("Wonder of U"),          QStringLiteral("Walking Heart"),
        QStringLiteral("Cream Starter"),        QStringLiteral("November Rain"),
        QStringLiteral("Smooth Operators"),     QStringLiteral("The Matte Kudasai"),
    };
    listWidget->addItems(stands);
    listWidget->setMinimumHeight(380);
    listWidget->setMaximumHeight(380);
    page->addExampleCard(navTx("A simple ListView"), listWidget, listViewSource);

    // Table view
    auto *tableWidget = new TableWidget;
    tableWidget->verticalHeader()->hide();
    tableWidget->setBorderRadius(8);
    tableWidget->setBorderVisible(true);
    tableWidget->setColumnCount(5);
    tableWidget->setHorizontalHeaderLabels({
        tx("TableFrame", "Title"),
        tx("TableFrame", "Artist"),
        tx("TableFrame", "Album"),
        tx("TableFrame", "Year"),
        tx("TableFrame", "Duration"),
    });

    const QList<QStringList> songs = {
        {QStringLiteral("かばん"), QStringLiteral("aiko"), QStringLiteral("かばん"), QStringLiteral("2004"), QStringLiteral("5:04")},
        {QStringLiteral("爱你"), QStringLiteral("王心凌"), QStringLiteral("爱你"), QStringLiteral("2004"), QStringLiteral("3:39")},
        {QStringLiteral("星のない世界"), QStringLiteral("aiko"), QStringLiteral("星のない世界/横顔"), QStringLiteral("2007"), QStringLiteral("5:30")},
        {QStringLiteral("横顔"), QStringLiteral("aiko"), QStringLiteral("星のない世界/横顔"), QStringLiteral("2007"), QStringLiteral("5:06")},
        {QStringLiteral("秘密"), QStringLiteral("aiko"), QStringLiteral("秘密"), QStringLiteral("2008"), QStringLiteral("6:27")},
        {QStringLiteral("シアワセ"), QStringLiteral("aiko"), QStringLiteral("秘密"), QStringLiteral("2008"), QStringLiteral("5:25")},
        {QStringLiteral("二人"), QStringLiteral("aiko"), QStringLiteral("二人"), QStringLiteral("2008"), QStringLiteral("5:00")},
        {QStringLiteral("スパークル"), QStringLiteral("RADWIMPS"), QStringLiteral("君の名は。"), QStringLiteral("2016"), QStringLiteral("8:54")},
        {QStringLiteral("なんでもないや"), QStringLiteral("RADWIMPS"), QStringLiteral("君の名は。"), QStringLiteral("2016"), QStringLiteral("3:16")},
        {QStringLiteral("前前前世"), QStringLiteral("RADWIMPS"), QStringLiteral("人間開花"), QStringLiteral("2016"), QStringLiteral("4:35")},
        {QStringLiteral("恋をしたのは"), QStringLiteral("aiko"), QStringLiteral("恋をしたのは"), QStringLiteral("2016"), QStringLiteral("6:02")},
        {QStringLiteral("夏バテ"), QStringLiteral("aiko"), QStringLiteral("恋をしたのは"), QStringLiteral("2016"), QStringLiteral("4:41")},
        {QStringLiteral("もっと"), QStringLiteral("aiko"), QStringLiteral("もっと"), QStringLiteral("2016"), QStringLiteral("4:50")},
        {QStringLiteral("問題集"), QStringLiteral("aiko"), QStringLiteral("もっと"), QStringLiteral("2016"), QStringLiteral("4:18")},
        {QStringLiteral("半袖"), QStringLiteral("aiko"), QStringLiteral("もっと"), QStringLiteral("2016"), QStringLiteral("5:50")},
        {QStringLiteral("ひねくれ"), QStringLiteral("鎖那"), QStringLiteral("Hush a by little girl"), QStringLiteral("2017"), QStringLiteral("3:54")},
        {QStringLiteral("シュテルン"), QStringLiteral("鎖那"), QStringLiteral("Hush a by little girl"), QStringLiteral("2017"), QStringLiteral("3:16")},
        {QStringLiteral("愛は勝手"), QStringLiteral("aiko"), QStringLiteral("湿った夏の始まり"), QStringLiteral("2018"), QStringLiteral("5:31")},
        {QStringLiteral("ドライブモード"), QStringLiteral("aiko"), QStringLiteral("湿った夏の始まり"), QStringLiteral("2018"), QStringLiteral("3:37")},
        {QStringLiteral("うん。"), QStringLiteral("aiko"), QStringLiteral("湿った夏の始まり"), QStringLiteral("2018"), QStringLiteral("5:48")},
        {QStringLiteral("キラキラ"), QStringLiteral("aiko"), QStringLiteral("aikoの詩。"), QStringLiteral("2019"), QStringLiteral("5:08")},
        {QStringLiteral("恋のスーパーボール"), QStringLiteral("aiko"), QStringLiteral("aikoの詩。"), QStringLiteral("2019"), QStringLiteral("4:31")},
        {QStringLiteral("磁石"), QStringLiteral("aiko"), QStringLiteral("どうしたって伝えられないから"), QStringLiteral("2021"), QStringLiteral("4:24")},
        {QStringLiteral("食べた愛"), QStringLiteral("aiko"), QStringLiteral("食べた愛/あたしたち"), QStringLiteral("2021"), QStringLiteral("5:17")},
        {QStringLiteral("列車"), QStringLiteral("aiko"), QStringLiteral("食べた愛/あたしたち"), QStringLiteral("2021"), QStringLiteral("4:18")},
        {QStringLiteral("花の塔"), QStringLiteral("さユり"), QStringLiteral("花の塔"), QStringLiteral("2022"), QStringLiteral("4:35")},
        {QStringLiteral("夏恋のライフ"), QStringLiteral("aiko"), QStringLiteral("夏恋のライフ"), QStringLiteral("2022"), QStringLiteral("5:03")},
        {QStringLiteral("あかときリロード"), QStringLiteral("aiko"), QStringLiteral("あかときリロード"), QStringLiteral("2023"), QStringLiteral("4:04")},
        {QStringLiteral("荒れた唇は恋を失くす"), QStringLiteral("aiko"), QStringLiteral("今の二人をお互いが見てる"), QStringLiteral("2023"), QStringLiteral("4:07")},
        {QStringLiteral("ワンツースリー"), QStringLiteral("aiko"), QStringLiteral("今の二人をお互いが見てる"), QStringLiteral("2023"), QStringLiteral("4:47")},
    };
    const int rowCount = songs.size() * 2;
    tableWidget->setRowCount(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        const auto &songInfo = songs[i % songs.size()];
        for (int j = 0; j < 5; ++j) {
            tableWidget->setItem(i, j, new QTableWidgetItem(songInfo[j]));
        }
    }
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->resizeColumnsToContents();
    tableWidget->setFixedSize(625, 440);
    page->addExampleCard(navTx("A simple TableView"), tableWidget, tableViewSource);

    // Tree view
    {
        auto *treeWidget = new TreeWidget;
        treeWidget->setHeaderHidden(true);

        auto *item1 = new QTreeWidgetItem({tx("TreeFrame", "JoJo 1 - Phantom Blood")});
        item1->addChildren({
            new QTreeWidgetItem({tx("TreeFrame", "Jonathan Joestar")}),
            new QTreeWidgetItem({tx("TreeFrame", "Dio Brando")}),
            new QTreeWidgetItem({tx("TreeFrame", "Will A. Zeppeli")}),
        });
        treeWidget->addTopLevelItem(item1);

        auto *item2 = new QTreeWidgetItem({tx("TreeFrame", "JoJo 3 - Stardust Crusaders")});
        auto *item21 = new QTreeWidgetItem({tx("TreeFrame", "Jotaro Kujo")});
        item21->addChildren({
            new QTreeWidgetItem({QStringLiteral("空条承太郎")}),
            new QTreeWidgetItem({QStringLiteral("空条蕉太狼")}),
            new QTreeWidgetItem({QStringLiteral("阿强")}),
            new QTreeWidgetItem({QStringLiteral("卖鱼强")}),
            new QTreeWidgetItem({QStringLiteral("那个无敌的男人")}),
        });
        item2->addChild(item21);
        treeWidget->addTopLevelItem(item2);

        treeWidget->expandAll();
        treeWidget->setBorderVisible(true);
        treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
        treeWidget->setMinimumHeight(380);
        treeWidget->setMaximumHeight(380);
        page->addExampleCard(navTx("A simple TreeView"), treeWidget, treeViewSource);
    }

    // Tree view with multi-selection
    {
        auto *treeWidget = new TreeWidget;
        treeWidget->setHeaderHidden(true);

        auto *item1 = new QTreeWidgetItem({tx("TreeFrame", "JoJo 1 - Phantom Blood")});
        item1->addChildren({
            new QTreeWidgetItem({tx("TreeFrame", "Jonathan Joestar")}),
            new QTreeWidgetItem({tx("TreeFrame", "Dio Brando")}),
            new QTreeWidgetItem({tx("TreeFrame", "Will A. Zeppeli")}),
        });
        treeWidget->addTopLevelItem(item1);

        auto *item2 = new QTreeWidgetItem({tx("TreeFrame", "JoJo 3 - Stardust Crusaders")});
        auto *item21 = new QTreeWidgetItem({tx("TreeFrame", "Jotaro Kujo")});
        item21->addChildren({
            new QTreeWidgetItem({QStringLiteral("空条承太郎")}),
            new QTreeWidgetItem({QStringLiteral("空条蕉太狼")}),
            new QTreeWidgetItem({QStringLiteral("阿强")}),
            new QTreeWidgetItem({QStringLiteral("卖鱼强")}),
            new QTreeWidgetItem({QStringLiteral("那个无敌的男人")}),
        });
        item2->addChild(item21);
        treeWidget->addTopLevelItem(item2);

        treeWidget->expandAll();
        treeWidget->setBorderVisible(true);
        treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
        treeWidget->setMinimumHeight(380);
        treeWidget->setMaximumHeight(380);

        // Enable checkboxes for all items
        QTreeWidgetItemIterator it(treeWidget);
        while (*it) {
            (*it)->setCheckState(0, Qt::Unchecked);
            ++it;
        }

        page->addExampleCard(navTx("A TreeView with Multi-selection enabled"), treeWidget, treeViewSource);
    }

    // Flip view
    auto *flipView = new HorizontalFlipView;
    flipView->addImages(QStringList{QStringLiteral(":/gallery/images/Shoko1.jpg"),
                                    QStringLiteral(":/gallery/images/Shoko2.jpg"),
                                    QStringLiteral(":/gallery/images/Shoko3.jpg"),
                                    QStringLiteral(":/gallery/images/Shoko4.jpg")});
    page->addExampleCard(navTx("Flip view"), flipView, flipViewSource);

    return page;
}
