// List, table and tree views, scroll bars and areas, flip views, cards and pips pagers.

#include <algorithm>
#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Layout/VBoxLayout.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/ComboBox.h>
#include <FluentQtWidgets/Widgets/CommandBar.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/StateToolTip.h>
#include <FluentQtWidgets/Widgets/SwitchButton.h>
#include <FluentQtWidgets/Widgets/TabView.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Window/FluentWindow.h>
#include <functional>
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QScopedPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QEnterEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionSlider>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

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

    void itemViewsExposeFluentState()
    {
        FluentQt::ListWidget list;
        list.addItems({QStringLiteral("First"), QStringLiteral("Second")});
        list.setCurrentRow(1);
        list.setSelectRightClickedRow(true);
        list.setCheckedColor(QColor(QStringLiteral("#010203")), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(list.property("fqw").toString(), QStringLiteral("ListWidget"));
        QCOMPARE(list.currentRow(), 1);
        QVERIFY(list.isSelectRightClickedRow());
        QCOMPARE(list.delegate(), list.fluentItemDelegate());
        QCOMPARE(list.property("delegate").value<FluentQt::ListItemDelegate *>(), list.delegate());
        QCOMPARE(list.property("scrollDelegate").value<FluentQt::SmoothScrollDelegate *>(), list.scrollDelegate());
        QCOMPARE(list.fluentItemDelegate()->checkedColor(FluentQt::Theme::Light), QColor(QStringLiteral("#010203")));
        QCOMPARE(list.fluentItemDelegate()->checkedColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#aabbcc")));

        FluentQt::ListView listView;
        QStringListModel listModel({QStringLiteral("First"), QStringLiteral("Second")});
        listView.setModel(&listModel);
        QCOMPARE(listView.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(listView.scrollDelegate()->verticalScrollBar()->parentWidget(), &listView);

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
        QCOMPARE(table.item(0, 1)->text(), QStringLiteral("Ready"));
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
        QCOMPARE(tree.topLevelItemCount(), 1);
        QCOMPARE(tree.topLevelItem(0)->child(0)->text(0), QStringLiteral("ListWidget"));
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

    void treeHoverDoesNotBleedAcrossParents()
    {
        const FluentQt::Theme previousTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::TreeWidget tree;
        tree.setColumnCount(1);

        auto *firstParent = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("First parent")});
        auto *firstChild = new QTreeWidgetItem(firstParent, QStringList{QStringLiteral("Same row child A")});
        auto *secondParent = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Second parent")});
        auto *secondChild = new QTreeWidgetItem(secondParent, QStringList{QStringLiteral("Same row child B")});

        const QModelIndex firstChildIndex = tree.indexFromItem(firstChild);
        const QModelIndex secondChildIndex = tree.indexFromItem(secondChild);
        QCOMPARE(firstChildIndex.row(), secondChildIndex.row());
        QVERIFY(firstChildIndex.parent() != secondChildIndex.parent());

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

        FluentQt::ThemeManager::instance()->setTheme(previousTheme);
    }

    void treeCheckBoxPaintsInsideStyleIndicatorRect()
    {
        const FluentQt::Theme previousTheme = FluentQt::ThemeManager::instance()->theme();
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

        FluentQt::ThemeManager::instance()->setTheme(previousTheme);
    }

    void listViewsUseFluentOverlayScrollBars()
    {
        auto populate = [](auto *view) {
            for (int i = 0; i < 80; ++i) {
                view->addItem(QStringLiteral("Item %1").arg(i));
            }
            view->resize(260, 180);
            view->show();
            QVERIFY(QTest::qWaitForWindowExposed(view));
            QCoreApplication::processEvents();
        };

        FluentQt::ListWidget listWidget;
        populate(&listWidget);
        QCOMPARE(listWidget.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(listWidget.scrollDelegate()->verticalScrollBar()->parentWidget(), &listWidget);
        QVERIFY(listWidget.scrollDelegate()->verticalScrollBar()->maximum() > 0);
        QVERIFY(!listWidget.verticalScrollBar()->isVisible());
        QVERIFY(listWidget.scrollDelegate()->verticalScrollBar()->isVisible());

        listWidget.scrollDelegate()->verticalScrollBar()->setValue(24);
        QCOMPARE(listWidget.verticalScrollBar()->value(), 24);

        QStringListModel model;
        QStringList items;
        for (int i = 0; i < 80; ++i) {
            items.append(QStringLiteral("Item %1").arg(i));
        }
        model.setStringList(items);

        FluentQt::ListView listView;
        listView.setModel(&model);
        listView.resize(260, 180);
        listView.show();
        QVERIFY(QTest::qWaitForWindowExposed(&listView));
        QCoreApplication::processEvents();

        QCOMPARE(listView.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(listView.scrollDelegate()->verticalScrollBar()->parentWidget(), &listView);
        QVERIFY(listView.scrollDelegate()->verticalScrollBar()->maximum() > 0);
        QVERIFY(!listView.verticalScrollBar()->isVisible());
        QVERIFY(listView.scrollDelegate()->verticalScrollBar()->isVisible());

        listView.scrollDelegate()->verticalScrollBar()->setValue(24);
        QCOMPARE(listView.verticalScrollBar()->value(), 24);
    }

    void overlayScrollBarsFollowNativeLayout_data()
    {
        QTest::addColumn<int>("consumer");
        QTest::newRow("scroll-area") << 0;
        QTest::newRow("plain-text-edit") << 1;
        QTest::newRow("table-view") << 2;
    }

    void overlayScrollBarsFollowNativeLayout()
    {
        QFETCH(int, consumer);
        QAbstractScrollArea *area = nullptr;
        FluentQt::SmoothScrollDelegate *delegate = nullptr;
        if (consumer == 0) {
            auto *scroll = new FluentQt::ScrollArea;
            auto *content = new QWidget;
            content->setMinimumSize(780, 780);
            scroll->setWidget(content);
            area = scroll;
            delegate = scroll->scrollDelegate();
        } else if (consumer == 1) {
            auto *editor = new FluentQt::PlainTextEdit;
            editor->setLineWrapMode(QPlainTextEdit::NoWrap);
            QStringList lines;
            for (int i = 0; i < 80; ++i) {
                lines.append(QString::number(i) + QString(200, QLatin1Char('x')));
            }
            editor->setPlainText(lines.join(QLatin1Char('\n')));
            area = editor;
            delegate = editor->scrollDelegate();
        } else {
            auto *table = new FluentQt::TableView;
            table->setModel(new QStandardItemModel(80, 1, table));
            table->setColumnWidth(0, 1200);
            area = table;
            delegate = table->scrollDelegate();
        }
        QScopedPointer<QAbstractScrollArea> owner(area);
        area->resize(370, 650);
        area->show();
        QVERIFY(QTest::qWaitForWindowExposed(area));

        for (int height : {650, 550, 700}) {
            area->resize(370, height);
            const QList<QScrollBar *> nativeBars = {area->verticalScrollBar(), area->horizontalScrollBar()};
            const QList<FluentQt::ScrollBar *> overlayBars = {delegate->verticalScrollBar(),
                                                            delegate->horizontalScrollBar()};
            for (int i = 0; i < nativeBars.size(); ++i) {
                auto *native = nativeBars.at(i);
                auto *overlay = overlayBars.at(i);
                QTRY_VERIFY(native->maximum() > native->minimum());
                QTRY_COMPARE(overlay->minimum(), native->minimum());
                QTRY_COMPARE(overlay->maximum(), native->maximum());
                QTRY_COMPARE(overlay->pageStep(), native->pageStep());
                QTRY_COMPARE(overlay->singleStep(), native->singleStep());
                native->setValue(qMin(40, native->maximum()));
                QCOMPARE(overlay->value(), native->value());
                overlay->setValue(0);
                QCOMPARE(native->value(), 0);
            }
        }
    }

    void overlayScrollBarReadsStepsAfterRangeAndViewportChanges()
    {
        QAbstractScrollArea area;
        FluentQt::SmoothScrollDelegate delegate(&area, false);
        area.resize(370, 250);
        area.show();
        QVERIFY(QTest::qWaitForWindowExposed(&area));

        auto *native = area.verticalScrollBar();
        auto *overlay = delegate.verticalScrollBar();
        native->setRange(0, 1000);
        native->setPageStep(180);
        native->setSingleStep(37);
        native->setValue(700);
        QTRY_COMPARE(overlay->pageStep(), 180);
        QCOMPARE(overlay->singleStep(), 37);
        QCOMPARE(overlay->maximum(), 1000);
        QCOMPARE(overlay->value(), 700);

        // The range can stay unchanged when only the viewport's page step changes.
        native->setPageStep(210);
        native->setSingleStep(45);
        area.resize(370, 300);
        QTRY_COMPARE(overlay->pageStep(), 210);
        QCOMPARE(overlay->singleStep(), 45);
        QCOMPARE(overlay->maximum(), 1000);
        QCOMPARE(overlay->value(), 700);
    }

    void overlayPendingSyncSurvivesPartnerAndAreaDestruction()
    {
        auto *area = new QAbstractScrollArea;
        auto *delegate = new FluentQt::SmoothScrollDelegate(area, false);
        QPointer<FluentQt::ScrollBar> overlay = delegate->verticalScrollBar();
        QPointer<QScrollBar> partner = area->verticalScrollBar();
        partner->setRange(0, 1000);
        auto *replacement = new QScrollBar(Qt::Vertical, area);
        area->setVerticalScrollBar(replacement);
        QTRY_VERIFY(partner.isNull());
        area->resize(370, 250);
        QCoreApplication::processEvents();
        QCOMPARE(area->verticalScrollBar(), replacement);
        QVERIFY(!overlay.isNull());
        delete area;
        QVERIFY(overlay.isNull());
        QCoreApplication::processEvents();

        area = new QAbstractScrollArea;
        delegate = new FluentQt::SmoothScrollDelegate(area, false);
        overlay = delegate->verticalScrollBar();
        area->verticalScrollBar()->setRange(0, 1000);
        delete area;
        QVERIFY(overlay.isNull());
        QCoreApplication::processEvents();
    }

    void scrollAreasExposeFluentPolicies()
    {
        FluentQt::ScrollBar bar(Qt::Vertical);
        QCOMPARE(bar.property("fqw").toString(), QStringLiteral("ScrollBar"));
        QVERIFY(bar.groove() != nullptr);
        QVERIFY(bar.handle() != nullptr);
        QVERIFY(bar.upButton() != nullptr);
        QVERIFY(bar.downButton() != nullptr);
        QCOMPARE(bar.groove()->property("fqw").toString(), QStringLiteral("ScrollBarGroove"));
        QCOMPARE(bar.handle()->property("fqw").toString(), QStringLiteral("ScrollBarHandle"));
        QCOMPARE(bar.upButton()->property("fqw").toString(), QStringLiteral("ArrowButton"));
        QCOMPARE(bar.groove()->orientation(), Qt::Vertical);
        QCOMPARE(bar.handle()->orientation(), Qt::Vertical);
        QCOMPARE(bar.upButton()->iconType(), FluentQt::FluentIcon::Up);
        QCOMPARE(bar.downButton()->iconType(), FluentQt::FluentIcon::ArrowDown);
        QCOMPARE(bar.groove()->opacity(), 0.0);
        QCOMPARE(bar.handle()->opacity(), 1.0);
        bar.setHandleDisplayMode(FluentQt::ScrollBarHandleDisplayMode::OnHover);
        QCOMPARE(bar.handleDisplayMode(), FluentQt::ScrollBarHandleDisplayMode::OnHover);
        QCOMPARE(bar.property("handleDisplayMode").toString(), QStringLiteral("OnHover"));
        QCOMPARE(bar.property("expanded").toBool(), false);
        bar.setHandleColor(QColor(QStringLiteral("#010203")), QColor(QStringLiteral("#aabbcc")));
        bar.setArrowColor(QColor(QStringLiteral("#111213")), QColor(QStringLiteral("#ddeeff")));
        bar.setGrooveColor(QColor(252, 252, 252, 217), QColor(44, 44, 44, 245));
        QCOMPARE(bar.handle()->lightColor(), QColor(QStringLiteral("#010203")));
        QCOMPARE(bar.handle()->darkColor(), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(bar.upButton()->lightColor(), QColor(QStringLiteral("#111213")));
        QCOMPARE(bar.downButton()->darkColor(), QColor(QStringLiteral("#ddeeff")));
        QCOMPARE(bar.groove()->lightBackgroundColor(), QColor(252, 252, 252, 217));
        QCOMPARE(bar.handleColor(FluentQt::Theme::Light), QColor(QStringLiteral("#010203")));
        QCOMPARE(bar.handleColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(bar.arrowColor(FluentQt::Theme::Light), QColor(QStringLiteral("#111213")));
        QCOMPARE(bar.arrowColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#ddeeff")));
        bar.resize(12, 120);
        bar.setRange(0, 100);
        bar.setPageStep(20);
        bar.expand();
        QCOMPARE(bar.property("expanded").toBool(), true);
        QTRY_VERIFY(bar.groove()->opacity() > 0.0);
        bar.setValue(50);
        QVERIFY(bar.handle()->geometry().height() >= 30);
        QVERIFY(bar.handle()->geometry().y() > 0);
        QTest::mouseClick(bar.downButton(), Qt::LeftButton);
        QCOMPARE(bar.value(), 70);
        QTest::mouseClick(bar.upButton(), Qt::LeftButton);
        QCOMPARE(bar.value(), 50);
        bar.collapse();
        QCOMPARE(bar.property("expanded").toBool(), false);
        QVERIFY(FluentQt::StyleSheetManager::instance()
                    ->customStyleSheet(&bar, FluentQt::Theme::Light)
                    .contains(QStringLiteral("rgba(1, 2, 3")));
        bar.setForceHidden(true);
        QVERIFY(bar.isForceHidden());

        FluentQt::ScrollArea area;
        QVERIFY(area.verticalFluentScrollBar() != nullptr);
        QVERIFY(area.horizontalFluentScrollBar() != nullptr);
        QCOMPARE(area.vScrollBar(), area.verticalFluentScrollBar());
        QCOMPARE(area.hScrollBar(), area.horizontalFluentScrollBar());
        QCOMPARE(area.scrollDelagate(), area.scrollDelegate());
        QCOMPARE(area.property("vScrollBar").value<FluentQt::ScrollBar *>(), area.vScrollBar());
        QCOMPARE(area.property("hScrollBar").value<FluentQt::ScrollBar *>(), area.hScrollBar());
        QCOMPARE(area.property("scrollDelagate").value<FluentQt::SmoothScrollDelegate *>(), area.scrollDelegate());
        QCOMPARE(area.scrollDelegate()->vScrollBar(), area.scrollDelegate()->verticalScrollBar());
        QCOMPARE(area.scrollDelegate()->hScrollBar(), area.scrollDelegate()->horizontalScrollBar());
        QCOMPARE(area.property("fqw").toString(), QStringLiteral("ScrollArea"));
        area.setViewportMargins(1, 2, 3, 4);
        QCOMPARE(area.viewportMargins(), QMargins(1, 2, 3, 4));
        area.setViewportMargins(QMargins(5, 6, 7, 8));
        QCOMPARE(area.viewportMargins(), QMargins(5, 6, 7, 8));
        area.enableTransparentBackground();
        QCOMPARE(area.property("transparent").toBool(), true);
        QCOMPARE(area.viewport()->property("transparent").toBool(), true);
        auto *transparentContent = new QWidget;
        area.setWidget(transparentContent);
        QCOMPARE(transparentContent->property("transparent").toBool(), true);
        QVERIFY(!transparentContent->autoFillBackground());
        area.enableTransparentBackground(false);
        auto *opaqueContent = new QWidget;
        area.setWidget(opaqueContent);
        QCOMPARE(opaqueContent->property("transparent").toBool(), false);
        QVERIFY(opaqueContent->autoFillBackground());
        area.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QVERIFY(area.verticalFluentScrollBar()->isForceHidden());
        area.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QVERIFY(!area.verticalFluentScrollBar()->isForceHidden());

        FluentQt::SingleDirectionScrollArea single(Qt::Vertical);
        QCOMPARE(single.orientation(), Qt::Vertical);
        QCOMPARE(single.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        single.setOrientation(Qt::Horizontal);
        QCOMPARE(single.orientation(), Qt::Horizontal);
        QCOMPARE(single.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);

        FluentQt::SmoothScrollArea smooth;
        QCOMPARE(smooth.delegate(), smooth.scrollDelegate());
        QCOMPARE(smooth.property("delegate").value<FluentQt::SmoothScrollDelegate *>(), smooth.delegate());
        smooth.setScrollAnimation(Qt::Horizontal, 240, QEasingCurve::OutQuad);
        QCOMPARE(smooth.scrollAnimationDuration(Qt::Horizontal), 240);
        QCOMPARE(smooth.scrollAnimationEasing(Qt::Horizontal), QEasingCurve::OutQuad);
    }

    void scrollBarHandleStaysInsideForHugeRanges()
    {
        SmoothScrollBar bar(Qt::Vertical);
        bar.resize(12, 600);
        bar.setRange(0, 5000000);
        bar.setPageStep(600);
        bar.setValue(4000000);
        const QRect handle = bar.handle()->geometry();
        QVERIFY2(handle.y() > 300 && handle.bottom() <= 600, qPrintable(QString::number(handle.y())));
    }

    void smoothScrollAreaKeepsOnlyItsOwnBars()
    {
        SmoothScrollArea area;
        auto *content = new QWidget;
        content->setMinimumHeight(5000);
        area.setWidget(content);
        area.resize(300, 300);
        area.show();
        FqwTest::flushDeletes();
        QCOMPARE(area.findChildren<SmoothScrollBar *>().size(), 2);

        area.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QTest::qWait(50);
        for (SmoothScrollBar *bar : area.findChildren<SmoothScrollBar *>()) {
            if (bar->orientation() == Qt::Vertical) {
                QVERIFY(!bar->isVisible());
            }
        }
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

    void pagerTracksCurrentPageAndButtons()
    {
        FluentQt::PipsPager pager;
        QSignalSpy currentSpy(&pager, &FluentQt::PipsPager::currentIndexChanged);

        pager.setPageNumber(8);
        pager.setVisibleNumber(5);
        pager.setPreviousButtonDisplayMode(FluentQt::PipsScrollButtonDisplayMode::Always);
        pager.setNextButtonDisplayMode(FluentQt::PipsScrollButtonDisplayMode::Always);

        QCOMPARE(pager.pageNumber(), 8);
        QCOMPARE(pager.getPageNumber(), 8);
        QCOMPARE(pager.visibleNumber(), 5);
        QCOMPARE(pager.getVisibleNumber(), 5);
        QCOMPARE(pager.currentIndex(), 0);
        QVERIFY(pager.isHorizontal());
        QVERIFY(!pager.isHover());
        QVERIFY(pager.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(pager.metaObject()->indexOfProperty("pipContainer") >= 0);
        QVERIFY(pager.metaObject()->indexOfProperty("scrollAnimation") >= 0);
        QVERIFY(pager.pipContainer() != nullptr);
        QVERIFY(pager.scrollAnimation() != nullptr);
        QCOMPARE(pager.scrollAnimation()->propertyName(), QByteArray("pipScrollOffset"));
        QCOMPARE(pager.scrollAnimation()->duration(), 500);
        QCOMPARE(pager.property("pipContainer").value<QWidget *>(), pager.pipContainer());
        QCOMPARE(pager.property("scrollAnimation").value<QPropertyAnimation *>(), pager.scrollAnimation());
        QCOMPARE(pager.sizeHint(), QSize(90, 12));
        QCOMPARE(pager.pipButtons().size(), 8);
        QCOMPARE(pager.pipButtons().first()->parentWidget(), pager.pipContainer());
        QCOMPARE(pager.previousButton(), pager.preButton());
        QCOMPARE(pager.previousButton()->size(), QSize(12, 12));
        QCOMPARE(pager.nextButton()->size(), QSize(12, 12));
        QVERIFY(pager.previousButton()->isHidden());
        QVERIFY(!pager.nextButton()->isHidden());
        QVERIFY(!pager.isPreviousButtonVisible());
        QVERIFY(pager.isNextButtonVisible());
        QVERIFY(pager.pipButtons().at(0)->isChecked());

        pager.setCurrentIndex(4);
        QCOMPARE(pager.currentIndex(), 4);
        QCOMPARE(currentSpy.last().at(0).toInt(), 4);
        QVERIFY(!pager.previousButton()->isHidden());
        QVERIFY(!pager.nextButton()->isHidden());
        QVERIFY(pager.isPreviousButtonVisible());
        QVERIFY(pager.isNextButtonVisible());
        QVERIFY(pager.pipButtons().at(4)->isChecked());

        int visiblePips = 0;
        for (QToolButton *button : pager.pipButtons()) {
            visiblePips += !button->isHidden() ? 1 : 0;
        }
        QCOMPARE(visiblePips, 5);
        QCOMPARE(pager.scrollAnimation()->endValue().toReal(), 2.0);

        pager.scrollNext();
        QCOMPARE(pager.currentIndex(), 5);
        pager.scrollPrevious();
        QCOMPARE(pager.currentIndex(), 4);

        pager.setCurrentIndex(99);
        QCOMPARE(pager.currentIndex(), 4);
    }

    void pagerSupportsOrientationAndClampsState()
    {
        FluentQt::VerticalPipsPager pager;
        QCOMPARE(pager.orientation(), Qt::Vertical);
        QVERIFY(!pager.isHorizontal());
        pager.setPageNumber(2);
        pager.setVisibleNumber(0);
        QCOMPARE(pager.visibleNumber(), 1);
        QCOMPARE(pager.getVisibleNumber(), 1);
        QCOMPARE(pager.sizeHint(), QSize(12, 42));

        QSignalSpy orientationSpy(&pager, &FluentQt::PipsPager::orientationChanged);
        pager.setOrientation(Qt::Horizontal);
        QCOMPARE(pager.orientation(), Qt::Horizontal);
        QCOMPARE(orientationSpy.count(), 1);

        pager.setCurrentIndex(1);
        QCOMPARE(pager.currentIndex(), 1);
        pager.setPageNumber(1);
        QCOMPARE(pager.currentIndex(), 0);

        pager.setPageNumber(0);
        QCOMPARE(pager.currentIndex(), -1);
        QCOMPARE(pager.pipButtons().size(), 0);
    }

    void pipsPagerReportsOnlyRealIndexChanges()
    {
        PipsPager pager;
        pager.setPageNumber(5);
        pager.setCurrentIndex(2);
        QSignalSpy spy(&pager, &PipsPager::currentIndexChanged);
        pager.setPageNumber(8);
        QCOMPARE(spy.count(), 0);
        pager.setPageNumber(2);
        QCOMPARE(spy.count(), 1);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ViewTest)
#include "tst_view.moc"
