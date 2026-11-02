#include <FluentQtWidgets/Dialogs/FolderListDialog.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QFileInfo>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include <algorithm>

namespace FluentQt {

namespace {

bool isDark() { return ThemeManager::instance()->effectiveTheme() == Theme::Dark; }

// Python's getIconColor(): the images come in a black and a white variant.
QString iconColor() { return isDark() ? QStringLiteral("white") : QStringLiteral("black"); }

QPixmap folderListImage(const QString &name, int size)
{
    return QPixmap(QStringLiteral(":/qfluentwidgets/images/folder_list_dialog/%1_%2.png").arg(name, iconColor()))
        .scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

} // namespace

ClickableWindow::ClickableWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(292, 72);
}

bool ClickableWindow::isPressed() const { return m_isPressed; }

bool ClickableWindow::isEnter() const { return m_isEnter; }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ClickableWindow::enterEvent(QEnterEvent *)
#else
void ClickableWindow::enterEvent(QEvent *)
#endif
{
    m_isEnter = true;
    update();
}

void ClickableWindow::leaveEvent(QEvent *)
{
    m_isEnter = false;
    update();
}

void ClickableWindow::mousePressEvent(QMouseEvent *)
{
    m_isPressed = true;
    update();
}

void ClickableWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    update();
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void ClickableWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const bool dark = isDark();
    int bg = dark ? 51 : 204;
    QBrush brush(QColor(bg, bg, bg));
    painter.setPen(Qt::NoPen);

    if (!m_isEnter) {
        painter.setBrush(brush);
        painter.drawRoundedRect(rect(), 4, 4);
    } else {
        painter.setPen(QPen(QColor(bg, bg, bg), 2));
        painter.drawRect(1, 1, width() - 2, height() - 2);
        painter.setPen(Qt::NoPen);
        if (!m_isPressed) {
            bg = dark ? 24 : 230;
            brush.setColor(QColor(bg, bg, bg));
            painter.setBrush(brush);
            painter.drawRect(2, 2, width() - 4, height() - 4);
        } else {
            brush.setColor(QColor(153, 153, 153));
            painter.setBrush(brush);
            painter.drawRoundedRect(5, 1, width() - 10, height() - 2, 2, 2);
        }
    }
}

FolderCard::FolderCard(const QString &folderPath, QWidget *parent)
    : ClickableWindow(parent), m_folderPath(folderPath), m_folderName(QFileInfo(folderPath).fileName()),
      m_closeIcon(folderListImage(QStringLiteral("Close"), 12))
{
}

QString FolderCard::folderPath() const { return m_folderPath; }

QString FolderCard::folderName() const { return m_folderName; }

void FolderCard::paintEvent(QPaintEvent *event)
{
    ClickableWindow::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    painter.setPen(isDark() ? Qt::white : Qt::black);
    if (m_isPressed) {
        drawText(&painter, 12, 8, 12, 7);
        painter.drawPixmap(width() - 26, 18, m_closeIcon);
    } else {
        drawText(&painter, 10, 9, 10, 8);
        painter.drawPixmap(width() - 24, 20, m_closeIcon);
    }
}

void FolderCard::drawText(QPainter *painter, int x1, int fontSize1, int x2, int fontSize2)
{
    // The folder name.
    QFont font(QStringLiteral("Microsoft YaHei"), fontSize1, QFont::Bold);
    painter->setFont(font);
    const QString name = QFontMetrics(font).elidedText(m_folderName, Qt::ElideRight, width() - 48);
    painter->drawText(x1, 30, name);

    // The folder path.
    font = QFont(QStringLiteral("Microsoft YaHei"), fontSize2);
    painter->setFont(font);
    const QString path = QFontMetrics(font).elidedText(m_folderPath, Qt::ElideRight, width() - 24);
    painter->drawText(x2, 37, width() - 16, 18, Qt::AlignLeft, path);
}

AddFolderCard::AddFolderCard(QWidget *parent)
    : ClickableWindow(parent), m_iconPix(folderListImage(QStringLiteral("Add"), 22))
{
}

