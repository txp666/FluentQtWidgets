// Acrylic labels and brushes.

#include <algorithm>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <functional>
#include <QtCore/QBuffer>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QEnterEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyleOptionSlider>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class MaterialTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void acrylicLabelTracksImageAndMaterialProperties()
    {
        QImage image(24, 16, QImage::Format_ARGB32_Premultiplied);
        image.fill(QColor(20, 80, 180));

        FluentQt::AcrylicLabel label(15, QColor(105, 114, 168, 102));
        QCOMPARE(label.property("fqw").toString(), QStringLiteral("AcrylicLabel"));
        QCOMPARE(label.blurRadius(), 15);
        QCOMPARE(label.tintColor(), QColor(105, 114, 168, 102));

        QSignalSpy blurSpy(&label, &FluentQt::AcrylicLabel::blurRadiusChanged);
        label.setBlurRadius(4);
        QCOMPARE(label.blurRadius(), 4);
        QCOMPARE(blurSpy.count(), 1);

        label.setImage(image);
        QVERIFY(!label.isNull());
        QCOMPARE(label.image().size(), QSize(24, 16));
        label.resize(80, 48);

        QImage rendered(label.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        QPainter painter(&rendered);
        label.render(&painter);
        painter.end();
        QVERIFY(qAlpha(rendered.pixel(10, 10)) > 0);

        QTemporaryDir acrylicDir;
        QVERIFY(acrylicDir.isValid());
        const QString acrylicPath = acrylicDir.filePath(QStringLiteral("acrylic.png"));
        QVERIFY(image.save(acrylicPath));
        FluentQt::AcrylicLabel pathLabel(acrylicPath);
        QVERIFY(!pathLabel.isNull());
        QCOMPARE(pathLabel.image().size(), QSize(24, 16));
        pathLabel.setImage(QImage());
        QVERIFY(pathLabel.isNull());
        QVERIFY(pathLabel.setImage(acrylicPath));
        QVERIFY(!pathLabel.isNull());
        QVERIFY(!label.setImagePath(QStringLiteral("__missing_acrylic_image__.png")));

        QWidget brushDevice;
        brushDevice.resize(40, 30);
        FluentQt::AcrylicBrush brush(&brushDevice, 15);
        QCOMPARE(brush.device(), &brushDevice);
        QCOMPARE(brush.blurRadius(), 15);
        QCOMPARE(brush.tintColor(), QColor(242, 242, 242, 150));
        QCOMPARE(brush.luminosityColor(), QColor(255, 255, 255, 10));
        QCOMPARE(brush.noiseOpacity(), 0.03);
        // Acrylic shows the screen through only where it may capture it (on macOS once the user has
        // allowed screen recording), and never once capturing is turned off.
        QVERIFY(FluentQt::AcrylicBrush::isScreenCaptureEnabled());
        QCOMPARE(brush.isAvailable(), FluentQt::AcrylicBrush::isScreenCaptureAvailable());
        FluentQt::AcrylicBrush::setScreenCaptureEnabled(false);
        QVERIFY(!FluentQt::AcrylicBrush::isScreenCaptureAvailable());
        QVERIFY(!brush.isAvailable());
        FluentQt::AcrylicBrush::setScreenCaptureEnabled(true);
#if !defined(Q_OS_MACOS)
        if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"))) {
            QVERIFY(brush.isAvailable());
        }
#endif

        brush.setImage(image);
        QVERIFY(!brush.isNull());
        QCOMPARE(brush.originalImage().size(), QSize(24, 16));
        QVERIFY(!brush.image().isNull());
        brush.setBlurPicSize(QSize(18, 18));
        QCOMPARE(brush.blurPicSize(), QSize(18, 18));
        brush.setTintColor(QColor(32, 32, 32, 200));
        brush.setLuminosityColor(QColor(0, 0, 0, 0));
        brush.setNoiseOpacity(0.2);
        QCOMPARE(brush.tintColor(), QColor(32, 32, 32, 200));
        QCOMPARE(brush.luminosityColor(), QColor(0, 0, 0, 0));
        QCOMPARE(brush.noiseOpacity(), 0.2);
        QVERIFY(!brush.textureImage().isNull());

        QPainterPath clipPath;
        clipPath.addRoundedRect(QRectF(brushDevice.rect()), 4, 4);
        brush.setClipPath(clipPath);
        QVERIFY(!brush.clipPath().isEmpty());

        QImage brushRendered(brushDevice.size(), QImage::Format_ARGB32_Premultiplied);
        brushRendered.fill(Qt::transparent);
        QPainter brushPainter(&brushRendered);
        brush.paint(&brushPainter);
        brushPainter.end();
        QVERIFY(qAlpha(brushRendered.pixel(brushRendered.rect().center())) > 0);
        QVERIFY(brush.setImagePath(acrylicPath));
        QVERIFY(!brush.setImagePath(QStringLiteral("__missing_acrylic_brush__.png")));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(MaterialTest)
#include "tst_material.moc"
