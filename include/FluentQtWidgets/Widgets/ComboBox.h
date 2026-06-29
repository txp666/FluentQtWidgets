#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtCore/QMetaObject>
#include <QtCore/QPointer>
#include <QtCore/QVariant>
#include <QtCore/QVariantAnimation>
#include <QtWidgets/QPushButton>

namespace FluentQt {

class RoundMenu;
class ComboBoxMenu;

struct FQW_API ComboItem
{
    QString text;
    QIcon icon;
    QVariant userData;
    bool isEnabled = true;

    ComboItem() = default;
    ComboItem(const QString &t, const QIcon &ico = QIcon(), const QVariant &data = QVariant(), bool enabled = true)
        : text(t), icon(ico), userData(data), isEnabled(enabled)
    {
    }
};

class FQW_API ArrowAnimation : public QVariantAnimation
{
    Q_OBJECT
    Q_PROPERTY(qreal y READ y NOTIFY yChanged)

  public:
    explicit ArrowAnimation(QObject *parent = nullptr)
        : QVariantAnimation(parent), m_y(0)
    {
        setStartValue(0.0);
        connect(this, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_y = v.toReal();
            emit yChanged(m_y);
        });
    }

    qreal y() const { return m_y; }

  signals:
    void yChanged(qreal y);

  private:
    qreal m_y;
};

class FQW_API ComboBox : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaceholderText READ isPlaceholderText WRITE setPlaceholderTextProperty NOTIFY
                   placeholderTextChanged)
    Q_PROPERTY(ArrowAnimation *arrowAni READ arrowAni)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit ComboBox(QWidget *parent = nullptr);

    void addItem(const QString &text, const QIcon &icon = QIcon(), const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void removeItem(int index);
    void insertItem(int index, const QString &text, const QIcon &icon = QIcon(),
                    const QVariant &userData = QVariant());
    void insertItems(int index, const QStringList &texts);

    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentText() const;
    QVariant currentData() const;
    void setCurrentText(const QString &text);

    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    QVariant itemData(int index) const;
    void setItemText(int index, const QString &text);
    void setItemIcon(int index, const QIcon &icon);
    void setItemData(int index, const QVariant &value);
    void setItemEnabled(int index, bool enabled);

    int findText(const QString &text) const;
    int findData(const QVariant &data) const;
    int count() const;
    void clear();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);

    QString placeholderText() const;
    void setPlaceholderText(const QString &text);

    bool isPlaceholderText() const;
    void setPlaceholderTextProperty(bool v);
    bool isPressed() const;
    bool isHover() const;
    ArrowAnimation *arrowAni() const;
    ComboBoxMenu *dropMenu() const;

    QList<ComboItem> &items();

  signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString &text);
    void activated(int index);
    void textActivated(const QString &text);
    void placeholderTextChanged(bool);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent *e) override;
    virtual ComboBoxMenu *createComboMenu();

  private:
    void showComboMenu();
    void closeComboMenu();
    void toggleComboMenu();
    void onItemClicked(int index);
    void onDropMenuClosed();

    QList<ComboItem> m_items;
    int m_currentIndex = -1;
    int m_maxVisibleItems = -1;
    QString m_placeholderText;
    bool m_isPlaceholderText = false;
    bool m_isHover = false;
    bool m_isPressed = false;
    ArrowAnimation *m_arrowAni = nullptr;
    QPointer<ComboBoxMenu> m_dropMenu;
};

class FQW_API ComboBoxMenu : public RoundMenu
{
    Q_OBJECT

  public:
    explicit ComboBoxMenu(QWidget *parent = nullptr);
    QAction *exec(const QPoint &pos, bool animated = true,
                  MenuAnimationType animationType = MenuAnimationType::DropDown);
};

