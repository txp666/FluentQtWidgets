#include <FluentQtWidgets/Dialogs/FolderListDialog.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include "DialogShadow_p.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QFileSystemModel>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

FolderPickerDialog::FolderPickerDialog(const QString &directory, QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setModal(true);
    FluentStyleSheet::setRole(this, QStringLiteral("FolderListDialog"));
    setProperty("fqw", QStringLiteral("FolderPickerDialogPopup"));
    setAttribute(Qt::WA_TranslucentBackground);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(12, 8, 12, 20);
    outerLayout->setSizeConstraint(QLayout::SetMinimumSize);

    m_view = new QFrame(this);
    FluentStyleSheet::setRole(m_view, QStringLiteral("FolderPickerDialog"));
    outerLayout->addWidget(m_view);

    auto *layout = new QVBoxLayout(m_view);
    layout->setContentsMargins(22, 20, 22, 18);
    layout->setSpacing(12);

    auto *titleLabel = new SubtitleLabel(tr("Choose folder"), m_view);
    layout->addWidget(titleLabel);

    m_pathEdit = new LineEdit(m_view);
    m_pathEdit->setClearButtonEnabled(true);
    layout->addWidget(m_pathEdit);

    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Drives);
    m_model->setRootPath(QString());

    m_treeView = new TreeView(m_view);
    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(true);
    m_treeView->setMinimumSize(420, 260);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int column = 1; column < m_model->columnCount(); ++column) {
        m_treeView->hideColumn(column);
    }
    layout->addWidget(m_treeView);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 8, 0, 0);
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch(1);

    m_cancelButton = new PushButton(tr("Cancel"), m_view);
    m_acceptButton = new PrimaryPushButton(tr("Select folder"), m_view);
    m_cancelButton->setMinimumWidth(96);
    m_acceptButton->setMinimumWidth(128);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_acceptButton);
    layout->addLayout(buttonLayout);

    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current) {
                const QString path = m_model->filePath(current);
                if (!path.isEmpty()) {
                    m_pathEdit->setText(path);
                }
            });
    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        if (index.isValid() && m_model->isDir(index)) {
            m_pathEdit->setText(m_model->filePath(index));
            accept();
        }
    });
    connect(m_acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setInitialDirectory(directory);
}

QString FolderPickerDialog::selectedFolder() const
{
    return QDir::cleanPath(m_pathEdit ? m_pathEdit->text() : QString());
}

TreeView *FolderPickerDialog::treeView() const { return m_treeView; }

LineEdit *FolderPickerDialog::pathEdit() const { return m_pathEdit; }

QPushButton *FolderPickerDialog::acceptButton() const { return m_acceptButton; }

QPushButton *FolderPickerDialog::cancelButton() const { return m_cancelButton; }

void FolderPickerDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
    if (!m_view) {
        return;
    }

    QPainter painter(this);
    Private::paintFluentWindowShadow(&painter, QRectF(m_view->geometry()), 30, QPoint(0, 8),
                                     QColor(0, 0, 0, 30));
}

