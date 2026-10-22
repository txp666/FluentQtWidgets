// The realtime plot, chart and audio waveform widgets.

#include <array>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <limits>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QRegion>
#include <QtTest/QtTest>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class ChartsTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void realtimePlotWidgetSupportsMultipleSeriesAndLegend()
    {
        FluentQt::RealtimePlotWidget plot;
        QCOMPARE(plot.seriesCount(), 1);
        QCOMPARE(plot.maximumVisiblePoints(), 10000);
        QCOMPARE(plot.visibleSpan(), 9999.0);
        plot.setSeriesName(0, QStringLiteral("CPU"));
        plot.setSeriesColor(0, QColor(0, 159, 170));
        const int memorySeries = plot.addSeries(QStringLiteral("Memory"), QColor(22, 163, 74));
        QCOMPARE(plot.seriesCount(), 2);
        QCOMPARE(plot.seriesName(memorySeries), QStringLiteral("Memory"));

        QSignalSpy samplesSpy(&plot, &FluentQt::RealtimePlotWidget::samplesChanged);
        QSignalSpy seriesSpy(&plot, &FluentQt::RealtimePlotWidget::seriesChanged);

        plot.setCapacity(4);
        plot.appendSamples(0, QVector<qreal>{1, 2, 3, 4, 5});
        QCOMPARE(plot.points(0).size(), 4);
        QCOMPARE(plot.points(0).first().y(), 2.0);
        plot.setSamples(memorySeries, QVector<qreal>{5, 6, 7, 8});
        QCOMPARE(plot.sampleCount(), 4);
        QVERIFY(samplesSpy.count() >= 2);

        QVERIFY(plot.isSeriesVisible(memorySeries));
        plot.setSeriesVisible(memorySeries, false);
        QVERIFY(!plot.isSeriesVisible(memorySeries));
        plot.setSeriesVisible(memorySeries, true);
        QVERIFY(plot.isSeriesVisible(memorySeries));
        QVERIFY(seriesSpy.count() >= 2);

        QSignalSpy crosshairSpy(&plot, &FluentQt::RealtimePlotWidget::crosshairMoved);
        plot.setAutoScroll(true);
        plot.setMaximumVisiblePoints(6);
        QCOMPARE(plot.maximumVisiblePoints(), 6);
        QCOMPARE(plot.visibleSpan(), 5.0);

        plot.resize(420, 260);
        plot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&plot));
        auto sendHover = [&](const QPoint &hoverPosition) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QMouseEvent hoverEvent(QEvent::MouseMove, QPointF(hoverPosition), QPointF(hoverPosition),
                                   QPointF(plot.mapToGlobal(hoverPosition)), Qt::NoButton, Qt::NoButton,
                                   Qt::NoModifier);
#else
            QMouseEvent hoverEvent(QEvent::MouseMove, hoverPosition, plot.mapToGlobal(hoverPosition), Qt::NoButton,
                                   Qt::NoButton, Qt::NoModifier);
