#pragma once

#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QStringList>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

class QLabel;
class QPainter;
class QPushButton;
class QVBoxLayout;

namespace FluentQt {

class SingleDirectionScrollArea;

// A 292 x 72 card that paints its own hover and pressed states and emits clicked() on release.
class FQW_API ClickableWindow : public QWidget
{
    Q_OBJECT

  public:
    explicit ClickableWindow(QWidget *parent = nullptr);

    bool isPressed() const;
    bool isEnter() const;

  signals:
    void clicked();

  protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    bool m_isPressed = false;
    bool m_isEnter = false;
};

// One watched folder: its name in bold, its path below and a close mark; a click asks to remove it.
class FQW_API FolderCard : public ClickableWindow
{
    Q_OBJECT
    Q_PROPERTY(QString folderPath READ folderPath)
    Q_PROPERTY(QString folderName READ folderName)

  public:
    explicit FolderCard(const QString &folderPath, QWidget *parent = nullptr);

    QString folderPath() const;
    QString folderName() const;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void drawText(QPainter *painter, int x1, int fontSize1, int x2, int fontSize2);

    QString m_folderPath;
    QString m_folderName;
    QPixmap m_closeIcon;
};

// The card with a plus sign at the top of the list; a click opens a QFileDialog to add a folder.
class FQW_API AddFolderCard : public ClickableWindow
{
    Q_OBJECT

  public:
    explicit AddFolderCard(QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QPixmap m_iconPix;
};

class FQW_API FolderListDialog : public MaskDialogBase
{
    Q_OBJECT
    Q_PROPERTY(QStringList folderPaths READ folderPaths NOTIFY folderChanged)

  public:
    FolderListDialog(const QStringList &folderPaths, const QString &title, const QString &content,
                     QWidget *parent = nullptr);

    QString title() const;
    QString content() const;
    QStringList folderPaths() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    SingleDirectionScrollArea *scrollArea() const;
    QWidget *scrollWidget() const;
    QPushButton *completeButton() const;
    AddFolderCard *addFolderCard() const;
    QList<FolderCard *> folderCards() const;
    QVBoxLayout *vBoxLayout() const;
    QVBoxLayout *scrollLayout() const;

  signals:
    // Emitted by the Done button when the folders differ from the ones the dialog was opened with.
    void folderChanged(const QStringList &folderPaths);

  private:
    void initWidget();
    void initLayout();
    void setQss();
    void showFileDialog();
    void showDeleteFolderCardDialog(FolderCard *card);
    void deleteFolderCard(FolderCard *card);
    void onButtonClicked();
    void adjustWidgetSize();
    FolderCard *createFolderCard(const QString &folderPath);

    QString m_title;
    QString m_content;
    QStringList m_originalPaths;
    QStringList m_folderPaths;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    SingleDirectionScrollArea *m_scrollArea = nullptr;
    QWidget *m_scrollWidget = nullptr;
    QPushButton *m_completeButton = nullptr;
    AddFolderCard *m_addFolderCard = nullptr;
    QList<FolderCard *> m_folderCards;
    QVBoxLayout *m_scrollLayout = nullptr;
};

} // namespace FluentQt
