// Acrylic labels and brushes, and the Gaussian blur behind them.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QTemporaryDir>
#include <QtGui/QColor>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

#include <iterator>

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

    // gaussianBlur against scipy.ndimage.gaussian_filter as Python's image_utils calls it, computed
    // independently with numpy (symmetric padding, a full correlation, uint8 truncation per pass):
    // a 9x7 RGB image from the LCG below, at sigma 1.5, at sigma 4 (whose radius-16 kernel reflects
    // more than once across the image), and at sigma 2 with Python's 0.85 brightness.
    void gaussianBlurMatchesScipy()
    {
        QFETCH(qreal, sigma);
        QFETCH(qreal, brightFactor);
        QFETCH(QList<int>, expected);

        const QImage source = lcgImage(9, 7);
        const QImage blurred = FluentQt::gaussianBlur(source, sigma, brightFactor);
        QCOMPARE(blurred.format(), QImage::Format_RGB888);
        QCOMPARE(blurred.size(), QSize(9, 7));
        QList<int> actual;
        for (int y = 0; y < 7; ++y) {
            const uchar *line = blurred.constScanLine(y);
            for (int i = 0; i < 9 * 3; ++i) {
                actual.append(line[i]);
            }
        }
        QCOMPARE(actual, expected);
    }

    void gaussianBlurMatchesScipy_data()
    {
        const int kSigma1_5[] = {
            156, 46,  124, 145, 62,  131, 138, 83,  131, 137, 100, 118, 134, 110, 108, 127, 117, 115, 117, 117, 130,
            104, 110, 137, 96,  100, 137, 152, 57,  127, 144, 71,  133, 140, 89,  132, 139, 102, 121, 136, 112, 113,
            128, 121, 119, 120, 126, 131, 115, 122, 138, 112, 116, 139, 149, 77,  132, 145, 87,  135, 143, 99,  133,
            142, 106, 125, 138, 114, 119, 131, 125, 124, 130, 134, 133, 134, 137, 139, 138, 135, 139, 148, 100, 133,
            147, 106, 134, 146, 110, 132, 143, 113, 128, 138, 117, 125, 135, 127, 126, 139, 137, 132, 151, 143, 136,
            161, 145, 139, 149, 124, 133, 149, 124, 134, 146, 124, 134, 140, 123, 132, 136, 123, 130, 134, 128, 128,
            140, 134, 128, 153, 140, 132, 165, 143, 136, 149, 142, 134, 148, 139, 135, 142, 135, 136, 135, 133, 137,
            130, 130, 136, 126, 129, 132, 127, 129, 128, 135, 132, 129, 145, 135, 133, 147, 151, 136, 145, 146, 136,
            139, 142, 138, 132, 139, 141, 126, 135, 142, 118, 130, 138, 113, 126, 130, 116, 126, 128, 124, 128, 131};
        const int kSigma4[] = {
            141, 105, 129, 140, 106, 129, 139, 108, 129, 137, 110, 129, 136, 113, 130, 134, 116, 130, 133, 119, 130,
            132, 120, 130, 131, 121, 131, 141, 106, 130, 140, 107, 130, 139, 109, 130, 138, 112, 130, 136, 114, 130,
            134, 117, 130, 133, 119, 130, 132, 121, 131, 132, 122, 131, 141, 109, 130, 140, 110, 130, 139, 112, 130,
            138, 114, 130, 136, 116, 130, 135, 119, 131, 133, 121, 131, 132, 122, 131, 132, 123, 131, 141, 112, 131,
            140, 113, 131, 139, 114, 131, 138, 116, 131, 136, 119, 131, 135, 121, 131, 134, 122, 131, 133, 124, 131,
            133, 124, 131, 141, 115, 132, 140, 116, 132, 139, 117, 131, 138, 119, 131, 137, 121, 131, 135, 122, 131,
            134, 124, 131, 133, 125, 131, 133, 126, 131, 141, 118, 132, 140, 119, 132, 139, 120, 132, 138, 121, 132,
            137, 123, 132, 136, 124, 132, 135, 125, 132, 134, 126, 132, 134, 127, 132, 141, 120, 132, 140, 120, 132,
            139, 121, 132, 138, 122, 132, 137, 124, 132, 136, 125, 132, 135, 126, 132, 134, 127, 132, 134, 127, 132};
        const int kSigma2Bright[] = {
            125, 55,  109, 123, 62,  109, 119, 73,  107, 116, 84,  103, 113, 92,  102, 107, 97,  103, 102, 100, 108,
            97,  99,  113, 94,  97,  115, 125, 62,  110, 124, 68,  110, 120, 77,  108, 117, 86,  105, 113, 95,  103,
            110, 100, 105, 106, 103, 109, 103, 103, 113, 102, 102, 116, 124, 73,  111, 124, 78,  111, 121, 84,  109,
            119, 91,  107, 115, 97,  105, 113, 103, 107, 112, 107, 110, 112, 109, 113, 113, 110, 116, 124, 88,  113,
            124, 90,  112, 121, 94,  111, 119, 97,  109, 116, 102, 108, 114, 107, 108, 116, 111, 111, 119, 113, 113,
            121, 115, 114, 125, 102, 113, 124, 102, 113, 121, 103, 113, 118, 104, 112, 115, 106, 111, 113, 108, 110,
            115, 112, 111, 119, 114, 112, 123, 116, 113, 124, 113, 114, 123, 113, 114, 119, 111, 114, 116, 110, 114,
            112, 109, 113, 110, 110, 112, 111, 111, 111, 114, 112, 111, 117, 113, 112, 124, 119, 114, 122, 118, 115,
            119, 116, 116, 114, 113, 116, 110, 112, 115, 107, 110, 113, 107, 110, 111, 109, 110, 111, 111, 111, 111};

        QTest::addColumn<qreal>("sigma");
        QTest::addColumn<qreal>("brightFactor");
        QTest::addColumn<QList<int>>("expected");
        auto list = [](const int *values, int size) { return QList<int>(values, values + size); };
        QTest::newRow("sigma 1.5") << 1.5 << 1.0 << list(kSigma1_5, int(std::size(kSigma1_5)));
        QTest::newRow("sigma 4") << 4.0 << 1.0 << list(kSigma4, int(std::size(kSigma4)));
        QTest::newRow("sigma 2, 0.85") << 2.0 << 0.85 << list(kSigma2Bright, int(std::size(kSigma2Bright)));
    }

    void gaussianBlurShrinksToBlurPicSizeAndKeepsAlpha()
    {
        QImage source(40, 20, QImage::Format_ARGB32);
        for (int y = 0; y < source.height(); ++y) {
            for (int x = 0; x < source.width(); ++x) {
                source.setPixelColor(x, y, QColor(x * 6, y * 12, 90, 40 + x * 5));
            }
        }

        // The smaller ratio wins, and only shrinking happens (PIL's resize in Python).
        QCOMPARE(FluentQt::gaussianBlur(source, 2, 1, QSize(10, 10)).size(), QSize(10, 5));
        QCOMPARE(FluentQt::gaussianBlur(source, 2, 1, QSize(80, 80)).size(), QSize(40, 20));

        const QImage blurred = FluentQt::gaussianBlur(source, 3);
        QCOMPARE(blurred.format(), QImage::Format_RGBA8888);
        for (int x = 0; x < source.width(); ++x) {
            QCOMPARE(blurred.pixelColor(x, 10).alpha(), source.pixelColor(x, 10).alpha());
        }
        // Sigma 0 leaves the image alone, as gaussian_filter does.
        const QImage unblurred = FluentQt::gaussianBlur(source, 0);
        QCOMPARE(unblurred.convertToFormat(QImage::Format_ARGB32), source);
    }

    // Python's BlurCoverThread: blurs on a worker thread at 0.85 brightness and caps the image at
    // 450 px unless told otherwise.
    void blurCoverThreadBlursOffTheGuiThread()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("cover.png"));
        QVERIFY(lcgImage(900, 300).save(path));

        FluentQt::BlurCoverThread thread;
        QSignalSpy finished(&thread, &FluentQt::BlurCoverThread::blurFinished);
        QThread *emittingThread = nullptr;
        connect(
            &thread, &FluentQt::BlurCoverThread::blurFinished, this,
            [&emittingThread]() { emittingThread = QThread::currentThread(); }, Qt::DirectConnection);
        thread.blur(path, 1);
        QTRY_COMPARE(finished.count(), 1);
        QVERIFY(emittingThread != QThread::currentThread());
        QCOMPARE(thread.maxSize(), QSize(450, 450));
        const QImage image = finished.constFirst().constFirst().value<QImage>();
        QCOMPARE(image, FluentQt::gaussianBlur(path, 1, 0.85, QSize(450, 450)));
        QCOMPARE(image.size(), QSize(450, 150));

        // An invalid size keeps the previous one, like Python's `maxSize or self.maxSize`.
        QVERIFY(thread.wait());
        thread.blur(path, 2, QSize());
        QTRY_COMPARE(finished.count(), 2);
        QCOMPARE(thread.maxSize(), QSize(450, 450));
        QCOMPARE(thread.blurRadius(), 2);
    }

    // Python's AcrylicLabel is a QLabel that shows the blurred image as its pixmap, takes the
    // image's size, and has an AcrylicTextureLabel on top for the tint, luminosity and noise.
    void acrylicLabelIsPythonsQLabel()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("acrylic.png"));
        QVERIFY(lcgImage(300, 150).save(path));

        FluentQt::AcrylicLabel label(2, QColor(105, 114, 168, 102));
        QVERIFY(qobject_cast<QLabel *>(&label) != nullptr);
        QCOMPARE(label.blurRadius(), 2);
        QVERIFY(!label.maxBlurSize().isValid());
        FluentQt::AcrylicTextureLabel *texture = label.acrylicTextureLabel();
        QVERIFY(texture != nullptr);
        QCOMPARE(texture->parentWidget(), &label);
        QCOMPARE(texture->tintColor(), QColor(105, 114, 168, 102));
        QCOMPARE(texture->luminosityColor(), QColor(255, 255, 255, 0));
        QCOMPARE(texture->noiseOpacity(), 0.03);
        QVERIFY(!texture->noiseImage().isNull());
        QVERIFY(texture->testAttribute(Qt::WA_TranslucentBackground));

        label.show();
        label.setImage(path);
        QCOMPARE(label.imagePath(), path);
        QTRY_VERIFY(!label.blurPixmap().isNull());
        // No size limit unless one is given: Python passes maxBlurSize (None) to the thread.
        const QImage expected = FluentQt::gaussianBlur(path, 2, 0.85);
        QCOMPARE(label.blurPixmap().toImage().convertToFormat(QImage::Format_RGB888), expected);
        QCOMPARE(label.size(), QSize(300, 150));
        QCOMPARE(texture->size(), label.size());

        label.resize(100, 100);
        QCOMPARE(texture->size(), QSize(100, 100));
        QCOMPARE(label.pixmap().size(), QSize(200, 100));

        label.setTintColor(QColor(10, 20, 30, 40));
        QCOMPARE(texture->tintColor(), QColor(10, 20, 30, 40));

        // The blur radius applies from the next setImage(), as in Python's gallery.
        label.setBlurRadius(3);
        QCOMPARE(label.blurRadius(), 3);
        QCOMPARE(label.blurPixmap().toImage().convertToFormat(QImage::Format_RGB888), expected);
        label.setMaxBlurSize(QSize(100, 100));
        label.setImage(path);
        QTRY_COMPARE(label.blurPixmap().size(), QSize(100, 50));
        QCOMPARE(label.blurPixmap().toImage().convertToFormat(QImage::Format_RGB888),
                 FluentQt::gaussianBlur(path, 3, 0.85, QSize(100, 100)));
        QCOMPARE(label.size(), QSize(100, 50));
    }

    void acrylicTextureLabelPaintsTheTextureLayers()
    {
        FluentQt::AcrylicTextureLabel texture(QColor(0, 0, 255, 255), QColor(255, 0, 0, 255), 0);
        texture.resize(20, 20);
        QImage rendered(texture.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        texture.render(&rendered);
        QCOMPARE(rendered.pixelColor(10, 10), QColor(0, 0, 255));

        texture.setTintColor(QColor(0, 0, 0, 0));
        QCOMPARE(texture.tintColor(), QColor(0, 0, 0, 0));
        rendered.fill(Qt::transparent);
        texture.render(&rendered);
        QCOMPARE(rendered.pixelColor(10, 10), QColor(255, 0, 0));
    }

    void acrylicBrushBlursLikePython()
    {
        QImage image(24, 16, QImage::Format_RGB32);
        image.fill(QColor(20, 80, 180));
        image.setPixelColor(5, 5, Qt::white);

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
        QCOMPARE(brush.originalImage(), image);
        QCOMPARE(brush.image(), FluentQt::gaussianBlur(image, 15));

        // Python's setBlurPicSize only applies to the next image; setBlurRadius re-blurs.
        brush.setBlurPicSize(QSize(12, 12));
        QCOMPARE(brush.blurPicSize(), QSize(12, 12));
        QCOMPARE(brush.image().size(), QSize(24, 16));
        brush.setBlurRadius(2);
        QCOMPARE(brush.image(), FluentQt::gaussianBlur(image, 2, 1, QSize(12, 12)));
        QCOMPARE(brush.image().size(), QSize(12, 8));

        brush.setTintColor(QColor(32, 32, 32, 200));
        brush.setLuminosityColor(QColor(0, 0, 0, 0));
        brush.setNoiseOpacity(0.2);
        QCOMPARE(brush.tintColor(), QColor(32, 32, 32, 200));
        QCOMPARE(brush.luminosityColor(), QColor(0, 0, 0, 0));
        QCOMPARE(brush.noiseOpacity(), 0.2);
        QCOMPARE(brush.textureImage().size(), QSize(64, 64));

        QPainterPath clipPath;
        clipPath.addRoundedRect(QRectF(brushDevice.rect()), 4, 4);
        brush.setClipPath(clipPath);
        QCOMPARE(brush.clipPath(), clipPath);

        QImage brushRendered(brushDevice.size(), QImage::Format_ARGB32_Premultiplied);
        brushRendered.fill(Qt::transparent);
        QPainter brushPainter(&brushRendered);
        brush.paint(&brushPainter);
        brushPainter.end();
        QVERIFY(qAlpha(brushRendered.pixel(brushRendered.rect().center())) > 0);

        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("brush.png"));
        QVERIFY(image.save(path));
        QVERIFY(brush.setImage(path));
        QCOMPARE(brush.originalImage().size(), QSize(24, 16));
        QVERIFY(!brush.setImage(QStringLiteral("__missing_acrylic_brush__.png")));
    }

    // The acrylic widgets hand the brush their clip path and colours on every paint, as Python
    // assigns them there; that must not schedule yet another paint.
    void acrylicBrushSettersDoNotRepaintWhenUnchanged()
    {
        PaintCounter device;
        device.resize(40, 30);
        FluentQt::AcrylicBrush brush(&device, 2);
        device.brush = &brush;
        device.show();
        QVERIFY(QTest::qWaitForWindowExposed(&device));
        QTest::qWait(50);
        const int paints = device.paints;
        QTest::qWait(150);
        QVERIFY2(device.paints - paints <= 1, qPrintable(QStringLiteral("%1 more paints").arg(device.paints - paints)));
    }

    // Python's acrylic menu views move the 6 px top and bottom margins into placeholder rows so that
    // the acrylic covers them; the views come out as tall as the plain ones (the completer's is 4 px
    // taller: 6 px placeholders where CompleterMenu has a 2 px top margin).
    void acrylicMenuViewsAreAsTallAsPythons()
    {
        auto addActions = [](FluentQt::RoundMenu &menu) {
            for (const char *text : {"Copy", "Paste", "Select all"}) {
                menu.addAction(new QAction(QString::fromLatin1(text), &menu));
            }
        };
        FluentQt::RoundMenu plainMenu;
        FluentQt::AcrylicMenu acrylicMenu;
        addActions(plainMenu);
        addActions(acrylicMenu);
        QCOMPARE(acrylicMenu.view()->height(), plainMenu.view()->height());
        QCOMPARE(acrylicMenu.layout()->sizeHint().height(), plainMenu.layout()->sizeHint().height());

        FluentQt::AcrylicCheckableMenu acrylicCheckableMenu;
        addActions(acrylicCheckableMenu);
        QCOMPARE(acrylicCheckableMenu.view()->height(), plainMenu.view()->height());

        QWidget host;
        FluentQt::ComboBoxMenu plainComboMenu(&host);
        FluentQt::AcrylicComboBoxMenu acrylicComboMenu(&host);
        addActions(plainComboMenu);
        addActions(acrylicComboMenu);
        QVERIFY(qobject_cast<FluentQt::AcrylicComboMenuActionListWidget *>(acrylicComboMenu.view()));
        QCOMPARE(acrylicComboMenu.view()->height(), plainComboMenu.view()->height());

        FluentQt::LineEdit lineEdit;
        FluentQt::CompleterMenu plainCompleter(&lineEdit);
        FluentQt::AcrylicCompleterMenu acrylicCompleter(&lineEdit);
        const QStringList items{QStringLiteral("apple"), QStringLiteral("banana"), QStringLiteral("cherry")};
        plainCompleter.setItems(items);
        acrylicCompleter.setItems(items);
        QVERIFY(qobject_cast<FluentQt::AcrylicCompleterMenuActionListWidget *>(acrylicCompleter.view()));
        QCOMPARE(acrylicCompleter.view()->height(), plainCompleter.view()->height() + 4);
        QCOMPARE(acrylicCompleter.view()->objectName(), QStringLiteral("completerListWidget"));
    }

    // The acrylic covers the margins too, inside Python's clip paths: the menu's rounded rect, and
    // the completer's with square corners on the side that touches the line edit.
    void acrylicMenuViewsPaintAcrossTheirMargins()
    {
        FluentQt::AcrylicBrush::setScreenCaptureEnabled(false);
        FluentQt::AcrylicMenu menu;
        menu.addAction(new QAction(QStringLiteral("Copy"), &menu));
        auto *view = static_cast<FluentQt::AcrylicMenuActionListWidget *>(menu.view());
        QPainterPath expected;
        expected.addRoundedRect(QRectF(view->rect()).adjusted(0, 0, -2.5, -2.5), 8, 8);
        QCOMPARE(view->clipPath(), expected);

        QImage rendered(view->size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        view->render(&rendered);
        // Inside the 6 px top margin, below the 1 px border.
        QCOMPARE(qAlpha(rendered.pixel(view->width() / 2, 4)), 255);
        QCOMPARE(qAlpha(rendered.pixel(view->width() / 2, view->height() - 6)), 255);

        FluentQt::LineEdit lineEdit;
        FluentQt::AcrylicCompleterMenu completer(&lineEdit);
        completer.setItems({QStringLiteral("apple")});
        auto *completerView = static_cast<FluentQt::AcrylicMenuActionListWidget *>(completer.view());
        const int w = completerView->width();
        const int h = completerView->height();
        completerView->setProperty("dropDown", true);
        QVERIFY(completerView->clipPath().contains(QPointF(1.5, 1.5)));
        QVERIFY(!completerView->clipPath().contains(QPointF(1.5, h - 2.0)));
        completerView->setProperty("dropDown", false);
        QVERIFY(!completerView->clipPath().contains(QPointF(1.5, 1.5)));
        QVERIFY(completerView->clipPath().contains(QPointF(w - 2.0, h - 2.0)));
        FluentQt::AcrylicBrush::setScreenCaptureEnabled(true);
    }

    // Python's AcrylicSystemTrayMenu is an AcrylicMenu: it has no SystemTrayMenu size hint.
    void acrylicSystemTrayMenuSizesLikeAnAcrylicMenu()
    {
        FluentQt::AcrylicSystemTrayMenu trayMenu;
        FluentQt::AcrylicMenu menu;
        trayMenu.addAction(new QAction(QStringLiteral("Show"), &trayMenu));
        menu.addAction(new QAction(QStringLiteral("Show"), &menu));
        QCOMPARE(trayMenu.sizeHint(), menu.sizeHint());
    }

    // Python's AcrylicToolTip swaps the container for an AcrylicToolTipContainer: the plain tool
    // tip's transparent style (border rgba(0, 0, 0, 0.06)) over a blurred capture of the screen.
    void acrylicToolTipUsesAnAcrylicContainer()
    {
        QWidget window;
        window.resize(300, 200);
        FluentQt::ToolTip plain(QStringLiteral("Hello tooltip"), &window);
        FluentQt::AcrylicToolTip tip(QStringLiteral("Hello tooltip"), &window);
        FluentQt::AcrylicToolTipContainer *container = tip.acrylicContainer();
        QVERIFY(container != nullptr);
        QCOMPARE(tip.container(), container);
        QCOMPARE(container->parentWidget(), &tip);
        QCOMPARE(container->objectName(), QStringLiteral("container"));
        QCOMPARE(container->property("transparent").toBool(), true);
        QCOMPARE(tip.label()->parentWidget(), container);
        QCOMPARE(container->graphicsEffect(), tip.shadowEffect());
        QCOMPARE(tip.size(), plain.size());
        QCOMPARE(container->acrylicBrush()->blurRadius(), 30);

        QPainterPath clip;
        clip.addRoundedRect(QRectF(container->rect().adjusted(1, 1, -1, -1)), 3, 3);
        QCOMPARE(container->acrylicClipPath(), clip);

        FluentQt::AcrylicBrush::setScreenCaptureEnabled(false);
        container->setGraphicsEffect(nullptr);
        QImage rendered(container->size(), QImage::Format_ARGB32);
        rendered.fill(Qt::transparent);
        container->render(&rendered);
        FluentQt::AcrylicBrush::setScreenCaptureEnabled(true);
        // The border is Python's faint dark line, not a light one.
        const QColor border = rendered.pixelColor(0, container->height() / 2);
        QVERIFY2(border.alpha() < 40 && border.red() < 60, qPrintable(border.name(QColor::HexArgb)));
        // Inside it, the acrylic tint.
        const QColor inside = rendered.pixelColor(container->width() / 2, container->height() / 2);
        QCOMPARE(inside.alpha(), 255);
        QVERIFY(inside.red() > 230);
    }

    void acrylicToolTipCapturesTheScreenWhenShown()
    {
        QWidget window;
        window.resize(300, 200);
        FluentQt::AcrylicToolTip tip(QStringLiteral("Hello tooltip"), &window);
        QVERIFY(tip.acrylicContainer()->acrylicBrush()->originalImage().isNull());
        tip.show();
        const QImage captured = tip.acrylicContainer()->acrylicBrush()->originalImage();
        if (!FluentQt::AcrylicBrush::isScreenCaptureAvailable() || captured.isNull()) {
            QSKIP("This platform gives no screen capture");
        }
        QCOMPARE(captured.size() / captured.devicePixelRatio(), tip.container()->size());
    }

    // Python's acrylic flyout views draw FlyoutViewBase::borderColor() around the acrylic.
    void acrylicFlyoutViewsDrawTheFlyoutBorderColor()
    {
        struct RedBorderView : FluentQt::AcrylicFlyoutViewBase
        {
            QColor borderColor() const override { return Qt::red; }
        };
        struct RedBorderFlyoutView : FluentQt::AcrylicFlyoutView
        {
            using FluentQt::AcrylicFlyoutView::AcrylicFlyoutView;
            QColor borderColor() const override { return Qt::red; }
        };

        FluentQt::AcrylicBrush::setScreenCaptureEnabled(false);
        RedBorderView base;
        base.resize(120, 80);
        RedBorderFlyoutView view(QStringLiteral("Acrylic"), QStringLiteral("Material"));
        view.resize(view.sizeHint());
        for (QWidget *widget : {static_cast<QWidget *>(&base), static_cast<QWidget *>(&view)}) {
            QImage rendered(widget->size(), QImage::Format_ARGB32);
            rendered.fill(Qt::transparent);
            widget->render(&rendered, QPoint(), QRegion(), QWidget::DrawWindowBackground);
            const QColor border = rendered.pixelColor(widget->width() / 2, 1);
            QVERIFY2(border.red() > 200 && border.green() < 200, qPrintable(border.name(QColor::HexArgb)));
        }
        FluentQt::AcrylicBrush::setScreenCaptureEnabled(true);
    }

  private:
    struct PaintCounter : QWidget
    {
        FluentQt::AcrylicBrush *brush = nullptr;
        int paints = 0;

      protected:
        void paintEvent(QPaintEvent *) override
        {
            ++paints;
            QPainterPath path;
            path.addRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
            brush->setClipPath(path);
            brush->setTintColor(QColor(255, 255, 255, 180));
            QPainter painter(this);
            brush->paint(&painter);
        }
    };

    static QImage lcgImage(int width, int height)
    {
        QImage image(width, height, QImage::Format_RGB888);
        quint32 state = 12345;
        for (int y = 0; y < height; ++y) {
            uchar *line = image.scanLine(y);
            for (int i = 0; i < width * 3; ++i) {
                state = (state * 1103515245u + 12345u) & 0x7fffffffu;
                line[i] = uchar((state >> 16) & 255);
            }
        }
        return image;
    }

    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(MaterialTest)
#include "tst_material.moc"
