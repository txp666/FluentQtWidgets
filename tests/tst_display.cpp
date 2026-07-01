#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QBuffer>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
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
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyleOptionSlider>
#include <QtTest/QtTest>

#include <algorithm>
#include <functional>

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

class DisplayTest : public QObject
{
    Q_OBJECT

  private slots:
    void iconWidgetTracksIconAndSize()
    {
        FluentQt::IconWidget widget;
        QCOMPARE(widget.property("fqw").toString(), QStringLiteral("IconWidget"));
        QCOMPARE(widget.iconSize(), QSize(20, 20));

        QSignalSpy iconSpy(&widget, &FluentQt::IconWidget::iconChanged);
        widget.setIcon(FluentQt::FluentIcon::Setting);
        QCOMPARE(iconSpy.count(), 1);

        widget.setIconSize(QSize(32, 24));
        QCOMPARE(widget.iconSize(), QSize(32, 24));
        QCOMPARE(widget.sizeHint(), QSize(32, 24));

        QCOMPARE(FluentQt::iconName(FluentQt::FluentIcon::Basketball), QStringLiteral("basketball"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Basketball), QStringLiteral("Basketball"));
        QVERIFY(QFile::exists(FluentQt::iconPath(FluentQt::FluentIcon::Basketball, FluentQt::Theme::Light)));
        QVERIFY(!FluentQt::icon(FluentQt::FluentIcon::Basketball).isNull());

        const QVector<QPair<FluentQt::FluentIcon, QString>> addedGalleryIcons = {
            {FluentQt::FluentIcon::Tag, QStringLiteral("Tag")},
            {FluentQt::FluentIcon::BookShelf, QStringLiteral("BookShelf")},
            {FluentQt::FluentIcon::Mail, QStringLiteral("Mail")},
            {FluentQt::FluentIcon::Send, QStringLiteral("Send")},
            {FluentQt::FluentIcon::Save, QStringLiteral("Save")},
        };
        for (const auto &entry : addedGalleryIcons) {
            QCOMPARE(FluentQt::iconResourceName(entry.first), entry.second);
            QVERIFY(QFile::exists(FluentQt::iconPath(entry.first, FluentQt::Theme::Light)));
            QVERIFY(!FluentQt::icon(entry.first).isNull());
        }
    }

