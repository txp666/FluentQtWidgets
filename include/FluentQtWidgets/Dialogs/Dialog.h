#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

class QPaintEvent;

namespace FluentQt {

class FQW_API Dialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QFrame *view READ view)
    Q_PROPERTY(QFrame *widget READ widget)
    Q_PROPERTY(QLabel *windowTitleLabel READ windowTitleLabel)
    Q_PROPERTY(QLabel *titleLabel READ titleLabel)
    Q_PROPERTY(QLabel *contentLabel READ contentLabel)
    Q_PROPERTY(QFrame *buttonGroup READ buttonGroup)
    Q_PROPERTY(QPushButton *yesButton READ yesButton)
    Q_PROPERTY(QPushButton *cancelButton READ cancelButton)
    Q_PROPERTY(QVBoxLayout *vBoxLayout READ vBoxLayout)
    Q_PROPERTY(QVBoxLayout *viewLayout READ viewLayout)
    Q_PROPERTY(QVBoxLayout *textLayout READ textLayout)
    Q_PROPERTY(QHBoxLayout *buttonLayout READ buttonLayout)

  public:
    explicit Dialog(QWidget *parent = nullptr);
    Dialog(const QString &title, const QString &message, QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setContentCopyable(bool copyable);
    void setTitleBarVisible(bool visible);
    void hideYesButton();
    void hideCancelButton();
    QFrame *view() const;
    QFrame *widget() const;
    QLabel *windowTitleLabel() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    QLabel *messageLabel() const;
    QFrame *buttonGroup() const;
    QPushButton *yesButton() const;
    QPushButton *acceptButton() const;
    QPushButton *cancelButton() const;
    QVBoxLayout *vBoxLayout() const;
    QVBoxLayout *viewLayout() const;
    QVBoxLayout *textLayout() const;
    QHBoxLayout *buttonLayout() const;

  signals:
    void yesSignal();
    void cancelSignal();

  protected:
    void showEvent(QShowEvent *event) override;
    virtual bool validate() const;

  private:
    void installFadeInAnimation();
    void addButtonStretchOnce();

  protected:
    QFrame *m_view = nullptr;

  private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_windowTitleLabel = nullptr;
    QLabel *m_messageLabel = nullptr;
    QFrame *m_buttonGroup = nullptr;
    QPushButton *m_acceptButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QVBoxLayout *m_textLayout = nullptr;
    QHBoxLayout *m_buttonLayout = nullptr;
    bool m_buttonStretchInserted = false;
};

class FQW_API MaskDialogBase : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QFrame *widget READ widget)
    Q_PROPERTY(QWidget *windowMask READ windowMask)
    Q_PROPERTY(QHBoxLayout *hBoxLayout READ hBoxLayout)
    Q_PROPERTY(bool closableOnMaskClicked READ isClosableOnMaskClicked WRITE setClosableOnMaskClicked)
    Q_PROPERTY(bool draggable READ isDraggable WRITE setDraggable)

  public:
    explicit MaskDialogBase(QWidget *parent = nullptr);

    QFrame *widget() const;
    QWidget *windowMask() const;
    QHBoxLayout *hBoxLayout() const;
    bool isClosableOnMaskClicked() const;
    void setClosableOnMaskClicked(bool closable);
    bool isDraggable() const;
    void setDraggable(bool draggable);
    QColor maskColor() const;
    void setMaskColor(const QColor &color);
    void setShadowEffect(int blurRadius = 60, const QPoint &offset = QPoint(0, 10),
                         const QColor &color = QColor(0, 0, 0, 100));

  protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void done(int result) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void syncGeometryToParent();
    void setCenterWidgetRole(const QString &role);

    QFrame *m_widget = nullptr;
    QWidget *m_windowMask = nullptr;
    QHBoxLayout *m_hBoxLayout = nullptr;

  private:
    void installFadeInAnimation();
    void installFadeOutAnimation(int result);
    void updateMaskStyle();

    bool m_closableOnMaskClicked = false;
    bool m_draggable = false;
    bool m_finishing = false;
    QPoint m_dragPos;
    QColor m_maskColor;
    int m_shadowBlurRadius = 60;
    QPoint m_shadowOffset = QPoint(0, 10);
    QColor m_shadowColor = QColor(0, 0, 0, 100);
};

class FQW_API MessageBoxBase : public MaskDialogBase
{
    Q_OBJECT
    Q_PROPERTY(QFrame *view READ view)
    Q_PROPERTY(QFrame *buttonGroup READ buttonGroup)
    Q_PROPERTY(QPushButton *yesButton READ yesButton)
    Q_PROPERTY(QPushButton *cancelButton READ cancelButton)
    Q_PROPERTY(QVBoxLayout *vBoxLayout READ vBoxLayout)
    Q_PROPERTY(QVBoxLayout *viewLayout READ viewLayout)
    Q_PROPERTY(QHBoxLayout *buttonLayout READ buttonLayout)

  public:
    explicit MessageBoxBase(QWidget *parent = nullptr);

    QFrame *view() const;
    QFrame *buttonGroup() const;
    QPushButton *yesButton() const;
    QPushButton *acceptButton() const;
    QPushButton *cancelButton() const;
    QVBoxLayout *vBoxLayout() const;
    QVBoxLayout *viewLayout() const;
    QHBoxLayout *buttonLayout() const;
    void hideYesButton();
    void hideCancelButton();

  signals:
    void yesSignal();
    void cancelSignal();

  protected:
    virtual bool validate() const;
    void addButtonStretchOnce();

    QFrame *m_buttonGroup = nullptr;
    QPushButton *m_acceptButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QVBoxLayout *m_viewLayout = nullptr;
    QHBoxLayout *m_buttonLayout = nullptr;
    bool m_buttonStretchInserted = false;
};

class FQW_API MessageBox : public MessageBoxBase
{
    Q_OBJECT
    Q_PROPERTY(QLabel *titleLabel READ titleLabel)
    Q_PROPERTY(QLabel *contentLabel READ contentLabel)

  public:
    MessageBox(const QString &title, const QString &message, QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setContentCopyable(bool copyable);
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    QLabel *messageLabel() const;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    void adjustMessageSize();

    QString m_content;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_messageLabel = nullptr;
};

class FQW_API MessageDialog : public MessageBox
{
    Q_OBJECT

  public:
    MessageDialog(const QString &title, const QString &message, QWidget *parent = nullptr);
};

} // namespace FluentQt
