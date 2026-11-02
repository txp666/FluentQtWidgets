// List, table and tree views, flip views and cards.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QColor>
#include <QtGui/QEnterEvent>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <algorithm>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

namespace {

struct TableDelegatePeek : TableItemDelegate
{
    using TableItemDelegate::isRowSelected;
};

bool delegateShowsRowSelected(TableItemDelegate *delegate, int row)
{
    return (delegate->*(&TableDelegatePeek::isRowSelected))(row);
}

} // namespace

class ViewTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void itemViewsExposeRolesDelegatesAndOverlayPolicies()
    {
        FluentQt::ListWidget list;
        list.addItems({QStringLiteral("First"), QStringLiteral("Second")});
        list.setSelectRightClickedRow(true);
        list.setCheckedColor(QColor(QStringLiteral("#010203")), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(list.property("fqw").toString(), QStringLiteral("ListWidget"));
        QVERIFY(list.isSelectRightClickedRow());
        QCOMPARE(list.delegate(), list.fluentItemDelegate());
        QCOMPARE(list.property("delegate").value<FluentQt::ListItemDelegate *>(), list.delegate());
        QCOMPARE(list.property("scrollDelegate").value<FluentQt::SmoothScrollDelegate *>(), list.scrollDelegate());
        QCOMPARE(list.fluentItemDelegate()->checkedColor(FluentQt::Theme::Light), QColor(QStringLiteral("#010203")));
        QCOMPARE(list.fluentItemDelegate()->checkedColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#aabbcc")));

        FluentQt::TableWidget table;
        table.setColumnCount(2);
        table.setRowCount(1);
        table.setItem(0, 0, new QTableWidgetItem(QStringLiteral("Component")));
        table.setItem(0, 1, new QTableWidgetItem(QStringLiteral("Ready")));
        table.setBorderVisible(true);
        table.setCurrentCell(0, 0);
        QCOMPARE(table.property("fqw").toString(), QStringLiteral("TableWidget"));
        QVERIFY(table.isBorderVisible());
        QVERIFY(table.alternatingRowColors());
        QCOMPARE(table.selectionBehavior(), QAbstractItemView::SelectRows);
        QCOMPARE(table.horizontalHeader()->sectionResizeMode(0), QHeaderView::Interactive);
        QCOMPARE(table.delegate(), table.fluentItemDelegate());
        QCOMPARE(table.scrollDelagate(), table.scrollDelegate());
        QCOMPARE(table.property("delegate").value<FluentQt::TableItemDelegate *>(), table.delegate());
        QCOMPARE(table.property("scrollDelagate").value<FluentQt::SmoothScrollDelegate *>(), table.scrollDelegate());
        table.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QCOMPARE(table.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(table.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QVERIFY(!table.scrollDelegate()->verticalScrollBar()->isForceHidden());
        QVERIFY(!table.scrollDelegate()->horizontalScrollBar()->isForceHidden());
        table.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QVERIFY(table.scrollDelegate()->verticalScrollBar()->isForceHidden());
        QVERIFY(table.scrollDelegate()->horizontalScrollBar()->isForceHidden());
        table.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QStyleOptionViewItem editorOption;
        editorOption.text = QStringLiteral("Component");
        QWidget *editor = table.delegate()->createEditor(&table, editorOption, table.model()->index(0, 0));
        auto *lineEditor = qobject_cast<FluentQt::LineEdit *>(editor);
        QVERIFY(lineEditor != nullptr);
        QCOMPARE(lineEditor->text(), QStringLiteral("Component"));
        QCOMPARE(lineEditor->property("transparent").toBool(), false);
        QVERIFY(lineEditor->isClearButtonEnabled());
        delete editor;

        FluentQt::TreeWidget tree;
        tree.setHeaderLabels({QStringLiteral("Area"), QStringLiteral("Status")});
        auto *root = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Views"), QStringLiteral("MVP")});
        root->addChild(new QTreeWidgetItem(QStringList{QStringLiteral("ListWidget"), QStringLiteral("Ready")}));
        tree.setBorderVisible(true);
        QCOMPARE(tree.property("fqw").toString(), QStringLiteral("TreeWidget"));
        QVERIFY(tree.isBorderVisible());
        QCOMPARE(tree.iconSize(), QSize(16, 16));
        QCOMPARE(tree.delegate(), tree.fluentItemDelegate());
        QCOMPARE(tree.scrollDelagate(), tree.scrollDelegate());
        QCOMPARE(tree.property("delegate").value<FluentQt::TreeItemDelegate *>(), tree.delegate());
        QCOMPARE(tree.property("scrollDelagate").value<FluentQt::SmoothScrollDelegate *>(), tree.scrollDelegate());
        tree.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QCOMPARE(tree.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QVERIFY(!tree.scrollDelegate()->verticalScrollBar()->isForceHidden());
    }

    void tableHighlightFollowsInsertedRows()
    {
        TableWidget table;
        table.setRowCount(5);
        table.setColumnCount(1);
        for (int row = 0; row < 5; ++row) {
            table.setItem(row, 0, new QTableWidgetItem(QString::number(row)));
        }
        table.selectRow(2);
        table.insertRow(0);
        QCOMPARE(table.currentRow(), 3);
        QVERIFY(!delegateShowsRowSelected(table.delegate(), 2));
        QVERIFY(delegateShowsRowSelected(table.delegate(), 3));
    }

    // As in Python, a horizontal header is 33 px plus its borders whatever the label, and its
    // sections are split by one line: no left border, none right of the last. The port's scoped
    // section rule outranked the `:horizontal` and `:last` rules, which drew a line on both sides of
    // every section, and its `min-height` let a multi-line label grow the header.
    void tableHeaderKeepsThePythonSections()
    {
        auto *manager = ThemeManager::instance();
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            manager->setTheme(theme);
            for (const bool bordered : {false, true}) {
                TableWidget table;
                table.setBorderVisible(bordered);
                table.setColumnCount(2);
                table.setRowCount(2);
                table.setHorizontalHeaderLabels({QStringLiteral("A\nB\nC"), QString()});
                table.show();
                QVERIFY(QTest::qWaitForWindowExposed(&table));
                QCOMPARE(table.horizontalHeader()->height(), bordered ? 34 : 35);

                table.setHorizontalHeaderLabels({QString(), QString()});
                QHeaderView *header = table.horizontalHeader();
                const QImage image = header->grab().toImage();
                const qreal ratio = image.devicePixelRatio();
                const int y = image.height() / 2;
                QList<int> lines;
                bool previous = false;
                for (int x = 0; x < image.width(); ++x) {
                    const bool drawn = image.pixelColor(x, y).alpha() > 0;
                    if (drawn && !previous) {
                        lines.append(qRound(x / ratio));
                    }
                    previous = drawn;
                }
                QCOMPARE(lines, QList<int>{header->sectionSize(0) - 1});
            }
        }
    }

    // Python's delegates paint items in getFont(13), 13 px, unless the item has its own font. The
    // port used the view's font, which is 12 px on Windows (9 pt) and a point size elsewhere.
    void itemDelegatesUseThePythonFont()
    {
        ListWidget list;
        list.addItem(QStringLiteral("plain"));
        TableWidget table;
        table.setRowCount(1);
        table.setColumnCount(1);
        table.setItem(0, 0, new QTableWidgetItem(QStringLiteral("plain")));
        TreeWidget tree;
        new QTreeWidgetItem(&tree, QStringList{QStringLiteral("plain")});

        const QList<std::pair<QAbstractItemView *, QModelIndex>> views{
            {&list, list.model()->index(0, 0)},
            {&table, table.model()->index(0, 0)},
            {&tree, tree.model()->index(0, 0)},
        };
        for (const auto &[view, index] : views) {
            auto *delegate = qobject_cast<TableItemDelegate *>(view->itemDelegate());
            QVERIFY(delegate);
            QStyleOptionViewItem option;
            delegate->initStyleOption(&option, index);
            QCOMPARE(option.font.pixelSize(), 13);

            QFont own;
            own.setPixelSize(20);
            view->model()->setData(index, own, Qt::FontRole);
            QStyleOptionViewItem ownOption;
            delegate->initStyleOption(&ownOption, index);
            QCOMPARE(ownOption.font.pixelSize(), 20);
        }
    }

    // A Python table selects rows with Qt's default extended selection; the port limited it to one.
    void tablesKeepExtendedSelection()
    {
        TableWidget table;
        TableView view;
        QCOMPARE(table.selectionMode(), QAbstractItemView::ExtendedSelection);
        QCOMPARE(view.selectionMode(), QAbstractItemView::ExtendedSelection);
    }

    // The item's QSS padding places the text as in Python: as far into the first column as into the
    // others. The port pushed the first column's text 10 px further right.
    void firstColumnTextKeepsTheItemPadding()
    {
        ThemeManager::instance()->setTheme(Theme::Light);
        TableWidget table;
        table.verticalHeader()->hide();
        table.setRowCount(2);
        table.setColumnCount(3);
        for (int row = 0; row < 2; ++row) {
            for (int column = 0; column < 3; ++column) {
                table.setItem(row, column, new QTableWidgetItem(QStringLiteral("MMMM")));
            }
        }
        table.resize(400, 200);
        table.show();
        QVERIFY(QTest::qWaitForWindowExposed(&table));

        const QImage image = table.viewport()->grab().toImage();
        const qreal ratio = image.devicePixelRatio();
        auto textOffset = [&](int column) {
            const QRect cell = table.visualItemRect(table.item(1, column));
            for (int x = qRound(cell.left() * ratio); x < qRound(cell.right() * ratio); ++x) {
                for (int y = qRound((cell.top() + 4) * ratio); y < qRound((cell.bottom() - 4) * ratio); ++y) {
                    const QColor color = image.pixelColor(x, y);
                    if (color.alpha() > 200 && color.lightness() < 120) {
                        return qRound(x / ratio) - cell.left();
                    }
                }
            }
            return -1;
        };
        QVERIFY(textOffset(1) > 0);
        QCOMPARE(textOffset(0), textOffset(1));
    }

    // A Python list's rows start at its top edge; the port kept 2 px viewport margins above and
    // below them.
    void listRowsReachTheViewEdges()
    {
        ListWidget list;
        ListView view;
        for (QAbstractItemView *widget :
             {static_cast<QAbstractItemView *>(&list), static_cast<QAbstractItemView *>(&view)}) {
            widget->resize(300, 200);
            widget->show();
            QVERIFY(QTest::qWaitForWindowExposed(widget));
            QCOMPARE(widget->viewport()->geometry().top(), 0);
            QCOMPARE(widget->viewport()->height(), widget->height());
        }
    }

    // As in Python, a table shows the selected row's indicator only while it is not scrolled
    // sideways; scrolled, the first column is no longer at the edge where it is drawn.
    void tableIndicatorHidesWhenScrolledSideways()
    {
        ThemeManager::instance()->setTheme(Theme::Light);
        TableWidget table;
        table.setCheckedColor(Qt::red, Qt::red);
        table.setRowCount(2);
        table.setColumnCount(4);
        for (int column = 0; column < 4; ++column) {
            table.setColumnWidth(column, 200);
        }
        table.resize(300, 200);
        table.show();
        QVERIFY(QTest::qWaitForWindowExposed(&table));
        table.selectRow(0);
        QVERIFY(table.horizontalScrollBar()->maximum() > 0);

        auto indicatorShown = [&table] {
            const QModelIndex index = table.model()->index(0, 0);
            QImage image(QSize(200, 38), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);
            QStyleOptionViewItem option;
            option.rect = image.rect();
            option.state = QStyle::State_Enabled | QStyle::State_Selected;
            option.widget = &table;
            QPainter painter(&image);
            table.delegate()->paint(&painter, option, index);
            painter.end();
            const QColor color = image.pixelColor(5, image.height() / 2);
            return color.red() > 200 && color.green() < 80;
        };
        QVERIFY(indicatorShown());
        table.horizontalScrollBar()->setValue(50);
        QVERIFY(!indicatorShown());
    }

    // Python draws a table's border with square corners until setBorderRadius() rounds them; the
    // port rounded them by 6 px.
    void tableBorderHasSquareCorners()
    {
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            ThemeManager::instance()->setTheme(theme);
            TableWidget table;
            table.setBorderVisible(true);
            table.setRowCount(2);
            table.setColumnCount(2);
            table.resize(300, 200);
            table.show();
            QVERIFY(QTest::qWaitForWindowExposed(&table));
            const QImage image = table.grab().toImage();
            QVERIFY(image.pixelColor(0, 0).alpha() > 0);

            table.setBorderRadius(8);
            QCOMPARE(table.grab().toImage().pixelColor(0, 0).alpha(), 0);
        }
    }

    // A selected tree row is one rounded fill across its columns, as in Python. The port filled the
    // whole row width for every column, so a two-column row was covered twice.
    void treeRowFillsEachColumnOnce()
    {
        ThemeManager::instance()->setTheme(Theme::Light);
        TreeWidget tree;
        tree.setColumnCount(2);
        tree.setHeaderHidden(true);
        auto *item = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("a"), QStringLiteral("b")});
        new QTreeWidgetItem(&tree, QStringList{QStringLiteral("c"), QStringLiteral("d")});
        tree.resize(400, 200);
        tree.setColumnWidth(0, 200);
        tree.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tree));
        tree.setCurrentItem(item);
        QTest::mouseMove(tree.viewport(), QPoint(300, 150));

        const QRect row = tree.visualItemRect(item);
        const QImage image = tree.viewport()->grab().toImage();
        const qreal ratio = image.devicePixelRatio();
        for (const int x : {190, 210, 370}) {
            const QColor color = image.pixelColor(qRound(x * ratio), qRound(row.center().y() * ratio));
            QVERIFY2(qAbs(color.alpha() - 9) <= 1,
                     qPrintable(QStringLiteral("x=%1 alpha=%2").arg(x).arg(color.alpha())));
        }
    }

    // Python's table and list delegates draw an item's check box 15 px into the item; the port
    // centred it on the style's indicator rect, about 4 px further right in a table.
    void tableCheckBoxSitsWherePythonDrawsIt()
    {
        ThemeManager::instance()->setTheme(Theme::Light);
        TableWidget table;
        table.setCheckedColor(Qt::red, Qt::red);
        table.setRowCount(1);
        table.setColumnCount(2);
        auto *item = new QTableWidgetItem(QStringLiteral("MMMM"));
        item->setCheckState(Qt::Checked);
        table.setItem(0, 0, item);
        ListWidget list;
        list.setCheckedColor(Qt::red, Qt::red);
        auto *listItem = new QListWidgetItem(QStringLiteral("MMMM"));
        listItem->setCheckState(Qt::Checked);
        list.addItem(listItem);

        const QList<std::pair<QAbstractItemView *, QModelIndex>> views{
            {&table, table.model()->index(0, 0)},
            {&list, list.model()->index(0, 0)},
        };
        for (const auto &[view, index] : views) {
            QImage image(QSize(200, 38), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);
            QStyleOptionViewItem option;
            option.rect = image.rect();
            option.state = QStyle::State_Enabled;
            option.widget = view;
            QPainter painter(&image);
            view->itemDelegate()->paint(&painter, option, index);
            painter.end();

            int left = -1;
            for (int x = 0; x < image.width() && left < 0; ++x) {
                const QColor color = image.pixelColor(x, image.height() / 2);
                if (color.red() > 200 && color.green() < 120) {
                    left = x;
                }
            }
            QVERIFY2(left == 14 || left == 15, qPrintable(QString::number(left)));
        }
    }

    // Python's tree delegate sizes items as Qt does; the port's inherited the table delegate's 2 px
    // above and below, which made tree rows 4 px taller than Python's.
    void treeRowsKeepThePythonHeight()
    {
        TreeWidget tree;
        auto *item = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("MMMM")});
        const QModelIndex index = tree.indexFromItem(item);
        // Qt's sizing of the item as the Fluent delegate styles it.
        struct PlainDelegate : QStyledItemDelegate
        {
            TreeItemDelegate *fluent = nullptr;
            void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
            {
                fluent->initStyleOption(option, index);
            }
        } plain;
        plain.fluent = tree.delegate();
        QStyleOptionViewItem option;
        option.widget = &tree;
        QCOMPARE(tree.delegate()->sizeHint(option, index), plain.sizeHint(option, index));
    }

    // Python's SimpleCardWidget and ElevatedCardWidget come without a layout, so a subclass gives
    // them its own. The port's had a 16 px VBox layout, which made `new QVBoxLayout(card)` fail.
    void simpleCardsComeWithoutALayout()
    {
        SimpleCardWidget simple;
        ElevatedCardWidget elevated;
        QCOMPARE(simple.layout(), nullptr);
        QCOMPARE(elevated.layout(), nullptr);

        HeaderCardWidget header;
        QVERIFY(qobject_cast<QVBoxLayout *>(header.layout()));
        QCOMPARE(header.layout()->contentsMargins(), QMargins());
        QCOMPARE(header.layout()->count(), 3);
    }

    // As Python's BackgroundAnimationWidget, a card fades its background to the state's colour in
    // 120 ms, shows the pressed colour on any press, and simple and elevated cards paint the colour
    // of their state. The port switched at once, pressed only clickable cards, and painted simple and
    // elevated cards in their normal colour whatever the state.
    void cardsFadeToTheirStateColours()
    {
        auto *manager = ThemeManager::instance();
        auto centerAlpha = [](QWidget &card) {
            const QImage image = FqwTest::render(card);
            return image.pixelColor(image.rect().center()).alpha();
        };
        auto enter = [](QWidget &card) {
            QEnterEvent event(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
            QApplication::sendEvent(&card, &event);
        };

        manager->setTheme(Theme::Light);
        CardWidget card;
        card.resize(100, 60);
        QCOMPARE(centerAlpha(card), 170);
        enter(card);
        QVERIFY(centerAlpha(card) > 150);
        QTRY_COMPARE(centerAlpha(card), 64);

        ElevatedCardWidget elevated;
        elevated.resize(100, 60);
        QCOMPARE(centerAlpha(elevated), 170);
        enter(elevated);
        // Without the hover shadow, which render() would draw in place of the hidden card.
        elevated.setGraphicsEffect(nullptr);
        QTRY_COMPARE(centerAlpha(elevated), 255);

        SimpleCardWidget simple;
        simple.resize(100, 60);
        enter(simple);
        QVERIFY(FqwTest::waitForAnimations(&simple));
        QCOMPARE(centerAlpha(simple), 170);

        manager->setTheme(Theme::Dark);
        CardWidget dark;
        dark.resize(100, 60);
        dark.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dark));
        QVERIFY(!dark.isClickEnabled());
        QTest::mousePress(&dark, Qt::LeftButton, Qt::NoModifier, QPoint(50, 30));
        QTRY_COMPARE(centerAlpha(dark), 8);
        QTest::mouseRelease(&dark, Qt::LeftButton, Qt::NoModifier, QPoint(50, 30));
    }

    // Python's ElevatedCardWidget has no shadow at rest: under the mouse a 38 px blurred shadow
    // fades in to alpha 20 in 150 ms, and once it has faded out again the effect is removed. The
    // port's shadow was always there.
    void elevatedCardShadowShowsOnlyUnderTheMouse()
    {
        ElevatedCardWidget card;
        card.resize(100, 60);
        QCOMPARE(card.graphicsEffect(), nullptr);

        QEnterEvent enter(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
        QApplication::sendEvent(&card, &enter);
        auto *shadow = qobject_cast<QGraphicsDropShadowEffect *>(card.graphicsEffect());
        QVERIFY(shadow);
        QCOMPARE(shadow->blurRadius(), 38.0);
        QCOMPARE(shadow->offset(), QPointF(0, 5));
        QVERIFY(shadow->color().alpha() < 20);
        QTRY_COMPARE(shadow->color(), QColor(0, 0, 0, 20));

        QEvent leave(QEvent::Leave);
        QApplication::sendEvent(&card, &leave);
        QTRY_COMPARE(card.graphicsEffect(), nullptr);
    }

    void treeHoverDoesNotBleedAcrossParents()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::TreeWidget tree;
        tree.setColumnCount(1);

        auto *firstParent = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("First parent")});
        auto *firstChild = new QTreeWidgetItem(firstParent, QStringList{QStringLiteral("Same row child A")});
        auto *secondParent = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Second parent")});
        auto *secondChild = new QTreeWidgetItem(secondParent, QStringList{QStringLiteral("Same row child B")});

        const QModelIndex firstChildIndex = tree.indexFromItem(firstChild);
        const QModelIndex secondChildIndex = tree.indexFromItem(secondChild);
        tree.delegate()->setHoverIndex(secondChildIndex);

        auto renderIndexSample = [&tree](const QModelIndex &index) {
            QImage image(QSize(180, 30), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);

            QStyleOptionViewItem option;
            option.rect = image.rect();
            option.state = QStyle::State_Enabled;
            option.widget = &tree;

            QPainter painter(&image);
            tree.delegate()->paint(&painter, option, index);
            painter.end();
            return image.pixelColor(165, image.height() / 2);
        };

        const QColor firstColor = renderIndexSample(firstChildIndex);
        const QColor secondColor = renderIndexSample(secondChildIndex);

        QVERIFY2(secondColor.red() + 4 < firstColor.red(),
                 qPrintable(QStringLiteral("first=%1 second=%2").arg(firstColor.name(QColor::HexArgb),
                                                                     secondColor.name(QColor::HexArgb))));
    }

    void treeCheckBoxSitsLeftOfTheText()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::TreeWidget tree;
        tree.setColumnCount(1);
        auto *item = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Tree text")});
        item->setCheckState(0, Qt::Unchecked);
        const QModelIndex index = tree.indexFromItem(item);

        QStyleOptionViewItem option;
        option.rect = QRect(40, 0, 140, 32);
        option.state = QStyle::State_Enabled;
        option.widget = &tree;
        tree.delegate()->initStyleOption(&option, index);

        const QRect indicatorRect =
            tree.style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, &tree);
        const QRect textRect = tree.style()->subElementRect(QStyle::SE_ItemViewItemText, &option, &tree);
        QVERIFY(indicatorRect.isValid());
        QVERIFY(textRect.isValid());
        QVERIFY2(indicatorRect.right() < textRect.left(),
                 qPrintable(QStringLiteral("indicator=%1 text=%2").arg(QString::number(indicatorRect.right()),
                                                                        QString::number(textRect.left()))));

        QImage image(QSize(220, 36), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);

        QPainter painter(&image);
        tree.delegate()->paint(&painter, option, index);
        painter.end();

        QVERIFY(image.rect().contains(indicatorRect.center()));
        QVERIFY(image.pixelColor(indicatorRect.center()) != QColor(Qt::white));
    }

    void flipViewTracksImagesAndCurrentIndex()
    {
        QImage first(32, 18, QImage::Format_ARGB32_Premultiplied);
        first.fill(Qt::red);
        QImage second(32, 18, QImage::Format_ARGB32_Premultiplied);
        second.fill(Qt::blue);

        FluentQt::HorizontalFlipView flipView;
        QCOMPARE(flipView.property("fqw").toString(), QStringLiteral("FlipView"));
        QCOMPARE(flipView.itemSize(), QSize(480, 270));
        QCOMPARE(flipView.spacing(), 0);
        QCOMPARE(flipView.aspectRatioMode(), Qt::IgnoreAspectRatio);
        QCOMPARE(flipView.currentIndex(), -1);

        flipView.addImages(QList<QImage>{first, second});
        QCOMPARE(flipView.count(), 2);
        QCOMPARE(flipView.currentIndex(), 0);
        QVERIFY(!flipView.image(0).isNull());
        QVERIFY(!flipView.addImage(QStringLiteral("__missing_flip_image__.png")));

        QSignalSpy indexSpy(&flipView, &FluentQt::FlipView::currentIndexChanged);
        flipView.scrollNext();
        QCOMPARE(flipView.currentIndex(), 1);
        QCOMPARE(indexSpy.count(), 1);
        flipView.scrollPrevious();
        QCOMPARE(flipView.currentIndex(), 0);
        QCOMPARE(indexSpy.count(), 2);

        flipView.setItemSize(QSize(120, 80));
        flipView.setBorderRadius(6);
        flipView.setSpacing(15);
        flipView.setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        QCOMPARE(flipView.itemSize(), QSize(120, 80));
        QCOMPARE(flipView.borderRadius(), 6);
        QCOMPARE(flipView.spacing(), 15);
        QCOMPARE(flipView.property("spacing").toInt(), 15);
        QCOMPARE(flipView.aspectRatioMode(), Qt::KeepAspectRatioByExpanding);
        QCOMPARE(flipView.previousButtonOpacity(), 0.0);
        QCOMPARE(flipView.nextButtonOpacity(), 0.0);

        flipView.resize(120, 80);
        QImage rendered(flipView.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        QPainter painter(&rendered);
        flipView.render(&painter);
        painter.end();
        QVERIFY(qAlpha(rendered.pixel(10, 10)) > 0);

        flipView.show();
        QVERIFY(QTest::qWaitForWindowExposed(&flipView));
        QEnterEvent enterEvent(QPointF(60, 40), QPointF(60, 40), QPointF(60, 40));
        QApplication::sendEvent(&flipView, &enterEvent);
        QTRY_VERIFY(flipView.nextButtonOpacity() > 0.99);
        QCOMPARE(flipView.previousButtonOpacity(), 0.0);

        flipView.scrollNext();
        QTRY_VERIFY(flipView.previousButtonOpacity() > 0.99);
        QTRY_VERIFY(flipView.nextButtonOpacity() < 0.01);

        QEvent leaveEvent(QEvent::Leave);
        QApplication::sendEvent(&flipView, &leaveEvent);
        QTRY_VERIFY(flipView.previousButtonOpacity() < 0.01);
    }

    void cardWidgetsExposePythonStructureAndSignals()
    {
        FluentQt::CardWidget card;
        QCOMPARE(card.property("fqw").toString(), QStringLiteral("CardWidget"));
        QVERIFY(!card.isClickEnabled());
        QSignalSpy clickedSpy(&card, &FluentQt::CardWidget::clicked);
        QTest::mouseRelease(&card, Qt::LeftButton);
        QCOMPARE(clickedSpy.count(), 0);
        card.setClickEnabled(true);
        QVERIFY(card.isClickEnabled());
        QTest::mouseClick(&card, Qt::LeftButton, Qt::NoModifier, card.rect().center());
        QCOMPARE(clickedSpy.count(), 1);
        QVERIFY(!card.isSelected());
        QSignalSpy selectedSpy(&card, &FluentQt::CardWidget::selectedChanged);
        card.resize(64, 64);
        card.setSelected(true);
        QVERIFY(card.isSelected());
        QCOMPARE(card.property("selected").toBool(), true);
        QCOMPARE(selectedSpy.count(), 1);
        QImage selectedImage(card.size(), QImage::Format_ARGB32_Premultiplied);
        selectedImage.fill(Qt::transparent);
        card.render(&selectedImage);
        const QColor selectedCenter = selectedImage.pixelColor(selectedImage.rect().center());
        QCOMPARE(selectedCenter.red(), FluentQt::themeColor().red());
        QCOMPARE(selectedCenter.green(), FluentQt::themeColor().green());
        QCOMPARE(selectedCenter.blue(), FluentQt::themeColor().blue());
        card.setBorderRadius(9);
        QCOMPARE(card.borderRadius(), 9);

        FluentQt::SimpleCardWidget simpleCard;
        QCOMPARE(simpleCard.property("fqw").toString(), QStringLiteral("SimpleCardWidget"));

        FluentQt::HeaderCardWidget headerCard;
        QCOMPARE(headerCard.property("fqw").toString(), QStringLiteral("HeaderCardWidget"));
        QVERIFY(headerCard.headerView() != nullptr);
        QVERIFY(headerCard.view() != nullptr);
        QVERIFY(headerCard.separator() != nullptr);
        QVERIFY(headerCard.headerLabel() != nullptr);
        QCOMPARE(headerCard.headerLabel(), headerCard.titleLabel());
        QVERIFY(headerCard.headerLayout() != nullptr);
        QVERIFY(headerCard.viewLayout() != nullptr);
        QCOMPARE(headerCard.headerView()->objectName(), QStringLiteral("headerView"));
        QCOMPARE(headerCard.view()->objectName(), QStringLiteral("view"));
        QCOMPARE(headerCard.headerLabel()->objectName(), QStringLiteral("headerLabel"));
        headerCard.setTitle(QStringLiteral("General"));
        QCOMPARE(headerCard.title(), QStringLiteral("General"));
        QVERIFY(headerCard.metaObject()->indexOfProperty("title") >= 0);

        auto icon = FluentQt::icon(FluentQt::FluentIcon::Setting);
        FluentQt::CardGroupWidget group(icon, QStringLiteral("Title"), QStringLiteral("Content"));
        QCOMPARE(group.title(), QStringLiteral("Title"));
        QCOMPARE(group.content(), QStringLiteral("Content"));
        QVERIFY(group.iconWidget() != nullptr);
        QVERIFY(group.titleLabel() != nullptr);
        QVERIFY(group.contentLabel() != nullptr);
        QVERIFY(group.separator() != nullptr);
        QVERIFY(group.hBoxLayout() != nullptr);
        QVERIFY(group.textLayout() != nullptr);
        QVERIFY(!group.isSeparatorVisible());
        group.setSeparatorVisible(true);
        QVERIFY(group.isSeparatorVisible());

        FluentQt::GroupHeaderCardWidget groupedCard;
        groupedCard.setTitle(QStringLiteral("Groups"));
        QCOMPARE(groupedCard.title(), QStringLiteral("Groups"));
        auto *extra = new QLabel(QStringLiteral("Extra"));
        auto *addedGroup = groupedCard.addGroup(icon, QStringLiteral("A"), QStringLiteral("B"), extra);
        QVERIFY(addedGroup != nullptr);
        QCOMPARE(groupedCard.groupCount(), 1);
        QCOMPARE(groupedCard.groupWidgets().size(), 1);
        QCOMPARE(groupedCard.groupWidgets().first(), addedGroup);
    }

    void headerCardTitleFollowsThemeChanges()
    {
        // The title is a plain QLabel under the card's style sheet, which is the same in both themes, so
        // a theme change never repolished it: it kept the colour of the theme it was created in.
        auto *manager = FluentQt::ThemeManager::instance();
        manager->setTheme(FluentQt::Theme::Light);
        FluentQt::HeaderCardWidget card(QStringLiteral("Title"));
        card.resize(320, 160);
        card.show();
        QVERIFY(QTest::qWaitForWindowExposed(&card));
        FluentQt::GroupHeaderCardWidget hiddenCard;
        hiddenCard.setTitle(QStringLiteral("Hidden"));

        const auto titleColor = [](FluentQt::HeaderCardWidget &c) {
            return c.titleLabel()->palette().color(QPalette::WindowText);
        };
        // The lightest and darkest pixels of the drawn title, over the card's own background.
        const auto titleLightness = [](FluentQt::HeaderCardWidget &c) {
            QLabel *title = c.titleLabel();
            const QImage image = c.grab(QRect(title->mapTo(&c, QPoint(0, 0)), title->size())).toImage();
            int lightest = 0;
            int darkest = 255;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const int lightness = image.pixelColor(x, y).lightness();
                    lightest = qMax(lightest, lightness);
                    darkest = qMin(darkest, lightness);
                }
            }
            return std::make_pair(darkest, lightest);
        };
        QCOMPARE(titleColor(card), QColor(Qt::black));
        QVERIFY(titleLightness(card).first < 80);
        // As in Python: setFont(headerLabel, 15, QFont.DemiBold), black in light mode and white in dark.
        QCOMPARE(card.titleLabel()->font().pixelSize(), 15);
        QCOMPARE(card.titleLabel()->font().weight(), QFont::DemiBold);

        manager->setTheme(FluentQt::Theme::Dark);
        QCOMPARE(titleColor(card), QColor(Qt::white));
        QVERIFY(titleLightness(card).second > 200);
        hiddenCard.resize(320, 160);
        hiddenCard.show();
        QVERIFY(QTest::qWaitForWindowExposed(&hiddenCard));
        QCOMPARE(titleColor(hiddenCard), QColor(Qt::white));

        manager->setTheme(FluentQt::Theme::Light);
        QCOMPARE(titleColor(card), QColor(Qt::black));
        QVERIFY(titleLightness(card).first < 80);
        QCOMPARE(titleColor(hiddenCard), QColor(Qt::black));
    }

    // Python's flip view buttons draw their arrow brighter under the mouse, not only when pressed.
    void flipViewArrowBrightensUnderTheMouse()
    {
        ThemeManager::instance()->setTheme(Theme::Light);
        FlipView view;
        QImage white(480, 270, QImage::Format_ARGB32);
        white.fill(Qt::white);
        view.addImages(QList<QImage>{white, white});
        view.resize(480, 270);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));

        const QRect next(view.width() - 18, view.height() / 2 - 19, 16, 38);
        auto darkestArrowPixel = [&](const QPoint &mouse) -> int {
            QEnterEvent enter(mouse, view.mapToGlobal(mouse), view.mapToGlobal(mouse));
            QApplication::sendEvent(&view, &enter);
            QMouseEvent move(QEvent::MouseMove, mouse, view.mapToGlobal(mouse), Qt::NoButton, Qt::NoButton,
                             Qt::NoModifier);
            QApplication::sendEvent(&view, &move);
            if (!QTest::qWaitFor([&] { return view.nextButtonOpacity() == 1.0; })) {
                return -1;
            }
            const QImage image = FqwTest::render(view, Qt::white);
            int darkest = 255;
            for (int y = next.top(); y <= next.bottom(); ++y) {
                for (int x = next.left(); x <= next.right(); ++x) {
                    darkest = qMin(darkest, image.pixelColor(x, y).lightness());
                }
            }
            return darkest;
        };
        const int away = darkestArrowPixel(QPoint(100, 135));
        const int over = darkestArrowPixel(next.center());
        QVERIFY(over >= 0 && away >= 0);
        QVERIFY2(over + 20 < away, qPrintable(QStringLiteral("over=%1 away=%2").arg(over).arg(away)));
    }

    // Python's flip view buttons fade in and out linearly over 150 ms; the port eased them.
    void flipViewButtonsFadeLinearly()
    {
        FlipView view;
        const auto animations = view.findChildren<QPropertyAnimation *>();
        int fades = 0;
        for (QPropertyAnimation *animation : animations) {
            if (animation->propertyName().endsWith("ButtonOpacity")) {
                QCOMPARE(animation->duration(), 150);
                QCOMPARE(animation->easingCurve().type(), QEasingCurve::Linear);
                ++fades;
            }
        }
        QCOMPARE(fades, 2);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ViewTest)
#include "tst_view.moc"