void AddFolderCard::paintEvent(QPaintEvent *event)
{
    ClickableWindow::paintEvent(event);
    QPainter painter(this);
    const int w = width();
    const int h = height();
    const int pw = m_iconPix.width();
    const int ph = m_iconPix.height();
    if (!m_isPressed) {
        painter.drawPixmap(int(w / 2.0 - pw / 2.0), int(h / 2.0 - ph / 2.0), m_iconPix);
    } else {
        painter.drawPixmap(int(w / 2.0 - (pw - 4) / 2.0), int(h / 2.0 - (ph - 4) / 2.0), pw - 4, ph - 4, m_iconPix);
    }
}

FolderListDialog::FolderListDialog(const QStringList &folderPaths, const QString &title, const QString &content,
                                   QWidget *parent)
    : MaskDialogBase(parent), m_title(title), m_content(content), m_originalPaths(folderPaths),
      m_folderPaths(folderPaths)
{
    m_vBoxLayout = new QVBoxLayout(widget());
    m_titleLabel = new QLabel(title, widget());
    m_contentLabel = new QLabel(content, widget());
    m_scrollArea = new SingleDirectionScrollArea(widget());
    m_scrollWidget = new QWidget(m_scrollArea);
    m_completeButton = new QPushButton(tr("Done"), widget());
    m_addFolderCard = new AddFolderCard(m_scrollWidget);
    for (const QString &folderPath : folderPaths) {
        m_folderCards.append(createFolderCard(folderPath));
    }
    initWidget();
}

QString FolderListDialog::title() const { return m_title; }

QString FolderListDialog::content() const { return m_content; }

QStringList FolderListDialog::folderPaths() const { return m_folderPaths; }

QLabel *FolderListDialog::titleLabel() const { return m_titleLabel; }

QLabel *FolderListDialog::contentLabel() const { return m_contentLabel; }

SingleDirectionScrollArea *FolderListDialog::scrollArea() const { return m_scrollArea; }

QWidget *FolderListDialog::scrollWidget() const { return m_scrollWidget; }

QPushButton *FolderListDialog::completeButton() const { return m_completeButton; }

AddFolderCard *FolderListDialog::addFolderCard() const { return m_addFolderCard; }

QList<FolderCard *> FolderListDialog::folderCards() const { return m_folderCards; }

QVBoxLayout *FolderListDialog::vBoxLayout() const { return m_vBoxLayout; }

QVBoxLayout *FolderListDialog::scrollLayout() const { return m_scrollLayout; }

void FolderListDialog::initWidget()
{
    setQss();

    const int w = std::max({m_titleLabel->width() + 48, m_contentLabel->width() + 48, 352});
    widget()->setFixedWidth(w);
    m_scrollArea->resize(294, 72);
    m_scrollWidget->resize(292, 72);
    m_scrollArea->setFixedWidth(294);
    m_scrollWidget->setFixedWidth(292);
    m_scrollArea->setMaximumHeight(400);
    m_scrollArea->setViewportMargins(0, 0, 0, 0);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_scrollWidget);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->hScrollBar()->setForceHidden(true);
    initLayout();

    connect(m_addFolderCard, &ClickableWindow::clicked, this, &FolderListDialog::showFileDialog);
    connect(m_completeButton, &QPushButton::clicked, this, &FolderListDialog::onButtonClicked);
}