#endif
            QCoreApplication::sendEvent(&plot, &hoverEvent);
        };
        sendHover(QPoint(60, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() >= -0.01);
        crosshairSpy.clear();
        sendHover(QPoint(360, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() <= 4.1);

        plot.setMaximumVisiblePoints(10000);
        crosshairSpy.clear();
        sendHover(QPoint(60, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() >= -0.01);
        crosshairSpy.clear();
        sendHover(QPoint(360, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() <= 4.1);

        plot.setMaximumVisiblePoints(6);
        plot.appendPoint(memorySeries, 10.0, 9.0);
        crosshairSpy.clear();
        sendHover(QPoint(60, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() > 4.0);

        QImage rendered(plot.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        plot.render(&rendered);

        QVERIFY(FqwTest::countPixels(rendered, [](const QColor &pixel) {
                    return pixel.green() > 110 && pixel.red() < 120 && pixel.blue() < 150;
                }) > 0);

        QVERIFY(plot.isSeriesVisible(0));
        QTest::mouseClick(&plot, Qt::LeftButton, Qt::NoModifier, QPoint(166, 36));
        QVERIFY(!plot.isSeriesVisible(0));

        QTest::mouseClick(&plot, Qt::RightButton, Qt::NoModifier, QPoint(220, 140));
        FluentQt::CheckableMenu *contextMenu = nullptr;
        QTRY_VERIFY((contextMenu = FqwTest::findVisibleTopLevel<FluentQt::CheckableMenu>()) != nullptr);
        QVERIFY(contextMenu->menuActions().size() >= 9);
        FluentQt::CheckableMenu *maximumPointsMenu = nullptr;
        const auto childMenus = contextMenu->findChildren<FluentQt::CheckableMenu *>();
        for (FluentQt::CheckableMenu *menu : childMenus) {
            if (menu->title() == QStringLiteral("Maximum visible points")) {
                maximumPointsMenu = menu;
                break;
            }
        }
        QVERIFY(maximumPointsMenu != nullptr);
        QAction *tenThousandPoints = nullptr;
        QAction *exportCsvAction = nullptr;
        QAction *exportImageAction = nullptr;
        for (QAction *action : contextMenu->menuActions()) {
            if (action->text() == QStringLiteral("Export CSV")) {
                exportCsvAction = action;
            } else if (action->text() == QStringLiteral("Export image")) {
                exportImageAction = action;
            }
        }
        QVERIFY(exportCsvAction != nullptr);
        QVERIFY(exportImageAction != nullptr);
        for (QAction *action : maximumPointsMenu->menuActions()) {
            if (action->text() == QStringLiteral("10000")) {
                tenThousandPoints = action;
                break;
            }
        }
        QVERIFY(tenThousandPoints != nullptr);
        tenThousandPoints->trigger();
        QCOMPARE(plot.maximumVisiblePoints(), 10000);
        QCOMPARE(plot.visibleSpan(), 9999.0);
        plot.setXRange(1.0, 2.0);
        plot.setYRange(6.0, 7.0);
        contextMenu->menuActions().constFirst()->trigger();
        QVERIFY(plot.autoScroll());
        QVERIFY(plot.autoYRange());
        QCOMPARE(plot.maximumVisiblePoints(), 10000);
        QCOMPARE(plot.visibleSpan(), 9999.0);
        plot.appendPoint(memorySeries, 10.0, 9.0);
        QVERIFY(plot.autoScroll());

        QTemporaryDir exportDir;
        QVERIFY(exportDir.isValid());
        const QString previousDownloadFolder = FluentQt::FluentConfig::instance()->downloadFolder();
        FluentQt::FluentConfig::instance()->setDownloadFolder(exportDir.path());
        auto acceptSaveDialog = []() {
            QTimer::singleShot(0, qApp, []() {
                auto *dialog = FqwTest::findVisibleTopLevel<QFileDialog>();
                QVERIFY(dialog != nullptr);
                static_cast<QDialog *>(dialog)->done(QDialog::Accepted);
            });
        };

        QPointer<FluentQt::CheckableMenu> csvMenu(contextMenu);
        acceptSaveDialog();
        exportCsvAction->trigger();

        if (csvMenu && csvMenu->isVisible()) {
            csvMenu->close();
            QTRY_VERIFY(!csvMenu || !csvMenu->isVisible());
        }

        contextMenu = nullptr;
        QTest::mouseClick(&plot, Qt::RightButton, Qt::NoModifier, QPoint(220, 140));
        QTRY_VERIFY((contextMenu = FqwTest::findVisibleTopLevel<FluentQt::CheckableMenu>()) != nullptr);
        exportImageAction = nullptr;
        for (QAction *action : contextMenu->menuActions()) {
            if (action->text() == QStringLiteral("Export image")) {
                exportImageAction = action;
                break;
            }
        }
        QVERIFY(exportImageAction != nullptr);
        QPointer<FluentQt::CheckableMenu> imageMenu(contextMenu);
        acceptSaveDialog();
        exportImageAction->trigger();

        const QStringList csvFiles = QDir(exportDir.path()).entryList(QStringList{QStringLiteral("*.csv")});
        const QStringList pngFiles = QDir(exportDir.path()).entryList(QStringList{QStringLiteral("*.png")});
        QCOMPARE(csvFiles.size(), 1);
        QCOMPARE(pngFiles.size(), 1);
        QFile csvFile(QDir(exportDir.path()).filePath(csvFiles.constFirst()));
        QVERIFY(csvFile.open(QIODevice::ReadOnly | QIODevice::Text));
        const QByteArray csv = csvFile.readAll();
        QVERIFY(csv.startsWith("x;CPU;Memory\n"));
        QVERIFY(csv.contains("2;3;7\n"));
        QVERIFY(csv.contains("3;4;8\n"));
        QVERIFY(csv.contains("10;;9\n"));
        FluentQt::FluentConfig::instance()->setDownloadFolder(previousDownloadFolder);

        if (imageMenu && imageMenu->isVisible()) {
            imageMenu->close();
            QTRY_VERIFY(!imageMenu || !imageMenu->isVisible());
        }

        plot.setXRange(0.0, 4.0);
        plot.setYRange(4.0, 10.0);
        const qreal previousPanXMinimum = plot.xMinimum();
        const qreal previousPanYMinimum = plot.yMinimum();
        QTest::mousePress(&plot, Qt::LeftButton, Qt::NoModifier, QPoint(220, 140));
        QTest::mouseMove(&plot, QPoint(260, 180));
        QTest::mouseRelease(&plot, Qt::LeftButton, Qt::NoModifier, QPoint(260, 180));
        QVERIFY(plot.xMinimum() < previousPanXMinimum);
        QVERIFY(plot.yMinimum() > previousPanYMinimum);

        plot.setXRange(0.0, 4.0);
        plot.setYRange(4.0, 10.0);
        const qreal previousXSpan = plot.xMaximum() - plot.xMinimum();
        const qreal previousYSpan = plot.yMaximum() - plot.yMinimum();
        QTest::mousePress(&plot, Qt::RightButton, Qt::NoModifier, QPoint(220, 140));
        QTest::mouseMove(&plot, QPoint(300, 180));
        QTest::mouseRelease(&plot, Qt::RightButton, Qt::NoModifier, QPoint(300, 180));
        QVERIFY((plot.xMaximum() - plot.xMinimum()) < previousXSpan);
        QVERIFY((plot.yMaximum() - plot.yMinimum()) > previousYSpan);
    }

    void realtimePlotWidgetSupportsUnlimitedHistory()
    {
        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(4);
        plot.appendSamples(QVector<qreal>{1, 2, 3, 4, 5});
        QCOMPARE(plot.points().size(), 4);
        QCOMPARE(plot.points().first().y(), 2.0);

        plot.setCapacity(0);
        QCOMPARE(plot.capacity(), 0);
        plot.appendSamples(QVector<qreal>{6, 7, 8, 9, 10});
        QCOMPARE(plot.points().size(), 9);
        QCOMPARE(plot.points().first().y(), 2.0);
        QCOMPARE(plot.points().last().y(), 10.0);

        QVector<qreal> chunkedSamples;
        constexpr int chunkBoundarySampleCount = 5000;
        chunkedSamples.reserve(chunkBoundarySampleCount);
        for (int i = 0; i < chunkBoundarySampleCount; ++i) {
            chunkedSamples.append(1000.0 + i);
        }
        plot.setSamples(chunkedSamples);
        QCOMPARE(plot.points().size(), chunkBoundarySampleCount);
        QCOMPARE(plot.points().at(4095).y(), 5095.0);
        QCOMPARE(plot.points().at(4096).y(), 5096.0);
        QCOMPARE(plot.points().last().y(), 5999.0);

        plot.setCapacity(4);
        QCOMPARE(plot.capacity(), 4);
        QCOMPARE(plot.points().size(), 4);
        QCOMPARE(plot.points().first().y(), 5996.0);
        QCOMPARE(plot.points().last().y(), 5999.0);
    }

    void realtimePlotWidgetSetSamplesFiltersInvalidValues()
    {
        const qreal nan = std::numeric_limits<qreal>::quiet_NaN();
        const qreal infinity = std::numeric_limits<qreal>::infinity();

        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(4);
        plot.setSamples(QVector<qreal>{1.0, nan, 2.0, infinity, 3.0, 4.0, 5.0});
        QCOMPARE(plot.points().size(), 4);
        QCOMPARE(plot.points().first().x(), 2.0);
        QCOMPARE(plot.points().first().y(), 2.0);
        QCOMPARE(plot.points().last().x(), 6.0);
        QCOMPARE(plot.points().last().y(), 5.0);

        plot.appendSample(6.0);
        QCOMPARE(plot.points().last().x(), 7.0);
        QCOMPARE(plot.points().last().y(), 6.0);

        plot.setCapacity(0);
        plot.setSamples(QVector<qreal>{nan, 1.0, infinity, 2.0});
        QCOMPARE(plot.points().size(), 2);
        QCOMPARE(plot.points().first().x(), 1.0);
        QCOMPARE(plot.points().first().y(), 1.0);
        QCOMPARE(plot.points().last().x(), 3.0);
        QCOMPARE(plot.points().last().y(), 2.0);

        plot.appendSample(3.0);
        QCOMPARE(plot.points().last().x(), 4.0);
        QCOMPARE(plot.points().last().y(), 3.0);
    }

    void realtimePlotWidgetBatchesMultiSeriesSamples()
    {
        FluentQt::RealtimePlotWidget plot;
        QCOMPARE(plot.refreshRate(), 60);

        QSignalSpy refreshSpy(&plot, &FluentQt::RealtimePlotWidget::refreshRateChanged);
        plot.setRefreshRate(30);
        QCOMPARE(plot.refreshRate(), 30);
        QCOMPARE(refreshSpy.count(), 1);
        QCOMPARE(refreshSpy.takeFirst().at(0).toInt(), 30);

        plot.setRefreshRate(-1);
        QCOMPARE(plot.refreshRate(), 0);
        QCOMPARE(refreshSpy.count(), 1);
        QCOMPARE(refreshSpy.takeFirst().at(0).toInt(), 0);

        QSignalSpy samplesSpy(&plot, &FluentQt::RealtimePlotWidget::samplesChanged);
        plot.appendSamples(QVector<QVector<qreal>>{QVector<qreal>{1, 2, 3}, QVector<qreal>{4, 5, 6}});
        QCOMPARE(samplesSpy.count(), 1);
        QCOMPARE(plot.seriesCount(), 2);
        QCOMPARE(plot.sampleCount(), 3);
        QCOMPARE(plot.points(0).last().y(), 3.0);
        QCOMPARE(plot.points(1).last().y(), 6.0);
    }

    void realtimePlotWidgetRendersLargeMultiSeriesData()
    {
        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(60000);
        plot.setMaximumVisiblePoints(50000);
        plot.setLegendVisible(false);
        plot.setPointsVisible(false);
        plot.setSeriesName(0, QStringLiteral("CPU"));
        plot.setSeriesColor(0, QColor(0, 159, 170));
        const int memorySeries = plot.addSeries(QStringLiteral("Memory"), QColor(22, 163, 74));
        const int ioSeries = plot.addSeries(QStringLiteral("I/O"), QColor(245, 158, 11));

        constexpr int sampleCount = 50000;
        QVector<qreal> cpu;
        QVector<qreal> memory;
        QVector<qreal> io;
        cpu.reserve(sampleCount);
        memory.reserve(sampleCount);
        io.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            const qreal ramp = static_cast<qreal>(i % 400) / 4.0;
            const qreal folded = i % 800 < 400 ? ramp : 100.0 - ramp;
            const qreal pulse = i % 997 < 18 ? 42.0 : 0.0;
            cpu.append(30.0 + folded * 0.45 + pulse);
            memory.append(42.0 + static_cast<qreal>((i * 17) % 300) / 7.0);
            io.append(20.0 + static_cast<qreal>((i * 31) % 500) / 8.0 + (i % 1409 < 12 ? 55.0 : 0.0));
        }
        plot.setSamples(0, cpu);
        plot.setSamples(memorySeries, memory);
        plot.setSamples(ioSeries, io);

        plot.resize(900, 360);
        plot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&plot));

        QImage rendered(plot.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        plot.render(&rendered);

        QVERIFY(FqwTest::countPixels(rendered, [](const QColor &pixel) {
                    const bool tealPixel = pixel.red() < 90 && pixel.green() > 95 && pixel.blue() > 95;
                    const bool greenPixel = pixel.red() < 100 && pixel.green() > 100 && pixel.blue() < 150;
                    const bool orangePixel = pixel.red() > 170 && pixel.green() > 90 && pixel.blue() < 90;
                    return pixel.alpha() > 0 && (tealPixel || greenPixel || orangePixel);
                }) > 0);
    }

    void realtimePlotWidgetRendersUnlimitedLargeData()
    {
        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(0);
        plot.setMaximumVisiblePoints(160000);
        plot.setLegendVisible(false);
        plot.setPointsVisible(false);
        plot.setSeriesName(0, QStringLiteral("Unlimited"));
        plot.setSeriesColor(0, QColor(0, 159, 170));

        constexpr int sampleCount = 160000;
        QVector<qreal> samples;
        samples.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            const qreal ramp = static_cast<qreal>(i % 1200) / 12.0;
            const qreal folded = i % 2400 < 1200 ? ramp : 100.0 - ramp;
            samples.append(20.0 + folded * 0.6 + (i % 5003 < 16 ? 55.0 : 0.0));
        }
        plot.setSamples(samples);
        QCOMPARE(plot.capacity(), 0);
        QCOMPARE(plot.sampleCount(), sampleCount);

        plot.resize(520, 260);
        plot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&plot));

        QImage rendered(plot.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        plot.render(&rendered);

        QVERIFY(FqwTest::countPixels(rendered, [](const QColor &pixel) {
                    return pixel.alpha() > 0 && pixel.red() < 90 && pixel.green() > 95 && pixel.blue() > 95;
                }) > 0);
    }

    void chartWidgetRendersNativeChart()
    {
        QJsonObject option{
            {QStringLiteral("xAxis"), QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                                                  {QStringLiteral("data"), QJsonArray{QStringLiteral("A"), QStringLiteral("B")}}}},
            {QStringLiteral("yAxis"), QJsonObject{{QStringLiteral("type"), QStringLiteral("value")}}},
            {QStringLiteral("series"),
             QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("bar")},
                                    {QStringLiteral("data"), QJsonArray{1, 2}}}}}};
        FluentQt::ChartWidget chart(option);
        QCOMPARE(chart.option(), option);
        QCOMPARE(chart.chartTheme(), QStringLiteral("auto"));
        chart.setOptionJson(QStringLiteral("{\"title\":{\"text\":\"Updated\"}}"));
        QVERIFY(chart.option().contains(QStringLiteral("title")));
        chart.setChartTheme(QStringLiteral("dark"));
        QCOMPARE(chart.chartTheme(), QStringLiteral("dark"));
        chart.setOption(option);

        QSignalSpy loadSpy(&chart, &FluentQt::ChartWidget::loadFinished);
        chart.resize(420, 260);
        chart.show();
        QVERIFY(QTest::qWaitForWindowExposed(&chart));
        chart.reload();
        if (loadSpy.isEmpty()) {
            QVERIFY(loadSpy.wait(1000));
        }
        QVERIFY(loadSpy.last().value(0).toBool());

        QImage rendered(chart.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        chart.render(&rendered);

        QVERIFY(FqwTest::countPixels(rendered, [](const QColor &pixel) {
                    return pixel.alpha() > 0 && pixel.green() > 100 && pixel.red() < 80;
                }) > 0);
    }

    void audioWaveformWidgetTracksSamplesAndProgress()
    {
        FluentQt::AudioWaveformWidget waveform;
        QVector<qreal> samples{0.0, 0.2, 0.8, 1.4, -0.5};
        QSignalSpy samplesSpy(&waveform, &FluentQt::AudioWaveformWidget::samplesChanged);
        QSignalSpy progressSpy(&waveform, &FluentQt::AudioWaveformWidget::progressChanged);
        QSignalSpy clickSpy(&waveform, &FluentQt::AudioWaveformWidget::waveformClicked);

        waveform.setSamples(samples);
        QCOMPARE(samplesSpy.count(), 1);
        QCOMPARE(waveform.samples().size(), 5);
        QCOMPARE(waveform.samples().at(3), 1.0);
        QCOMPARE(waveform.samples().at(4), 0.5);
        QCOMPARE(waveform.sampleLevels().size(), 5);

        waveform.setProgress(1.8);
        QCOMPARE(waveform.progress(), 1.0);
        waveform.setProgress(-0.4);
        QCOMPARE(waveform.progress(), 0.0);
        QVERIFY(progressSpy.count() >= 2);

        waveform.resize(360, 120);
        waveform.show();
        QVERIFY(QTest::qWaitForWindowExposed(&waveform));
        QTest::mouseClick(&waveform, Qt::LeftButton, Qt::NoModifier, QPoint(180, 60));
        QCOMPARE(clickSpy.count(), 1);
        QVERIFY(waveform.progress() > 0.40);
        QVERIFY(waveform.progress() < 0.60);

        QImage rendered(waveform.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        waveform.render(&rendered);
        QVERIFY(FqwTest::countPixels(rendered, [](const QColor &pixel) {
                    return pixel.alpha() > 0 && pixel.red() < 180 && pixel.green() < 180 && pixel.blue() < 180;
                }) > 0);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ChartsTest)
#include "tst_charts.moc"