class FQW_API EditableComboBox : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaceholderText READ isPlaceholderText WRITE setPlaceholderTextProperty NOTIFY
                   placeholderTextChanged)
    Q_PROPERTY(LineEditButton *dropButton READ dropButton)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit EditableComboBox(QWidget *parent = nullptr);

    void addItem(const QString &text, const QIcon &icon = QIcon(), const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void removeItem(int index);
    void insertItem(int index, const QString &text, const QIcon &icon = QIcon(),
                    const QVariant &userData = QVariant());
    void insertItems(int index, const QStringList &texts);

    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentText() const;
    QVariant currentData() const;
    void setCurrentText(const QString &text);

    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    QVariant itemData(int index) const;
    void setItemText(int index, const QString &text);
    void setItemIcon(int index, const QIcon &icon);
    void setItemData(int index, const QVariant &value);
    void setItemEnabled(int index, bool enabled);

    int findText(const QString &text) const;
    int findData(const QVariant &data) const;
    int count() const;
    void clear();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);

    void setCompleterMenu(CompleterMenu *menu) override;

    QString placeholderText() const;
    void setPlaceholderText(const QString &text);

    bool isPlaceholderText() const;
    void setPlaceholderTextProperty(bool v);
    bool isPressed() const;
    bool isHover() const;
    LineEditButton *dropButton() const;
    ComboBoxMenu *dropMenu() const;
    QList<ComboItem> &items();

  signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString &text);
    void activated(int index);
    void textActivated(const QString &text);
    void placeholderTextChanged(bool);

  protected:
    bool event(QEvent *event) override;
    virtual ComboBoxMenu *createComboMenu();
    void showComboMenu();
    void closeComboMenu();
    void toggleComboMenu();
    void onItemClicked(int index);
    void onDropMenuClosed();
    void onComboTextChanged(const QString &text);
    void onReturnPressed();
    void onClearButtonClicked();
    void onCompleterActivated(const QString &text);
    void updatePlaceholderState(bool isPlaceholder);

    QList<ComboItem> m_items;
    int m_currentIndex = -1;
    int m_maxVisibleItems = -1;
    QString m_placeholderText;
    bool m_isPlaceholderText = false;
    bool m_isHover = false;
    bool m_isPressed = false;
    LineEditButton *m_dropButton = nullptr;
    QPointer<ComboBoxMenu> m_dropMenu;
};

class FQW_API ModelComboBox : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaceholderText READ isPlaceholderText WRITE setPlaceholderTextProperty NOTIFY
                   placeholderTextChanged)
    Q_PROPERTY(bool iconVisible READ isIconVisible WRITE setIconVisible)
    Q_PROPERTY(ArrowAnimation *arrowAni READ arrowAni)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit ModelComboBox(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    QModelIndex addItem(const QString &text, const QIcon &icon = QIcon(), const QVariant &userData = QVariant());
    QModelIndex insertItem(int index, const QString &text, const QIcon &icon = QIcon(),
                           const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void insertItems(int index, const QStringList &texts);
    void removeItem(int index);

    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentText() const;
    QVariant currentData(int role = Qt::UserRole) const;
    void setCurrentText(const QString &text);

    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    QVariant itemData(int index, int role = Qt::UserRole) const;
    void setItemText(int index, const QString &text);
    void setItemIcon(int index, const QIcon &icon);
    void setItemData(int index, const QVariant &value, int role = Qt::UserRole);

    int findText(const QString &text, Qt::MatchFlags flags = Qt::MatchExactly) const;
    int findData(const QVariant &data, int role = Qt::UserRole,
                 Qt::MatchFlags flags = Qt::MatchExactly) const;
    int count() const;
    void clear();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);

    QString placeholderText() const;
    void setPlaceholderText(const QString &text);

    bool isPlaceholderText() const;
    void setPlaceholderTextProperty(bool v);
    bool isPressed() const;
    bool isHover() const;
    ArrowAnimation *arrowAni() const;

    bool isIconVisible() const;
    void setIconVisible(bool visible);
    ComboBoxMenu *dropMenu() const;

  signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString &text);
    void activated(int index);
    void textActivated(const QString &text);
    void placeholderTextChanged(bool);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    virtual ComboBoxMenu *createComboMenu();

  private:
    QModelIndex modelIndex(int row) const;
    bool isValidIndex(int index) const;
    void bindModelSignals();
    void updateCurrentIcon();
    void showComboMenu();
    void closeComboMenu();
    void toggleComboMenu();
    void onItemClicked(int index);
    void onDropMenuClosed();
    void onModelRowsInserted(const QModelIndex &parent, int first, int last);
    void onModelRowsRemoved(const QModelIndex &parent, int first, int last);
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                            const QList<int> &roles);
    void onModelReset();

    QAbstractItemModel *m_model = nullptr;
    QList<QMetaObject::Connection> m_modelConnections;
    int m_currentIndex = -1;
    int m_maxVisibleItems = -1;
    QString m_placeholderText;
    bool m_isPlaceholderText = false;
    bool m_isIconVisible = true;
    bool m_isHover = false;
    bool m_isPressed = false;
    ArrowAnimation *m_arrowAni = nullptr;
    QPointer<ComboBoxMenu> m_dropMenu;
};

