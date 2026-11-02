// Dialogs, message boxes, the colour and folder dialogs, flyouts and teaching tips.

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class DialogFlyoutTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void dialogsBuildFramelessViewsAndEmitResults()
    {
        FluentQt::Dialog dialog(QStringLiteral("Title"), QStringLiteral("Message"));
        QCOMPARE(dialog.property("fqw").toString(), QStringLiteral("DialogPopup"));
        QVERIFY(dialog.testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(dialog.windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(dialog.windowFlags() & Qt::NoDropShadowWindowHint);
        QVERIFY(dialog.findChildren<QDialogButtonBox *>().isEmpty());

        auto *view = dialog.findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(view != nullptr);
        QCOMPARE(view->property("fqw").toString(), QStringLiteral("Dialog"));
        QCOMPARE(dialog.view(), view);
        QCOMPARE(dialog.property("view").value<QFrame *>(), dialog.view());
        QCOMPARE(dialog.widget(), dialog.view());
        QCOMPARE(dialog.property("widget").value<QFrame *>(), dialog.widget());
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(view->graphicsEffect()) != nullptr);
        QVERIFY(dialog.titleLabel() != nullptr);
        QVERIFY(dialog.windowTitleLabel() != nullptr);
        QVERIFY(dialog.contentLabel() != nullptr);
        QCOMPARE(dialog.messageLabel(), dialog.contentLabel());
        QCOMPARE(dialog.windowTitleLabel()->objectName(), QStringLiteral("windowTitleLabel"));
        QCOMPARE(dialog.titleLabel()->objectName(), QStringLiteral("titleLabel"));
        QCOMPARE(dialog.contentLabel()->objectName(), QStringLiteral("contentLabel"));
        QCOMPARE(dialog.windowTitleLabel()->text(), QStringLiteral("Title"));
        QCOMPARE(dialog.titleLabel()->text(), QStringLiteral("Title"));
        QCOMPARE(dialog.contentLabel()->text(), QStringLiteral("Message"));
        QVERIFY(!dialog.contentLabel()->wordWrap());
        QCOMPARE(dialog.property("windowTitleLabel").value<QLabel *>(), dialog.windowTitleLabel());
        dialog.setTitleBarVisible(false);
        QVERIFY(dialog.windowTitleLabel()->isHidden());
        dialog.setTitleBarVisible(true);
        QVERIFY(!dialog.windowTitleLabel()->isHidden());
        QVERIFY(dialog.buttonGroup() != nullptr);
        QCOMPARE(dialog.buttonGroup()->objectName(), QStringLiteral("buttonGroup"));
        QCOMPARE(dialog.buttonGroup()->parentWidget(), dialog.view());
        QCOMPARE(dialog.buttonGroup()->height(), 81);
        QVERIFY(dialog.vBoxLayout() != nullptr);
        QVERIFY(dialog.viewLayout() != nullptr);
        QVERIFY(dialog.textLayout() != nullptr);
        QCOMPARE(dialog.viewLayout(), dialog.textLayout());
        QVERIFY(dialog.buttonLayout() != nullptr);
        QCOMPARE(dialog.textLayout()->contentsMargins(), QMargins(24, 24, 24, 24));
        QCOMPARE(dialog.buttonLayout()->contentsMargins(), QMargins(24, 24, 24, 24));
        QCOMPARE(dialog.property("titleLabel").value<QLabel *>(), dialog.titleLabel());
        QCOMPARE(dialog.property("contentLabel").value<QLabel *>(), dialog.contentLabel());
        QCOMPARE(dialog.property("buttonGroup").value<QFrame *>(), dialog.buttonGroup());
        QCOMPARE(dialog.property("vBoxLayout").value<QVBoxLayout *>(), dialog.vBoxLayout());
        QCOMPARE(dialog.property("viewLayout").value<QVBoxLayout *>(), dialog.viewLayout());
        QCOMPARE(dialog.property("textLayout").value<QVBoxLayout *>(), dialog.textLayout());
        QCOMPARE(dialog.property("buttonLayout").value<QHBoxLayout *>(), dialog.buttonLayout());
        QVERIFY(dialog.acceptButton() != nullptr);
        QVERIFY(dialog.cancelButton() != nullptr);
        QCOMPARE(dialog.yesButton(), dialog.acceptButton());
        QCOMPARE(dialog.property("yesButton").value<QPushButton *>(), dialog.yesButton());
        QCOMPARE(dialog.property("cancelButton").value<QPushButton *>(), dialog.cancelButton());
        QCOMPARE(dialog.cancelButton()->objectName(), QStringLiteral("cancelButton"));
        QCOMPARE(dialog.acceptButton()->property("fqw").toString(), QStringLiteral("PrimaryPushButton"));
        QCOMPARE(dialog.cancelButton()->property("fqw").toString(), QStringLiteral("PushButton"));
        dialog.setContentCopyable(true);
        QVERIFY(dialog.contentLabel()->textInteractionFlags() & Qt::TextSelectableByMouse);
        dialog.setContentCopyable(false);
        QCOMPARE(dialog.contentLabel()->textInteractionFlags(), Qt::NoTextInteraction);

        QSignalSpy acceptedSpy(&dialog, &QDialog::accepted);
        QSignalSpy yesSpy(&dialog, &FluentQt::Dialog::yesSignal);
        dialog.acceptButton()->click();
        QCOMPARE(acceptedSpy.count(), 1);
        QCOMPARE(yesSpy.count(), 1);

        FluentQt::Dialog hiddenButtonDialog(QStringLiteral("Title"), QStringLiteral("Message"));
        hiddenButtonDialog.hideYesButton();
        QVERIFY(!hiddenButtonDialog.yesButton()->isVisible());
        hiddenButtonDialog.hideCancelButton();
        QVERIFY(!hiddenButtonDialog.cancelButton()->isVisible());

        FluentQt::MessageBoxBase customBase;
        customBase.viewLayout()->addWidget(new FluentQt::LineEdit(customBase.widget()));
        QCOMPARE(customBase.widget(), customBase.view());
        QVERIFY(customBase.windowMask() != nullptr);
        QCOMPARE(customBase.windowMask()->objectName(), QStringLiteral("windowMask"));

        class ValidatingMessageBox : public FluentQt::MessageBoxBase
        {
          public:
            using FluentQt::MessageBoxBase::MessageBoxBase;
            void setValid(bool valid) { m_valid = valid; }

          protected:
            bool validate() const override { return m_valid; }

          private:
            bool m_valid = false;
        };

        ValidatingMessageBox validatingDialog;
        QSignalSpy validatingAcceptedSpy(&validatingDialog, &QDialog::accepted);
        QSignalSpy validatingYesSpy(&validatingDialog, &FluentQt::MessageBoxBase::yesSignal);
        validatingDialog.yesButton()->click();
        QCOMPARE(validatingAcceptedSpy.count(), 0);
        QCOMPARE(validatingYesSpy.count(), 0);
        validatingDialog.setValid(true);
        validatingDialog.yesButton()->click();
        QTRY_COMPARE(validatingAcceptedSpy.count(), 1);
        QCOMPARE(validatingYesSpy.count(), 1);

        FluentQt::MessageBox messageBox(QStringLiteral("Title"), QStringLiteral("Message"));
        QCOMPARE(messageBox.property("fqw").toString(), QStringLiteral("MessageBoxPopup"));
        messageBox.setClosableOnMaskClicked(true);
        messageBox.setDraggable(true);
        QVERIFY(messageBox.isClosableOnMaskClicked());
        QVERIFY(messageBox.isDraggable());
        QVERIFY(messageBox.windowMask() != nullptr);
        QCOMPARE(messageBox.windowMask()->objectName(), QStringLiteral("windowMask"));
        QCOMPARE(messageBox.maskColor(), QColor(0, 0, 0, 76));
        auto *messageView = messageBox.findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(messageView != nullptr);
        QCOMPARE(messageView->property("fqw").toString(), QStringLiteral("MessageBox"));
        QCOMPARE(messageView->objectName(), QStringLiteral("centerWidget"));
        QCOMPARE(messageBox.buttonGroup()->parentWidget(), messageBox.view());
        QCOMPARE(messageBox.titleLabel()->text(), QStringLiteral("Title"));
        QCOMPARE(messageBox.contentLabel()->text(), QStringLiteral("Message"));
        QVERIFY(!messageBox.contentLabel()->wordWrap());

        messageBox.show();
        QVERIFY(QTest::qWaitForWindowExposed(&messageBox));
        QSignalSpy messageAcceptedSpy(&messageBox, &QDialog::accepted);
        QSignalSpy messageYesSpy(&messageBox, &FluentQt::MessageBoxBase::yesSignal);
        QSignalSpy rejectedSpy(&messageBox, &QDialog::rejected);
        QSignalSpy cancelSpy(&messageBox, &FluentQt::MessageBoxBase::cancelSignal);
        QTest::mouseMove(messageBox.acceptButton(), messageBox.acceptButton()->rect().center());
        QTest::mouseMove(messageBox.cancelButton(), messageBox.cancelButton()->rect().center());
        QCOMPARE(messageAcceptedSpy.count(), 0);
        QCOMPARE(messageYesSpy.count(), 0);
        QCOMPARE(rejectedSpy.count(), 0);
        QCOMPARE(cancelSpy.count(), 0);
        QTest::mouseClick(messageBox.cancelButton(), Qt::LeftButton);
        QTRY_COMPARE(rejectedSpy.count(), 1);
        QCOMPARE(cancelSpy.count(), 1);

        QWidget maskParent;
        maskParent.resize(520, 360);
        maskParent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&maskParent));

        FluentQt::MessageBox maskedBox(QStringLiteral("Mask"), QStringLiteral("Parent overlay"), &maskParent);
        maskedBox.setClosableOnMaskClicked(true);
        maskedBox.show();
        QVERIFY(QTest::qWaitForWindowExposed(&maskedBox));
        QCOMPARE(maskedBox.geometry(), maskParent.geometry());
        QCOMPARE(maskedBox.windowMask()->size(), maskedBox.size());
        QVERIFY(maskedBox.widget()->geometry().center().x() > 0);
        QVERIFY(maskedBox.widget()->geometry().center().y() > 0);

        maskParent.resize(640, 420);
        QCoreApplication::processEvents();
        QTRY_COMPARE(maskedBox.geometry(), maskParent.geometry());
        QCOMPARE(maskedBox.windowMask()->size(), maskedBox.size());

        QSignalSpy maskRejectedSpy(&maskedBox, &QDialog::rejected);
        QTest::mouseClick(maskedBox.windowMask(), Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
        QTRY_COMPARE(maskRejectedSpy.count(), 1);

        FluentQt::MessageDialog messageDialog(QStringLiteral("Title"), QStringLiteral("Message"));
        QCOMPARE(messageDialog.property("fqw").toString(), QStringLiteral("MessageDialog"));
        auto *messageDialogView = messageDialog.findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(messageDialogView != nullptr);
        QCOMPARE(messageDialogView->property("fqw").toString(), QStringLiteral("MessageDialog"));
    }

    void returnKeyAcceptsDialogsLikePython()
    {
        // The buttons keep QPushButton's autoDefault, as in qfluentwidgets, so Return on the
        // focused OK button accepts the dialog.
        QWidget parent;
        parent.resize(600, 400);
        parent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&parent));

        FluentQt::MessageBox box(QStringLiteral("Title"), QStringLiteral("Message"), &parent);
        QVERIFY(box.yesButton()->autoDefault());
        QVERIFY(box.cancelButton()->autoDefault());
        QSignalSpy boxAccepted(&box, &QDialog::accepted);
        box.show();
        QVERIFY(QTest::qWaitForWindowExposed(&box));
        // The button row neither stretches nor squeezes the buttons (32 px in Python's font).
        QCOMPARE(box.yesButton()->height(), box.yesButton()->sizeHint().height());
        QTest::keyClick(box.yesButton(), Qt::Key_Return);
        QTRY_COMPARE(boxAccepted.count(), 1);

        FluentQt::Dialog dialog(QStringLiteral("Title"), QStringLiteral("Message"), &parent);
        QVERIFY(dialog.yesButton()->autoDefault());
        QVERIFY(dialog.cancelButton()->autoDefault());
        QSignalSpy dialogAccepted(&dialog, &QDialog::accepted);
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        QTest::keyClick(dialog.yesButton(), Qt::Key_Return);
        QTRY_COMPARE(dialogAccepted.count(), 1);
    }

    void messageBoxBaseButtonsHaveNoMinimumWidth()
    {
        // qfluentwidgets sets no minimum width on the OK and Cancel buttons: in an empty
        // MessageBoxBase with one-letter labels they come out about 36 and 31 px wide.
        QWidget parent;
        parent.resize(600, 400);
        parent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&parent));
        FluentQt::MessageBoxBase box(&parent);
        box.yesButton()->setText(QStringLiteral("Y"));
        box.cancelButton()->setText(QStringLiteral("N"));
        box.show();
        QVERIFY(QTest::qWaitForWindowExposed(&box));
        QCOMPARE(box.yesButton()->minimumWidth(), 0);
        QCOMPARE(box.cancelButton()->minimumWidth(), 0);
        QVERIFY(box.yesButton()->width() < 60);
        QVERIFY(box.widget()->width() < 160);

        FluentQt::Dialog dialog(QStringLiteral("T"), QStringLiteral("M"), &parent);
        QCOMPARE(dialog.yesButton()->minimumWidth(), 0);
        QCOMPARE(dialog.cancelButton()->minimumWidth(), 0);
    }

    void galleryDialogKeepsItsContentOnOneLineAndCentred()
    {
        QWidget dialogParent;
        dialogParent.resize(1000, 640);
        dialogParent.move(80, 90);
        dialogParent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialogParent));
        // The dialog wraps its content to a width taken from the parent's: in a narrower parent the
        // content wraps, as it should.
        if (dialogParent.width() < 1000) {
            QSKIP("The screen is too narrow for a 1000-pixel-wide window");
        }
        FluentQt::Dialog galleryDialog(
            QStringLiteral("This is a frameless message dialog"),
            QStringLiteral("If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will "
                           "automatically wrap like this."),
            &dialogParent);
        QVERIFY(!galleryDialog.contentLabel()->wordWrap());
        QVERIFY(!galleryDialog.contentLabel()->text().contains(QLatin1Char('\n')));
        galleryDialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&galleryDialog));
        const QPoint viewCenter = galleryDialog.widget()->mapToGlobal(galleryDialog.widget()->rect().center());
        const QPoint parentCenter = dialogParent.frameGeometry().center();
        QVERIFY(qAbs(viewCenter.x() - parentCenter.x()) <= 2);
        QVERIFY(qAbs(viewCenter.y() - parentCenter.y()) <= 2);
        galleryDialog.close();
    }

    void dialogFitsItsContentInsideItsWindowLikePython()
    {
        // The Gallery pinned the view to 240 x 192, taking qfluentwidgets' resize(240, 192) of the
        // window for a fixed view size: the content needed more, so the view stuck out of the window
        // and its button row was cut off. Python's 240 x 192 is only a floor the layout grows past.
        QWidget parent;
        parent.resize(1000, 640);
        parent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&parent));
        if (parent.width() < 1000) {
            QSKIP("The screen is too narrow for a 1000-pixel-wide window");
        }

        const QString galleryTitle = QStringLiteral("This is a frameless message dialog");
        const QString galleryContent =
            QStringLiteral("If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will "
                           "automatically wrap like this.");
        struct Case
        {
            QString title;
            QString content;
        };
        const QList<Case> cases = {
            {galleryTitle, galleryContent},
            {QStringLiteral("这是一个无边框对话框"),
             QStringLiteral("如果消息框的内容特别长，它会像这样自动换行。\n第二行\n第三行")},
            {QStringLiteral("T"), QStringLiteral("M")},
        };

        int pythonSizesCompared = 0;
        for (const bool dark : {false, true}) {
            FluentQt::ThemeManager::instance()->setTheme(dark ? Theme::Dark : Theme::Light);
            for (const Case &c : cases) {
                FluentQt::Dialog dialog(c.title, c.content, &parent);
                dialog.show();
                QVERIFY(QTest::qWaitForWindowExposed(&dialog));
                const QString where = QStringLiteral("%1 / %2").arg(dark ? "dark" : "light", c.title);
                const QByteArray whereText = where.toUtf8();

                // The view, its shadow and the frame around it fit in the window.
                const QRect view = dialog.widget()->geometry();
                const QRect shadow =
                    dialog.widget()->graphicsEffect()->boundingRectFor(view).toAlignedRect().adjusted(-1, -1, 1, 1);
                QVERIFY2(dialog.rect().contains(shadow), whereText.constData());
                QVERIFY2(view.width() >= 240 && view.height() >= 192, whereText.constData());

                // Every part of the dialog lies inside the view, the button row at its foot.
                for (QWidget *part :
                     {static_cast<QWidget *>(dialog.windowTitleLabel()), static_cast<QWidget *>(dialog.titleLabel()),
                      static_cast<QWidget *>(dialog.contentLabel()), static_cast<QWidget *>(dialog.buttonGroup())}) {
                    QVERIFY2(dialog.widget()->rect().contains(part->geometry()), whereText.constData());
                    if (qobject_cast<QLabel *>(part)) {
                        QVERIFY2(part->height() >= part->minimumSizeHint().height(), whereText.constData());
                    }
                }
                for (QWidget *button :
                     {static_cast<QWidget *>(dialog.yesButton()), static_cast<QWidget *>(dialog.cancelButton())}) {
                    QVERIFY2(dialog.buttonGroup()->rect().contains(button->geometry()), whereText.constData());
                }
                QCOMPARE(dialog.buttonGroup()->height(), 81);
                QCOMPARE(dialog.buttonGroup()->geometry().bottom(), view.height() - 1);
                QCOMPARE(dialog.buttonGroup()->width(), view.width());

                // Python's offsets: the title bar at the top edge, the texts 24 px in.
                QCOMPARE(dialog.windowTitleLabel()->geometry().topLeft(), QPoint(0, 0));
                QCOMPARE(dialog.windowTitleLabel()->width(), view.width());
                QCOMPARE(dialog.titleLabel()->geometry().left(), 24);
                QCOMPARE(dialog.titleLabel()->geometry().top(), dialog.windowTitleLabel()->height() + 24);
                QCOMPARE(dialog.yesButton()->geometry().left(), 24);
                QCOMPARE(dialog.cancelButton()->geometry().right(), view.width() - 1 - 24);

                // Measured natively from qfluentwidgets 1.11.3 on macOS (a 29 px title bar, 20 px
                // text lines); text widths need Python's font, so only there.
                const QSize pythonSize = c.title == QLatin1String("T") ? QSize(240, 218)
                                         : c.title == galleryTitle     ? QSize(770, 218)
                                                                       : QSize(356, 258);
                if (c.title == QLatin1String("T")) {
                    // qfluentwidgets' 240 px floor holds; the height is the content's.
                    QCOMPARE(view.size(), pythonSize);
                } else if (QFontInfo(dialog.contentLabel()->font()).family() == QLatin1String("PingFang SC")) {
                    QCOMPARE(dialog.windowTitleLabel()->height(), 29);
                    QCOMPARE(view.size(), pythonSize);
                    ++pythonSizesCompared;
                }
                dialog.close();
            }
        }
        if (pythonSizesCompared == 0) {
            qInfo("Python's text-dependent sizes were not compared: PingFang SC is not the dialog font here");
        }
    }

    void dialogFadeInDropsOpacityEffectBeforeButtonHover()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = manager->theme();
        manager->setTheme(FluentQt::Theme::Light);

        FluentQt::Dialog dialog(QStringLiteral("Title"), QStringLiteral("Message"));
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        QTRY_VERIFY(dialog.graphicsEffect() == nullptr);

        auto hasPaintedPixel = [](QWidget *widget) {
            const QImage image = widget->grab().toImage();
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    if (color.alpha() > 0 &&
                        (color.red() < 245 || color.green() < 245 || color.blue() < 245)) {
                        return true;
                    }
                }
            }
            return false;
        };

        auto hoverKeepsButtonPainted = [&](QPushButton *button) {
            QTest::mouseMove(button, button->rect().center());
            QCoreApplication::processEvents();
            QTest::qWait(20);
            return button->isVisible() && hasPaintedPixel(button);
        };

        QVERIFY2(hoverKeepsButtonPainted(dialog.acceptButton()),
                 "Dialog accept button should stay painted after hover");
        QVERIFY2(hoverKeepsButtonPainted(dialog.cancelButton()),
                 "Dialog cancel button should stay painted after hover");
        QVERIFY(qobject_cast<QGraphicsOpacityEffect *>(dialog.graphicsEffect()) == nullptr);

        manager->setTheme(previousTheme);
    }

    void maskedDialogFadeKeepsNativeWindowOpaque()
    {
        FluentQt::MessageBox dialog(QStringLiteral("Title"), QStringLiteral("Message"));
        dialog.show();
        QCoreApplication::processEvents();

        QCOMPARE(dialog.windowOpacity(), 1.0);
        QVERIFY(qobject_cast<QGraphicsOpacityEffect *>(dialog.graphicsEffect()) != nullptr);
        QTRY_VERIFY(dialog.graphicsEffect() == nullptr);
        QCOMPARE(dialog.windowOpacity(), 1.0);
    }

    void messageBoxWidensForAddedButtons()
    {
        QWidget parent;
        parent.resize(800, 600);
        parent.show();
        FluentQt::MessageBox box(QStringLiteral("保存更改？"), QStringLiteral("“未命名”有未保存的更改。"), &parent);
        box.yesButton()->setText(QStringLiteral("保存"));
        box.cancelButton()->setText(QStringLiteral("取消"));
        auto *discard = new FluentQt::PushButton(QStringLiteral("不保存"), box.buttonGroup());
        box.buttonLayout()->insertWidget(1, discard);
        box.show();
        QVERIFY(QTest::qWaitForWindowExposed(&box));

        const QRect yes = box.yesButton()->geometry();
        const QRect middle = discard->geometry();
        const QRect cancel = box.cancelButton()->geometry();
        QVERIFY(yes.right() < middle.left());
        QVERIFY(middle.right() < cancel.left());
        QVERIFY(cancel.right() < box.buttonGroup()->width());
    }

    void maskDialogFinishesOnce()
    {
        QWidget parent;
        parent.resize(800, 600);
        parent.show();
        auto *dialog = new MessageBoxBase(&parent);
        QSignalSpy finished(dialog, &QDialog::finished);
        QSignalSpy accepted(dialog, &QDialog::accepted);
        QSignalSpy rejected(dialog, &QDialog::rejected);
        dialog->show();
        QTest::qWait(50);

        dialog->accept();
        dialog->reject();
        QTRY_COMPARE(finished.count(), 1);
        QTest::qWait(200);
        QCOMPARE(finished.count(), 1);
        QCOMPARE(accepted.count(), 1);
        QCOMPARE(rejected.count(), 0);
    }

    void colorDialogSizesAndCommitsOnlyOnAccept()
    {
        FluentQt::ColorDialog alphaDialog(QColor(QStringLiteral("#5012aaa2")),
                                          QStringLiteral("Background Color"), nullptr, true);
        QCOMPARE(alphaDialog.property("fqw").toString(), QStringLiteral("ColorDialogPopup"));
        QVERIFY(alphaDialog.testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(alphaDialog.windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(alphaDialog.windowFlags() & Qt::NoDropShadowWindowHint);
        QVERIFY(alphaDialog.widget() != nullptr);
        QCOMPARE(alphaDialog.widget()->property("fqw").toString(), QStringLiteral("ColorDialog"));
        QCOMPARE(alphaDialog.widget()->size(), QSize(488, 736));
        QCOMPARE(alphaDialog.widget()->minimumSize(), QSize(488, 736));
        QCOMPARE(alphaDialog.widget()->maximumSize(), QSize(488, 736));
        QCOMPARE(alphaDialog.scrollWidget()->size(), QSize(440, 600));
        QCOMPARE(alphaDialog.huePanel()->size(), QSize(256, 256));
        QVERIFY(alphaDialog.brightnessSlider() != nullptr);
        QCOMPARE(alphaDialog.hexLineEdit()->text(), QStringLiteral("5012aaa2"));
        QVERIFY(!alphaDialog.opacityLineEdit()->isHidden());
        QVERIFY(alphaDialog.findChildren<QColorDialog *>().isEmpty());

        QSignalSpy dialogSpy(&alphaDialog, &FluentQt::ColorDialog::colorChanged);
        alphaDialog.setColor(QColor(QStringLiteral("#ff445566")));
        alphaDialog.cancelButton()->click();
        QCOMPARE(dialogSpy.count(), 0);

        FluentQt::ColorDialog acceptDialog(QColor(QStringLiteral("#112233")),
                                           QStringLiteral("Background Color"));
        QSignalSpy acceptSpy(&acceptDialog, &FluentQt::ColorDialog::colorChanged);
        acceptDialog.setColor(QColor(QStringLiteral("#445566")));
        acceptDialog.yesButton()->click();
        QCOMPARE(acceptSpy.count(), 1);
        QCOMPARE(acceptSpy.takeFirst().at(0).value<QColor>(), QColor(QStringLiteral("#445566")));
        QCOMPARE(acceptDialog.widget()->size(), QSize(488, 696));
        QCOMPARE(acceptDialog.widget()->minimumSize(), QSize(488, 696));
        QCOMPARE(acceptDialog.widget()->maximumSize(), QSize(488, 696));
        QCOMPARE(acceptDialog.scrollWidget()->size(), QSize(440, 560));
        QVERIFY(acceptDialog.opacityLineEdit()->isHidden());

        FluentQt::ColorSettingCard card(QColor(QStringLiteral("#112233")), FluentQt::FluentIcon::Heart,
                                        QStringLiteral("Accent"));
        FluentQt::ColorPickerButton picker(QColor(QStringLiteral("#5012aaa2")),
                                           QStringLiteral("Background Color"), nullptr, true);
        QCOMPARE(picker.title(), QStringLiteral("Background Color"));
        QVERIFY(picker.isAlphaEnabled());
        QCOMPARE(picker.toolTip(), QStringLiteral("Choose Background Color"));

        card.show();
        QVERIFY(QTest::qWaitForWindowExposed(&card));

        QSignalSpy colorSpy(&card, &FluentQt::ColorSettingCard::colorChanged);
        bool dialogInspected = false;
        QPointer<FluentQt::ColorDialog> dialogGuard;
        QTimer::singleShot(0, &card, [&card]() { card.colorPicker()->click(); });
        QTimer::singleShot(100, &card, [&]() {
            auto *popup = FqwTest::findVisibleTopLevelByRole(QStringLiteral("ColorDialogPopup"));
            auto *dialog = qobject_cast<FluentQt::ColorDialog *>(popup);
            if (!dialog) {
                return;
            }

            dialogInspected = true;
            dialogGuard = dialog;
            QVERIFY(dialog->testAttribute(Qt::WA_TranslucentBackground));
            QVERIFY(dialog->windowFlags() & Qt::NoDropShadowWindowHint);
            QCOMPARE(dialog->widget()->property("fqw").toString(), QStringLiteral("ColorDialog"));
            QCOMPARE(dialog->widget()->size(), QSize(488, 696));
            QCOMPARE(dialog->scrollWidget()->size(), QSize(440, 560));
            QCOMPARE(dialog->buttonGroup()->size(), QSize(486, 81));
            QCOMPARE(dialog->scrollArea()->verticalScrollBar()->maximum(), 0);
            QCOMPARE(dialog->findChildren<FluentQt::BrightnessSlider *>().size(), 1);
            QVERIFY(dialog->findChildren<QColorDialog *>().isEmpty());

            dialog->redLineEdit()->setFocus();
            dialog->redLineEdit()->clear();
            QTest::keyClicks(dialog->redLineEdit(), QStringLiteral("68"));
            QCOMPARE(card.color(), QColor(QStringLiteral("#112233")));
            QCOMPARE(colorSpy.count(), 0);
            dialog->yesButton()->click();
        });
        QTRY_COMPARE(colorSpy.count(), 1);
        QVERIFY(dialogInspected);
        QCOMPARE(card.color().red(), 68);
        QVERIFY(dialogGuard.isNull() || !dialogGuard->isVisible());
        QCOMPARE(colorSpy.count(), 1);

        FluentQt::CustomColorSettingCard customCard(QColor(QStringLiteral("#005fb8")), QColor(QStringLiteral("#112233")),
                                                    FluentQt::FluentIcon::Heart, QStringLiteral("Custom accent"));
        customCard.setAlphaEnabled(true);
        customCard.show();
        QVERIFY(QTest::qWaitForWindowExposed(&customCard));
        bool alphaDialogInspected = false;
        QTimer::singleShot(0, &customCard, [&customCard]() { customCard.colorPicker()->click(); });
        QTimer::singleShot(100, &customCard, [&]() {
            auto *alphaPopup = FqwTest::findVisibleTopLevelByRole(QStringLiteral("ColorDialogPopup"));
            auto *alphaPopupDialog = qobject_cast<FluentQt::ColorDialog *>(alphaPopup);
            if (!alphaPopupDialog) {
                return;
            }

            alphaDialogInspected = true;
            QCOMPARE(alphaPopupDialog->widget()->size(), QSize(488, 736));
            QCOMPARE(alphaPopupDialog->scrollWidget()->size(), QSize(440, 600));
            QCOMPARE(alphaPopupDialog->buttonGroup()->size(), QSize(486, 81));
            QCOMPARE(alphaPopupDialog->scrollArea()->verticalScrollBar()->maximum(), 0);
            QVERIFY(!alphaPopupDialog->opacityLineEdit()->isHidden());
            alphaPopupDialog->cancelButton()->click();
        });
        QTRY_VERIFY(alphaDialogInspected);
    }

    void folderListDialogEditsItsFolders()
    {
        FluentQt::FolderListDialog listDialog(QStringList{QStringLiteral("/tmp/music")},
                                              QStringLiteral("Build your collection"),
                                              QStringLiteral("Right now, we're watching these folders:"));
        QCOMPARE(listDialog.folders(), QStringList{QStringLiteral("/tmp/music")});
        QVERIFY(listDialog.scrollArea() != nullptr);
        QVERIFY(listDialog.scrollWidget() != nullptr);
        QVERIFY(listDialog.addFolderButton() != nullptr);
        QVERIFY(listDialog.addFolder(QStringLiteral("/tmp/podcasts")));
        QVERIFY(!listDialog.addFolder(QStringLiteral("/tmp/podcasts")));
        QVERIFY(listDialog.removeFolder(QStringLiteral("/tmp/music")));
        QCOMPARE(listDialog.folders(), QStringList{QStringLiteral("/tmp/podcasts")});
        QSignalSpy changedSpy(&listDialog, &FluentQt::FolderListDialog::folderChanged);
        listDialog.yesButton()->click();
        QCOMPARE(changedSpy.count(), 1);
        QCOMPARE(changedSpy.takeFirst().at(0).toStringList(), QStringList{QStringLiteral("/tmp/podcasts")});
    }

    void flyoutFactoriesSetPopupModeAndCloseOnOutsideClick()
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

    void defaultFlyoutViewIsUsable()
    {
        FlyoutView view;
        QVERIFY(view.titleLabel());
        QVERIFY(view.imageLabel());
        view.setTitle(QStringLiteral("title"));
        QCOMPARE(view.titleLabel()->text(), QStringLiteral("title"));
        view.show();
    }

    void flyoutViewMatchesPythonLooks()
    {
        // qfluentwidgets paints the view itself: a 248 / 40 grey face and an rgba(0, 0, 0, 17 / 45)
        // pen on a rounded rect inset by 1 px. Its title is 14 px bold and its content 14 px, from
        // the teaching tip style sheet, and its icon sits in a 36 x 54 box (36 x 36 when the
        // title or the content is missing), as in a teaching tip.
        QPixmap pixmap(300, 150);
        pixmap.fill(Qt::red);
        for (const bool dark : {false, true}) {
            FluentQt::ThemeManager::instance()->setTheme(dark ? Theme::Dark : Theme::Light);
            FlyoutView view(QStringLiteral("Title"), QStringLiteral("Content"),
                            FluentQt::icon(FluentQt::FluentIcon::Info), pixmap, true);
            view.ensurePolished();
            view.titleLabel()->ensurePolished();
            view.contentLabel()->ensurePolished();
            QCOMPARE(view.titleLabel()->font().pixelSize(), 14);
            QCOMPARE(view.titleLabel()->font().weight(), QFont::Bold);
            QCOMPARE(view.contentLabel()->font().pixelSize(), 14);
            QCOMPARE(view.contentLabel()->font().weight(), QFont::Normal);
            QCOMPARE(view.iconWidget()->size(), QSize(36, 54));
            QCOMPARE(view.iconWidget()->iconSize(), QSize(20, 20));

            view.adjustSize();
            QCOMPARE(view.imageLabel()->pos(), QPoint(1, 1));
            QImage face(view.size(), QImage::Format_ARGB32_Premultiplied);
            face.fill(Qt::transparent);
            view.render(&face, QPoint(), QRegion(), QWidget::DrawWindowBackground);
            const QColor expected = dark ? QColor(40, 40, 40) : QColor(248, 248, 248);
            QCOMPARE(QColor(face.pixel(view.width() / 2, view.height() - 4)), expected);
            // Only the antialiased edge of the inset pen reaches the outer row.
            QVERIFY(qAlpha(face.pixel(view.width() / 2, 0)) < 32);
            QCOMPARE(view.backgroundColor(), expected);
            QCOMPARE(view.borderColor(), dark ? QColor(0, 0, 0, 45) : QColor(0, 0, 0, 17));
        }

        FlyoutView titleOnly(QStringLiteral("Title"), QString(), FluentQt::icon(FluentQt::FluentIcon::Info), QPixmap(),
                             false);
        QCOMPARE(titleOnly.iconWidget()->size(), QSize(36, 36));
    }

    void unanimatedFlyoutOpensAboveItsTarget()
    {
        // qfluentwidgets' DummyFlyoutAnimationManager places the flyout above the target's left
        // edge: target.mapToGlobal((-margins.left, -height + margins.bottom - 8)), less the 4 px
        // every manager takes off y.
        QWidget window;
        window.resize(400, 400);
        const QRect available = QGuiApplication::primaryScreen()->availableGeometry();
        window.move(available.center() - QPoint(200, 200));
        auto *target = new QWidget(&window);
        target->setGeometry(150, 300, 60, 30);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *flyout = Flyout::make(new FlyoutView(QStringLiteral("Title"), QStringLiteral("Content")), target, &window,
                                    FlyoutAnimationType::None);
        const QMargins m = flyout->layout()->contentsMargins();
        const QPoint expected =
            target->mapToGlobal(QPoint(-m.left(), -flyout->sizeHint().height() + m.bottom() - 8)) - QPoint(0, 4);
        QCOMPARE(flyout->pos(), expected);
        flyout->close();
    }

    void flyoutReusesItsAnimationManager()
    {
        QWidget target;
        target.resize(100, 30);
        target.show();
        Flyout flyout(new FlyoutView(QStringLiteral("t"), QStringLiteral("c")), nullptr, false);
        const auto managers = [&flyout]() {
            return flyout.findChildren<QObject *>(QStringLiteral("fqwFlyoutAnimationManager"),
                                                  Qt::FindDirectChildrenOnly)
                .size();
        };
        flyout.exec(&target);
        flyout.hide();
        flyout.exec(&target);
        flyout.hide();
        flyout.exec(&target);
        QCOMPARE(managers(), 1);
    }

    void teachingTipViewExposesContentAndBodyLayout()
    {
        FluentQt::TeachingTipView view(QStringLiteral("Try this"), QStringLiteral("Use the button below."));
        QCOMPARE(view.title(), QStringLiteral("Try this"));
        QCOMPARE(view.content(), QStringLiteral("Use the button below."));
        QCOMPARE(view.property("fqw").toString(), QStringLiteral("TeachingTipView"));
        QVERIFY(view.bodyLayout() != nullptr);

        auto *extra = new QLabel(QStringLiteral("Extra"));
        view.bodyLayout()->addWidget(extra);
        view.setClosable(false);
        QVERIFY(!view.isClosable());

        QPixmap pixmap(20, 12);
        pixmap.fill(Qt::green);
        FluentQt::TeachingTipView richView(QStringLiteral("Lesson"), QStringLiteral("Content"),
                                           FluentQt::icon(FluentQt::FluentIcon::Info), pixmap, true,
                                           FluentQt::TeachingTipTailPosition::Right);
        QCOMPARE(richView.title(), QStringLiteral("Lesson"));
        QCOMPARE(richView.content(), QStringLiteral("Content"));
        QVERIFY(richView.isClosable());
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("tip.png"));
        QVERIFY(pixmap.save(path));
        QVERIFY(richView.setImagePath(path));

        auto *button = new FluentQt::PushButton(QStringLiteral("Action"));
        richView.addWidget(button, 0, Qt::AlignRight);
        QCOMPARE(button->parentWidget(), &richView);
    }

    void teachingTipViewSizesTextAndImageLikePython()
    {
        // In qfluentwidgets the labels do not word-wrap (TextWrap breaks long text into lines),
        // and the image is scaled to the full width (or, beside the text, the full height) of the
        // text row, so it spans the view inside its 1 px margins.
        QPixmap pixmap(300, 150);
        pixmap.fill(Qt::red);
        for (const auto tail : {FluentQt::TeachingTipTailPosition::Bottom, FluentQt::TeachingTipTailPosition::Left}) {
            FluentQt::TeachingTipView view(QStringLiteral("Title text"),
                                           QStringLiteral("Content text that is fairly long"),
                                           FluentQt::icon(FluentQt::FluentIcon::Info), pixmap, true, tail);
            view.show();
            QVERIFY(QTest::qWaitForWindowExposed(&view));
            view.adjustSize();
            auto *content = view.findChild<QLabel *>(QStringLiteral("contentLabel"));
            QVERIFY(content);
            QVERIFY(!content->wordWrap());
            QVERIFY(!view.findChild<QLabel *>(QStringLiteral("titleLabel"))->wordWrap());
            QCOMPARE(content->height(), content->fontMetrics().height());
            auto *image = view.findChild<FluentQt::ImageLabel *>();
            QVERIFY(image);
            if (tail == FluentQt::TeachingTipTailPosition::Bottom) {
                QCOMPARE(image->width(), view.width() - 2);
            } else {
                QCOMPARE(image->height(), view.height() - 2);
            }
        }
    }

    void teachingTipTracksTargetAndTail()
    {
        QWidget target;
        target.resize(120, 32);

        auto *view = new FluentQt::TeachingTipView(QStringLiteral("Title"), QStringLiteral("Content"));
        FluentQt::TeachingTip tip(view, &target, 0, FluentQt::TeachingTipTailPosition::None);
        QCOMPARE(tip.view(), view);
        QCOMPARE(tip.target(), &target);
        QCOMPARE(tip.duration(), 0);
        QCOMPARE(tip.tailPosition(), FluentQt::TeachingTipTailPosition::None);
        QCOMPARE(tip.property("fqw").toString(), QStringLiteral("TeachingTip"));
        QCOMPARE(tip.layout()->contentsMargins(), QMargins(15, 8, 15, 20));
        QVERIFY(view->parentWidget() != nullptr);
        QVERIFY(view->parentWidget() != &tip);
        QCOMPARE(view->parentWidget()->property("fqw").toString(), QStringLiteral("TeachingTipBubble"));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(view->parentWidget()->graphicsEffect()) != nullptr);
        QVERIFY(tip.graphicsEffect() == nullptr);

        tip.setTailPosition(FluentQt::TeachingTipTailPosition::Top);
        QCOMPARE(tip.tailPosition(), FluentQt::TeachingTipTailPosition::Top);
        QCOMPARE(tip.property("tailPosition").toString(), QStringLiteral("Top"));
        QCOMPARE(view->parentWidget()->layout()->contentsMargins(), QMargins(0, 8, 0, 0));
        tip.reposition();

        auto *created = FluentQt::TeachingTip::create(QStringLiteral("Created"), QStringLiteral("Body"),
                                                      FluentQt::icon(FluentQt::FluentIcon::Info),
                                                      QPixmap(), true, &target,
                                                      FluentQt::TeachingTipTailPosition::None, -1,
                                                      nullptr, false);
        QVERIFY(created != nullptr);
        QCOMPARE(created->isDeleteOnClose(), false);
        created->close();
        created->deleteLater();

        auto *popup = FluentQt::PopupTeachingTip::make(
            new FluentQt::TeachingTipView(QStringLiteral("Popup"), QStringLiteral("Body")), &target,
            FluentQt::TeachingTipTailPosition::None, -1, nullptr, false);
        QVERIFY(popup != nullptr);
        QCOMPARE(popup->isDeleteOnClose(), false);
        QCOMPARE(popup->windowFlags() & Qt::Popup, Qt::Popup);
        popup->close();
        popup->deleteLater();
    }

    void teachingTipBubbleMatchesPythonShadowMarginsAndPosition()
    {
        QWidget window;
        window.resize(560, 420);

        QPushButton target(QStringLiteral("Target"), &window);
        target.resize(120, 32);
        target.move(220, 250);

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *view = new FluentQt::TeachingTipView(QStringLiteral("Lesson 4"),
                                                   QStringLiteral("Advance towards a new stage."));
        FluentQt::TeachingTip tip(view, &target, -1, FluentQt::TeachingTipTailPosition::Bottom, &window);
        tip.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tip));
        QCoreApplication::processEvents();
        tip.reposition();

        QWidget *bubble = view->parentWidget();
        QVERIFY(bubble != nullptr);
        QCOMPARE(bubble->layout()->contentsMargins(), QMargins(0, 0, 0, 8));

        const int bubbleBottom = bubble->mapToGlobal(QPoint(0, bubble->height())).y();
        const int targetTop = target.mapToGlobal(QPoint(0, 0)).y();
        QCOMPARE(bubbleBottom, targetTop);

        const int bubbleCenter = bubble->mapToGlobal(QPoint(bubble->width() / 2, 0)).x();
        const int targetCenter = target.mapToGlobal(QPoint(target.width() / 2, 0)).x();
        QVERIFY(qAbs(bubbleCenter - targetCenter) <= 1);
    }

    void teachingTipBubbleRendersAntialiasedTransparentCorners()
    {
        QWidget window;
        window.resize(560, 420);

        QPushButton target(QStringLiteral("Target"), &window);
        target.resize(120, 32);
        target.move(220, 250);

        auto *view = new FluentQt::TeachingTipView(QStringLiteral("Lesson 4"),
                                                   QStringLiteral("Advance towards a new stage."));
        FluentQt::TeachingTip tip(view, &target, -1, FluentQt::TeachingTipTailPosition::Bottom, &window);
        tip.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tip));
        QCoreApplication::processEvents();
        tip.reposition();

        QWidget *bubble = view->parentWidget();
        QVERIFY(bubble != nullptr);
        QVERIFY(bubble->testAttribute(Qt::WA_TranslucentBackground));

        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(bubble->graphicsEffect()) != nullptr);
        bubble->setGraphicsEffect(nullptr);

        QImage image(bubble->size(), QImage::Format_ARGB32_Premultiplied);
        QVERIFY(!image.isNull());
        image.fill(Qt::transparent);
        bubble->render(&image);

        QVERIFY(qAlpha(image.pixel(0, 0)) < 32);

        bool hasAntialiasedEdge = false;
        const int scanSize = qMin(18, qMin(image.width(), image.height()));
        for (int y = 0; y < scanSize && !hasAntialiasedEdge; ++y) {
            for (int x = 0; x < scanSize; ++x) {
                const int alpha = qAlpha(image.pixel(x, y));
                if (alpha > 0 && alpha < 255) {
                    hasAntialiasedEdge = true;
                    break;
                }
            }
        }
        QVERIFY(hasAntialiasedEdge);
    }

    void teachingTipCloseButtonRespectsDeleteOnClose()
    {
        QWidget window;
        window.resize(400, 300);
        auto *target = new QWidget(&window);
        target->setGeometry(50, 50, 60, 30);
        window.show();

        const QPointer<TeachingTip> tip =
            TeachingTip::create(QStringLiteral("a"), QStringLiteral("b"), QIcon(), QPixmap(), true, target,
                                TeachingTipTailPosition::Bottom, -1, nullptr, false);
        emit tip->view()->closed();
        FqwTest::flushDeletes();
        QVERIFY(tip);
        QVERIFY(!tip->isVisible());
        delete tip.data();
    }

    void teachingTipClosesWhenTargetIsDeleted()
    {
        QWidget window;
        window.resize(400, 300);
        auto *target = new QWidget(&window);
        target->setGeometry(50, 50, 60, 30);
        window.show();

        const QPointer<TeachingTip> tip = TeachingTip::create(
            QStringLiteral("a"), QStringLiteral("b"), target, TeachingTipTailPosition::Bottom, -1, nullptr);
        QVERIFY(tip);

        delete target;
        window.resize(420, 320);
        QTRY_VERIFY(tip.isNull());
        window.resize(440, 340);
    }

    void keptTeachingTipIgnoresWindowAfterTargetIsDeleted()
    {
        QWidget window;
        window.resize(400, 300);
        auto *target = new QWidget(&window);
        target->setGeometry(50, 50, 60, 30);
        window.show();

        auto *tip = TeachingTip::create(QStringLiteral("a"), QStringLiteral("b"), QIcon(), QPixmap(), true, target,
                                        TeachingTipTailPosition::Bottom, -1, nullptr, false);
        delete target;
        QVERIFY(!tip->target());
        QVERIFY(!tip->isVisible());

        window.resize(420, 320);
        QTest::qWait(20);
        delete tip;
    }

    void dialogTitlesAndMessageBoxFrameMatchPython()
    {
        // qfluentwidgets styles the title as a plain QLabel at 20 px regular weight, and draws the
        // message box's centre widget with a 10 px radius, an rgb(144, 144, 142) / rgb(58, 58, 58)
        // border and a white / rgb(43, 43, 43) face.
        QWidget parent;
        parent.resize(800, 600);
        for (const bool dark : {false, true}) {
            FluentQt::ThemeManager::instance()->setTheme(dark ? Theme::Dark : Theme::Light);

            FluentQt::Dialog dialog(QStringLiteral("Title"), QStringLiteral("Message"), &parent);
            dialog.ensurePolished();
            dialog.titleLabel()->ensurePolished();
            QCOMPARE(dialog.titleLabel()->font().pixelSize(), 20);
            QCOMPARE(dialog.titleLabel()->font().weight(), QFont::Normal);

            FluentQt::MessageBox box(QStringLiteral("Title"), QStringLiteral("Message"), &parent);
            box.ensurePolished();
            box.titleLabel()->ensurePolished();
            QCOMPARE(box.titleLabel()->font().pixelSize(), 20);
            QCOMPARE(box.titleLabel()->font().weight(), QFont::Normal);
            QCOMPARE(box.titleLabel()->palette().windowText().color(), dark ? QColor(Qt::white) : QColor(Qt::black));

            // Selected content text: --ThemeColorLight1 under white text, or in dark mode
            // --ThemeColorPrimary under black text.
            box.contentLabel()->ensurePolished();
            const QPalette contentPalette = box.contentLabel()->palette();
            const QColor accent = FluentQt::ThemeManager::instance()->accentColor();
            QCOMPARE(contentPalette.color(QPalette::Highlight).rgba(),
                     FluentQt::derivedThemeColor(accent,
                                                 dark ? FluentQt::ThemeColor::Primary : FluentQt::ThemeColor::Light1,
                                                 dark ? Theme::Dark : Theme::Light)
                         .rgba());
            QCOMPARE(contentPalette.color(QPalette::HighlightedText), dark ? QColor(Qt::black) : QColor(Qt::white));

            QImage frame(box.widget()->size(), QImage::Format_ARGB32_Premultiplied);
            frame.fill(Qt::transparent);
            box.widget()->render(&frame, QPoint(), QRegion(), QWidget::DrawChildren);
            const int midY = box.titleLabel()->geometry().center().y();
            const QColor border = dark ? QColor(58, 58, 58) : QColor(144, 144, 142);
            const QColor face = dark ? QColor(43, 43, 43) : QColor(255, 255, 255);
            QCOMPARE(QColor(frame.pixel(0, midY)), border);
            QCOMPARE(QColor(frame.pixel(3, midY)), face);
            // A 10 px radius leaves the pixel two in from the corner outside the frame; 7 px would not.
            QVERIFY(qAlpha(frame.pixel(2, 2)) < 32);
        }
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(DialogFlyoutTest)
#include "tst_dialog_flyout.moc"
