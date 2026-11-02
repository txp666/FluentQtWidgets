// The Gallery screenshots of the README, and the frames of its tour GIF.
// usage: galleryshots <zh_CN|en> <home|charts|datepicker|menu> <out.jpg>
//        galleryshots <zh_CN|en> frames <dir>    (NNN.png at 640 x 522, and delays.txt in ms per frame)
#include "GalleryTranslator.h"
#include "GalleryWindow.h"
#include "components/GalleryComponents.h"
#include "shotutil.h"

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QStandardPaths>
#include <QtGui/QCursor>
#include <QtGui/QEnterEvent>
#include <QtGui/QHoverEvent>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStyleFactory>

using namespace FluentQt;

static void hoverAt(QWidget *w, const QPoint &local)
{
    const QPointF lp(local);
    const QPointF gp(w->mapToGlobal(local));
    QHoverEvent he(QEvent::HoverEnter, lp, gp, QPointF(-1, -1));
    QApplication::sendEvent(w, &he);
    QHoverEvent hm(QEvent::HoverMove, lp, gp, lp);
    QApplication::sendEvent(w, &hm);
    QMouseEvent mm(QEvent::MouseMove, lp, gp, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(w, &mm);
}

// Hovers the row of `menu` whose text contains `text`, which opens its submenu.
static QListWidgetItem *hoverMenuRow(RoundMenu *menu, const QString &text)
{
    auto *view = menu->findChild<QListWidget *>();
    if (!view)
        return nullptr;
    for (int i = 0; i < view->count(); ++i) {
        QListWidgetItem *item = view->item(i);
        if (!item->text().contains(text))
            continue;
        QWidget *iw = view->itemWidget(item);
        const QRect r = view->visualItemRect(item);
        hoverAt(view->viewport(), r.center());
        if (iw) {
            QEnterEvent ee(QPointF(5, 5), QPointF(iw->mapTo(iw->window(), QPoint(5, 5))),
                           QPointF(iw->mapToGlobal(QPoint(5, 5))));
            QApplication::sendEvent(iw, &ee);
        }
        return item;
    }
    std::fprintf(stderr, "menu row %s not found\n", qPrintable(text));
    return nullptr;
}

int main(int argc, char *argv[])
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setApplicationName(QStringLiteral("FluentQtWidgets Gallery"));
    QCoreApplication::setOrganizationName(QStringLiteral("FluentQtWidgets"));
    QApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 4) {
        std::fprintf(stderr, "usage: galleryshots <zh_CN|en> <home|charts|datepicker|menu|frames> <out>\n");
        return 2;
    }
    const QString lang = args.at(1), shot = args.at(2), out = args.at(3);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/gallery/images/logo.png")));
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QLocale::setDefault(QLocale(lang == QLatin1String("en") ? QStringLiteral("en_US") : lang));
    FluentConfig::instance()->setLocaleName(lang);
    GalleryTranslation::installTranslators(&app, lang);
    ThemeManager::instance()->setTheme(Theme::Light);
    ThemeManager::instance()->setAccentColor(FluentConfig::instance()->themeColor());

    auto *window = new GalleryWindow(nullptr, false);
    window->setGeometry(100, 60, 1280, 800);
    window->show();
    QCursor::setPos(1590, 990);
    waitMs(1500);

    auto popups = [&]() { return visiblePopups(window); };

    if (shot == QLatin1String("home")) {
        waitMs(500);
        return saveShot(grabComposite(window, {}), out) ? 0 : 1;
    }
    if (shot == QLatin1String("charts")) {
        window->switchTo(QStringLiteral("chartInterface"));
        waitMs(600);
        if (auto *g = qobject_cast<GalleryInterface *>(window->currentInterface()))
            g->scrollToCard(1);
        waitMs(800);
        if (auto *plot = window->currentInterface()->findChild<RealtimePlotWidget *>()) {
            QElapsedTimer guard;
            guard.start();
            while (plot->sampleCount() < 1057 && guard.elapsed() < 20000)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
            for (auto *t : plot->findChildren<QTimer *>(QString(), Qt::FindDirectChildrenOnly))
                t->stop();
            std::fprintf(stderr, "plot samples %d\n", plot->sampleCount());
        }
        waitMs(800);
        return saveShot(grabComposite(window, {}), out) ? 0 : 1;
    }
    if (shot == QLatin1String("datepicker")) {
        window->switchTo(QStringLiteral("dateTimeInterface"));
        waitMs(600);
        if (auto *g = qobject_cast<GalleryInterface *>(window->currentInterface()))
            g->scrollToCard(3);
        waitMs(1200);
        auto *picker = window->currentInterface()->findChild<DatePicker *>();
        if (!picker) {
            std::fprintf(stderr, "no date picker\n");
            return 1;
        }
        QMetaObject::invokeMethod(picker, "showPickerPanel");
        waitMs(1200);
        return saveShot(grabComposite(window, popups()), out) ? 0 : 1;
    }
    if (shot == QLatin1String("menu")) {
        window->switchTo(QStringLiteral("menuInterface"));
        waitMs(800);
        PushButton *btn = nullptr;
        for (auto *b : window->currentInterface()->findChildren<PushButton *>()) {
            if (b->text() == QCoreApplication::translate("MenuInterface", "Show menu")) {
                btn = b;
                break;
            }
        }
        if (!btn) {
            std::fprintf(stderr, "no menu button\n");
            return 1;
        }
        btn->click();
        waitMs(900);
        RoundMenu *menu = nullptr;
        for (QWidget *w : popups())
            if (auto *m = qobject_cast<RoundMenu *>(w))
                menu = m;
        if (!menu) {
            std::fprintf(stderr, "no menu\n");
            return 1;
        }
        hoverMenuRow(menu, QCoreApplication::translate("MenuInterface", "Add to"));
        waitMs(1200);
        for (QWidget *w : popups()) {
            auto *m = qobject_cast<RoundMenu *>(w);
            if (m && m != menu)
                hoverMenuRow(m, QCoreApplication::translate("MenuInterface", "Video"));
        }
        waitMs(600);
        QList<QWidget *> list = popups();
        std::stable_sort(list.begin(), list.end(), [menu](QWidget *a, QWidget *b) { return a == menu && b != menu; });
        return saveShot(grabComposite(window, list), out) ? 0 : 1;
    }
    if (shot == QLatin1String("frames")) {
        window->setGeometry(100, 60, 960, 783);
        waitMs(1200);
        QDir().mkpath(out);
        QFile delays(out + QStringLiteral("/delays.txt"));
        if (!delays.open(QIODevice::WriteOnly | QIODevice::Text)) {
            std::fprintf(stderr, "cannot write %s\n", qPrintable(delays.fileName()));
            return 1;
        }
        int index = 0;
        auto snap = [&](int delayMs) {
            QImage img = grabComposite(window, popups())
                             .convertToFormat(QImage::Format_RGB32)
                             .scaled(640, 522, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            img.save(QStringLiteral("%1/%2.png").arg(out).arg(index++, 3, 10, QLatin1Char('0')));
            delays.write(QByteArray::number(delayMs) + "\n");
        };
        auto film = [&](int count, int stepMs) {
            for (int i = 0; i < count; ++i) {
                waitMs(stepMs);
                snap(stepMs);
            }
        };
        auto gallery = [&]() { return qobject_cast<GalleryInterface *>(window->currentInterface()); };
        snap(1400);
        window->switchTo(QStringLiteral("chartInterface"));
        film(5, 70);
        waitMs(300);
        snap(700);
        if (auto *g = gallery())
            g->scrollToCard(1);
        film(6, 70);
        film(6, 160);
        window->switchTo(QStringLiteral("dateTimeInterface"));
        film(5, 70);
        waitMs(300);
        snap(600);
        if (auto *g = gallery())
            g->scrollToCard(3);
        film(5, 70);
        waitMs(300);
        if (auto *picker = window->currentInterface()->findChild<DatePicker *>()) {
            QMetaObject::invokeMethod(picker, "showPickerPanel");
            film(5, 70);
            waitMs(400);
            snap(1300);
            for (QWidget *w : popups())
                w->close();
            waitMs(400);
        }
        window->switchTo(QStringLiteral("menuInterface"));
        film(5, 70);
        waitMs(300);
        snap(500);
        for (auto *b : window->currentInterface()->findChildren<PushButton *>()) {
            if (b->text() == QCoreApplication::translate("MenuInterface", "Show menu")) {
                b->click();
                break;
            }
        }
        film(4, 70);
        waitMs(400);
        RoundMenu *menu = nullptr;
        for (QWidget *w : popups())
            if (auto *m = qobject_cast<RoundMenu *>(w))
                menu = m;
        if (menu) {
            snap(500);
            hoverMenuRow(menu, QCoreApplication::translate("MenuInterface", "Add to"));
            waitMs(700);
            for (QWidget *w : popups()) {
                auto *m = qobject_cast<RoundMenu *>(w);
                if (m && m != menu)
                    hoverMenuRow(m, QCoreApplication::translate("MenuInterface", "Video"));
            }
            film(3, 70);
            waitMs(300);
            snap(1300);
            for (QWidget *w : popups())
                w->close();
            waitMs(500);
        }
        window->switchTo(QStringLiteral("home"));
        film(5, 70);
        waitMs(300);
        snap(1200);
        std::fprintf(stderr, "wrote %d frames\n", index);
        return 0;
    }
    std::fprintf(stderr, "unknown shot\n");
    return 2;
}