void FolderPickerDialog::setInitialDirectory(const QString &directory)
{
    QString initialPath = directory;
    if (initialPath.isEmpty() || !QDir(initialPath).exists()) {
        initialPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    if (initialPath.isEmpty()) {
        initialPath = QDir::rootPath();
    }

    initialPath = QDir::cleanPath(initialPath);
    m_pathEdit->setText(initialPath);
    const QModelIndex index = m_model->index(initialPath);
    if (index.isValid()) {
        m_treeView->setRootIndex(m_model->index(QDir::rootPath()));
        m_treeView->setCurrentIndex(index);
        m_treeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

FolderListDialog::FolderListDialog(const QStringList &folderPaths, const QString &title, const QString &content,
                                   QWidget *parent)
    : Dialog(title, content, parent), m_originalFolders(folderPaths)
{
    FluentStyleSheet::setRole(this, QStringLiteral("FolderListDialog"));

    yesButton()->setText(tr("Done"));
    hideCancelButton();

    m_scrollArea = new ScrollArea(widget());
    m_scrollArea->enableTransparentBackground();
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFixedWidth(320);
    m_scrollArea->setMaximumHeight(400);

    m_scrollWidget = new QWidget(m_scrollArea);
    m_scrollWidget->setObjectName(QStringLiteral("scrollWidget"));
    m_scrollWidget->setFixedWidth(300);
    m_folderLayout = new QVBoxLayout(m_scrollWidget);
    m_folderLayout->setContentsMargins(0, 0, 0, 0);
    m_folderLayout->setSpacing(8);

    m_addFolderButton = new PushButton(FluentQt::icon(FluentIcon::Folder), tr("Add folder"), m_scrollWidget);
    m_addFolderButton->setFixedHeight(42);
    m_addFolderButton->setMinimumWidth(292);
    m_folderLayout->addWidget(m_addFolderButton, 0, Qt::AlignTop);
    connect(m_addFolderButton, &QPushButton::clicked, this, &FolderListDialog::chooseFolder);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_scrollWidget);
    viewLayout()->addSpacing(4);
    viewLayout()->addWidget(m_scrollArea, 0, Qt::AlignHCenter);

    setFolders(folderPaths);
    connect(this, &Dialog::yesSignal, this, &FolderListDialog::finishDialog);
}

QStringList FolderListDialog::folders() const { return m_folders; }

QPushButton *FolderListDialog::addFolderButton() const { return m_addFolderButton; }

ScrollArea *FolderListDialog::scrollArea() const { return m_scrollArea; }

QWidget *FolderListDialog::scrollWidget() const { return m_scrollWidget; }

void FolderListDialog::setFolders(const QStringList &folders)
{
    QStringList uniqueFolders;
    for (const QString &folder : folders) {
        const QString cleanFolder = QDir::cleanPath(folder);
        if (!cleanFolder.isEmpty() && !uniqueFolders.contains(cleanFolder)) {
            uniqueFolders.append(cleanFolder);
        }
    }

    m_folders = uniqueFolders;
    rebuildFolderButtons();
}

bool FolderListDialog::addFolder(const QString &folder)
{
    const QString cleanFolder = QDir::cleanPath(folder);
    if (cleanFolder.isEmpty() || m_folders.contains(cleanFolder)) {
        return false;
    }

    m_folders.append(cleanFolder);
    addFolderButtonRow(cleanFolder);
    return true;
}

bool FolderListDialog::removeFolder(const QString &folder)
{
    const QString cleanFolder = QDir::cleanPath(folder);
    const int index = m_folders.indexOf(cleanFolder);
    if (index < 0) {
        return false;
    }

    m_folders.removeAt(index);
    if (index < m_folderButtons.size()) {
        auto *button = m_folderButtons.takeAt(index);
        m_folderLayout->removeWidget(button);
        button->deleteLater();
    }
    return true;
}

void FolderListDialog::chooseFolder()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), QStringLiteral("./"));
    if (!path.isEmpty()) {
        addFolder(path);
    }
}

void FolderListDialog::rebuildFolderButtons()
{
    if (!m_folderLayout) {
        return;
    }

    for (auto *button : std::as_const(m_folderButtons)) {
        m_folderLayout->removeWidget(button);
        button->deleteLater();
    }
    m_folderButtons.clear();

    for (const QString &folder : std::as_const(m_folders)) {
        addFolderButtonRow(folder);
    }
}

void FolderListDialog::addFolderButtonRow(const QString &folder)
{
    if (!m_folderLayout) {
        return;
    }

    auto *button = new PushButton(FluentQt::icon(FluentIcon::Folder), folder, m_scrollWidget);
    button->setFixedHeight(42);
    button->setMinimumWidth(292);
    button->setToolTip(folder);
    m_folderButtons.append(button);
    m_folderLayout->addWidget(button, 0, Qt::AlignTop);

    connect(button, &QPushButton::clicked, this, [this, folder]() {
        const QString folderName = QFileInfo(folder).fileName().isEmpty() ? folder : QFileInfo(folder).fileName();
        Dialog dialog(tr("Are you sure you want to delete the folder?"),
                      tr("If you delete the \"%1\" folder and remove it from the list, the folder will no longer "
                         "appear in the list, but will not be deleted.")
                          .arg(folderName),
                      window());
        connect(&dialog, &Dialog::yesSignal, this, [this, folder]() { removeFolder(folder); });
        dialog.exec();
    });
}

void FolderListDialog::finishDialog()
{
    QStringList original = m_originalFolders;
    QStringList current = m_folders;
    original.sort();
    current.sort();
    if (original != current) {
        emit folderChanged(m_folders);
    }
}

} // namespace FluentQt