    void infoBadgesMatchPythonDefaultsAndSizing()
    {
        FluentQt::InfoBadge badge;
        QCOMPARE(badge.property("fqw").toString(), QStringLiteral("InfoBadge"));
        QCOMPARE(badge.level(), FluentQt::InfoLevel::Attention);
        QCOMPARE(badge.property("level").toString(), QStringLiteral("Attention"));
        QCOMPARE(badge.testAttribute(Qt::WA_TranslucentBackground), true);
        QCOMPARE(badge.testAttribute(Qt::WA_TransparentForMouseEvents), true);
        QCOMPARE(badge.sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);
        QCOMPARE(badge.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
        QVERIFY(badge.metaObject()->indexOfProperty("lightBackgroundColor") >= 0);
        QVERIFY(badge.metaObject()->indexOfProperty("darkBackgroundColor") >= 0);
        badge.setProperty("lightBackgroundColor", QColor(QStringLiteral("#123456")));
        badge.setProperty("darkBackgroundColor", QColor(QStringLiteral("#654321")));
        QCOMPARE(badge.lightBackgroundColor(), QColor(QStringLiteral("#123456")));
        QCOMPARE(badge.darkBackgroundColor(), QColor(QStringLiteral("#654321")));
        badge.resize(badge.sizeHint());
        QImage badgeImage(badge.size(), QImage::Format_ARGB32_Premultiplied);
        badgeImage.fill(Qt::transparent);
        badge.render(&badgeImage);
        const QColor expectedBadgeColor =
            FluentQt::ThemeManager::instance()->effectiveTheme() == FluentQt::Theme::Dark
                ? QColor(QStringLiteral("#654321"))
                : QColor(QStringLiteral("#123456"));
        QCOMPARE(QColor::fromRgba(badgeImage.pixel(badgeImage.rect().center())), expectedBadgeColor);

        FluentQt::InfoBadge textBadge(QStringLiteral("3"));
        QCOMPARE(textBadge.level(), FluentQt::InfoLevel::Attention);

        FluentQt::InfoBadge singleDigitBadge(QStringLiteral("1"));
        QVERIFY(singleDigitBadge.sizeHint().width() >= singleDigitBadge.sizeHint().height());
        QVERIFY(singleDigitBadge.minimumSizeHint().width() >= singleDigitBadge.minimumSizeHint().height());

        auto *infoBadge = FluentQt::InfoBadge::info(120);
        QCOMPARE(infoBadge->text(), QStringLiteral("120"));
        QCOMPARE(infoBadge->level(), FluentQt::InfoLevel::Info);
        delete infoBadge;

        auto *stringBadge = FluentQt::InfoBadge::success(QStringLiteral("new"));
        QCOMPARE(stringBadge->text(), QStringLiteral("new"));
        QCOMPARE(stringBadge->level(), FluentQt::InfoLevel::Success);
        delete stringBadge;

        auto *madeBadge = FluentQt::InfoBadge::make(QStringLiteral("draft"), nullptr, FluentQt::InfoLevel::Warning);
        QCOMPARE(madeBadge->text(), QStringLiteral("draft"));
        QCOMPARE(madeBadge->level(), FluentQt::InfoLevel::Warning);
        delete madeBadge;

        FluentQt::DotInfoBadge dot;
        QCOMPARE(dot.property("fqw").toString(), QStringLiteral("DotInfoBadge"));
        QCOMPARE(dot.size(), QSize(4, 4));
        QCOMPARE(dot.minimumSize(), QSize(4, 4));
        QCOMPARE(dot.maximumSize(), QSize(4, 4));
        QCOMPARE(dot.level(), FluentQt::InfoLevel::Attention);

        FluentQt::IconInfoBadge iconBadge(FluentQt::FluentIcon::Accept);
        QCOMPARE(iconBadge.property("fqw").toString(), QStringLiteral("IconInfoBadge"));
        QCOMPARE(iconBadge.level(), FluentQt::InfoLevel::Attention);
        QCOMPARE(iconBadge.size(), QSize(16, 16));
        QCOMPARE(iconBadge.minimumSize(), QSize(16, 16));
        QCOMPARE(iconBadge.maximumSize(), QSize(16, 16));
        QCOMPARE(iconBadge.sizeHint(), QSize(16, 16));
        QCOMPARE(iconBadge.iconSize(), QSize(8, 8));
        QVERIFY(!iconBadge.icon().isNull());
        QVERIFY(!iconBadge.pixmap());

        QSignalSpy iconSpy(&iconBadge, &FluentQt::IconInfoBadge::iconChanged);
        iconBadge.setIcon(FluentQt::FluentIcon::Close);
        QCOMPARE(iconSpy.count(), 1);
        iconBadge.setIconSize(QSize(10, 6));
        QCOMPARE(iconBadge.iconSize(), QSize(10, 6));

        auto *warningIconBadge = FluentQt::IconInfoBadge::warning(FluentQt::FluentIcon::Close);
        QCOMPARE(warningIconBadge->level(), FluentQt::InfoLevel::Warning);
        QCOMPARE(warningIconBadge->property("level").toString(), QStringLiteral("Warning"));
        delete warningIconBadge;

        QWidget container;
        QWidget target(&container);
        target.setGeometry(20, 30, 100, 40);
        auto *attachedBadge = FluentQt::InfoBadge::info(8, nullptr, &target, FluentQt::InfoBadgePosition::BottomRight);
        QVERIFY(attachedBadge->manager());
        QCOMPARE(attachedBadge->parentWidget(), &container);
        QCOMPARE(attachedBadge->manager()->target(), &target);
        QCOMPARE(attachedBadge->manager()->badge(), attachedBadge);
        QCOMPARE(attachedBadge->pos(), attachedBadge->manager()->position());

        auto *attachedDot = FluentQt::DotInfoBadge::custom(QColor(QStringLiteral("#010203")),
                                                           QColor(QStringLiteral("#040506")), nullptr, &target,
                                                           FluentQt::InfoBadgePosition::TopLeft);
        QVERIFY(attachedDot->manager());
        QCOMPARE(attachedDot->lightBackgroundColor(), QColor(QStringLiteral("#010203")));
        QCOMPARE(attachedDot->darkBackgroundColor(), QColor(QStringLiteral("#040506")));

        auto *attachedIcon = FluentQt::IconInfoBadge::success(FluentQt::FluentIcon::Accept, nullptr, &target,
                                                              FluentQt::InfoBadgePosition::Right);
        QVERIFY(attachedIcon->manager());
        QCOMPARE(attachedIcon->level(), FluentQt::InfoLevel::Success);
    }

    void avatarWidgetTracksTextImageAndRadius()
    {
        FluentQt::DisplayLabel display(QStringLiteral("Display"));
        QCOMPARE(display.property("fqw").toString(), QStringLiteral("DisplayLabel"));

        QPixmap imagePixmap(10, 8);
        imagePixmap.fill(Qt::blue);
        FluentQt::ImageLabel imageLabel(imagePixmap);
        QCOMPARE(imageLabel.property("fqw").toString(), QStringLiteral("ImageLabel"));
        QVERIFY(!imageLabel.isNull());
        QCOMPARE(imageLabel.image().size(), QSize(10, 8));
        imageLabel.setBorderRadius(4);
        QCOMPARE(imageLabel.borderRadius(), 4);
        QVERIFY(!imageLabel.setImagePath(QStringLiteral("__missing_image__.png")));

        QTemporaryDir imageDir;
        QVERIFY(imageDir.isValid());
        const QString imagePath = imageDir.filePath(QStringLiteral("image-label.png"));
        QVERIFY(imagePixmap.save(imagePath));
        FluentQt::ImageLabel pathImageLabel(imagePath);
        QVERIFY(!pathImageLabel.isNull());
        QCOMPARE(pathImageLabel.image().size(), QSize(10, 8));

        QSignalSpy imageClickSpy(&imageLabel, &FluentQt::ImageLabel::clicked);
        QTest::mouseRelease(&imageLabel, Qt::LeftButton);
        QCOMPARE(imageClickSpy.count(), 1);

        FluentQt::AvatarWidget avatar;
        avatar.setText(QStringLiteral("Fluent"));
        QCOMPARE(avatar.property("fqw").toString(), QStringLiteral("AvatarWidget"));
        QCOMPARE(avatar.text(), QStringLiteral("Fluent"));
        QCOMPARE(avatar.radius(), 48);
        QCOMPARE(avatar.sizeHint(), QSize(96, 96));

        QSignalSpy radiusSpy(&avatar, &FluentQt::AvatarWidget::radiusChanged);
        avatar.setRadius(18);
        QCOMPARE(avatar.radius(), 18);
        QCOMPARE(avatar.size(), QSize(36, 36));
        QCOMPARE(radiusSpy.count(), 1);

        avatar.setBackgroundColor(QColor(QStringLiteral("#112233")), QColor(QStringLiteral("#445566")));
        QCOMPARE(avatar.lightBackgroundColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(avatar.darkBackgroundColor(), QColor(QStringLiteral("#445566")));

        QPixmap pixmap(12, 12);
        pixmap.fill(Qt::red);
        avatar.setPixmap(pixmap);
        QVERIFY(!avatar.imagePixmap().isNull());
        QCOMPARE(avatar.size(), QSize(36, 36));

        const QString avatarPath = imageDir.filePath(QStringLiteral("avatar.png"));
        QVERIFY(pixmap.save(avatarPath));
        FluentQt::AvatarWidget pathAvatar(avatarPath);
        QVERIFY(!pathAvatar.imagePixmap().isNull());

        QImage remoteImage(14, 10, QImage::Format_ARGB32_Premultiplied);
        remoteImage.fill(QColor(12, 34, 56));
        QByteArray remotePng;
        QBuffer remoteBuffer(&remotePng);
        QVERIFY(remoteBuffer.open(QIODevice::WriteOnly));
        QVERIFY(remoteImage.save(&remoteBuffer, "PNG"));

        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        connect(&server, &QTcpServer::newConnection, &server, [&server, remotePng]() {
            while (auto *socket = server.nextPendingConnection()) {
                QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, remotePng]() {
                    socket->readAll();
                    const QByteArray header = QByteArray("HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: ") +
                                              QByteArray::number(remotePng.size()) +
                                              QByteArray("\r\nConnection: close\r\n\r\n");
                    socket->write(header);
                    socket->write(remotePng);
                    socket->disconnectFromHost();
                });
                QObject::connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
            }
        });

        FluentQt::AvatarWidget remoteAvatar;
        QSignalSpy loadedSpy(&remoteAvatar, &FluentQt::AvatarWidget::imageLoaded);
        QSignalSpy failedSpy(&remoteAvatar, &FluentQt::AvatarWidget::imageLoadFailed);
        QVERIFY(loadedSpy.isValid());
        QVERIFY(failedSpy.isValid());
        QVERIFY(!remoteAvatar.setImageUrl(QUrl(QStringLiteral("ftp://localhost/avatar.png"))));
        QVERIFY(remoteAvatar.setImage(QStringLiteral("http://127.0.0.1:%1/avatar.png").arg(server.serverPort())));
        QTRY_COMPARE(loadedSpy.count(), 1);
        QCOMPARE(failedSpy.count(), 0);
        QCOMPARE(remoteAvatar.image().size(), remoteImage.size());
        QCOMPARE(remoteAvatar.size(), QSize(96, 96));
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

    void pixmapLabelHandlesEmptyAndHighDpiPixmapPainting()
    {
        FluentQt::PixmapLabel label;
        label.resize(24, 24);

        QImage emptyRender(label.size(), QImage::Format_ARGB32_Premultiplied);
        emptyRender.fill(Qt::transparent);
        QPainter emptyPainter(&emptyRender);
        label.render(&emptyPainter);
        emptyPainter.end();
        QVERIFY(label.pixmap().isNull());

        QPixmap pixmap(10, 8);
        pixmap.fill(Qt::green);
        label.setPixmap(pixmap);
        QCOMPARE(label.pixmap().size(), QSize(10, 8));
        QCOMPARE(label.size(), QSize(10, 8));

        QImage rendered(label.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        QPainter painter(&rendered);
        label.render(&painter);
        painter.end();
        QVERIFY(rendered.pixelColor(5, 4).green() > 120);
    }

    void labelContextMenuMatchesPythonActions()
    {
        FluentQt::BodyLabel label(QStringLiteral("Alpha Beta"));
        label.setTextInteractionFlags(Qt::TextSelectableByMouse);

        FluentQt::LabelContextMenu plainMenu(&label);
        plainMenu.createActions();
        QCOMPARE(plainMenu.menuActions().size(), 1);
        QCOMPARE(plainMenu.menuActions().first(), plainMenu.selectAllAction());
        plainMenu.selectAllAction()->trigger();
        QVERIFY(label.hasSelectedText());
        QCOMPARE(label.selectedText(), label.text());

        label.setSelection(0, 5);
        FluentQt::LabelContextMenu selectedMenu(&label);
        selectedMenu.createActions();
        QCOMPARE(selectedMenu.selectedText(), QStringLiteral("Alpha"));
        QCOMPARE(selectedMenu.menuActions().size(), 2);
        QCOMPARE(selectedMenu.menuActions().at(0), selectedMenu.copyAction());
        QCOMPARE(selectedMenu.menuActions().at(1), selectedMenu.selectAllAction());

        QApplication::clipboard()->clear();
        selectedMenu.copyAction()->trigger();
        QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("Alpha"));
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

    void cycleListWidgetMatchesPythonPickerColumnBehavior()
    {
        FluentQt::CycleListWidget shortList(QStringList{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")},
                                            QSize(50, 20), Qt::AlignCenter);
        shortList.setVisibleNumber(5);
        QCOMPARE(shortList.property("fqw").toString(), QStringLiteral("CycleListWidget"));
        QCOMPARE(shortList.property("cycleColumn").toBool(), true);
        QCOMPARE(shortList.itemSize(), QSize(50, 20));
        QCOMPARE(shortList.visibleNumber(), 5);
        QCOMPARE(shortList.size(), QSize(58, 100));
        QCOMPARE(shortList.originItems(), QStringList({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}));
        QVERIFY(!shortList.isCycle());
        QCOMPARE(shortList.count(), 7);
        QCOMPARE(shortList.currentCycleIndex(), 2);
        QVERIFY(shortList.upButton() != nullptr);
        QVERIFY(shortList.downButton() != nullptr);
        QVERIFY(shortList.verticalSmoothScrollBar() != nullptr);
        QCOMPARE(shortList.verticalSmoothScrollBar()->isForceHidden(), true);
        QCOMPARE(shortList.upButton()->fluentIcon(), FluentQt::FluentIcon::Up);
        QCOMPARE(shortList.downButton()->fluentIcon(), FluentQt::FluentIcon::ArrowDown);
        QCOMPARE(shortList.upButton()->property("pickerOperation").toBool(), true);
        QCOMPARE(shortList.downButton()->property("pickerOperation").toBool(), true);
        QVERIFY(shortList.upButton()->metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(shortList.upButton()->metaObject()->indexOfProperty("fluentIcon") >= 0);

        shortList.setScrollButtonRepeatEnabled(false);
        QVERIFY(!shortList.isScrollButtonRepeatEnabled());
        QCOMPARE(shortList.upButton()->autoRepeat(), false);
        QCOMPARE(shortList.downButton()->autoRepeat(), false);
        shortList.setScrollButtonRepeatEnabled(true);
        QVERIFY(shortList.isScrollButtonRepeatEnabled());
        QCOMPARE(shortList.upButton()->autoRepeat(), true);
        QCOMPARE(shortList.downButton()->autoRepeat(), true);

        shortList.setSelectedItem(QStringLiteral("B"));
        QCOMPARE(shortList.currentCycleIndex(), 3);
        QCOMPARE(shortList.currentItem()->text(), QStringLiteral("B"));
        shortList.scrollUp();
        QCOMPARE(shortList.currentCycleIndex(), 2);
        shortList.scrollUp();
        QCOMPARE(shortList.currentCycleIndex(), 2);

        shortList.setScrollDuration(123);
        QCOMPARE(shortList.scrollDuration(), 123);
        QCOMPARE(shortList.verticalSmoothScrollBar()->scrollAnimationDuration(), 123);

        FluentQt::CycleListWidget cycleList(
            QStringList{QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"),
                        QStringLiteral("4"), QStringLiteral("5"), QStringLiteral("6"), QStringLiteral("7"),
                        QStringLiteral("8"), QStringLiteral("9"), QStringLiteral("10"), QStringLiteral("11")},
            QSize(42, 18), Qt::AlignRight);
        QVERIFY(cycleList.isCycle());
        QCOMPARE(cycleList.count(), 24);
        QCOMPARE(cycleList.currentCycleIndex(), 12);
        QCOMPARE(cycleList.alignment(), Qt::AlignRight);

        QSignalSpy changedSpy(&cycleList, &FluentQt::CycleListWidget::cycleCurrentItemChanged);
        cycleList.scrollDown();
        QCOMPARE(cycleList.currentCycleIndex(), 13);
        QCOMPARE(changedSpy.count(), 1);
        QCOMPARE(cycleList.currentItem()->text(), QStringLiteral("1"));

        cycleList.setCurrentIndex(cycleList.count() - 1);
        QCOMPARE(cycleList.currentCycleIndex(), 11);
        QCOMPARE(cycleList.currentItem()->text(), QStringLiteral("11"));

        QTest::keyClick(&cycleList, Qt::Key_Up);
        QCOMPARE(cycleList.currentCycleIndex(), 10);
        QTest::keyClick(&cycleList, Qt::Key_Down);
        QCOMPARE(cycleList.currentCycleIndex(), 11);
    }

    void hyperlinkLabelAcceptsStringUrlAndUnderlineState()
    {
        FluentQt::HyperlinkLabel label(QStringLiteral("Open"));
        QCOMPARE(label.property("fqw").toString(), QStringLiteral("HyperlinkLabel"));
        QCOMPARE(label.text(), QStringLiteral("Open"));
        QCOMPARE(label.underlineVisible(), false);
        QCOMPARE(label.property("underline").toBool(), false);

        label.setUrl(QStringLiteral(FQW_REPOSITORY_URL));
        QCOMPARE(label.url(), QUrl(QStringLiteral(FQW_REPOSITORY_URL)));

        label.setUnderlineVisible(true);
        QCOMPARE(label.underlineVisible(), true);
        QCOMPARE(label.property("underline").toBool(), true);
        QVERIFY(label.font().underline());

        label.setUnderlineVisible(false);
        QCOMPARE(label.underlineVisible(), false);
        QCOMPARE(label.property("underline").toBool(), false);
        QVERIFY(!label.font().underline());
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
        QVERIFY(brush.isAvailable());

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

        flipView.resize(120, 80);
        QImage rendered(flipView.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        QPainter painter(&rendered);
        flipView.render(&painter);
        painter.end();
        QVERIFY(qAlpha(rendered.pixel(10, 10)) > 0);
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

    void editableComboBoxEditsAndTracksItems()
    {
        FluentQt::EditableComboBox combo;
        QCOMPARE(combo.property("fqw").toString(), QStringLiteral("EditableComboBox"));
        QVERIFY(qobject_cast<QLineEdit *>(&combo));
        QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("dropButton") >= 0);
        QVERIFY(!combo.isPressed());
        QVERIFY(!combo.isHover());
        QCOMPARE(combo.isClearButtonEnabled(), false);
        QCOMPARE(combo.textMargins().right(), 29);
        QVERIFY(combo.dropButton() != nullptr);
        QCOMPARE(combo.dropButton()->size(), QSize(30, 25));
        QCOMPARE(combo.dropMenu(), nullptr);

        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        QCOMPARE(combo.count(), 2);
        QCOMPARE(combo.currentIndex(), 0);
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));

        QSignalSpy indexSpy(&combo, &FluentQt::EditableComboBox::currentIndexChanged);
        QSignalSpy textSpy(&combo, &FluentQt::EditableComboBox::currentTextChanged);

        combo.setCurrentIndex(1);
        QCOMPARE(combo.currentIndex(), 1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));
        QVERIFY(indexSpy.count() >= 1);
        QVERIFY(textSpy.count() >= 1);

        combo.setText(QStringLiteral("Gamma"));
        QCOMPARE(combo.currentIndex(), -1);
        QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));

        QVERIFY(QMetaObject::invokeMethod(&combo, "returnPressed"));
        QCOMPARE(combo.count(), 3);
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));

        auto *completer = new QCompleter(QStringList{QStringLiteral("Alpha"), QStringLiteral("Beta")}, &combo);
        combo.setCompleter(completer);
        auto *menu = new FluentQt::CompleterMenu(&combo);
        combo.setCompleterMenu(menu);
        emit menu->activated(QStringLiteral("Beta"));
        QCOMPARE(combo.currentIndex(), 1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));
    }

    void comboBoxPopupUsesFluentIndicatorMenu()
    {
        FluentQt::ComboBox combo;
        QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("arrowAni") >= 0);
        QVERIFY(!combo.isPressed());
        QVERIFY(!combo.isHover());
        QVERIFY(combo.arrowAni() != nullptr);
        QVERIFY(combo.arrowAni()->metaObject()->indexOfProperty("y") >= 0);
        QCOMPARE(combo.arrowAni()->y(), 0.0);
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
        combo.setCurrentIndex(1);
        combo.resize(160, 32);
        combo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&combo));

        QTest::mousePress(&combo, Qt::LeftButton, Qt::NoModifier, combo.rect().center());
        QVERIFY(combo.isPressed());
        QVERIFY(combo.property("isPressed").toBool());
        QTest::mouseRelease(&combo, Qt::LeftButton, Qt::NoModifier, combo.rect().center());
        QVERIFY(!combo.isPressed());

        FluentQt::ComboBoxMenu *menu = nullptr;
        QTRY_VERIFY((menu = findVisibleComboMenu()) != nullptr);
        QCOMPARE(combo.dropMenu(), menu);
        auto *view = menu->view();

        QCOMPARE(menu->property("fqw").toString(), QStringLiteral("ComboBoxMenu"));
        QCOMPARE(view->objectName(), QStringLiteral("comboListWidget"));
        QCOMPARE(view->viewportPadding(), QMargins(0, 2, 0, 6));
        QCOMPARE(view->verticalScrollBarPolicy(), Qt::ScrollBarAsNeeded);
        QVERIFY(qobject_cast<FluentQt::IndicatorMenuItemDelegate *>(view->itemDelegate()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::ShortcutMenuItemDelegate *>(view->itemDelegate()) == nullptr);
        QCOMPARE(menu->itemHeight(), 33);
        QCOMPARE(view->currentRow(), 1);
        QVERIFY(!menu->menuActions().at(1)->isCheckable());
        QVERIFY(menu->width() >= combo.width());
        QVERIFY(view->width() >= combo.width());

        const int comboCenterX = combo.mapToGlobal(combo.rect().center()).x();
        QVERIFY(qAbs(menu->geometry().center().x() - comboCenterX) <= 2);

        QSignalSpy activatedSpy(&combo, &FluentQt::ComboBox::activated);
        QSignalSpy textActivatedSpy(&combo, &FluentQt::ComboBox::textActivated);
        QListWidgetItem *gammaItem = view->item(2);
        QVERIFY(gammaItem != nullptr);
        QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier,
                          view->visualItemRect(gammaItem).center());
        QTRY_VERIFY(combo.dropMenu() == nullptr);
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));
        QCOMPARE(combo.text(), QStringLiteral("Gamma"));
        QCOMPARE(activatedSpy.count(), 1);
        QCOMPARE(activatedSpy.takeFirst().at(0).toInt(), 2);
        QCOMPARE(textActivatedSpy.count(), 1);
        QCOMPARE(textActivatedSpy.takeFirst().at(0).toString(), QStringLiteral("Gamma"));

        FluentQt::EditableComboBox editableCombo;
        editableCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        editableCombo.resize(160, 32);
        editableCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&editableCombo));
        QTest::mouseClick(editableCombo.dropButton(), Qt::LeftButton);
        FluentQt::ComboBoxMenu *editableMenu = nullptr;
        QTRY_VERIFY((editableMenu = findVisibleComboMenu()) != nullptr);
        QCOMPARE(editableCombo.dropMenu(), editableMenu);
        editableMenu->close();
        QTRY_VERIFY(editableCombo.dropMenu() == nullptr);

        FluentQt::ModelComboBox modelCombo;
        modelCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        modelCombo.resize(160, 32);
        modelCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&modelCombo));
        QTest::mouseRelease(&modelCombo, Qt::LeftButton, Qt::NoModifier, modelCombo.rect().center());
        FluentQt::ComboBoxMenu *modelMenu = nullptr;
        QTRY_VERIFY((modelMenu = findVisibleComboMenu()) != nullptr);
        QCOMPARE(modelCombo.dropMenu(), modelMenu);
        modelMenu->close();
        QTRY_VERIFY(modelCombo.dropMenu() == nullptr);

        FluentQt::EditableModelComboBox editableModelCombo;
        editableModelCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        editableModelCombo.resize(160, 32);
        editableModelCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&editableModelCombo));
        QTest::mouseClick(editableModelCombo.dropButton(), Qt::LeftButton);
        FluentQt::ComboBoxMenu *editableModelMenu = nullptr;
        QTRY_VERIFY((editableModelMenu = findVisibleComboMenu()) != nullptr);
        QCOMPARE(editableModelCombo.dropMenu(), editableModelMenu);
        editableModelMenu->close();
        QTRY_VERIFY(editableModelCombo.dropMenu() == nullptr);

        FluentQt::AcrylicComboBox acrylicCombo;
        QCOMPARE(acrylicCombo.property("fqw").toString(), QStringLiteral("AcrylicComboBox"));
        acrylicCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        acrylicCombo.resize(160, 32);
        acrylicCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&acrylicCombo));
        QTest::mouseRelease(&acrylicCombo, Qt::LeftButton, Qt::NoModifier, acrylicCombo.rect().center());

        FluentQt::ComboBoxMenu *acrylicMenu = nullptr;
        QTRY_VERIFY((acrylicMenu = findVisibleComboMenu()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::AcrylicComboBoxMenu *>(acrylicMenu));
        QVERIFY(qobject_cast<FluentQt::AcrylicMenuActionListWidget *>(acrylicMenu->view()));
        QCOMPARE(acrylicMenu->view()->property("transparent").toBool(), true);
        acrylicMenu->close();
        QTRY_VERIFY(acrylicCombo.dropMenu() == nullptr);
    }

    void comboBoxArrowUsesPythonLightTint()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::ComboBox combo;
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        combo.resize(160, 32);
        combo.ensurePolished();

        QPixmap pixmap(combo.size());
        pixmap.fill(Qt::transparent);
        combo.render(&pixmap);

        const QImage image = pixmap.toImage();
        const QRect arrowRect(combo.width() - 22, combo.height() / 2 - 5, 10, 10);
        int sampled = 0;
        int graySum = 0;
        for (int y = arrowRect.top(); y <= arrowRect.bottom(); ++y) {
            for (int x = arrowRect.left(); x <= arrowRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                const int gray = qGray(color.rgb());
                if (color.alpha() > 0 && gray < 180) {
                    ++sampled;
                    graySum += gray;
                }
            }
        }

        QVERIFY(sampled > 0);
        QVERIFY(graySum / sampled > 40);
        QVERIFY(graySum / sampled < 130);
    }

    void comboBoxPlaceholderAndInsertMatchPythonBehavior()
    {
        FluentQt::ComboBox combo;
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        QCOMPARE(combo.currentIndex(), 0);
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));

        combo.setPlaceholderText(QStringLiteral("Choose one"));
        QCOMPARE(combo.placeholderText(), QStringLiteral("Choose one"));
        QCOMPARE(combo.isPlaceholderText(), true);
        QCOMPARE(combo.text(), QStringLiteral("Choose one"));
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));

        combo.setCurrentIndex(1);
        QCOMPARE(combo.isPlaceholderText(), false);
        QCOMPARE(combo.text(), QStringLiteral("Beta"));

        combo.insertItem(-10, QStringLiteral("First"));
        QCOMPARE(combo.itemText(0), QStringLiteral("First"));
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));

        combo.insertItems(99, {QStringLiteral("Last A"), QStringLiteral("Last B")});
        QCOMPARE(combo.itemText(combo.count() - 2), QStringLiteral("Last A"));
        QCOMPARE(combo.itemText(combo.count() - 1), QStringLiteral("Last B"));

        FluentQt::ComboBox firstRemovalCombo;
        firstRemovalCombo.addItems({QStringLiteral("One"), QStringLiteral("Two")});
        QCOMPARE(firstRemovalCombo.currentIndex(), 0);
        QCOMPARE(firstRemovalCombo.currentText(), QStringLiteral("One"));
        QSignalSpy removalTextSpy(&firstRemovalCombo, &FluentQt::ComboBox::currentTextChanged);
        QSignalSpy removalIndexSpy(&firstRemovalCombo, &FluentQt::ComboBox::currentIndexChanged);
        firstRemovalCombo.removeItem(0);
        QCOMPARE(firstRemovalCombo.count(), 1);
        QCOMPARE(firstRemovalCombo.currentIndex(), 0);
        QCOMPARE(firstRemovalCombo.currentText(), QStringLiteral("Two"));
        QCOMPARE(firstRemovalCombo.text(), QStringLiteral("Two"));
        QCOMPARE(removalTextSpy.count(), 1);
        QCOMPARE(removalTextSpy.takeFirst().at(0).toString(), QStringLiteral("Two"));
        QCOMPARE(removalIndexSpy.count(), 1);
        QCOMPARE(removalIndexSpy.takeFirst().at(0).toInt(), 0);
    }

    void dropDownButtonsUseFluentArrowIcons()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        auto averageArrowGray = [](QWidget &widget, int minimumGray, int maximumGray) {
            widget.resize(160, 32);
            widget.ensurePolished();

            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);

            const QImage image = pixmap.toImage();
            const QRect arrowRect(widget.width() - 22, widget.height() / 2 - 5, 10, 10);
            int sampled = 0;
            int graySum = 0;
            for (int y = arrowRect.top(); y <= arrowRect.bottom(); ++y) {
                for (int x = arrowRect.left(); x <= arrowRect.right(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    const int gray = qGray(color.rgb());
                    if (color.alpha() > 0 && gray >= minimumGray && gray <= maximumGray) {
                        ++sampled;
                        graySum += gray;
                    }
                }
            }

            return sampled > 0 ? graySum / sampled : -1;
        };

        FluentQt::DropDownPushButton button(QStringLiteral("Email"));
        const int normalGray = averageArrowGray(button, 40, 180);
        QVERIFY(normalGray > 40);
        QVERIFY(normalGray < 140);

        FluentQt::PrimaryDropDownPushButton primaryButton(QStringLiteral("Email"));
        const int primaryGray = averageArrowGray(primaryButton, 220, 255);
        QVERIFY(primaryGray > 170);

        auto verifyArrowAnimation = [](QWidget *widget) {
            widget->resize(160, 32);
            widget->show();
            QVERIFY(QTest::qWaitForWindowExposed(widget));
            QCOMPARE(widget->property("arrowOffset").toInt(), 0);

            QTest::mousePress(widget, Qt::LeftButton, Qt::NoModifier, widget->rect().center());
            QTRY_COMPARE(widget->property("arrowOffset").toInt(), 2);

            QTest::mouseRelease(widget, Qt::LeftButton, Qt::NoModifier, widget->rect().center());
            QTRY_COMPARE(widget->property("arrowOffset").toInt(), 0);
            widget->hide();
        };

        FluentQt::DropDownPushButton animatedPush(QStringLiteral("Email"));
        verifyArrowAnimation(&animatedPush);

        FluentQt::PrimaryDropDownPushButton animatedPrimaryPush(QStringLiteral("Email"));
        verifyArrowAnimation(&animatedPrimaryPush);

        FluentQt::DropDownToolButton animatedTool(FluentQt::icon(FluentQt::FluentIcon::Heart));
        verifyArrowAnimation(&animatedTool);

        FluentQt::PrimaryDropDownToolButton animatedPrimaryTool(FluentQt::icon(FluentQt::FluentIcon::Heart));
        verifyArrowAnimation(&animatedPrimaryTool);
    }

    void dropDownButtonPopupUsesRoundMenuSizing()
    {
        FluentQt::DropDownPushButton button(QStringLiteral("Actions"));
        button.resize(180, 32);

        FluentQt::RoundMenu menu(&button);
        QAction *copyAction = menu.addAction(QStringLiteral("Copy"));
        menu.addAction(QStringLiteral("Paste"));
        button.setMenu(&menu);
        QCOMPARE(button.menu(), &menu);
        QCOMPARE(button.dropDownMenu(), &menu);
        QSignalSpy copySpy(copyAction, &QAction::triggered);

        button.show();
        QVERIFY(QTest::qWaitForWindowExposed(&button));

        QTest::mouseClick(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());

        FluentQt::RoundMenu *visibleMenu = nullptr;
        QTRY_VERIFY((visibleMenu = findVisibleRoundMenu()) != nullptr);
        QCOMPARE(visibleMenu, &menu);
        QVERIFY(menu.view()->minimumWidth() >= button.width());
        QVERIFY(menu.animationType() == FluentQt::MenuAnimationType::DropDown ||
                menu.animationType() == FluentQt::MenuAnimationType::PullUp);

        QListWidgetItem *copyItem = menu.view()->item(0);
        QVERIFY(copyItem != nullptr);
        QTest::mouseClick(menu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          menu.view()->visualItemRect(copyItem).center());
        QTRY_VERIFY(!menu.isVisible());
        QCOMPARE(copySpy.count(), 1);

        FluentQt::PrimaryDropDownPushButton primaryButton(QStringLiteral("Actions"));
        primaryButton.setMenu(&menu);
        QCOMPARE(primaryButton.menu(), &menu);

        FluentQt::DropDownToolButton toolButton(FluentQt::icon(FluentQt::FluentIcon::Heart));
        toolButton.setMenu(&menu);
        QCOMPARE(toolButton.menu(), &menu);

        FluentQt::PrimaryDropDownToolButton primaryToolButton(FluentQt::icon(FluentQt::FluentIcon::Heart));
        primaryToolButton.setMenu(&menu);
        QCOMPARE(primaryToolButton.menu(), &menu);
    }

    void splitButtonsExposePrimaryVariantsAndDropRoles()
    {
        auto averageSplitArrowGray = [](QToolButton *button, int minimumGray, int maximumGray) {
            button->ensurePolished();

            QPixmap pixmap(button->size());
            pixmap.fill(Qt::transparent);
            button->render(&pixmap);

            const QImage image = pixmap.toImage();
            const QRect arrowRect((image.width() - 10) / 2, image.height() / 2 - 5, 10, 10);
            int sampled = 0;
            int graySum = 0;
            for (int y = arrowRect.top(); y <= arrowRect.bottom(); ++y) {
                for (int x = arrowRect.left(); x <= arrowRect.right(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    const int gray = qGray(color.rgb());
                    if (color.alpha() > 0 && gray >= minimumGray && gray <= maximumGray) {
                        ++sampled;
                        graySum += gray;
                    }
                }
            }
            return sampled > 0 ? graySum / sampled : -1;
        };

        auto solidIcon = [](const QColor &color) {
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(QRectF(2, 2, 12, 12), 2, 2);
            painter.end();
            return QIcon(pixmap);
        };

        auto countPixelsNear = [](QToolButton *button, const QColor &expected) {
            button->ensurePolished();

            QPixmap pixmap(button->size());
            pixmap.fill(Qt::transparent);
            button->render(&pixmap);

            const QImage image = pixmap.toImage();
            int matched = 0;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    if (color.alpha() <= 0) {
                        continue;
                    }

                    const int expectedMax = std::max({expected.red(), expected.green(), expected.blue()});
                    const bool redDominant = expected.red() == expectedMax && color.red() > color.green() + 35 &&
                                             color.red() > color.blue() + 35;
                    const bool greenDominant = expected.green() == expectedMax && color.green() > color.red() + 35 &&
                                               color.green() > color.blue() + 35;
                    const bool blueDominant = expected.blue() == expectedMax && color.blue() > color.red() + 35 &&
                                              color.blue() > color.green() + 35;
                    if (redDominant || greenDominant || blueDominant) {
                        ++matched;
                    }
                }
            }
            return matched;
        };

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::SplitDropButton plainDrop;
        QCOMPARE(plainDrop.property("fqw").toString(), QStringLiteral("SplitDropButton"));
        QCOMPARE(plainDrop.minimumSize(), QSize(32, 32));
        QCOMPARE(plainDrop.maximumWidth(), 32);
        QVERIFY(plainDrop.maximumHeight() > plainDrop.minimumHeight());
        QCOMPARE(plainDrop.sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
        QCOMPARE(plainDrop.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
        QCOMPARE(plainDrop.iconSize(), QSize(10, 10));
        QCOMPARE(plainDrop.arrowOffset(), 0.0);
        QVERIFY(!plainDrop.hasCustomDropIcon());

        FluentQt::PrimarySplitDropButton primaryDrop;
        QCOMPARE(primaryDrop.property("fqw").toString(), QStringLiteral("PrimarySplitDropButton"));
        QCOMPARE(primaryDrop.minimumSize(), QSize(32, 32));
        QCOMPARE(primaryDrop.maximumWidth(), 32);
        QVERIFY(primaryDrop.maximumHeight() > primaryDrop.minimumHeight());
        QCOMPARE(primaryDrop.sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
        QCOMPARE(primaryDrop.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
        QVERIFY(qobject_cast<FluentQt::SplitDropButton *>(&primaryDrop) != nullptr);

        FluentQt::SplitWidgetBase splitBase;
        auto *replacementDrop = new QToolButton;
        splitBase.setDropButton(replacementDrop);
        QCOMPARE(splitBase.dropButton(), replacementDrop);
        splitBase.setDropIcon(solidIcon(QColor(1, 120, 200)));
        splitBase.setDropIconSize(QSize(9, 9));
        QCOMPARE(replacementDrop->iconSize(), QSize(9, 9));
        QSignalSpy baseDropSpy(&splitBase, &FluentQt::SplitWidgetBase::dropDownClicked);
        QTest::mouseClick(replacementDrop, Qt::LeftButton);
        QCOMPARE(baseDropSpy.count(), 1);
        auto *firstContent = new QLabel(QStringLiteral("First"));
        auto *secondContent = new QLabel(QStringLiteral("Second"));
        QPointer<QWidget> firstContentGuard(firstContent);
        splitBase.setWidget(firstContent);
        splitBase.setWidget(secondContent);
        QCOMPARE(splitBase.contentWidget(), secondContent);
        QVERIFY(firstContentGuard);
        QVERIFY(firstContent->isHidden());
        QCOMPARE(firstContent->parentWidget(), &splitBase);

        FluentQt::SplitPushButton splitPush(QStringLiteral("Normal"));
        QVERIFY(qobject_cast<FluentQt::SplitWidgetBase *>(&splitPush) != nullptr);
        QVERIFY(splitPush.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(splitPush.metaObject()->indexOfProperty("iconSize") >= 0);
        QCOMPARE(splitPush.button()->objectName(), QStringLiteral("splitPushButton"));
        QCOMPARE(splitPush.contentWidget(), splitPush.button());
        QCOMPARE(splitPush.dropButton()->property("fqw").toString(), QStringLiteral("SplitDropButton"));
        splitPush.ensurePolished();
        splitPush.adjustSize();
        QCOMPARE(splitPush.dropButton()->height(), qMax(32, splitPush.button()->sizeHint().height()));
        QCOMPARE(splitPush.dropButton()->height(), splitPush.button()->height());
        QCOMPARE(splitPush.dropButton()->width(), 32);
        splitPush.setIcon(solidIcon(QColor(16, 32, 48)));
        splitPush.setIconSize(QSize(18, 18));
        QCOMPARE(splitPush.iconSize(), QSize(18, 18));
        QCOMPARE(splitPush.button()->iconSize(), QSize(18, 18));
        QVERIFY(!splitPush.icon().isNull());
        const int normalGray = averageSplitArrowGray(splitPush.dropButton(), 40, 180);
        QVERIFY(normalGray > 40);
        QVERIFY(normalGray <= 180);
        splitPush.setDropIcon(solidIcon(QColor(220, 20, 60)));
        splitPush.setDropIconSize(QSize(14, 14));
        QCOMPARE(splitPush.dropButton()->iconSize(), QSize(14, 14));
        auto *splitDropButton = qobject_cast<FluentQt::SplitDropButton *>(splitPush.dropButton());
        QVERIFY(splitDropButton != nullptr);
        QVERIFY(splitDropButton->hasCustomDropIcon());
        QVERIFY(countPixelsNear(splitPush.dropButton(), QColor(220, 20, 60)) > 40);

        FluentQt::PrimarySplitPushButton primaryPush(QStringLiteral("Primary"));
        QVERIFY(qobject_cast<FluentQt::SplitWidgetBase *>(&primaryPush) != nullptr);
        QVERIFY(primaryPush.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(primaryPush.metaObject()->indexOfProperty("iconSize") >= 0);
        QVERIFY(primaryPush.button() != nullptr);
        QCOMPARE(primaryPush.button()->objectName(), QStringLiteral("primarySplitPushButton"));
        QCOMPARE(primaryPush.button()->property("fqw").toString(), QStringLiteral("PrimaryPushButton"));
        QCOMPARE(primaryPush.dropButton()->property("fqw").toString(), QStringLiteral("PrimarySplitDropButton"));
        primaryPush.ensurePolished();
        primaryPush.adjustSize();
        QCOMPARE(primaryPush.dropButton()->height(), qMax(32, primaryPush.button()->sizeHint().height()));
        QCOMPARE(primaryPush.dropButton()->height(), primaryPush.button()->height());
        QCOMPARE(primaryPush.dropButton()->width(), 32);
        primaryPush.setIcon(solidIcon(QColor(20, 80, 160)));
        primaryPush.setIconSize(QSize(19, 19));
        QCOMPARE(primaryPush.iconSize(), QSize(19, 19));
        QCOMPARE(primaryPush.button()->iconSize(), QSize(19, 19));
        QVERIFY(!primaryPush.icon().isNull());
        const int primaryGray = averageSplitArrowGray(primaryPush.dropButton(), 220, 255);
        QVERIFY(primaryGray > 170);
        QVERIFY(primaryGray > normalGray + 40);
        primaryPush.setDropIcon(solidIcon(QColor(30, 144, 255)));
        primaryPush.setDropIconSize(QSize(12, 12));
        QCOMPARE(primaryPush.dropButton()->iconSize(), QSize(12, 12));
        QVERIFY(countPixelsNear(primaryPush.dropButton(), QColor(30, 144, 255)) > 25);

        FluentQt::PrimarySplitToolButton primaryTool(FluentQt::icon(FluentQt::FluentIcon::Heart));
        QVERIFY(primaryTool.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(primaryTool.metaObject()->indexOfProperty("iconSize") >= 0);
        QVERIFY(primaryTool.button() != nullptr);
        QCOMPARE(primaryTool.button()->objectName(), QStringLiteral("primarySplitToolButton"));
        QCOMPARE(primaryTool.button()->property("fqw").toString(), QStringLiteral("PrimaryToolButton"));
        QCOMPARE(primaryTool.dropButton()->property("fqw").toString(), QStringLiteral("PrimarySplitDropButton"));
        primaryTool.ensurePolished();
        primaryTool.adjustSize();
        QCOMPARE(primaryTool.dropButton()->height(), qMax(32, primaryTool.button()->sizeHint().height()));
        QCOMPARE(primaryTool.dropButton()->height(), primaryTool.button()->height());
        QCOMPARE(primaryTool.dropButton()->width(), 32);
        primaryTool.setIconSize(QSize(17, 17));
        QCOMPARE(primaryTool.iconSize(), QSize(17, 17));
        QCOMPARE(primaryTool.button()->iconSize(), QSize(17, 17));
        QVERIFY(!primaryTool.icon().isNull());
        primaryTool.setDropIcon(solidIcon(QColor(80, 200, 120)));
        primaryTool.setDropIconSize(QSize(11, 11));
        QCOMPARE(primaryTool.dropButton()->iconSize(), QSize(11, 11));

        FluentQt::RoundMenu splitPushMenu(&splitPush);
        QAction *splitAction = splitPushMenu.addAction(QStringLiteral("Crazy Diamond"));
        QObject::connect(splitAction, &QAction::triggered, &splitPush,
                         [&splitPush]() { splitPush.setText(QStringLiteral("Crazy Diamond")); });
        splitPush.setFlyout(&splitPushMenu);
        splitPush.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitPush));
        QTest::mouseClick(splitPush.dropButton(), Qt::LeftButton);
        FluentQt::RoundMenu *visibleSplitMenu = nullptr;
        QTRY_VERIFY((visibleSplitMenu = findVisibleRoundMenu()) != nullptr);
        QCOMPARE(visibleSplitMenu, &splitPushMenu);
        QCOMPARE(splitPushMenu.view()->minimumWidth(), splitPush.width());
        QCOMPARE(splitPushMenu.animationType(), FluentQt::MenuAnimationType::DropDown);
        QListWidgetItem *splitItem = splitPushMenu.view()->item(0);
        QVERIFY(splitItem != nullptr);
        QTest::mouseClick(splitPushMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          splitPushMenu.view()->visualItemRect(splitItem).center());
        QTRY_VERIFY(!splitPushMenu.isVisible());
        QCOMPARE(splitPush.text(), QStringLiteral("Crazy Diamond"));
        bool splitMenuHiddenBeforeTrigger = false;
        FluentQt::RoundMenu timingMenu(&splitPush);
        QAction *timingAction = timingMenu.addAction(QStringLiteral("Timing"));
        QObject::connect(timingAction, &QAction::triggered, &timingMenu,
                         [&timingMenu, &splitMenuHiddenBeforeTrigger]() {
                             splitMenuHiddenBeforeTrigger = !timingMenu.isVisible();
                         });
        timingMenu.exec(splitPush.mapToGlobal(QPoint(0, splitPush.height())), true,
                        FluentQt::MenuAnimationType::DropDown);
        QTRY_VERIFY(timingMenu.isVisible());
        QListWidgetItem *timingItem = timingMenu.view()->item(0);
        QVERIFY(timingItem != nullptr);
        QTest::mouseClick(timingMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          timingMenu.view()->visualItemRect(timingItem).center());
        QTRY_VERIFY(!timingMenu.isVisible());
        QVERIFY(splitMenuHiddenBeforeTrigger);

        FluentQt::RoundMenu primarySplitPushMenu(&primaryPush);
        QAction *primarySplitAction = primarySplitPushMenu.addAction(QStringLiteral("Gold Experience"));
        QObject::connect(primarySplitAction, &QAction::triggered, &primaryPush,
                         [&primaryPush]() { primaryPush.setText(QStringLiteral("Gold Experience")); });
        primaryPush.setFlyout(&primarySplitPushMenu);
        primaryPush.show();
        QVERIFY(QTest::qWaitForWindowExposed(&primaryPush));
        QSignalSpy dropSpy(&primaryPush, &FluentQt::PrimarySplitPushButton::dropDownClicked);
        QTest::mouseClick(primaryPush.dropButton(), Qt::LeftButton);
        QCOMPARE(dropSpy.count(), 1);
        FluentQt::RoundMenu *visiblePrimarySplitMenu = nullptr;
        QTRY_VERIFY((visiblePrimarySplitMenu = findVisibleRoundMenu()) != nullptr);
        QCOMPARE(visiblePrimarySplitMenu, &primarySplitPushMenu);
        QCOMPARE(primarySplitPushMenu.view()->minimumWidth(), primaryPush.width());
        QCOMPARE(primarySplitPushMenu.animationType(), FluentQt::MenuAnimationType::DropDown);
        QListWidgetItem *primarySplitItem = primarySplitPushMenu.view()->item(0);
        QVERIFY(primarySplitItem != nullptr);
        QTest::mouseClick(primarySplitPushMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          primarySplitPushMenu.view()->visualItemRect(primarySplitItem).center());
        QTRY_VERIFY(!primarySplitPushMenu.isVisible());
        QCOMPARE(primaryPush.text(), QStringLiteral("Gold Experience"));

        FluentQt::RoundMenu flyout(&primaryPush);
        flyout.addAction(QStringLiteral("Copy"));
        primaryPush.setFlyout(&flyout);
        QCOMPARE(primaryPush.flyout(), &flyout);
        primaryPush.show();
        QVERIFY(QTest::qWaitForWindowExposed(&primaryPush));
        primaryPush.showFlyout();
        FluentQt::RoundMenu *visibleMenu = nullptr;
        QTRY_VERIFY((visibleMenu = findVisibleRoundMenu()) != nullptr);
        QCOMPARE(visibleMenu, &flyout);
        flyout.close();

        FluentQt::SplitPushButton splitAlias(QStringLiteral("Alias"));
        splitAlias.setFlyout(&flyout);
        QCOMPARE(splitAlias.flyout(), &flyout);

        FluentQt::SplitToolButton splitToolAlias(FluentQt::icon(FluentQt::FluentIcon::Heart));
        QVERIFY(splitToolAlias.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(splitToolAlias.metaObject()->indexOfProperty("iconSize") >= 0);
        QCOMPARE(splitToolAlias.button()->objectName(), QStringLiteral("splitToolButton"));
        QCOMPARE(splitToolAlias.dropButton()->property("fqw").toString(), QStringLiteral("SplitDropButton"));
        splitToolAlias.ensurePolished();
        splitToolAlias.adjustSize();
        QCOMPARE(splitToolAlias.dropButton()->height(), splitToolAlias.button()->height());
        QCOMPARE(splitToolAlias.dropButton()->width(), 32);
        splitToolAlias.setIconSize(QSize(15, 15));
        QCOMPARE(splitToolAlias.iconSize(), QSize(15, 15));
        QCOMPARE(splitToolAlias.button()->iconSize(), QSize(15, 15));
        QVERIFY(!splitToolAlias.icon().isNull());
        splitToolAlias.setDropIcon(solidIcon(QColor(255, 165, 0)));
        splitToolAlias.setDropIconSize(QSize(13, 13));
        QCOMPARE(splitToolAlias.dropButton()->iconSize(), QSize(13, 13));
        splitToolAlias.setFlyout(&flyout);
        QCOMPARE(splitToolAlias.flyout(), &flyout);

        FluentQt::PrimarySplitToolButton primaryToolAlias(FluentQt::icon(FluentQt::FluentIcon::Heart));
        primaryToolAlias.setFlyout(&flyout);
        QCOMPARE(primaryToolAlias.flyout(), &flyout);

        FluentQt::RoundMenu splitToolMenu(&splitToolAlias);
        QAction *toolAction = splitToolMenu.addAction(QStringLiteral("Jump"));
        QSignalSpy toolActionSpy(toolAction, &QAction::triggered);
        splitToolAlias.setFlyout(&splitToolMenu);
        splitToolAlias.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitToolAlias));
        QTest::mouseClick(splitToolAlias.dropButton(), Qt::LeftButton);
        FluentQt::RoundMenu *visibleToolMenu = nullptr;
        QTRY_VERIFY((visibleToolMenu = findVisibleRoundMenu()) != nullptr);
        QCOMPARE(visibleToolMenu, &splitToolMenu);
        QCOMPARE(splitToolMenu.view()->minimumWidth(), splitToolAlias.width());
        QCOMPARE(splitToolMenu.animationType(), FluentQt::MenuAnimationType::DropDown);
        QListWidgetItem *toolItem = splitToolMenu.view()->item(0);
        QVERIFY(toolItem != nullptr);
        QTest::mouseClick(splitToolMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          splitToolMenu.view()->visualItemRect(toolItem).center());
        QTRY_VERIFY(!splitToolMenu.isVisible());
        QCOMPARE(toolActionSpy.count(), 1);
    }

    void checkBoxSelfPaintsFluentIndicatorAndText()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        auto render = [](QWidget &widget) {
            widget.ensurePolished();
            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);
            return pixmap.toImage();
        };

        FluentQt::CheckBox checkBox(QStringLiteral("Fluent CheckBox"));
        QVERIFY(checkBox.metaObject()->indexOfProperty("lightCheckedColor") >= 0);
        QVERIFY(checkBox.metaObject()->indexOfProperty("darkCheckedColor") >= 0);
        QVERIFY(checkBox.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(checkBox.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(!checkBox.isPressed());
        QVERIFY(!checkBox.isHover());
        checkBox.setProperty("lightCheckedColor", QColor(QStringLiteral("#cc1122")));
        checkBox.setProperty("darkCheckedColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(checkBox.lightCheckedColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(checkBox.darkCheckedColor(), QColor(QStringLiteral("#22cc11")));
        checkBox.resize(160, 24);
        checkBox.setChecked(true);

        QImage image = render(checkBox);
        const QRect indicatorRect(1, 3, 18, 18);
        int accentPixels = 0;
        int whiteIconPixels = 0;
        int customCheckedPixels = 0;
        for (int y = indicatorRect.top(); y <= indicatorRect.bottom(); ++y) {
            for (int x = indicatorRect.left(); x <= indicatorRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() == 0) {
                    continue;
                }
                if (color.blue() > color.red() + 20 && color.blue() > color.green()) {
                    ++accentPixels;
                }
                if (color.red() > 220 && color.green() > 220 && color.blue() > 220) {
                    ++whiteIconPixels;
                }
                if (color.red() > 120 && color.green() < 80 && color.blue() < 90) {
                    ++customCheckedPixels;
                }
            }
        }
        QCOMPARE(accentPixels, 0);
        QVERIFY(customCheckedPixels > 80);
        QVERIFY(whiteIconPixels > 0);

        checkBox.setTristate(true);
        checkBox.setCheckState(Qt::PartiallyChecked);
        image = render(checkBox);
        whiteIconPixels = 0;
        for (int y = indicatorRect.top(); y <= indicatorRect.bottom(); ++y) {
            for (int x = indicatorRect.left(); x <= indicatorRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.red() > 220 && color.green() > 220 && color.blue() > 220) {
                    ++whiteIconPixels;
                }
            }
        }
        QVERIFY(whiteIconPixels > 0);

        checkBox.setCheckState(Qt::Unchecked);
        checkBox.setTextColor(QColor(QStringLiteral("#cc1122")), QColor(QStringLiteral("#22cc11")));
        image = render(checkBox);
        const QRect textRect(28, 0, image.width() - 28, image.height());
        int customTextPixels = 0;
        for (int y = textRect.top(); y <= textRect.bottom(); ++y) {
            for (int x = textRect.left(); x <= textRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() > 0 && color.red() > 120 && color.green() < 80 && color.blue() < 90) {
                    ++customTextPixels;
                }
            }
        }
        QVERIFY(customTextPixels > 0);
    }

    void radioButtonSelfPaintsFluentIndicatorAndText()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        auto render = [](QWidget &widget) {
            widget.ensurePolished();
            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);
            return pixmap.toImage();
        };

        FluentQt::RadioButton radio(QStringLiteral("Fluent RadioButton"));
        QVERIFY(radio.metaObject()->indexOfProperty("lightIndicatorColor") >= 0);
        QVERIFY(radio.metaObject()->indexOfProperty("darkIndicatorColor") >= 0);
        QVERIFY(radio.metaObject()->indexOfProperty("indicatorPos") >= 0);
        QVERIFY(radio.metaObject()->indexOfProperty("isHover") >= 0);
        QCOMPARE(radio.indicatorPos(), QPoint(11, 12));
        QVERIFY(!radio.isHover());
        radio.setIndicatorPos(QPoint(12, 12));
        QCOMPARE(radio.indicatorPos(), QPoint(12, 12));
        radio.resize(180, 24);
        radio.setChecked(true);
        radio.setProperty("lightIndicatorColor", QColor(QStringLiteral("#cc1122")));
        radio.setProperty("darkIndicatorColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(radio.lightIndicatorColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(radio.darkIndicatorColor(), QColor(QStringLiteral("#22cc11")));
        radio.setTextColor(QColor(QStringLiteral("#3355cc")), QColor(QStringLiteral("#22cc11")));

        const QImage image = render(radio);
        const QRect indicatorRect(1, 1, 21, 22);
        int customIndicatorPixels = 0;
        int customTextPixels = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() == 0) {
                    continue;
                }
                if (indicatorRect.contains(x, y) && color.red() > 120 && color.green() < 80 && color.blue() < 90) {
                    ++customIndicatorPixels;
                }
                if (x >= 29 && color.blue() > 120 && color.red() < 90 && color.green() < 110) {
                    ++customTextPixels;
                }
            }
        }

        QVERIFY(customIndicatorPixels > 40);
        QVERIFY(customTextPixels > 0);
    }

    void lineEditButtonKeepsActionIconsInFluentPaintPath()
    {
        auto filledIcon = [](const QColor &color) {
            QPixmap pixmap(16, 16);
            pixmap.fill(color);
            return QIcon(pixmap);
        };

        auto countPixels = [](QWidget &widget, const std::function<bool(const QColor &)> &predicate) {
            widget.ensurePolished();
            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);

            const QImage image = pixmap.toImage();
            int count = 0;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    if (color.alpha() > 0 && predicate(color)) {
                        ++count;
                    }
                }
            }
            return count;
        };

        FluentQt::LineEditButton button(filledIcon(Qt::red));
        button.resize(31, 23);

        QAction action(filledIcon(Qt::red), QStringLiteral("Action"), &button);
        button.setAction(&action);
        QVERIFY(button.icon().isNull());
        QVERIFY(countPixels(button, [](const QColor &color) {
                    return color.red() > 200 && color.green() < 80 && color.blue() < 80;
                }) > 80);

        action.setIcon(filledIcon(Qt::green));
        QVERIFY(button.icon().isNull());
        QVERIFY(countPixels(button, [](const QColor &color) {
                    return color.green() > 120 && color.red() < 80 && color.blue() < 80;
                }) > 80);

        button.show();
        QVERIFY(QTest::qWaitForWindowExposed(&button));
        QVERIFY(!button.isPressed());
        QTest::mousePress(&button, Qt::LeftButton);
        QVERIFY(button.isPressed());
        QVERIFY(button.property("isPressed").toBool());
        QTest::mouseRelease(&button, Qt::LeftButton);
        QVERIFY(!button.isPressed());
    }

    void lineEditExposesPythonStylePropertiesAndActionHelpers()
    {
        FluentQt::LineEdit editor;
        QCOMPARE(editor.property("fqw").toString(), QStringLiteral("LineEdit"));
        QVERIFY(editor.clearButton());
        QVERIFY(editor.hBoxLayout());
        QCOMPARE(editor.leftButtons().size(), 0);
        QCOMPARE(editor.rightButtons().size(), 0);
        QCOMPARE(editor.completerMenu(), nullptr);
        QVERIFY(editor.metaObject()->indexOfProperty("clearButton") >= 0);
        QVERIFY(editor.metaObject()->indexOfProperty("hBoxLayout") >= 0);
        QVERIFY(editor.metaObject()->indexOfProperty("completerMenu") >= 0);
        QVERIFY(editor.metaObject()->indexOfProperty("lightFocusedBorderColor") >= 0);
        QVERIFY(editor.metaObject()->indexOfProperty("darkFocusedBorderColor") >= 0);
        QCOMPARE(editor.property("clearButton").value<FluentQt::LineEditButton *>(), editor.clearButton());
        QCOMPARE(editor.property("hBoxLayout").value<QHBoxLayout *>(), editor.hBoxLayout());

        editor.setProperty("lightFocusedBorderColor", QColor(Qt::red));
        editor.setDarkFocusedBorderColor(QColor(Qt::blue));
        QCOMPARE(editor.lightFocusedBorderColor(), QColor(Qt::red));
        QCOMPARE(editor.darkFocusedBorderColor(), QColor(Qt::blue));

        QAction first(FluentQt::icon(FluentQt::FluentIcon::Add), QStringLiteral("First"), &editor);
        QAction second(FluentQt::icon(FluentQt::FluentIcon::Accept), QStringLiteral("Second"), &editor);
        editor.addActions({&first, &second}, QLineEdit::LeadingPosition);
        QCOMPARE(editor.textMargins().left(), 60);
        QCOMPARE(editor.leftButtons().size(), 2);
        QCOMPARE(editor.rightButtons().size(), 0);
        QVERIFY(editor.leftButtons().at(0)->metaObject()->indexOfProperty("action") >= 0);
        QCOMPARE(editor.leftButtons().at(0)->action(), &first);
        QCOMPARE(editor.leftButtons().at(0)->property("action").value<QAction *>(), &first);

        auto *completer = new QCompleter(QStringList{QStringLiteral("Alpha")}, &editor);
        auto *completerMenu = new FluentQt::CompleterMenu(&editor);
        editor.setCompleter(completer);
        editor.setCompleterMenu(completerMenu);
        QCOMPARE(editor.completerMenu(), completerMenu);
        QCOMPARE(editor.property("completerMenu").value<FluentQt::CompleterMenu *>(), completerMenu);

        FluentQt::SearchLineEdit searchEditor;
        QVERIFY(searchEditor.searchButton());
        QCOMPARE(searchEditor.textMargins().right(), 59);
        searchEditor.setClearButtonEnabled(true);
        QCOMPARE(searchEditor.textMargins().right(), 58);

        FluentQt::AcrylicLineEdit acrylicEditor;
        QCOMPARE(acrylicEditor.property("fqw").toString(), QStringLiteral("AcrylicLineEdit"));
        auto *acrylicCompleter = new QCompleter(QStringList{QStringLiteral("Alpha")}, &acrylicEditor);
        auto *acrylicCompleterMenu = new FluentQt::AcrylicCompleterMenu(&acrylicEditor);
        acrylicEditor.setCompleter(acrylicCompleter);
        acrylicEditor.setCompleterMenu(acrylicCompleterMenu);
        QCOMPARE(acrylicEditor.completerMenu(), acrylicCompleterMenu);
        QCOMPARE(acrylicCompleterMenu->view()->property("transparent").toBool(), true);

        FluentQt::AcrylicSearchLineEdit acrylicSearchEditor;
        QCOMPARE(acrylicSearchEditor.property("fqw").toString(), QStringLiteral("AcrylicSearchLineEdit"));

        FluentQt::PasswordLineEdit passwordEditor;
        QVERIFY(passwordEditor.viewButton());
        QVERIFY(passwordEditor.isViewPasswordButtonVisible());
        QVERIFY(passwordEditor.property("viewPasswordButtonVisible").toBool());
        passwordEditor.setProperty("viewPasswordButtonVisible", false);
        QVERIFY(!passwordEditor.isViewPasswordButtonVisible());
        QCOMPARE(passwordEditor.textMargins().right(), 0);
    }

    void editContextMenusExposePythonStyleActions()
    {
        auto actionTexts = [](FluentQt::RoundMenu &menu) {
            QStringList texts;
            for (QAction *action : menu.menuActions()) {
                texts.append(action->text());
            }
            return texts;
        };

        QApplication::clipboard()->setText(QStringLiteral("clip"));
        FluentQt::LineEdit emptyLineEdit;
        FluentQt::LineEditMenu pasteOnlyMenu(&emptyLineEdit);
        pasteOnlyMenu.createActions();
        QCOMPARE(pasteOnlyMenu.editor(), &emptyLineEdit);
        QCOMPARE(actionTexts(pasteOnlyMenu), QStringList{QStringLiteral("Paste")});

        FluentQt::AcrylicLineEditMenu acrylicPasteOnlyMenu(&emptyLineEdit);
        acrylicPasteOnlyMenu.createActions();
        QCOMPARE(acrylicPasteOnlyMenu.editor(), &emptyLineEdit);
        QCOMPARE(actionTexts(acrylicPasteOnlyMenu), QStringList{QStringLiteral("Paste")});
        QCOMPARE(acrylicPasteOnlyMenu.view()->property("transparent").toBool(), true);

        QApplication::clipboard()->clear();
        FluentQt::LineEdit selectedLineEdit(QStringLiteral("abcdef"));
        selectedLineEdit.setSelection(1, 3);
        FluentQt::LineEditMenu selectedMenu(&selectedLineEdit);
        selectedMenu.createActions();
        QCOMPARE(actionTexts(selectedMenu),
                 QStringList({QStringLiteral("Cut"), QStringLiteral("Copy"), QStringLiteral("Cancel"),
                              QStringLiteral("Select all")}));

        FluentQt::LineEdit readOnlyLineEdit(QStringLiteral("readonly"));
        readOnlyLineEdit.setReadOnly(true);
        readOnlyLineEdit.setSelection(0, 4);
        FluentQt::LineEditMenu readOnlyMenu(&readOnlyLineEdit);
        readOnlyMenu.createActions();
        QCOMPARE(actionTexts(readOnlyMenu), QStringList({QStringLiteral("Copy"), QStringLiteral("Select all")}));

        FluentQt::TextEdit textEdit;
        textEdit.setPlainText(QStringLiteral("hello world"));
        QTextCursor cursor = textEdit.textCursor();
        cursor.setPosition(0);
        cursor.setPosition(5, QTextCursor::KeepAnchor);
        textEdit.setTextCursor(cursor);
        FluentQt::TextEditMenu textMenu(&textEdit);
        textMenu.createActions();
        QCOMPARE(textMenu.editor(), &textEdit);
        QCOMPARE(actionTexts(textMenu),
                 QStringList({QStringLiteral("Cut"), QStringLiteral("Copy"), QStringLiteral("Cancel"),
                              QStringLiteral("Select all")}));

        FluentQt::PlainTextEdit plainTextEdit;
        plainTextEdit.setPlainText(QStringLiteral("plain"));
        plainTextEdit.selectAll();
        FluentQt::TextEditMenu plainTextMenu(&plainTextEdit);
        plainTextMenu.createActions();
        QCOMPARE(plainTextMenu.editor(), &plainTextEdit);
        QCOMPARE(actionTexts(plainTextMenu),
                 QStringList({QStringLiteral("Cut"), QStringLiteral("Copy"), QStringLiteral("Cancel"),
                              QStringLiteral("Select all")}));
    }

    void completerMenuAcceptsCurrentItemWithReturnKey()
    {
        FluentQt::LineEdit lineEdit;
        lineEdit.setText(QStringLiteral("B"));

        QStandardItemModel model;
        model.appendRow(new QStandardItem(QStringLiteral("Alpha")));
        model.appendRow(new QStandardItem(QStringLiteral("Beta")));

        FluentQt::CompleterMenu menu(&lineEdit);
        QVERIFY(menu.setCompletion(&model));
        menu.view()->setCurrentRow(1);

        QSignalSpy activatedSpy(&menu, &FluentQt::CompleterMenu::activated);
        QSignalSpy indexSpy(&menu, &FluentQt::CompleterMenu::indexActivated);

        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(&menu, &keyEvent);

        QCOMPARE(lineEdit.text(), QStringLiteral("Beta"));
        QCOMPARE(activatedSpy.count(), 1);
        QCOMPARE(activatedSpy.takeFirst().at(0).toString(), QStringLiteral("Beta"));
        QCOMPARE(indexSpy.count(), 1);
        QCOMPARE(indexSpy.takeFirst().at(0).toModelIndex().row(), 1);
    }

    void toolTipMatchesPythonDurationPositionsAndFilterDelay()
    {
        QWidget target;
        target.setToolTip(QStringLiteral("Delayed tooltip"));
        target.setToolTipDuration(120);
        target.resize(80, 40);
        target.move(220, 220);
        target.show();
        QVERIFY(QTest::qWaitForWindowExposed(&target));

        FluentQt::ToolTip tip(QStringLiteral("Hello"));
        QCOMPARE(tip.property("fqw").toString(), QStringLiteral("ToolTip"));
        QCOMPARE(tip.text(), QStringLiteral("Hello"));
        QCOMPARE(tip.property("text").toString(), QStringLiteral("Hello"));
        QVERIFY(tip.container() != nullptr);
        QCOMPARE(tip.container()->objectName(), QStringLiteral("container"));
        QCOMPARE(tip.container()->graphicsEffect(), tip.shadowEffect());
        QVERIFY(tip.shadowEffect() != nullptr);
        QCOMPARE(tip.shadowEffect()->blurRadius(), 25.0);
        QCOMPARE(tip.shadowEffect()->offset(), QPointF(0, 5));
        QVERIFY(tip.label() != nullptr);
        QCOMPARE(tip.label()->objectName(), QStringLiteral("contentLabel"));
        QCOMPARE(tip.label()->parentWidget(), tip.container());
        QVERIFY(tip.containerLayout() != nullptr);
        QCOMPARE(tip.containerLayout()->contentsMargins(), QMargins(8, 6, 8, 6));
        QVERIFY(tip.opacityAni() != nullptr);
        QCOMPARE(tip.opacityAni()->propertyName(), QByteArray("windowOpacity"));
        QCOMPARE(tip.opacityAni()->duration(), 150);
        QCOMPARE(tip.property("container").value<QFrame *>(), tip.container());
        QCOMPARE(tip.property("label").value<QLabel *>(), tip.label());
        QCOMPARE(tip.property("containerLayout").value<QHBoxLayout *>(), tip.containerLayout());
        QCOMPARE(tip.property("opacityAni").value<QPropertyAnimation *>(), tip.opacityAni());
        QCOMPARE(tip.property("shadowEffect").value<QGraphicsDropShadowEffect *>(), tip.shadowEffect());
        QCOMPARE(tip.duration(), 1000);
        tip.setDuration(-1);
        QCOMPARE(tip.duration(), -1);
        tip.setText(QStringLiteral("Updated"));
        QCOMPARE(tip.text(), QStringLiteral("Updated"));

        const QList<FluentQt::ToolTipPosition> positions = {
            FluentQt::ToolTipPosition::Top,        FluentQt::ToolTipPosition::Bottom,
            FluentQt::ToolTipPosition::Left,       FluentQt::ToolTipPosition::Right,
            FluentQt::ToolTipPosition::TopLeft,    FluentQt::ToolTipPosition::TopRight,
            FluentQt::ToolTipPosition::BottomLeft, FluentQt::ToolTipPosition::BottomRight};
        for (const auto position : positions) {
            tip.adjustPos(&target, position);
            QVERIFY(QGuiApplication::primaryScreen()->availableGeometry().contains(tip.pos()));
        }

        FluentQt::ToolTipFilter parentFilter(&target, 0, FluentQt::ToolTipPosition::BottomRight);
        QCOMPARE(parentFilter.toolTipDelay(), 0);
        parentFilter.setToolTipDelay(5);
        QCOMPARE(parentFilter.toolTipDelay(), 5);

        FluentQt::ToolTipFilter explicitTextFilter(QStringLiteral("Explicit tooltip"), 0,
                                                   FluentQt::ToolTipPosition::TopRight);
        target.installEventFilter(&explicitTextFilter);
        QEvent enterEvent(QEvent::Enter);
        QApplication::sendEvent(&target, &enterEvent);
        QTRY_VERIFY(!target.findChildren<FluentQt::ToolTip *>().isEmpty());
        auto *shownTip = target.findChild<FluentQt::ToolTip *>();
        QVERIFY(shownTip != nullptr);
        QCOMPARE(shownTip->text(), QStringLiteral("Explicit tooltip"));

        explicitTextFilter.hideToolTip();
        QVERIFY(!shownTip->isVisible());

        FluentQt::AcrylicToolTip acrylicTip(QStringLiteral("Acrylic"));
        QCOMPARE(acrylicTip.property("fqw").toString(), QStringLiteral("AcrylicToolTip"));
        QCOMPARE(acrylicTip.text(), QStringLiteral("Acrylic"));
        QVERIFY(acrylicTip.container() != nullptr);
        QCOMPARE(acrylicTip.container()->property("transparent").toBool(), true);

        FluentQt::AcrylicToolTipFilter acrylicFilter(QStringLiteral("Acrylic tooltip"), 0,
                                                     FluentQt::ToolTipPosition::Bottom);
        target.installEventFilter(&acrylicFilter);
        QApplication::sendEvent(&target, &enterEvent);
        QTRY_VERIFY(target.findChild<FluentQt::AcrylicToolTip *>() != nullptr);
        auto *shownAcrylicTip = target.findChild<FluentQt::AcrylicToolTip *>();
        QCOMPARE(shownAcrylicTip->text(), QStringLiteral("Acrylic tooltip"));
        acrylicFilter.hideToolTip();
        QVERIFY(!shownAcrylicTip->isVisible());
    }

    void flyoutExposesPythonStyleViewAndFactoryApi()
    {
        QWidget target;
        target.resize(80, 32);
        target.move(260, 260);
        target.show();
        QVERIFY(QTest::qWaitForWindowExposed(&target));

        QPixmap image(24, 12);
        image.fill(Qt::cyan);
        FluentQt::FlyoutView view(QStringLiteral("Title"), QStringLiteral("Content"),
                                  FluentQt::icon(FluentQt::FluentIcon::Info), image, true);
        QCOMPARE(view.property("fqw").toString(), QStringLiteral("FlyoutView"));
        QCOMPARE(view.title(), QStringLiteral("Title"));
        QCOMPARE(view.content(), QStringLiteral("Content"));
        QVERIFY(!view.viewIcon().isNull());
        QVERIFY(view.isClosable());
        QVERIFY(view.titleLabel() != nullptr);
        QVERIFY(view.contentLabel() != nullptr);
        QVERIFY(view.iconWidget() != nullptr);
        QVERIFY(view.imageLabel() != nullptr);
        QVERIFY(view.closeButton() != nullptr);
        QVERIFY(!view.closeButton()->isHidden());
        QVERIFY(!view.titleLabel()->wordWrap());
        QVERIFY(!view.contentLabel()->wordWrap());

        FluentQt::FlyoutView simpleView(QStringLiteral("Lesson 3"),
                                        QStringLiteral("Believe in the spin, just keep believing!"));
        QVERIFY(!simpleView.contentLabel()->wordWrap());
        QVERIFY(!simpleView.contentLabel()->text().contains(QLatin1Char('\n')));

        QPixmap largeImage(1680, 1050);
        largeImage.fill(Qt::cyan);
        FluentQt::FlyoutView dynamicImageView(QStringLiteral("Julius Zeppeli"),
                                              QStringLiteral("Where the tennis ball will land."));
        dynamicImageView.setImage(largeImage);
        QVERIFY(dynamicImageView.imageLabel()->width() < 500);

        auto *extra = new FluentQt::PushButton(QStringLiteral("Action"));
        view.addWidget(extra);
        QCOMPARE(extra->parentWidget(), &view);
        QVERIFY(view.widgetLayout() != nullptr);
        QVERIFY(view.bodyLayout() != nullptr);

        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString imagePath = dir.filePath(QStringLiteral("flyout.png"));
        QVERIFY(image.save(imagePath));
        FluentQt::FlyoutView pathView(QStringLiteral("Path"), QStringLiteral("Image"),
                                      QIcon(), imagePath, true);
        QVERIFY(!pathView.imageLabel()->isNull());
        QVERIFY(pathView.setImagePath(imagePath));

        auto *heldFlyout = FluentQt::Flyout::make(new FluentQt::FlyoutView(QStringLiteral("Held"),
                                                                            QStringLiteral("Not shown")),
                                                  nullptr, &target, FluentQt::FlyoutAnimationType::None, false);
        QVERIFY(heldFlyout != nullptr);
        QCOMPARE(heldFlyout->parentWidget(), &target);
        QCOMPARE(heldFlyout->isDeleteOnClose(), false);
        QCOMPARE(heldFlyout->isPopupMode(), true);
        QCOMPARE(heldFlyout->windowFlags() & Qt::Popup, Qt::Popup);
        QVERIFY(heldFlyout->metaObject()->indexOfProperty("popupMode") >= 0);
        heldFlyout->setPopupMode(false);
        QCOMPARE(heldFlyout->isPopupMode(), false);
        QCOMPARE(heldFlyout->windowFlags() & Qt::Tool, Qt::Tool);
        heldFlyout->setShadowEffect(20, QPoint(0, 4));
        heldFlyout->deleteLater();

        auto *pointFlyout = FluentQt::Flyout::create(QStringLiteral("Point"), QStringLiteral("By position"), QIcon(),
                                                     QPixmap(), true, QPoint(300, 300), &target,
                                                     FluentQt::FlyoutAnimationType::None, false);
        QVERIFY(pointFlyout != nullptr);
        QVERIFY(pointFlyout->isVisible());
        QCOMPARE(pointFlyout->animationType(), FluentQt::FlyoutAnimationType::None);
        QCOMPARE(pointFlyout->isDeleteOnClose(), false);
        QCOMPARE(pointFlyout->isPopupMode(), true);
        QCOMPARE(pointFlyout->windowFlags() & Qt::Popup, Qt::Popup);
        auto *pointView = qobject_cast<FluentQt::FlyoutView *>(pointFlyout->view());
        QVERIFY(pointView != nullptr);
        QVERIFY(pointView->isClosable());
        QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, QPoint(4, 4));
        QTRY_VERIFY(!pointFlyout->isVisible());
        pointFlyout->close();

        auto *nonPopupFlyout =
            FluentQt::Flyout::make(new FluentQt::FlyoutView(QStringLiteral("Non-popup"),
                                                            QStringLiteral("Outside click")),
                                   nullptr, &target, FluentQt::FlyoutAnimationType::None, false);
        QVERIFY(nonPopupFlyout != nullptr);
        nonPopupFlyout->setPopupMode(false);
        QCOMPARE(nonPopupFlyout->windowFlags() & Qt::Tool, Qt::Tool);
        nonPopupFlyout->exec(QPoint(360, 360), FluentQt::FlyoutAnimationType::None);
        QVERIFY(nonPopupFlyout->isVisible());
        QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, QPoint(8, 8));
        QTRY_VERIFY(!nonPopupFlyout->isVisible());
        nonPopupFlyout->close();

        auto *targetFlyout = FluentQt::Flyout::create(new FluentQt::FlyoutView(QStringLiteral("Target"),
                                                                               QStringLiteral("By widget")),
                                                      &target, FluentQt::FlyoutAnimationType::SlideRight);
        QVERIFY(targetFlyout != nullptr);
        QVERIFY(targetFlyout->isVisible());
        QCOMPARE(targetFlyout->animationType(), FluentQt::FlyoutAnimationType::SlideRight);
        targetFlyout->close();

        FluentQt::AcrylicFlyoutView acrylicView(QStringLiteral("Acrylic"), QStringLiteral("Material"),
                                                FluentQt::icon(FluentQt::FluentIcon::Accept), image, true);
        QCOMPARE(acrylicView.property("fqw").toString(), QStringLiteral("AcrylicFlyoutView"));
        QVERIFY(acrylicView.acrylicBrush() != nullptr);
        QCOMPARE(acrylicView.title(), QStringLiteral("Acrylic"));
        QVERIFY(acrylicView.isClosable());

        auto *acrylicFlyout = FluentQt::AcrylicFlyout::create(
            QStringLiteral("Acrylic"), QStringLiteral("By position"), FluentQt::icon(FluentQt::FluentIcon::Info),
            QPixmap(), true, QPoint(320, 320), &target, FluentQt::FlyoutAnimationType::None, false);
        QVERIFY(acrylicFlyout != nullptr);
        QVERIFY(acrylicFlyout->isVisible());
        QCOMPARE(acrylicFlyout->property("fqw").toString(), QStringLiteral("AcrylicFlyout"));
        QVERIFY(qobject_cast<FluentQt::AcrylicFlyoutView *>(acrylicFlyout->view()) != nullptr);
        acrylicFlyout->close();

        auto *customAcrylicView = new FluentQt::AcrylicFlyoutViewBase;
        auto *customLayout = new QVBoxLayout(customAcrylicView);
        auto *customLabel = new FluentQt::BodyLabel(QStringLiteral("Custom acrylic"), customAcrylicView);
        customLayout->addWidget(customLabel);
        auto *customFlyout =
            FluentQt::AcrylicFlyout::make(customAcrylicView, &target, &target,
                                          FluentQt::FlyoutAnimationType::None, false);
        QVERIFY(customFlyout != nullptr);
        QVERIFY(customFlyout->isVisible());
        QCOMPARE(customFlyout->view(), customAcrylicView);
        QVERIFY(customAcrylicView->acrylicBrush() != nullptr);
        customFlyout->close();
    }

    void passwordLineEditReflowsMarginsWhenViewButtonVisibilityChanges()
    {
        FluentQt::PasswordLineEdit editor;

        QCOMPARE(editor.property("fqw").toString(), QStringLiteral("PasswordLineEdit"));
        QCOMPARE(editor.echoMode(), QLineEdit::Password);
        QCOMPARE(editor.textMargins().right(), 30);

        editor.setClearButtonEnabled(true);
        QCOMPARE(editor.textMargins().right(), 58);

        editor.setViewPasswordButtonVisible(false);
        QCOMPARE(editor.textMargins().right(), 28);

        editor.setClearButtonEnabled(false);
        QCOMPARE(editor.textMargins().right(), 0);

        editor.setViewPasswordButtonVisible(true);
        QCOMPARE(editor.textMargins().right(), 30);
    }

    void buttonIconTextConstructorsKeepFluentRoles()
    {
        const QIcon starIcon = FluentQt::icon(FluentQt::FluentIcon::Heart);
        const QIcon heartIcon = FluentQt::icon(FluentQt::FluentIcon::Heart);
        QVERIFY(!starIcon.isNull());
        QVERIFY(!heartIcon.isNull());
        QTemporaryDir iconDir;
        QVERIFY(iconDir.isValid());
        const QString iconPath = iconDir.filePath(QStringLiteral("icon.png"));
        QPixmap pathPixmap(16, 16);
        pathPixmap.fill(QColor(QStringLiteral("#123456")));
        QVERIFY(pathPixmap.save(iconPath));

        FluentQt::TogglePushButton togglePush(starIcon, QStringLiteral("Toggle"));
        QCOMPARE(togglePush.property("fqw").toString(), QStringLiteral("TogglePushButton"));
        QVERIFY(togglePush.isCheckable());
        QCOMPARE(togglePush.text(), QStringLiteral("Toggle"));
        QVERIFY(togglePush.property("hasIcon").toBool());
        QVERIFY(!togglePush.icon().isNull());

        FluentQt::ToggleButton toggleButton(starIcon, QStringLiteral("Toggle alias"));
        QCOMPARE(toggleButton.property("fqw").toString(), QStringLiteral("TogglePushButton"));
        QVERIFY(toggleButton.isCheckable());
        QVERIFY(toggleButton.property("hasIcon").toBool());

        FluentQt::TransparentTogglePushButton transparentToggle(heartIcon, QStringLiteral("Transparent"));
        QCOMPARE(transparentToggle.property("fqw").toString(), QStringLiteral("TransparentTogglePushButton"));
        QVERIFY(transparentToggle.isCheckable());
        QVERIFY(transparentToggle.property("hasIcon").toBool());

        FluentQt::DropDownPushButton dropDown(heartIcon, QStringLiteral("Email"));
        QCOMPARE(dropDown.property("fqw").toString(), QStringLiteral("DropDownPushButton"));
        QCOMPARE(dropDown.text(), QStringLiteral("Email"));
        QVERIFY(dropDown.property("hasIcon").toBool());

        FluentQt::PrimaryDropDownPushButton primaryDropDown(heartIcon, QStringLiteral("Email"));
        QCOMPARE(primaryDropDown.property("fqw").toString(), QStringLiteral("PrimaryDropDownPushButton"));
        QVERIFY(primaryDropDown.property("hasIcon").toBool());

        FluentQt::TransparentDropDownPushButton transparentDropDown(heartIcon, QStringLiteral("Email"));
        QCOMPARE(transparentDropDown.property("fqw").toString(), QStringLiteral("TransparentDropDownPushButton"));
        QVERIFY(transparentDropDown.property("hasIcon").toBool());

        FluentQt::PillPushButton pill(FluentQt::icon(FluentQt::FluentIcon::Calendar), QStringLiteral("Tag"));
        QCOMPARE(pill.property("fqw").toString(), QStringLiteral("PillPushButton"));
        QVERIFY(pill.isCheckable());
        QVERIFY(pill.property("hasIcon").toBool());

        FluentQt::SplitPushButton splitPush(starIcon, QStringLiteral("Split"));
        QCOMPARE(splitPush.text(), QStringLiteral("Split"));
        QVERIFY(!splitPush.icon().isNull());

        FluentQt::PrimarySplitPushButton primarySplitPush(starIcon, QStringLiteral("Primary split"));
        QCOMPARE(primarySplitPush.text(), QStringLiteral("Primary split"));
        QVERIFY(!primarySplitPush.icon().isNull());

        FluentQt::ToolButton pathTool(iconPath);
        QVERIFY(!pathTool.icon().isNull());

        FluentQt::DropDownToolButton pathDropDownTool(iconPath);
        QCOMPARE(pathDropDownTool.property("fqw").toString(), QStringLiteral("DropDownToolButton"));
        QVERIFY(!pathDropDownTool.icon().isNull());

        FluentQt::SplitToolButton pathSplitTool(iconPath);
        QCOMPARE(pathSplitTool.button()->property("fqw").toString(), QStringLiteral("ToolButton"));
        QVERIFY(!pathSplitTool.icon().isNull());
    }

    void pushButtonsKeepHasIconInSyncWhenIconChanges()
    {
        auto filledIcon = [](const QColor &color) {
            QPixmap pixmap(16, 16);
            pixmap.fill(color);
            return QIcon(pixmap);
        };

        FluentQt::PushButton button(QStringLiteral("Dynamic"));
        QVERIFY(button.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(button.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(!button.isPressed());
        QVERIFY(!button.isHover());
        QCOMPARE(button.property("hasIcon").toBool(), false);
        button.setIcon(filledIcon(Qt::red));
        QCOMPARE(button.property("hasIcon").toBool(), true);
        button.setIcon(QIcon());
        QCOMPARE(button.property("hasIcon").toBool(), false);
        QTest::mousePress(&button, Qt::LeftButton);
        QVERIFY(button.isPressed());
        QVERIFY(button.property("isPressed").toBool());
        QTest::mouseRelease(&button, Qt::LeftButton);
        QVERIFY(!button.isPressed());

        FluentQt::HyperlinkButton link(QStringLiteral("https://example.com"), QStringLiteral("Link"));
        QVERIFY(link.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(link.metaObject()->indexOfProperty("isHover") >= 0);
        QCOMPARE(link.property("hasIcon").toBool(), false);
        link.setIcon(filledIcon(Qt::green));
        QCOMPARE(link.property("hasIcon").toBool(), true);
        link.setIcon(QIcon());
        QCOMPARE(link.property("hasIcon").toBool(), false);

        FluentQt::HyperlinkButton stateLink;
        QTest::mousePress(&stateLink, Qt::LeftButton);
        QVERIFY(stateLink.isPressed());
        QTest::mouseRelease(&stateLink, Qt::LeftButton);
        QVERIFY(!stateLink.isPressed());

        FluentQt::ToolButton tool(filledIcon(Qt::blue));
        QVERIFY(tool.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(tool.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(!tool.isPressed());
        QVERIFY(!tool.isHover());
        QTest::mousePress(&tool, Qt::LeftButton);
        QVERIFY(tool.isPressed());
        QTest::mouseRelease(&tool, Qt::LeftButton);
        QVERIFY(!tool.isPressed());

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);
        FluentQt::PrimaryPushButton primary(QStringLiteral("Primary"));
        primary.setIcon(filledIcon(Qt::red));
        QCOMPARE(primary.property("hasIcon").toBool(), true);

        QVERIFY(static_cast<QPushButton *>(&primary)->icon().isNull());
        const QImage image = primary.icon().pixmap(QSize(16, 16), QIcon::Normal).toImage();
        int redPixels = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() > 0 && color.red() > 220 && color.green() < 80 && color.blue() < 80) {
                    ++redPixels;
                }
            }
        }
        QVERIFY(redPixels > 200);
    }

    void hyperlinkButtonAcceptsStringUrl()
    {
        FluentQt::HyperlinkButton button(QStringLiteral("https://example.com"), QStringLiteral("Open"));
        QCOMPARE(button.property("fqw").toString(), QStringLiteral("HyperlinkButton"));
        QCOMPARE(button.text(), QStringLiteral("Open"));
        QCOMPARE(button.url(), QUrl(QStringLiteral("https://example.com")));

        button.setUrl(QStringLiteral(FQW_REPOSITORY_URL));
        QCOMPARE(button.url(), QUrl(QStringLiteral(FQW_REPOSITORY_URL)));
    }

    void spinBoxesExposePythonParityControlApi()
    {
        auto verifySharedState = [](auto &spin, const QString &role) {
            QCOMPARE(spin.property("fqw").toString(), role);
            QVERIFY(spin.metaObject()->indexOfProperty("isError") >= 0);
            QVERIFY(spin.metaObject()->indexOfProperty("symbolVisible") >= 0);
            QCOMPARE(spin.buttonSymbols(), QAbstractSpinBox::NoButtons);
            QCOMPARE(spin.property("transparent").toBool(), true);
            QCOMPARE(spin.property("symbolVisible").toBool(), true);
            QCOMPARE(spin.isSymbolVisible(), true);
            QCOMPARE(spin.height(), 33);
            QCOMPARE(spin.isAccelerated(), false);
            QVERIFY(spin.hBoxLayout() != nullptr);
            QCOMPARE(spin.hBoxLayout(), qobject_cast<QHBoxLayout *>(spin.layout()));

            QSignalSpy errorSpy(&spin, SIGNAL(errorChanged(bool)));
            spin.setError(true);
            QCOMPARE(spin.isError(), true);
            QCOMPARE(spin.property("isError").toBool(), true);
            QCOMPARE(errorSpy.count(), 1);

            spin.setError(true);
            QCOMPARE(errorSpy.count(), 1);

            spin.setError(false);
            QCOMPARE(spin.isError(), false);
            QCOMPARE(spin.property("isError").toBool(), false);
            QCOMPARE(errorSpy.count(), 2);

            spin.setReadOnly(true);
            QCOMPARE(spin.isReadOnly(), true);
            QCOMPARE(spin.property("symbolVisible").toBool(), false);
            QCOMPARE(spin.isSymbolVisible(), false);

            spin.setReadOnly(false);
            QCOMPARE(spin.isReadOnly(), false);
            QCOMPARE(spin.property("symbolVisible").toBool(), true);
            QCOMPARE(spin.isSymbolVisible(), true);

            QVERIFY(spin.setProperty("symbolVisible", false));
            QCOMPARE(spin.property("symbolVisible").toBool(), false);
            QCOMPARE(spin.isSymbolVisible(), false);

            QVERIFY(spin.setProperty("symbolVisible", true));
            QCOMPARE(spin.property("symbolVisible").toBool(), true);
            QCOMPARE(spin.isSymbolVisible(), true);
        };

        auto verifyInline = [&](auto &spin, const QString &role) {
            verifySharedState(spin, role);

            QVERIFY(spin.upButton() != nullptr);
            QVERIFY(spin.downButton() != nullptr);
            QCOMPARE(spin.upButton()->iconType(), FluentQt::SpinIconType::Up);
            QCOMPARE(spin.downButton()->iconType(), FluentQt::SpinIconType::Down);
            QVERIFY(!spin.upButton()->isPressed());
            QVERIFY(!spin.downButton()->isPressed());
            QVERIFY(spin.upButton()->metaObject()->indexOfProperty("iconType") >= 0);
            QVERIFY(spin.upButton()->metaObject()->indexOfProperty("isPressed") >= 0);

            auto buttons = spin.template findChildren<FluentQt::SpinButton *>(QString(), Qt::FindDirectChildrenOnly);
            QCOMPARE(buttons.size(), 2);
            QVERIFY(buttons.contains(spin.upButton()));
            QVERIFY(buttons.contains(spin.downButton()));
            for (auto *button : buttons) {
                QCOMPARE(button->property("fqw").toString(), QStringLiteral("SpinButton"));
                QCOMPARE(button->isVisibleTo(&spin), true);
                QCOMPARE(button->autoRepeat(), false);
            }

            spin.setAccelerated(true);
            QCOMPARE(spin.isAccelerated(), true);
            for (auto *button : buttons) {
                QCOMPARE(button->autoRepeat(), true);
            }

            spin.setSymbolVisible(false);
            QCOMPARE(spin.property("symbolVisible").toBool(), false);
            for (auto *button : buttons) {
                QCOMPARE(button->isVisibleTo(&spin), false);
            }
        };

        auto verifyCompact = [&](auto &spin, const QString &role) {
            verifySharedState(spin, role);

            QVERIFY(spin.compactSpinButton() != nullptr);
            QVERIFY(spin.spinFlyout() != nullptr);
            QVERIFY(spin.spinFlyoutView() != nullptr);

            auto buttons =
                spin.template findChildren<FluentQt::CompactSpinButton *>(QString(), Qt::FindDirectChildrenOnly);
            QCOMPARE(buttons.size(), 1);
            auto *compactButton = buttons.first();
            QCOMPARE(compactButton, spin.compactSpinButton());
            QCOMPARE(compactButton->property("fqw").toString(), QStringLiteral("CompactSpinButton"));
            QCOMPARE(compactButton->isVisibleTo(&spin), true);

            auto *flyout = spin.template findChild<FluentQt::Flyout *>(QString(), Qt::FindDirectChildrenOnly);
            QCOMPARE(flyout, spin.spinFlyout());
            QCOMPARE(flyout->isDeleteOnClose(), false);
            QCOMPARE(flyout->isPopupMode(), true);
            QCOMPARE(flyout->property("popupMode").toBool(), true);
            QCOMPARE(flyout->windowFlags() & Qt::Popup, Qt::Popup);
            QCOMPARE(qobject_cast<FluentQt::SpinFlyoutView *>(flyout->view()), spin.spinFlyoutView());
            QVERIFY(spin.spinFlyoutView()->upButton != nullptr);
            QVERIFY(spin.spinFlyoutView()->downButton != nullptr);
            QVERIFY(spin.spinFlyoutView()->vBoxLayout() != nullptr);
            QCOMPARE(spin.spinFlyoutView()->upButtonWidget(), spin.spinFlyoutView()->upButton);
            QCOMPARE(spin.spinFlyoutView()->downButtonWidget(), spin.spinFlyoutView()->downButton);
            const auto flyoutButtons = flyout->view()->template findChildren<QToolButton *>();
            QCOMPARE(flyoutButtons.size(), 2);
            QVERIFY(flyoutButtons.contains(spin.spinFlyoutView()->upButton));
            QVERIFY(flyoutButtons.contains(spin.spinFlyoutView()->downButton));
            for (auto *button : flyoutButtons) {
                QVERIFY(dynamic_cast<FluentQt::TransparentToolButton *>(button) != nullptr);
                QCOMPARE(button->property("fqw").toString(), QStringLiteral("TransparentToolButton"));
                QCOMPARE(button->autoRepeat(), false);
                QVERIFY(button->icon().isNull());
            }

            spin.setAccelerated(true);
            QCOMPARE(spin.isAccelerated(), true);
            for (auto *button : flyoutButtons) {
                QCOMPARE(button->autoRepeat(), true);
            }

            spin.setSymbolVisible(false);
            QCOMPARE(spin.property("symbolVisible").toBool(), false);
            QCOMPARE(compactButton->isVisibleTo(&spin), false);
        };

        FluentQt::SpinBox spin;
        verifyInline(spin, QStringLiteral("SpinBox"));

        FluentQt::DoubleSpinBox doubleSpin;
        verifyInline(doubleSpin, QStringLiteral("DoubleSpinBox"));

        FluentQt::TimeEdit timeEdit;
        verifyInline(timeEdit, QStringLiteral("TimeEdit"));

        FluentQt::DateEdit dateEdit;
        verifyInline(dateEdit, QStringLiteral("DateEdit"));

        FluentQt::DateTimeEdit dateTimeEdit;
        verifyInline(dateTimeEdit, QStringLiteral("DateTimeEdit"));

        FluentQt::CompactSpinBox compactSpin;
        verifyCompact(compactSpin, QStringLiteral("CompactSpinBox"));

        FluentQt::CompactDoubleSpinBox compactDoubleSpin;
        verifyCompact(compactDoubleSpin, QStringLiteral("CompactDoubleSpinBox"));

        FluentQt::CompactTimeEdit compactTimeEdit;
        verifyCompact(compactTimeEdit, QStringLiteral("CompactTimeEdit"));

        FluentQt::CompactDateEdit compactDateEdit;
        verifyCompact(compactDateEdit, QStringLiteral("CompactDateEdit"));

        FluentQt::CompactDateTimeEdit compactDateTimeEdit;
        verifyCompact(compactDateTimeEdit, QStringLiteral("CompactDateTimeEdit"));
    }

    void multilineTextEditorsUseFluentChromeAndSmoothScroll()
    {
        auto verifyEditor = [](auto &editor, const QString &role) {
            QCOMPARE(editor.property("fqw").toString(), role);
            QVERIFY(editor.metaObject()->indexOfProperty("layer") >= 0);
            QVERIFY(editor.metaObject()->indexOfProperty("scrollDelegate") >= 0);

            auto *layer = editor.template findChild<QWidget *>(QStringLiteral("editLayer"));
            QVERIFY(layer != nullptr);
            QCOMPARE(editor.layer(), layer);
            QCOMPARE(editor.property("layer").template value<QWidget *>(), layer);
            QCOMPARE(layer->property("fqw").toString(), QStringLiteral("EditLayer"));
            QCOMPARE(layer->testAttribute(Qt::WA_TransparentForMouseEvents), true);

            editor.resize(240, 120);
            QCoreApplication::processEvents();
            QCOMPARE(layer->size(), editor.viewport()->size());

            auto *delegate = editor.template findChild<FluentQt::SmoothScrollDelegate *>();
            QVERIFY(delegate != nullptr);
            QCOMPARE(editor.scrollDelegate(), delegate);
            QCOMPARE(editor.property("scrollDelegate").template value<FluentQt::SmoothScrollDelegate *>(), delegate);
            QVERIFY(delegate->verticalScrollBar() != nullptr);
            QVERIFY(delegate->horizontalScrollBar() != nullptr);
            QCOMPARE(editor.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
            QCOMPARE(editor.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        };

        FluentQt::TextEdit textEdit;
        verifyEditor(textEdit, QStringLiteral("TextEdit"));

        FluentQt::PlainTextEdit plainTextEdit;
        verifyEditor(plainTextEdit, QStringLiteral("PlainTextEdit"));

        FluentQt::TextBrowser textBrowser;
        verifyEditor(textBrowser, QStringLiteral("TextBrowser"));
    }

    void slidersTrackPythonParityGeometryAndClicking()
    {
        FluentQt::Slider slider(Qt::Horizontal);
        slider.setRange(0, 100);
        slider.resize(122, 24);
        slider.setValue(50);
        QCOMPARE(slider.property("fqw").toString(), QStringLiteral("Slider"));
        QCOMPARE(slider.minimumHeight(), 22);
        QVERIFY(slider.metaObject()->indexOfProperty("lightGrooveColor") >= 0);
        QVERIFY(slider.metaObject()->indexOfProperty("darkGrooveColor") >= 0);
        QVERIFY(slider.metaObject()->indexOfProperty("grooveLength") >= 0);
        QVERIFY(slider.metaObject()->indexOfProperty("pressedPos") >= 0);

        auto *handle = slider.handle();
        QVERIFY(handle != nullptr);
        QCOMPARE(handle, slider.findChild<FluentQt::SliderHandle *>(QString(), Qt::FindDirectChildrenOnly));
        QCOMPARE(handle->size(), QSize(22, 22));
        QCOMPARE(slider.grooveLength(), 100);
        QCOMPARE(handle->pos(), QPoint(50, 0));
        QVERIFY(handle->metaObject()->indexOfProperty("lightHandleColor") >= 0);
        QVERIFY(handle->metaObject()->indexOfProperty("darkHandleColor") >= 0);

        slider.setProperty("lightGrooveColor", QColor(QStringLiteral("#cc1122")));
        slider.setProperty("darkGrooveColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(slider.lightGrooveColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(slider.darkGrooveColor(), QColor(QStringLiteral("#22cc11")));
        QCOMPARE(handle->lightHandleColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(handle->darkHandleColor(), QColor(QStringLiteral("#22cc11")));

        auto sendLeftButtonMove = [](QWidget *widget, const QPoint &pos) {
            QMouseEvent moveEvent(QEvent::MouseMove, QPointF(pos), QPointF(pos),
                                  QPointF(widget->mapToGlobal(pos)), Qt::NoButton, Qt::LeftButton,
                                  Qt::NoModifier);
            QCoreApplication::sendEvent(widget, &moveEvent);
        };

        QSignalSpy clickedSpy(&slider, &FluentQt::Slider::clicked);
        QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(111, 12));
        QCOMPARE(slider.value(), 100);
        QCOMPARE(slider.pressedPos(), QPoint(111, 12));
        QCOMPARE(clickedSpy.count(), 1);
        QCOMPARE(clickedSpy.takeFirst().at(0).toInt(), 100);
        QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(111, 12));

        slider.setValue(50);
        const QPoint previousPressedPos = slider.pressedPos();
        QSignalSpy movedSpy(&slider, &FluentQt::Slider::sliderMoved);
        QTest::mouseMove(&slider, QPoint(11, 12));
        QCOMPARE(slider.value(), 50);
        QCOMPARE(slider.pressedPos(), previousPressedPos);
        QCOMPARE(movedSpy.count(), 0);
        sendLeftButtonMove(&slider, QPoint(11, 12));
        QCOMPARE(slider.value(), 50);
        QCOMPARE(slider.pressedPos(), previousPressedPos);
        QCOMPARE(movedSpy.count(), 0);

        QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(11, 12));
        QCOMPARE(slider.value(), 0);
        sendLeftButtonMove(&slider, QPoint(111, 12));
        QCOMPARE(slider.value(), 100);
        QCOMPARE(slider.pressedPos(), QPoint(111, 12));
        QCOMPARE(movedSpy.count(), 1);
        QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(111, 12));

        FluentQt::Slider vertical(Qt::Vertical);
        vertical.setRange(0, 100);
        vertical.resize(24, 122);
        vertical.setValue(50);
        QCOMPARE(vertical.minimumWidth(), 22);
        auto *verticalHandle = vertical.findChild<FluentQt::SliderHandle *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(verticalHandle != nullptr);
        QCOMPARE(verticalHandle->pos(), QPoint(0, 50));

        FluentQt::ClickableSlider clickable(Qt::Horizontal);
        clickable.setRange(0, 100);
        clickable.resize(200, 24);
        QSignalSpy clickableSpy(&clickable, &FluentQt::ClickableSlider::clicked);
        QTest::mousePress(&clickable, Qt::LeftButton, Qt::NoModifier, QPoint(150, 12));
        QCOMPARE(clickable.value(), 75);
        QCOMPARE(clickableSpy.count(), 1);

        FluentQt::HollowHandleStyle::Config config;
        config.grooveHeight = 7;
        config.handleMargin = 2;
        config.handleRingWidth = 3;
        config.handleHollowRadius = 5;
        FluentQt::HollowHandleStyle style(config);

        QStyleOptionSlider option;
        option.initFrom(&slider);
        option.orientation = Qt::Horizontal;
        option.minimum = 0;
        option.maximum = 100;
        option.sliderPosition = 50;
        option.rect = QRect(0, 0, 120, 20);

        const QRect groove = style.subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, &slider);
        QCOMPARE(groove.height(), 7);

        const QRect styleHandle = style.subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, &slider);
        QCOMPARE(styleHandle.size(), QSize(20, 20));
    }

    void progressIndicatorsExposePythonParityApi()
    {
        FluentQt::ProgressBar bar(nullptr, false);
        QCOMPARE(bar.property("fqw").toString(), QStringLiteral("ProgressBar"));
        QCOMPARE(bar.height(), 4);
        QCOMPARE(bar.useAni(), false);
        QVERIFY(bar.metaObject()->indexOfProperty("lightBarColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("darkBarColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("lightBackgroundColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("darkBackgroundColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("ani") >= 0);
        QVERIFY(bar.ani() != nullptr);
        QCOMPARE(bar.ani()->propertyName(), QByteArray("val"));
        QCOMPARE(bar.property("ani").value<QPropertyAnimation *>(), bar.ani());
        bar.setProperty("lightBarColor", QColor(QStringLiteral("#112233")));
        bar.setProperty("darkBarColor", QColor(QStringLiteral("#445566")));
        bar.setProperty("lightBackgroundColor", QColor(QStringLiteral("#778899")));
        bar.setProperty("darkBackgroundColor", QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(bar.lightBarColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(bar.darkBarColor(), QColor(QStringLiteral("#445566")));
        QCOMPARE(bar.lightBackgroundColor(), QColor(QStringLiteral("#778899")));
        QCOMPARE(bar.darkBackgroundColor(), QColor(QStringLiteral("#aabbcc")));
        bar.setValue(42);
        QCOMPARE(bar.val(), 42.0);
        bar.pause();
        QCOMPARE(bar.isPaused(), true);
        QCOMPARE(bar.isError(), false);
        bar.error();
        QCOMPARE(bar.isError(), true);
        bar.pause();
        QCOMPARE(bar.isPaused(), true);
        QCOMPARE(bar.isError(), true);
        bar.setError(false);
        QCOMPARE(bar.isError(), false);
        QCOMPARE(bar.isPaused(), false);

        FluentQt::IndeterminateProgressBar indeterminateBar(nullptr, false);
        QCOMPARE(indeterminateBar.property("fqw").toString(), QStringLiteral("IndeterminateProgressBar"));
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("lightBarColor") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("darkBarColor") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("shortBarAni") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("longBarAni") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("aniGroup") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("longBarAniGroup") >= 0);
        QVERIFY(indeterminateBar.shortBarAni() != nullptr);
        QVERIFY(indeterminateBar.longBarAni() != nullptr);
        QVERIFY(indeterminateBar.aniGroup() != nullptr);
        QVERIFY(indeterminateBar.longBarAniGroup() != nullptr);
        QCOMPARE(indeterminateBar.shortBarAni()->propertyName(), QByteArray("shortPos"));
        QCOMPARE(indeterminateBar.longBarAni()->propertyName(), QByteArray("longPos"));
        QCOMPARE(indeterminateBar.shortBarAni()->duration(), 833);
        QCOMPARE(indeterminateBar.longBarAni()->duration(), 1167);
        QCOMPARE(indeterminateBar.aniGroup()->loopCount(), -1);
        QCOMPARE(indeterminateBar.property("shortBarAni").value<QPropertyAnimation *>(), indeterminateBar.shortBarAni());
        QCOMPARE(indeterminateBar.property("aniGroup").value<QParallelAnimationGroup *>(), indeterminateBar.aniGroup());
        indeterminateBar.setProperty("lightBarColor", QColor(QStringLiteral("#123456")));
        indeterminateBar.setProperty("darkBarColor", QColor(QStringLiteral("#654321")));
        QCOMPARE(indeterminateBar.lightBarColor(), QColor(QStringLiteral("#123456")));
        QCOMPARE(indeterminateBar.darkBarColor(), QColor(QStringLiteral("#654321")));
        QCOMPARE(indeterminateBar.isStarted(), false);
        indeterminateBar.start();
        QCOMPARE(indeterminateBar.isStarted(), true);
        indeterminateBar.pause();
        QCOMPARE(indeterminateBar.isPaused(), true);
        indeterminateBar.setError(true);
        QCOMPARE(indeterminateBar.isError(), true);
        QCOMPARE(indeterminateBar.isStarted(), false);

        FluentQt::ProgressRing ring(nullptr, false);
        QCOMPARE(ring.property("fqw").toString(), QStringLiteral("ProgressRing"));
        QCOMPARE(ring.size(), QSize(100, 100));
        QCOMPARE(ring.sizeHint(), QSize(100, 100));
        QCOMPARE(ring.strokeWidth(), 6);
        QCOMPARE(ring.useAni(), false);
        ring.setProperty("lightBarColor", QColor(QStringLiteral("#112233")));
        ring.setProperty("darkBarColor", QColor(QStringLiteral("#445566")));
        ring.setProperty("lightBackgroundColor", QColor(QStringLiteral("#778899")));
        ring.setProperty("darkBackgroundColor", QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(ring.lightBarColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(ring.darkBarColor(), QColor(QStringLiteral("#445566")));
        QCOMPARE(ring.lightBackgroundColor(), QColor(QStringLiteral("#778899")));
        QCOMPARE(ring.darkBackgroundColor(), QColor(QStringLiteral("#aabbcc")));
        ring.setStrokeWidth(9);
        QCOMPARE(ring.strokeWidth(), 9);
        ring.setValue(72);
        QCOMPARE(ring.value(), 72);
        QCOMPARE(ring.val(), 72.0);
        ring.setTextVisible(true);
        QCOMPARE(ring.isTextVisible(), true);

        FluentQt::IndeterminateProgressRing indeterminateRing(nullptr, false);
        QCOMPARE(indeterminateRing.property("fqw").toString(), QStringLiteral("IndeterminateProgressRing"));
        QCOMPARE(indeterminateRing.size(), QSize(80, 80));
        QCOMPARE(indeterminateRing.strokeWidth(), 6);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("lightBarColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("darkBarColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("lightBackgroundColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("darkBackgroundColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("startAngleAni1") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("startAngleAni2") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("spanAngleAni1") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("spanAngleAni2") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("startAngleAniGroup") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("spanAngleAniGroup") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("aniGroup") >= 0);
        QVERIFY(indeterminateRing.startAngleAni1() != nullptr);
        QVERIFY(indeterminateRing.startAngleAni2() != nullptr);
        QVERIFY(indeterminateRing.spanAngleAni1() != nullptr);
        QVERIFY(indeterminateRing.spanAngleAni2() != nullptr);
        QVERIFY(indeterminateRing.startAngleAniGroup() != nullptr);
        QVERIFY(indeterminateRing.spanAngleAniGroup() != nullptr);
        QVERIFY(indeterminateRing.aniGroup() != nullptr);
        QCOMPARE(indeterminateRing.startAngleAni1()->propertyName(), QByteArray("startAngle"));
        QCOMPARE(indeterminateRing.spanAngleAni1()->propertyName(), QByteArray("spanAngle"));
        QCOMPARE(indeterminateRing.aniGroup()->loopCount(), -1);
        QCOMPARE(indeterminateRing.property("aniGroup").value<QParallelAnimationGroup *>(), indeterminateRing.aniGroup());
        indeterminateRing.setProperty("lightBarColor", QColor(QStringLiteral("#010203")));
        indeterminateRing.setProperty("darkBarColor", QColor(QStringLiteral("#040506")));
        indeterminateRing.setProperty("lightBackgroundColor", QColor(QStringLiteral("#070809")));
        indeterminateRing.setProperty("darkBackgroundColor", QColor(QStringLiteral("#0a0b0c")));
        QCOMPARE(indeterminateRing.lightBarColor(), QColor(QStringLiteral("#010203")));
        QCOMPARE(indeterminateRing.darkBarColor(), QColor(QStringLiteral("#040506")));
        QCOMPARE(indeterminateRing.lightBackgroundColor(), QColor(QStringLiteral("#070809")));
        QCOMPARE(indeterminateRing.darkBackgroundColor(), QColor(QStringLiteral("#0a0b0c")));
        QCOMPARE(indeterminateRing.startAngle(), -180);
        QCOMPARE(indeterminateRing.spanAngle(), 0);
        QCOMPARE(indeterminateRing.isStarted(), false);
        indeterminateRing.setStrokeWidth(8);
        QCOMPARE(indeterminateRing.strokeWidth(), 8);
        indeterminateRing.setStartAngle(120);
        indeterminateRing.setSpanAngle(160);
        QCOMPARE(indeterminateRing.startAngle(), 120);
        QCOMPARE(indeterminateRing.spanAngle(), 160);
        indeterminateRing.start();
        QCOMPARE(indeterminateRing.isStarted(), true);
        indeterminateRing.stop();
        QCOMPARE(indeterminateRing.isStarted(), false);
        QCOMPARE(indeterminateRing.startAngle(), 0);
        QCOMPARE(indeterminateRing.spanAngle(), 0);
    }

    void switchButtonExposesPythonParityApiAndThemeTextStyle()
    {
        FluentQt::SwitchButton button(QStringLiteral("Disabled"), nullptr, FluentQt::IndicatorPosition::Right);
        QCOMPARE(button.property("fqw").toString(), QStringLiteral("SwitchButton"));
        QCOMPARE(button.indicatorPosition(), FluentQt::IndicatorPosition::Right);
        QCOMPARE(button.indicatorPos(), FluentQt::IndicatorPosition::Right);
        QVERIFY(button.metaObject()->indexOfProperty("indicatorPos") >= 0);
        QCOMPARE(button.spacing(), 12);
        QCOMPARE(button.text(), QStringLiteral("Disabled"));
        QCOMPARE(button.offText(), QStringLiteral("Disabled"));
        QCOMPARE(button.onText(), QStringLiteral("On"));
        QCOMPARE(button.isChecked(), false);

        auto *indicator = button.indicator();
        QVERIFY(indicator != nullptr);
        QCOMPARE(indicator, button.findChild<FluentQt::SwitchIndicator *>());
        QVERIFY(button.label() != nullptr);
        QCOMPARE(button.hBox(), button.hBoxLayout());
        QVERIFY(button.hBoxLayout() != nullptr);
        QCOMPARE(button.label()->text(), QStringLiteral("Disabled"));
        button.setIndicatorPos(FluentQt::IndicatorPosition::Left);
        QCOMPARE(button.indicatorPosition(), FluentQt::IndicatorPosition::Left);
        QCOMPARE(button.indicatorPos(), FluentQt::IndicatorPosition::Left);
        button.setIndicatorPosition(FluentQt::IndicatorPosition::Right);
        QCOMPARE(button.indicatorPos(), FluentQt::IndicatorPosition::Right);
        QVERIFY(indicator->metaObject()->indexOfProperty("lightCheckedColor") >= 0);
        QVERIFY(indicator->metaObject()->indexOfProperty("darkCheckedColor") >= 0);
        QCOMPARE(indicator->size(), QSize(42, 22));
        QCOMPARE(indicator->sliderX(), 5.0);
        QCOMPARE(indicator->isDown(), false);
        QTest::mousePress(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());
        QCOMPARE(indicator->isDown(), true);
        QTest::mouseRelease(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());
        QCOMPARE(indicator->isDown(), false);
        QCOMPARE(button.isChecked(), true);

        button.setOnText(QStringLiteral("Enabled"));
        button.setChecked(true);
        QCOMPARE(button.isChecked(), true);
        QCOMPARE(button.text(), QStringLiteral("Enabled"));

        button.toggleChecked();
        QCOMPARE(button.isChecked(), false);
        QCOMPARE(button.text(), QStringLiteral("Disabled"));

        QSignalSpy checkedSpy(&button, &FluentQt::SwitchButton::checkedChanged);
        button.toggle();
        QCOMPARE(button.isChecked(), true);
        QCOMPARE(checkedSpy.count(), 1);

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);
        auto sampleTrackColor = [indicator]() {
            indicator->ensurePolished();
            QPixmap pixmap(indicator->size());
            pixmap.fill(Qt::transparent);
            indicator->render(&pixmap);
            return pixmap.toImage().pixelColor(32, indicator->height() / 2);
        };
        auto fuzzyCompareColor = [](const QColor &actual, const QColor &expected) {
            return qAbs(actual.red() - expected.red()) <= 2 && qAbs(actual.green() - expected.green()) <= 2 &&
                   qAbs(actual.blue() - expected.blue()) <= 2 && actual.alpha() > 240;
        };

        indicator->setHover(true);
        QVERIFY(fuzzyCompareColor(sampleTrackColor(),
                                  FluentQt::derivedThemeColor(FluentQt::themeColor(), FluentQt::ThemeColor::Light1,
                                                              FluentQt::Theme::Light)));
        indicator->setDown(true);
        QVERIFY(fuzzyCompareColor(sampleTrackColor(),
                                  FluentQt::derivedThemeColor(FluentQt::themeColor(), FluentQt::ThemeColor::Light2,
                                                              FluentQt::Theme::Light)));
        indicator->setDown(false);
        indicator->setHover(false);
        indicator->setProperty("lightCheckedColor", QColor(QStringLiteral("#cc1122")));
        indicator->setProperty("darkCheckedColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(indicator->lightCheckedColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(indicator->darkCheckedColor(), QColor(QStringLiteral("#22cc11")));
        const QColor customTrackColor = sampleTrackColor();
        QVERIFY(customTrackColor.red() > 120);
        QVERIFY(customTrackColor.green() < 80);
        QVERIFY(customTrackColor.blue() < 90);

        button.setSpacing(20);
        QCOMPARE(button.spacing(), 20);
        button.setIndicatorPosition(FluentQt::IndicatorPosition::Left);
        QCOMPARE(button.indicatorPosition(), FluentQt::IndicatorPosition::Left);

        auto *label = button.findChild<QLabel *>();
        QVERIFY(label != nullptr);
        QVERIFY(button.metaObject()->indexOfProperty("lightTextColor") >= 0);
        QVERIFY(button.metaObject()->indexOfProperty("darkTextColor") >= 0);
        button.setProperty("lightTextColor", QColor(QStringLiteral("#112233")));
        button.setProperty("darkTextColor", QColor(QStringLiteral("#ddeeff")));
        QCOMPARE(button.lightTextColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(button.darkTextColor(), QColor(QStringLiteral("#ddeeff")));
        const QString lightQss = FluentQt::StyleSheetManager::instance()->customStyleSheet(label, FluentQt::Theme::Light);
        const QString darkQss = FluentQt::StyleSheetManager::instance()->customStyleSheet(label, FluentQt::Theme::Dark);
        QVERIFY(lightQss.contains(QStringLiteral("#ff112233")));
        QVERIFY(darkQss.contains(QStringLiteral("#ffddeeff")));
    }

    void modelComboBoxTracksModelRowsDataAndIconVisibility()
    {
        FluentQt::ModelComboBox combo;
        QCOMPARE(combo.property("fqw").toString(), QStringLiteral("ModelComboBox"));
        QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("arrowAni") >= 0);
        QVERIFY(!combo.isPressed());
        QVERIFY(!combo.isHover());
        QVERIFY(combo.arrowAni() != nullptr);
        QVERIFY(combo.arrowAni()->metaObject()->indexOfProperty("y") >= 0);
        QCOMPARE(combo.arrowAni()->y(), 0.0);
        QCOMPARE(combo.dropMenu(), nullptr);
        auto *model = new QStandardItemModel(&combo);
        auto *alpha = new QStandardItem(QStringLiteral("Alpha"));
        alpha->setData(QStringLiteral("a"), Qt::UserRole);
        alpha->setIcon(FluentQt::icon(FluentQt::FluentIcon::Heart));
        auto *beta = new QStandardItem(QStringLiteral("Beta"));
        beta->setData(QStringLiteral("b"), Qt::UserRole);
        model->appendRow(alpha);
        model->appendRow(beta);

        combo.setModel(model);
        QCOMPARE(combo.count(), 2);
        QCOMPARE(combo.currentIndex(), 0);
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));
        QCOMPARE(combo.currentData().toString(), QStringLiteral("a"));
        QVERIFY(!combo.icon().isNull());

        combo.setIconVisible(false);
        QVERIFY(combo.icon().isNull());

        QSignalSpy indexSpy(&combo, &FluentQt::ModelComboBox::currentIndexChanged);
        combo.setCurrentIndex(1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));
        QVERIFY(indexSpy.count() >= 1);

        combo.setItemText(1, QStringLiteral("Beta Prime"));
        QCOMPARE(combo.currentText(), QStringLiteral("Beta Prime"));
        QCOMPARE(combo.findText(QStringLiteral("Beta Prime")), 1);
        QCOMPARE(combo.findData(QStringLiteral("a")), 0);

        combo.insertItem(0, QStringLiteral("Zero"));
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta Prime"));
        combo.removeItem(0);
        QCOMPARE(combo.currentIndex(), 1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta Prime"));

        FluentQt::ModelComboBox firstRemovalCombo;
        firstRemovalCombo.addItems({QStringLiteral("One"), QStringLiteral("Two")});
        QCOMPARE(firstRemovalCombo.currentIndex(), 0);
        QCOMPARE(firstRemovalCombo.currentText(), QStringLiteral("One"));
        firstRemovalCombo.removeItem(0);
        QCOMPARE(firstRemovalCombo.count(), 1);
        QCOMPARE(firstRemovalCombo.currentIndex(), 0);
        QCOMPARE(firstRemovalCombo.currentText(), QStringLiteral("Two"));
    }

    void editableModelComboBoxAddsEnteredTextToModel()
    {
        FluentQt::EditableModelComboBox combo;
        QCOMPARE(combo.property("fqw").toString(), QStringLiteral("EditableModelComboBox"));
        QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("dropButton") >= 0);
        QVERIFY(!combo.isPressed());
        QVERIFY(!combo.isHover());
        QCOMPARE(combo.isClearButtonEnabled(), false);
        QCOMPARE(combo.textMargins().right(), 29);
        QVERIFY(combo.dropButton() != nullptr);
        QCOMPARE(combo.dropButton()->size(), QSize(30, 25));
        QCOMPARE(combo.dropMenu(), nullptr);
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        QCOMPARE(combo.count(), 2);
        QCOMPARE(combo.currentIndex(), 0);

        combo.setText(QStringLiteral("Gamma"));
        QCOMPARE(combo.currentIndex(), -1);
        QVERIFY(QMetaObject::invokeMethod(&combo, "returnPressed"));
        QCOMPARE(combo.count(), 3);
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));
        QCOMPARE(combo.itemText(2), QStringLiteral("Gamma"));

        auto *completer = new QCompleter(QStringList{QStringLiteral("Alpha"), QStringLiteral("Beta")}, &combo);
        combo.setCompleter(completer);
        auto *menu = new FluentQt::CompleterMenu(&combo);
        combo.setCompleterMenu(menu);
        emit menu->activated(QStringLiteral("Beta"));
        QCOMPARE(combo.currentIndex(), 1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));

        FluentQt::EditableModelComboBox firstRemovalCombo;
        firstRemovalCombo.addItems({QStringLiteral("One"), QStringLiteral("Two")});
        QCOMPARE(firstRemovalCombo.currentIndex(), 0);
        QCOMPARE(firstRemovalCombo.currentText(), QStringLiteral("One"));
        firstRemovalCombo.removeItem(0);
        QCOMPARE(firstRemovalCombo.count(), 1);
        QCOMPARE(firstRemovalCombo.currentIndex(), 0);
        QCOMPARE(firstRemovalCombo.currentText(), QStringLiteral("Two"));
    }

  private:
    static FluentQt::ComboBoxMenu *findVisibleComboMenu()
    {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            auto *menu = qobject_cast<FluentQt::ComboBoxMenu *>(widget);
            if (menu && menu->isVisible()) {
                return menu;
            }
        }
        return nullptr;
    }

    static FluentQt::RoundMenu *findVisibleRoundMenu()
    {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            auto *menu = qobject_cast<FluentQt::RoundMenu *>(widget);
            if (menu && menu->isVisible()) {
                return menu;
            }
        }
        return nullptr;
    }
};

QTEST_MAIN(DisplayTest)

#include "tst_display.moc"
