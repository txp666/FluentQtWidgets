#pragma once

#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QStringList>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

class QFileSystemModel;
class QPaintEvent;

namespace FluentQt {

class LineEdit;
class PushButton;
class ScrollArea;
class TreeView;

class FQW_API FolderPickerDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString selectedFolder READ selectedFolder)

  public:
    explicit FolderPickerDialog(const QString &directory, QWidget *parent = nullptr);

    QString selectedFolder() const;
    TreeView *treeView() const;
    LineEdit *pathEdit() const;
    QPushButton *acceptButton() const;
    QPushButton *cancelButton() const;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void setInitialDirectory(const QString &directory);

    QFrame *m_view = nullptr;
    LineEdit *m_pathEdit = nullptr;
    TreeView *m_treeView = nullptr;
    QFileSystemModel *m_model = nullptr;
    QPushButton *m_acceptButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
};

class FQW_API FolderListDialog : public Dialog
{
    Q_OBJECT
    Q_PROPERTY(QStringList folders READ folders WRITE setFolders NOTIFY folderChanged)

  public:
    FolderListDialog(const QStringList &folderPaths, const QString &title, const QString &content,
                     QWidget *parent = nullptr);

    QStringList folders() const;
    QPushButton *addFolderButton() const;
    ScrollArea *scrollArea() const;
    QWidget *scrollWidget() const;

  public slots:
    void setFolders(const QStringList &folders);
    bool addFolder(const QString &folder);
    bool removeFolder(const QString &folder);

  signals:
    void folderChanged(const QStringList &folders);

  private:
    void chooseFolder();
    void rebuildFolderButtons();
    void addFolderButtonRow(const QString &folder);
    void finishDialog();

    QStringList m_originalFolders;
    QStringList m_folders;
    ScrollArea *m_scrollArea = nullptr;
    QWidget *m_scrollWidget = nullptr;
    QVBoxLayout *m_folderLayout = nullptr;
    QPushButton *m_addFolderButton = nullptr;
    QList<QPushButton *> m_folderButtons;
};

} // namespace FluentQt