class FQW_API EditableModelComboBox : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaceholderText READ isPlaceholderText WRITE setPlaceholderTextProperty NOTIFY
                   placeholderTextChanged)
    Q_PROPERTY(LineEditButton *dropButton READ dropButton)
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isHover READ isHover)

  public:
    explicit EditableModelComboBox(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    QModelIndex addItem(const QString &text, const QIcon &icon = QIcon(), const QVariant &userData = QVariant());
    QModelIndex insertItem(int index, const QString &text, const QIcon &icon = QIcon(),
                           const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void insertItems(int index, const QStringList &texts);
    void removeItem(int index);

    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentText() const;
    QVariant currentData(int role = Qt::UserRole) const;
    void setCurrentText(const QString &text);

    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    QVariant itemData(int index, int role = Qt::UserRole) const;
    void setItemText(int index, const QString &text);
    void setItemIcon(int index, const QIcon &icon);
    void setItemData(int index, const QVariant &value, int role = Qt::UserRole);

    int findText(const QString &text, Qt::MatchFlags flags = Qt::MatchExactly) const;
    int findData(const QVariant &data, int role = Qt::UserRole,
                 Qt::MatchFlags flags = Qt::MatchExactly) const;
    int count() const;
    void clear();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);

    void setCompleterMenu(CompleterMenu *menu) override;

    QString placeholderText() const;
    void setPlaceholderText(const QString &text);

    bool isPlaceholderText() const;
    void setPlaceholderTextProperty(bool v);
    bool isPressed() const;
    bool isHover() const;
    LineEditButton *dropButton() const;
    ComboBoxMenu *dropMenu() const;

  signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString &text);
    void activated(int index);
    void textActivated(const QString &text);
    void placeholderTextChanged(bool);

  protected:
    bool event(QEvent *event) override;
    virtual ComboBoxMenu *createComboMenu();
    QModelIndex modelIndex(int row) const;
    bool isValidIndex(int index) const;
    void bindModelSignals();
    void showComboMenu();
    void closeComboMenu();
    void toggleComboMenu();
    void onItemClicked(int index);
    void onDropMenuClosed();
    void onComboTextChanged(const QString &text);
    void onReturnPressed();
    void onClearButtonClicked();
    void onCompleterActivated(const QString &text);
    void updatePlaceholderState(bool isPlaceholder);
    void onModelRowsInserted(const QModelIndex &parent, int first, int last);
    void onModelRowsRemoved(const QModelIndex &parent, int first, int last);
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                            const QList<int> &roles);
    void onModelReset();

    QAbstractItemModel *m_model = nullptr;
    QList<QMetaObject::Connection> m_modelConnections;
    int m_currentIndex = -1;
    int m_maxVisibleItems = -1;
    QString m_placeholderText;
    bool m_isPlaceholderText = false;
    bool m_isHover = false;
    bool m_isPressed = false;
    LineEditButton *m_dropButton = nullptr;
    QPointer<ComboBoxMenu> m_dropMenu;
};

class FQW_API AcrylicComboBoxMenu : public ComboBoxMenu
{
    Q_OBJECT

public:
    explicit AcrylicComboBoxMenu(QWidget *parent = nullptr);
};

class FQW_API AcrylicComboBox : public ComboBox
{
    Q_OBJECT

public:
    explicit AcrylicComboBox(QWidget *parent = nullptr);

protected:
    ComboBoxMenu *createComboMenu() override;
};

class FQW_API AcrylicEditableComboBox : public EditableComboBox
{
    Q_OBJECT

public:
    explicit AcrylicEditableComboBox(QWidget *parent = nullptr);

protected:
    ComboBoxMenu *createComboMenu() override;
    CompleterMenu *createCompleterMenu() override;
    LineEditMenu *createContextMenu() override;
};

} // namespace FluentQt