void FolderListDialog::initLayout()
{
    m_vBoxLayout->setContentsMargins(24, 24, 24, 24);
    m_vBoxLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_vBoxLayout->setAlignment(Qt::AlignTop);
    m_vBoxLayout->setSpacing(0);

    // labels
    auto *layout1 = new QVBoxLayout;
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->setSpacing(6);
    layout1->addWidget(m_titleLabel, 0, Qt::AlignTop);
    layout1->addWidget(m_contentLabel, 0, Qt::AlignTop);
    m_vBoxLayout->addLayout(layout1, 0);
    m_vBoxLayout->addSpacing(12);

    // cards
    auto *layout2 = new QHBoxLayout;
    layout2->setAlignment(Qt::AlignCenter);
    layout2->setContentsMargins(4, 0, 4, 0);
    layout2->addWidget(m_scrollArea, 0, Qt::AlignCenter);
    m_vBoxLayout->addLayout(layout2, 1);
    m_vBoxLayout->addSpacing(24);

    m_scrollLayout = new QVBoxLayout(m_scrollWidget);
    m_scrollLayout->setAlignment(Qt::AlignTop);
    m_scrollLayout->setContentsMargins(0, 0, 0, 0);
    m_scrollLayout->setSpacing(8);
    m_scrollLayout->addWidget(m_addFolderCard, 0, Qt::AlignTop);
    for (FolderCard *card : std::as_const(m_folderCards)) {
        m_scrollLayout->addWidget(card, 0, Qt::AlignTop);
    }

    // buttons
    auto *layout3 = new QHBoxLayout;
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->addStretch(1);
    layout3->addWidget(m_completeButton);
    m_vBoxLayout->addLayout(layout3, 0);

    adjustWidgetSize();
}

void FolderListDialog::setQss()
{
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_contentLabel->setObjectName(QStringLiteral("contentLabel"));
    m_completeButton->setObjectName(QStringLiteral("completeButton"));
    m_scrollWidget->setObjectName(QStringLiteral("scrollWidget"));

    // As in Python, folder_list_dialog.qss styles the whole dialog, mask included.
    FluentStyleSheet::setRole(this, QStringLiteral("FolderListDialog"));
    ensurePolished();
    for (QWidget *child : findChildren<QWidget *>()) {
        child->ensurePolished();
    }

    m_titleLabel->adjustSize();
    m_contentLabel->adjustSize();
    m_completeButton->adjustSize();
}

FolderCard *FolderListDialog::createFolderCard(const QString &folderPath)
{
    auto *card = new FolderCard(folderPath, m_scrollWidget);
    connect(card, &ClickableWindow::clicked, this, [this, card]() { showDeleteFolderCardDialog(card); });
    return card;
}

void FolderListDialog::showFileDialog()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), QStringLiteral("./"));
    if (path.isEmpty() || m_folderPaths.contains(path)) {
        return;
    }

    FolderCard *card = createFolderCard(path);
    m_scrollLayout->addWidget(card, 0, Qt::AlignTop);
    card->show();

    m_folderPaths.append(path);
    m_folderCards.append(card);

    adjustWidgetSize();
}

void FolderListDialog::showDeleteFolderCardDialog(FolderCard *card)
{
    const QString title = tr("Are you sure you want to delete the folder?");
    const QString content = tr("If you delete the ") + QStringLiteral("\"%1\"").arg(card->folderName()) +
                            tr(" folder and remove it from the list, the folder will no longer appear in the list, "
                               "but will not be deleted.");
    Dialog dialog(title, content, window());
    connect(&dialog, &Dialog::yesSignal, this, [this, card]() { deleteFolderCard(card); });
    dialog.exec();
}

void FolderListDialog::deleteFolderCard(FolderCard *card)
{
    const auto index = m_folderCards.indexOf(card);
    if (index < 0) {
        return;
    }

    m_scrollLayout->removeWidget(card);
    m_folderCards.removeAt(index);
    m_folderPaths.removeAt(index);
    card->deleteLater();

    adjustWidgetSize();
}

void FolderListDialog::onButtonClicked()
{
    QStringList original = m_originalPaths;
    QStringList current = m_folderPaths;
    original.sort();
    current.sort();
    if (original != current) {
        setEnabled(false);
        QApplication::processEvents();
        emit folderChanged(m_folderPaths);
    }

    close();
}

void FolderListDialog::adjustWidgetSize()
{
    const int n = static_cast<int>(m_folderCards.size());
    const int h = 72 * (n + 1) + 8 * n;
    m_scrollArea->setFixedHeight(std::min(h, 400));
}

} // namespace FluentQt
