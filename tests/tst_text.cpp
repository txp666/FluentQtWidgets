// Labels, line edits and completers, spin boxes and multi-line editors.

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QBuffer>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryDir>
#include <QtCore/QtMath>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>

#include <functional>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class TextTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void displayLabelHasItsRole()
    {
        FluentQt::DisplayLabel display(QStringLiteral("Display"));
        QCOMPARE(display.property("fqw").toString(), QStringLiteral("DisplayLabel"));
    }

    void imageLabelLoadsPixmapsAndPathsAndEmitsClicked()
    {
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
    }

    void avatarWidgetRadiusSetsItsSizeAndFont()
    {
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
        // Python's setRadius() sets a regular font whose pixel size is the radius.
        QCOMPARE(avatar.font().pixelSize(), 18);
        QCOMPARE(avatar.font().weight(), QFont::Normal);
        avatar.setRadius(8);
        QCOMPARE(avatar.font().pixelSize(), 8);
    }

    void avatarWidgetKeepsBothBackgroundColours()
    {
        FluentQt::AvatarWidget avatar;
        avatar.setBackgroundColor(QColor(QStringLiteral("#112233")), QColor(QStringLiteral("#445566")));
        QCOMPARE(avatar.lightBackgroundColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(avatar.darkBackgroundColor(), QColor(QStringLiteral("#445566")));
    }

    void avatarWidgetTakesPixmapsAndPaths()
    {
        FluentQt::AvatarWidget avatar;
        avatar.setRadius(18);
        QPixmap pixmap(12, 12);
        pixmap.fill(Qt::red);
        avatar.setPixmap(pixmap);
        QVERIFY(!avatar.imagePixmap().isNull());
        QCOMPARE(avatar.size(), QSize(36, 36));

        QTemporaryDir imageDir;
        QVERIFY(imageDir.isValid());
        const QString avatarPath = imageDir.filePath(QStringLiteral("avatar.png"));
        QVERIFY(pixmap.save(avatarPath));
        FluentQt::AvatarWidget pathAvatar(avatarPath);
        QVERIFY(!pathAvatar.imagePixmap().isNull());
    }

    void avatarWidgetLoadsHttpImages()
    {
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

    void pixmapLabelPaintsAndSizesToItsPixmap()
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

    // Python colours its labels black in light and white in dark theme, the caption too.
    void labelsUsePythonDefaultTextColours()
    {
        auto *manager = ThemeManager::instance();
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            manager->setTheme(theme);
            QWidget window;
            auto *layout = new QHBoxLayout(&window);
            const QList<FluentLabelBase *> labels = {new CaptionLabel(QStringLiteral("Caption"), &window),
                                                     new BodyLabel(QStringLiteral("Body"), &window),
                                                     new StrongBodyLabel(QStringLiteral("Strong"), &window),
                                                     new SubtitleLabel(QStringLiteral("Subtitle"), &window),
                                                     new TitleLabel(QStringLiteral("Title"), &window),
                                                     new LargeTitleLabel(QStringLiteral("Large"), &window),
                                                     new DisplayLabel(QStringLiteral("Display"), &window)};
            for (FluentLabelBase *label : labels) {
                layout->addWidget(label);
            }
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));
            const QColor expected = theme == Theme::Dark ? QColor(Qt::white) : QColor(Qt::black);
            for (const FluentLabelBase *label : labels) {
                QCOMPARE(label->palette().color(QPalette::WindowText), expected);
                QCOMPARE(theme == Theme::Dark ? label->darkColor() : label->lightColor(), expected);
            }
        }
    }

    // setTextColor() has to beat label.qss, which colours the label through its style sheet: set
    // through the palette, the colour was lost on polish and never painted.
    void labelTextColorWinsOverStyleSheet()
    {
        auto countPixels = [](QWidget &widget, const QColor &target) {
            return FqwTest::countPixels(widget.grab().toImage(), [&target](const QColor &color) {
                return qAbs(color.red() - target.red()) < 40 && qAbs(color.green() - target.green()) < 40 &&
                       qAbs(color.blue() - target.blue()) < 40;
            });
        };

        auto *manager = ThemeManager::instance();
        manager->setTheme(Theme::Light);
        const QColor light(220, 0, 0);
        const QColor dark(0, 0, 220);

        BodyLabel before(QStringLiteral("MMMM"));
        before.setTextColor(light, dark);
        before.show();
        QVERIFY(QTest::qWaitForWindowExposed(&before));
        BodyLabel after(QStringLiteral("MMMM"));
        after.show();
        QVERIFY(QTest::qWaitForWindowExposed(&after));
        after.setTextColor(light, dark);

        for (BodyLabel *label : {&before, &after}) {
            QCOMPARE(label->palette().color(QPalette::WindowText), light);
            QVERIFY(countPixels(*label, light) > 0);
        }

        manager->setTheme(Theme::Dark);
        QCoreApplication::processEvents();
        for (BodyLabel *label : {&before, &after}) {
            QCOMPARE(label->palette().color(QPalette::WindowText), dark);
            QVERIFY(countPixels(*label, dark) > 0);
            QCOMPARE(countPixels(*label, light), 0);
        }
    }

    void plainLabelLeavesContextMenuToParent()
    {
        QWidget parent;
        auto *label = new BodyLabel(QStringLiteral("Plain text"), &parent);
        parent.show();

        QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(2, 2), label->mapToGlobal(QPoint(2, 2)));
        event.accept();
        QApplication::sendEvent(label, &event);
        QVERIFY(!event.isAccepted());
    }

    void lineEditButtonKeepsActionIconsInFluentPaintPath()
    {

        FluentQt::LineEditButton button(FqwTest::solidIcon(Qt::red));
        button.resize(31, 23);

        QAction action(FqwTest::solidIcon(Qt::red), QStringLiteral("Action"), &button);
        button.setAction(&action);
        QVERIFY(button.icon().isNull());
        QVERIFY(FqwTest::countPixels(FqwTest::render(button), [](const QColor &color) {
                    return color.red() > 200 && color.green() < 80 && color.blue() < 80;
                }) > 80);

        action.setIcon(FqwTest::solidIcon(Qt::green));
        QVERIFY(button.icon().isNull());
        QVERIFY(FqwTest::countPixels(FqwTest::render(button), [](const QColor &color) {
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

    // LineEdit fixes its height at 33 px, as in Python. A style sheet's min-height sizes the
    // content box, so the old `min-height: 32px` plus the borders raised it to 34.
    void lineEditsKeepThePythonHeight()
    {
        auto *manager = ThemeManager::instance();
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            manager->setTheme(theme);
            QWidget window;
            auto *layout = new QHBoxLayout(&window);
            const QList<QLineEdit *> edits = {new LineEdit(&window), new SearchLineEdit(&window),
                                              new PasswordLineEdit(&window)};
            for (QLineEdit *edit : edits) {
                layout->addWidget(edit, 0, Qt::AlignTop);
            }
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));
            for (const QLineEdit *edit : edits) {
                QCOMPARE(edit->height(), 33);
            }
        }
    }

    void lineEditPlacesActionButtonsAndCompleterMenus()
    {
        FluentQt::LineEdit editor;
        QCOMPARE(editor.property("fqw").toString(), QStringLiteral("LineEdit"));
        QVERIFY(editor.clearButton());
        QVERIFY(editor.hBoxLayout());
        QCOMPARE(editor.leftButtons().size(), 0);
        QCOMPARE(editor.rightButtons().size(), 0);
        QCOMPARE(editor.completerMenu(), nullptr);
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
        QCOMPARE(searchEditor.searchButton()->size(), QSize(31, 23));
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
    }

    void passwordLineEditReflowsMarginsWhenViewButtonVisibilityChanges()
    {
        FluentQt::PasswordLineEdit editor;

        QCOMPARE(editor.property("fqw").toString(), QStringLiteral("PasswordLineEdit"));
        QCOMPARE(editor.echoMode(), QLineEdit::Password);
        QVERIFY(editor.viewButton());
        QVERIFY(editor.isViewPasswordButtonVisible());
        QVERIFY(editor.property("viewPasswordButtonVisible").toBool());
        QCOMPARE(editor.textMargins().right(), 30);

        editor.setClearButtonEnabled(true);
        QCOMPARE(editor.textMargins().right(), 58);

        editor.setViewPasswordButtonVisible(false);
        QCOMPARE(editor.textMargins().right(), 28);

        editor.setClearButtonEnabled(false);
        QCOMPARE(editor.textMargins().right(), 0);

        editor.setViewPasswordButtonVisible(true);
        QCOMPARE(editor.textMargins().right(), 30);

        // Also as a property.
        QVERIFY(editor.setProperty("viewPasswordButtonVisible", false));
        QVERIFY(!editor.isViewPasswordButtonVisible());
        QCOMPARE(editor.textMargins().right(), 0);
    }

    void editMenusOfferActionsForSelectionAndClipboard()
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
        // As in Python, every action but "Select all" has an icon.
        QVERIFY(!readOnlyMenu.actions().constFirst()->icon().isNull());
        QVERIFY(readOnlyMenu.actions().constLast()->icon().isNull());

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
        QVERIFY(textMenu.actions().constLast()->icon().isNull());

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

    // As in Python, keys typed while the completer menu is open go to the line edit and to the
    // list, so the arrow keys move through the completions.
    void completerMenuArrowKeysMoveThroughCompletions()
    {
        FluentQt::LineEdit lineEdit;
        lineEdit.setText(QStringLiteral("a"));

        QStandardItemModel model;
        model.appendRow(new QStandardItem(QStringLiteral("Alpha")));
        model.appendRow(new QStandardItem(QStringLiteral("Beta")));

        FluentQt::CompleterMenu menu(&lineEdit);
        QVERIFY(menu.setCompletion(&model));
        menu.view()->setCurrentRow(0);

        QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QApplication::sendEvent(&menu, &down);
        QCOMPARE(menu.view()->currentRow(), 1);

        QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QApplication::sendEvent(&menu, &up);
        QCOMPARE(menu.view()->currentRow(), 0);

        // On macOS the Up key also took the edit's cursor to the start, as it does in Python.
        lineEdit.end(false);
        QKeyEvent letter(QEvent::KeyPress, Qt::Key_L, Qt::NoModifier, QStringLiteral("l"));
        QApplication::sendEvent(&menu, &letter);
        QCOMPARE(lineEdit.text(), QStringLiteral("al"));
    }

    void deletedCompleterIsNotUsed()
    {
        LineEdit edit;
        edit.show();
        auto *completer = new QCompleter(QStringList{QStringLiteral("aa"), QStringLiteral("ab")});
        edit.setCompleter(completer);

        delete completer;
        QVERIFY(!edit.completer());
        edit.setText(QStringLiteral("a"));
        emit edit.textEdited(QStringLiteral("a"));
        QTest::qWait(150);
    }

    void completerMenuIsFreedWithLineEdit()
    {
        auto *edit = new LineEdit;
        edit->setCompleter(new QCompleter(QStringList{QStringLiteral("aa"), QStringLiteral("ab")}, edit));
        edit->show();
        edit->setText(QStringLiteral("a"));
        emit edit->textEdited(QStringLiteral("a"));
        QTRY_VERIFY(edit->completerMenu());

        const QPointer<CompleterMenu> menu = edit->completerMenu();
        delete edit;
        QVERIFY(menu.isNull());
    }

    void completerMenuDoesNotPileUpActions()
    {
        LineEdit edit;
        CompleterMenu menu(&edit);
        const auto ownActions = [&menu]() { return menu.findChildren<QAction *>(Qt::FindDirectChildrenOnly).size(); };

        menu.setItems({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")});
        FqwTest::flushDeletes();
        const auto afterFirst = ownActions();

        menu.setItems({QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f")});
        FqwTest::flushDeletes();
        QCOMPARE(ownActions(), afterFirst);
        QCOMPARE(menu.menuActions().size(), 3);
    }

    void spinBoxesTrackErrorReadOnlyAndAcceleration()
    {
        auto verifySharedState = [](auto &spin, const QString &role) {
            QCOMPARE(spin.property("fqw").toString(), role);
            QVERIFY(spin.metaObject()->indexOfProperty("isError") >= 0);
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
            // Not a popup (unlike qfluentwidgets): it opens on focus-in, and a popup would take
            // the keyboard away from the spin box being typed into.
            QCOMPARE(flyout->isPopupMode(), false);
            QCOMPARE(flyout->property("popupMode").toBool(), false);
            QCOMPARE(flyout->windowFlags() & Qt::WindowType_Mask, Qt::Tool);
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

            auto *layer = editor.template findChild<QWidget *>(QStringLiteral("editLayer"));
            QVERIFY(layer != nullptr);
            QCOMPARE(editor.layer(), layer);
            QCOMPARE(editor.property("layer").template value<QWidget *>(), layer);
            QCOMPARE(layer->property("fqw").toString(), QStringLiteral("EditLayer"));
            QCOMPARE(layer->testAttribute(Qt::WA_TransparentForMouseEvents), true);

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

    void multilineFocusLineStaysPutWhileScrolling()
    {
        auto verifyEditor = [](auto &editor) {
            QStringList lines;
            for (int index = 0; index < 40; ++index) {
                lines.append(QStringLiteral("line %1").arg(index));
            }
            editor.setPlainText(lines.join(QLatin1Char('\n')));
            editor.resize(240, 90);
            editor.show();
            QVERIFY(QTest::qWaitForWindowExposed(&editor));
            auto *layer = editor.layer();
            QVERIFY(layer != nullptr);
            // Compare where both sit inside the editor, whatever their parents.
            const auto inEditor = [&editor](const QWidget *widget) {
                return QRect(widget->mapTo(&editor, QPoint()), widget->size());
            };
            QCOMPARE(inEditor(layer), editor.rect());

            // Scrolling moves the viewport's children with the content, so the
            // underline must not be one of them.
            QVERIFY(editor.verticalScrollBar()->maximum() > 0);
            editor.verticalScrollBar()->setValue(editor.verticalScrollBar()->maximum());
            QCoreApplication::processEvents();
            QCOMPARE(inEditor(layer), editor.rect());

            editor.resize(300, 120);
            QCoreApplication::processEvents();
            QCOMPARE(inEditor(layer), editor.rect());
        };

        FluentQt::TextEdit textEdit;
        verifyEditor(textEdit);

        FluentQt::PlainTextEdit plainTextEdit;
        verifyEditor(plainTextEdit);

        FluentQt::TextBrowser textBrowser;
        verifyEditor(textBrowser);
    }

    // As in Python, the multi-line editors draw their 2 px focus line along the bottom of the whole
    // box. Kept over the viewport, it sat 3 px higher and was inset by the padding.
    void multilineFocusLineSpansTheBox()
    {
        // One window for all three editors: it is activated once, and focus then moves inside it.
        QWidget window;
        auto *layout = new QVBoxLayout(&window);
        FluentQt::TextEdit textEdit(&window);
        FluentQt::PlainTextEdit plainTextEdit(&window);
        FluentQt::TextBrowser textBrowser(&window);
        for (QWidget *editor : {static_cast<QWidget *>(&textEdit), static_cast<QWidget *>(&plainTextEdit),
                                static_cast<QWidget *>(&textBrowser)}) {
            editor->setFixedSize(240, 90);
            layout->addWidget(editor);
        }
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        window.activateWindow();
        if (!QTest::qWaitForWindowActive(&window)) {
            // Natively, macOS keeps another application's window active while it holds the focus.
            QSKIP("The window cannot become active while another application has the focus");
        }

        auto verifyEditor = [](auto &editor) {
            editor.setFocus();
            QTRY_VERIFY(editor.hasFocus());
            QCOMPARE(QRect(editor.layer()->mapTo(&editor, QPoint()), editor.layer()->size()), editor.rect());

            const QColor accent = themeColor();
            const QImage image = editor.grab().toImage();
            int top = image.height();
            int bottom = -1;
            int left = image.width();
            int right = -1;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    if (qAbs(color.red() - accent.red()) < 30 && qAbs(color.green() - accent.green()) < 30 &&
                        qAbs(color.blue() - accent.blue()) < 30) {
                        top = qMin(top, y);
                        bottom = qMax(bottom, y);
                        left = qMin(left, x);
                        right = qMax(right, x);
                    }
                }
            }
            // In device pixels: the last two rows of the box, all but the rounded ends.
            const qreal dpr = image.devicePixelRatio();
            QVERIFY2(top >= qFloor(88 * dpr) - 1, qPrintable(QString::number(top)));
            QCOMPARE(bottom, image.height() - 1);
            QVERIFY2(left <= qCeil(3 * dpr), qPrintable(QString::number(left)));
            QVERIFY2(right >= image.width() - 1 - qCeil(3 * dpr), qPrintable(QString::number(right)));
        };

        verifyEditor(textEdit);
        verifyEditor(plainTextEdit);
        verifyEditor(textBrowser);
    }

    void ctrlWheelZoomsReadOnlyTextEdit()
    {
        PlainTextEdit edit;
        edit.setPlainText(QStringLiteral("zoom me"));
        edit.setReadOnly(true);
        edit.resize(300, 200);
        edit.show();

        const QFont before = edit.document()->defaultFont();
        const int beforeSize = before.pixelSize() > 0 ? before.pixelSize() : qRound(before.pointSizeF());
        QVERIFY(FqwTest::sendCtrlWheel(edit.viewport(), 120));
        const QFont after = edit.document()->defaultFont();
        const int afterSize = after.pixelSize() > 0 ? after.pixelSize() : qRound(after.pointSizeF());
        QCOMPARE(afterSize, beforeSize + 1);

        // Like QPlainTextEdit, editable text does not zoom.
        PlainTextEdit editable;
        editable.setPlainText(QStringLiteral("no zoom"));
        editable.resize(300, 200);
        editable.show();
        const QFont editableBefore = editable.document()->defaultFont();
        FqwTest::sendCtrlWheel(editable.viewport(), 120);
        QCOMPARE(editable.document()->defaultFont(), editableBefore);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(TextTest)
#include "tst_text.moc"
