// Flow, adaptive flow, expand and box layouts, and separators.

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
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QPointer>
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
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyleOptionSlider>
#include <QtWidgets/QTableWidgetItem>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

// A widget whose size hints are exactly the given size, for checking layout geometry.
class FixedHintWidget : public QWidget
{
  public:
    explicit FixedHintWidget(const QSize &hint, QWidget *parent = nullptr) : QWidget(parent), m_hint(hint)
    {
        setMinimumSize(hint);
        resize(hint);
    }

    QSize sizeHint() const override { return m_hint; }
    QSize minimumSizeHint() const override { return m_hint; }

  private:
    QSize m_hint;
};

class LayoutTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void flowLayoutWrapsAndManagesWidgets()
    {
        QWidget parent;
        FluentQt::FlowLayout layout(&parent);
        layout.setContentsMargins(0, 0, 0, 0);
        layout.setHorizontalSpacing(10);
        layout.setVerticalSpacing(10);

        auto *first = new FixedHintWidget(QSize(50, 20), &parent);
        auto *second = new FixedHintWidget(QSize(50, 20), &parent);
        auto *third = new FixedHintWidget(QSize(50, 20), &parent);
        layout.addWidget(first);
        layout.addWidget(second);
        layout.addWidget(third);

        layout.setGeometry(QRect(0, 0, 120, 100));
        QCOMPARE(layout.count(), 3);
        QCOMPARE(first->geometry(), QRect(0, 0, 50, 20));
        QCOMPARE(second->geometry(), QRect(60, 0, 50, 20));
        QCOMPARE(third->geometry(), QRect(0, 30, 50, 20));
        QVERIFY(layout.heightForWidth(120) >= 50);

        layout.setAnimationEnabled(true);
        layout.setAnimation(120, QEasingCurve::OutCubic);
        QCOMPARE(layout.isAnimationEnabled(), true);
        QCOMPARE(layout.animationDuration(), 120);
        QCOMPARE(layout.animationEasing(), QEasingCurve::OutCubic);

        QWidget *removed = layout.removeWidget(second);
        QCOMPARE(removed, second);
        QCOMPARE(layout.count(), 2);
        delete removed;

        layout.takeAllWidgets();
        QCOMPARE(layout.count(), 0);
    }

    void flowLayoutsSupportVerticalAlignment()
    {
        QWidget parent;
        FluentQt::FlowLayout layout(&parent);
        layout.setContentsMargins(0, 0, 0, 0);
        layout.setHorizontalSpacing(10);
        layout.setVerticalAlignment(Qt::AlignVCenter);

        auto *shortWidget = new FixedHintWidget(QSize(50, 20), &parent);
        auto *tallWidget = new FixedHintWidget(QSize(50, 30), &parent);
        layout.addWidget(shortWidget);
        layout.addWidget(tallWidget);
        layout.setGeometry(QRect(0, 0, 120, 60));

        QCOMPARE(layout.verticalAlignment(), Qt::Alignment(Qt::AlignVCenter));
        QCOMPARE(shortWidget->geometry(), QRect(0, 5, 50, 20));
        QCOMPARE(tallWidget->geometry(), QRect(60, 0, 50, 30));

        QWidget adaptiveParent;
        FluentQt::AdaptiveFlowLayout adaptiveLayout(&adaptiveParent);
        adaptiveLayout.setContentsMargins(0, 0, 0, 0);
        adaptiveLayout.setHorizontalSpacing(10);
        adaptiveLayout.setWidgetMinimumWidth(90);
        adaptiveLayout.setVerticalAlignment(Qt::AlignBottom);

        auto *adaptiveShort = new FixedHintWidget(QSize(80, 20), &adaptiveParent);
        auto *adaptiveTall = new FixedHintWidget(QSize(80, 30), &adaptiveParent);
        adaptiveLayout.addWidget(adaptiveShort);
        adaptiveLayout.addWidget(adaptiveTall);
        adaptiveLayout.setGeometry(QRect(0, 0, 190, 60));

        QCOMPARE(adaptiveLayout.verticalAlignment(), Qt::Alignment(Qt::AlignBottom));
        QCOMPARE(adaptiveShort->geometry(), QRect(0, 10, 90, 20));
        QCOMPARE(adaptiveTall->geometry(), QRect(100, 0, 90, 30));
    }

    void flowLayoutAnimatesEachWidgetOnce()
    {
        QWidget parent;
        auto *layout = new FlowLayout(&parent);
        layout->addWidget(new QWidget);
        layout->addItem(new QSpacerItem(10, 10));
        layout->addWidget(new QWidget);
        layout->setAnimationEnabled(true);
        layout->setAnimationEnabled(false);
        layout->setAnimationEnabled(true);
        // The animations live in the layout's animation group.
        QCOMPARE(layout->findChildren<QPropertyAnimation *>().size(), 2);

        delete layout->itemAt(0)->widget();
        parent.resize(300, 200);
        parent.show();
        QTest::qWait(50);
        QCOMPARE(layout->count(), 2);
    }

    void adaptiveAndExpandLayoutsMatchPythonGeometry()
    {
        QWidget adaptiveParent;
        FluentQt::AdaptiveFlowLayout adaptiveLayout(&adaptiveParent);
        adaptiveLayout.setContentsMargins(0, 0, 0, 0);
        adaptiveLayout.setHorizontalSpacing(10);
        adaptiveLayout.setVerticalSpacing(8);
        QCOMPARE(adaptiveLayout.widgetMinimumWidth(), 200);
        QCOMPARE(adaptiveLayout.widgetMaximumWidth(), -1);

        adaptiveLayout.setWidgetMinimumWidth(90);
        QCOMPARE(adaptiveLayout.widgetMinimumWidth(), 90);
        adaptiveLayout.setWidgetMaximumWidth(110);
        QCOMPARE(adaptiveLayout.widgetMaximumWidth(), 110);

        auto *first = new FixedHintWidget(QSize(80, 20), &adaptiveParent);
        auto *second = new FixedHintWidget(QSize(80, 30), &adaptiveParent);
        auto *third = new FixedHintWidget(QSize(80, 25), &adaptiveParent);
        adaptiveLayout.addWidget(first);
        adaptiveLayout.addWidget(second);
        adaptiveLayout.addWidget(third);
        adaptiveParent.show();
        QApplication::processEvents();

        adaptiveLayout.setGeometry(QRect(0, 0, 300, 120));
        QCOMPARE(first->geometry(), QRect(0, 0, 93, 20));
        QCOMPARE(second->geometry(), QRect(103, 0, 93, 30));
        QCOMPARE(third->geometry(), QRect(206, 0, 93, 25));
        QVERIFY(adaptiveLayout.heightForWidth(300) >= 30);

        adaptiveLayout.setWidgetMaximumWidth(90);
        adaptiveLayout.setGeometry(QRect(0, 0, 300, 120));
        QCOMPARE(first->geometry(), QRect(0, 0, 90, 20));
        QCOMPARE(second->geometry(), QRect(100, 0, 90, 30));

        adaptiveLayout.setTight(true);
        second->hide();
        adaptiveLayout.clearWidgetMaximumWidth();
        adaptiveLayout.setGeometry(QRect(0, 0, 190, 120));
        QCOMPARE(first->geometry(), QRect(0, 0, 90, 20));
        QCOMPARE(third->geometry(), QRect(100, 0, 90, 25));

        QWidget expandParent;
        expandParent.resize(120, 40);
        FluentQt::ExpandLayout expandLayout(&expandParent);
        expandLayout.setContentsMargins(0, 0, 0, 0);
        expandLayout.setSpacing(5);

        auto *top = new FixedHintWidget(QSize(40, 20), &expandParent);
        auto *bottom = new FixedHintWidget(QSize(40, 15), &expandParent);
        expandLayout.addWidget(top);
        expandLayout.addWidget(bottom);
        expandLayout.setGeometry(QRect(0, 0, 120, 100));

        QCOMPARE(expandLayout.count(), 2);
        QCOMPARE(top->geometry(), QRect(0, 0, 120, 20));
        QCOMPARE(bottom->geometry(), QRect(0, 25, 120, 15));
        QCOMPARE(expandLayout.expandingDirections(), Qt::Orientations(Qt::Vertical));
        QCOMPARE(expandLayout.heightForWidth(120), 40);

        bottom->hide();
        expandLayout.setGeometry(QRect(0, 0, 120, 100));
        QCOMPARE(top->geometry(), QRect(0, 0, 120, 20));
        QCOMPARE(expandLayout.heightForWidth(120), 20);

        auto *taken = expandLayout.takeAt(0);
        QVERIFY(taken != nullptr);
        delete taken;
        QCOMPARE(expandLayout.count(), 1);
    }

    void vBoxLayoutTracksWidgetsLikePythonReference()
    {
        QWidget parent;
        FluentQt::VBoxLayout layout(&parent);

        auto *first = new QWidget(&parent);
        auto *second = new QWidget(&parent);
        first->hide();
        second->hide();

        layout.addWidgets({first, second});
        QCOMPARE(layout.widgets().size(), 2);
        QCOMPARE(layout.widgets().at(0), first);
        QCOMPARE(layout.widgets().at(1), second);
        QVERIFY(!first->isHidden());
        QVERIFY(!second->isHidden());
        QCOMPARE(layout.count(), 2);

        layout.removeWidget(first);
        QCOMPARE(layout.widgets().size(), 1);
        QCOMPARE(layout.widgets().at(0), second);
        QCOMPARE(layout.count(), 1);
        QCOMPARE(first->parentWidget(), &parent);

        layout.deleteWidget(second);
        QCOMPARE(layout.widgets().size(), 0);
        QCOMPARE(layout.count(), 0);
        QVERIFY(second->isHidden());

        auto *third = new QWidget(&parent);
        auto *fourth = new QWidget(&parent);
        layout.addWidget(third);
        layout.addWidget(fourth);
        QCOMPARE(layout.count(), 2);
        layout.removeAllWidget();
        QCOMPARE(layout.widgets().size(), 0);
        QCOMPARE(layout.count(), 0);
        QCOMPARE(third->parentWidget(), &parent);
        QCOMPARE(fourth->parentWidget(), &parent);
    }

    void vBoxLayoutForgetsDeletedWidgets()
    {
        QWidget parent;
        auto *layout = new VBoxLayout(&parent);
        auto *child = new QWidget;
        layout->addWidget(child);
        QCOMPARE(layout->widgets().size(), 1);
        delete child;
        QCOMPARE(layout->widgets().size(), 0);
    }

    void separatorsMatchPythonMetricsAndThemeColors()
    {
        auto render = [](QWidget &widget, const QColor &background) {
            widget.ensurePolished();
            QPixmap pixmap(widget.size());
            pixmap.fill(background);
            widget.render(&pixmap);
            return pixmap.toImage();
        };

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::HorizontalSeparator horizontal;
        horizontal.resize(40, horizontal.height());
        QCOMPARE(horizontal.property("fqw").toString(), QStringLiteral("HorizontalSeparator"));
        QCOMPARE(horizontal.height(), 3);

        QImage horizontalImage = render(horizontal, Qt::white);
        int lightPaintedPixels = 0;
        for (int y = 0; y < horizontalImage.height(); ++y) {
            for (int x = 0; x < horizontalImage.width(); ++x) {
                const QColor color = horizontalImage.pixelColor(x, y);
                if (color.red() < 255 || color.green() < 255 || color.blue() < 255) {
                    ++lightPaintedPixels;
                }
            }
        }
        QVERIFY(lightPaintedPixels > 0);

        FluentQt::VerticalSeparator vertical;
        vertical.resize(vertical.width(), 40);
        QCOMPARE(vertical.property("fqw").toString(), QStringLiteral("VerticalSeparator"));
        QCOMPARE(vertical.width(), 3);

        QImage verticalImage = render(vertical, Qt::white);
        int verticalPaintedPixels = 0;
        for (int y = 0; y < verticalImage.height(); ++y) {
            for (int x = 0; x < verticalImage.width(); ++x) {
                const QColor color = verticalImage.pixelColor(x, y);
                if (color.red() < 255 || color.green() < 255 || color.blue() < 255) {
                    ++verticalPaintedPixels;
                }
            }
        }
        QVERIFY(verticalPaintedPixels > 0);

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Dark);
        horizontalImage = render(horizontal, Qt::black);
        int darkPaintedPixels = 0;
        for (int y = 0; y < horizontalImage.height(); ++y) {
            for (int x = 0; x < horizontalImage.width(); ++x) {
                const QColor color = horizontalImage.pixelColor(x, y);
                if (color.red() > 0 || color.green() > 0 || color.blue() > 0) {
                    ++darkPaintedPixels;
                }
            }
        }
        QVERIFY(darkPaintedPixels > 0);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(LayoutTest)
#include "tst_layout.moc"
