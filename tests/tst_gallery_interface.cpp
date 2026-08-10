#include "components/GalleryComponents.h"

#include <QtCore/QFile>
#include <QtTest/QtTest>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>

class GalleryInterfaceTest : public QObject
{
    Q_OBJECT

  private slots:
    void galleryInterfaceUsesTransparentRootSurface()
    {
        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));

        QCOMPARE(page.property("transparent").toBool(), true);
        QCOMPARE(page.viewport()->property("transparent").toBool(), true);
        QCOMPARE(page.contentWidget()->property("transparent").toBool(), true);
    }

    void homeInterfaceKeepsRoundedBannerBackgroundTransparent()
    {
        HomeInterface page;
        page.resize(960, 780);
        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));
        QCoreApplication::processEvents();

        QCOMPARE(page.property("transparent").toBool(), true);
        QCOMPARE(page.viewport()->property("transparent").toBool(), true);
        QCOMPARE(page.frameWidth(), 0);
        QCOMPARE(page.viewport()->pos(), QPoint(0, 0));
        QVERIFY(page.widget());
        QCOMPARE(page.widget()->property("transparent").toBool(), true);
        QCOMPARE(page.widget()->pos(), QPoint(0, 0));
        QVERIFY(page.widget()->layout());

        QWidget *banner = page.widget()->layout()->itemAt(0)->widget();
        QVERIFY(banner);
        QCOMPARE(banner->pos(), QPoint(0, 0));
        QCOMPARE(banner->width(), page.viewport()->width());
        QVERIFY(!banner->testAttribute(Qt::WA_StyledBackground));
        QVERIFY(!banner->autoFillBackground());
    }

    void scrollToCardMovesToRequestedExample()
    {
        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));
        page.resize(420, 240);

        for (int i = 0; i < 6; ++i) {
            auto *label = new QLabel(QStringLiteral("Example %1").arg(i));
            label->setFixedHeight(180);
            page.addExampleCard(QStringLiteral("Card %1").arg(i), label,
                                QStringLiteral(FQW_REPOSITORY_URL), 1);
        }

        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));
        QTRY_VERIFY(page.verticalScrollBar()->maximum() > 0);

        QLayoutItem *item = page.contentLayout()->itemAt(3);
        QVERIFY(item != nullptr);
        QWidget *targetCard = item->widget();
        QVERIFY(targetCard != nullptr);

        const int expected =
            qBound(page.verticalScrollBar()->minimum(), targetCard->y(), page.verticalScrollBar()->maximum());
        page.scrollToCard(3);
        QCOMPARE(page.verticalScrollBar()->value(), expected);
    }

    void galleryQssResourcesIncludeIconInterface()
    {
        QFile lightStyle(QStringLiteral(":/gallery/qss/light/icon_interface.qss"));
        QVERIFY(lightStyle.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString lightQss = QString::fromUtf8(lightStyle.readAll());
        QVERIFY(lightQss.contains(QStringLiteral("QFrame#iconCard")));
        QVERIFY(lightQss.contains(QStringLiteral("QFrame#iconInfoPanel")));

        QFile darkStyle(QStringLiteral(":/gallery/qss/dark/icon_interface.qss"));
        QVERIFY(darkStyle.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString darkQss = QString::fromUtf8(darkStyle.readAll());
        QVERIFY(darkQss.contains(QStringLiteral("QFrame#iconCard")));
        QVERIFY(darkQss.contains(QStringLiteral("QFrame#iconInfoPanel")));
    }

    void galleryStylesRefreshWhenThemeChanges()
    {
        const FluentQt::Theme originalTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));
        const QString lightStyle = page.styleSheet();
        QVERIFY(lightStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.667)")));

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Dark);
        QCoreApplication::processEvents();
        const QString darkStyle = page.styleSheet();
        QVERIFY(darkStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.09)")));
        QVERIFY(lightStyle != darkStyle);

        FluentQt::ThemeManager::instance()->setTheme(originalTheme);
    }

    void galleryStylesSurviveGlobalStyleRefresh()
    {
        const FluentQt::Theme originalTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));
        QVERIFY(page.styleSheet().contains(QStringLiteral("rgba(255, 255, 255, 0.667)")));

        FluentQt::FluentStyleSheet::updateAll(FluentQt::Theme::Dark);
        const QString darkStyle = page.styleSheet();
        QVERIFY(darkStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.09)")));
        QVERIFY(darkStyle.contains(QStringLiteral("GalleryInterface #card")));

        FluentQt::FluentStyleSheet::updateAll(FluentQt::Theme::Light);
        const QString lightStyle = page.styleSheet();
        QVERIFY(lightStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.667)")));
        QVERIFY(lightStyle.contains(QStringLiteral("GalleryInterface #card")));

        FluentQt::ThemeManager::instance()->setTheme(originalTheme);
    }
};

QTEST_MAIN(GalleryInterfaceTest)

#include "tst_gallery_interface.moc"
