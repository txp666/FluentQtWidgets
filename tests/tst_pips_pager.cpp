#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QFile>
#include <QtCore/QPropertyAnimation>
#include <QtTest/QtTest>
#include <QtWidgets/QToolButton>

class PipsPagerTest : public QObject
{
    Q_OBJECT

  private slots:
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

    void pagerQssLeavesPipsAndArrowsTransparent()
    {
        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/pips_pager.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/pips_pager.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"PipsPagerDot\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"PipsPagerScrollButton\"]")), qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("background: rgba")), qPrintable(path));
        }
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
};

QTEST_MAIN(PipsPagerTest)

#include "tst_pips_pager.moc"
