// Scroll areas, Fluent overlay scroll bars, smooth scrolling and pips pagers.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QPointer>
#include <QtCore/QStringListModel>
#include <QtGui/QColor>
#include <QtGui/QStandardItemModel>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class ScrollTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
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

    void scrollBarsAndScrollAreasExposePythonApi()
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
        const auto verticalBarHidden = [&area]() {
            const auto bars = area.findChildren<SmoothScrollBar *>();
            return std::none_of(bars.begin(), bars.end(), [](const SmoothScrollBar *bar) {
                return bar->orientation() == Qt::Vertical && bar->isVisible();
            });
        };
        QTRY_VERIFY(verticalBarHidden());
    }

    // Python's pager keeps its five-pip size with fewer pages, its pips start 15 px in and its
    // next button sits at the far end. The port centred the pips and put the button right after.
    void pagerWithFewPagesKeepsButtonsAtTheEnds()
    {
        for (const Qt::Orientation orientation : {Qt::Horizontal, Qt::Vertical}) {
            QWidget host;
            auto *hostLayout = new QVBoxLayout(&host);
            auto *pagerPointer = new PipsPager(orientation);
            hostLayout->addWidget(pagerPointer, 0, Qt::AlignLeft | Qt::AlignTop);
            PipsPager &pager = *pagerPointer;
            pager.setPageNumber(3);
            pager.setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
            pager.setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
            pager.setCurrentIndex(1);
            host.resize(300, 300);
            host.show();
            QVERIFY(QTest::qWaitForWindowExposed(&host));
            const bool horizontal = orientation == Qt::Horizontal;
            QCOMPARE(pager.size(), horizontal ? QSize(90, 12) : QSize(12, 90));

            QList<QRect> buttons;
            for (QAbstractButton *button : pager.findChildren<QAbstractButton *>()) {
                if (button->isVisible()) {
                    buttons.append(QRect(button->mapTo(&pager, QPoint(0, 0)), button->size()));
                }
            }
            const auto along = [horizontal](const QRect &rect) { return horizontal ? rect.x() : rect.y(); };
            std::sort(buttons.begin(), buttons.end(),
                      [&](const QRect &a, const QRect &b) { return along(a) < along(b); });
            QCOMPARE(buttons.size(), 5);
            QCOMPARE(along(buttons.at(0)), 0);
            QCOMPARE(along(buttons.at(1)), 15);
            QCOMPARE(along(buttons.at(4)), 78);
        }
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

    // Python fixes a pips pager's 12 px thickness at once and its length (visible pips x 12 plus two
    // 15 px button margins) when the page or visible number is set.
    void pipsPagerFixesItsSizeAsPythonDoes()
    {
        FluentQt::HorizontalPipsPager horizontal;
        QCOMPARE(horizontal.minimumHeight(), 12);
        QCOMPARE(horizontal.maximumHeight(), 12);
        QCOMPARE(horizontal.minimumWidth(), 0);
        QCOMPARE(horizontal.maximumWidth(), QWIDGETSIZE_MAX);
        horizontal.setPageNumber(8);
        QCOMPARE(horizontal.minimumSize(), QSize(90, 12));
        QCOMPARE(horizontal.maximumSize(), QSize(90, 12));
        horizontal.setVisibleNumber(3);
        QCOMPARE(horizontal.maximumSize(), QSize(66, 12));

        FluentQt::VerticalPipsPager vertical;
        QCOMPARE(vertical.maximumWidth(), 12);
        QCOMPARE(vertical.maximumHeight(), QWIDGETSIZE_MAX);
        vertical.setPageNumber(5);
        QCOMPARE(vertical.minimumSize(), QSize(12, 90));
        QCOMPARE(vertical.maximumSize(), QSize(12, 90));
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

QTEST_MAIN(ScrollTest)
#include "tst_scroll.moc"
