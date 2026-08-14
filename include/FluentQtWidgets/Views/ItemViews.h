#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>

#include <QtCore/QItemSelection>
#include <QtCore/QModelIndex>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QSet>
#include <QtGui/QColor>
#include <QtGui/QHelpEvent>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QTreeWidget>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QResizeEvent;
class QStyleOptionViewItem;

namespace FluentQt {

class FQW_API TableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit TableItemDelegate(QObject *parent = nullptr);

    void setHoverRow(int row);
    void setPressedRow(int row);
    void setSelectedRows(const QModelIndexList &indexes);
    void setCheckedColor(const QColor &light, const QColor &dark);
    QColor checkedColor(Theme theme = Theme::Auto) const;

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
                   const QModelIndex &index) override;

  protected:
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void drawIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void drawCheckBox(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    bool isRowSelected(int row) const;
    bool isRowHovered(int row) const;
    bool isRowPressed(int row) const;

    ItemViewToolTipDelegate *m_toolTipDelegate = nullptr;

  private:
    int m_margin = 2;
    int m_hoverRow = -1;
    int m_pressedRow = -1;
    QSet<int> m_selectedRows;
    QColor m_lightCheckedColor;
    QColor m_darkCheckedColor;
};

class FQW_API ListItemDelegate : public TableItemDelegate
{
    Q_OBJECT

  public:
    explicit ListItemDelegate(QObject *parent = nullptr);

  protected:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class FQW_API TreeItemDelegate : public TableItemDelegate
{
    Q_OBJECT

  public:
    explicit TreeItemDelegate(QObject *parent = nullptr);

    void setHoverIndex(const QModelIndex &index);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  protected:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawCheckBox(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const override;

  private:
    bool isIndexHovered(const QModelIndex &index) const;

    QPersistentModelIndex m_hoverIndex;
};

class FQW_API ListView : public QListView
{
    Q_OBJECT
    Q_PROPERTY(bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible)
    Q_PROPERTY(FluentQt::ListItemDelegate *delegate READ delegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelegate READ scrollDelegate)

  public:
    explicit ListView(QWidget *parent = nullptr);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool enabled);
    bool isBorderVisible() const;
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor &light, const QColor &dark);
    ListItemDelegate *fluentItemDelegate() const;
    ListItemDelegate *delegate() const;
    SmoothScrollDelegate *scrollDelegate() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  protected:
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

  private:
    void init();
    void updateSelectedRows();

    bool m_selectRightClickedRow = false;
    ListItemDelegate *m_delegate = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API ListWidget : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible)
    Q_PROPERTY(FluentQt::ListItemDelegate *delegate READ delegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelegate READ scrollDelegate)

  public:
    explicit ListWidget(QWidget *parent = nullptr);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool enabled);
    bool isBorderVisible() const;
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor &light, const QColor &dark);
    ListItemDelegate *fluentItemDelegate() const;
    ListItemDelegate *delegate() const;
    SmoothScrollDelegate *scrollDelegate() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  protected:
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

  private:
    void init();
    void updateSelectedRows();

    bool m_selectRightClickedRow = false;
    ListItemDelegate *m_delegate = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API TableView : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible)
    Q_PROPERTY(FluentQt::TableItemDelegate *delegate READ delegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelagate READ scrollDelagate)

  public:
    explicit TableView(QWidget *parent = nullptr);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool enabled);
    bool isBorderVisible() const;
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor &light, const QColor &dark);
    TableItemDelegate *fluentItemDelegate() const;
    TableItemDelegate *delegate() const;
    SmoothScrollDelegate *scrollDelegate() const;
    SmoothScrollDelegate *scrollDelagate() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  protected:
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

  private:
    void init();
    void updateSelectedRows();

    bool m_selectRightClickedRow = false;
    TableItemDelegate *m_delegate = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API TableWidget : public QTableWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible)
    Q_PROPERTY(FluentQt::TableItemDelegate *delegate READ delegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelagate READ scrollDelagate)

  public:
    explicit TableWidget(QWidget *parent = nullptr);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool enabled);
    bool isBorderVisible() const;
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor &light, const QColor &dark);
    TableItemDelegate *fluentItemDelegate() const;
    TableItemDelegate *delegate() const;
    SmoothScrollDelegate *scrollDelegate() const;
    SmoothScrollDelegate *scrollDelagate() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  protected:
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

  private:
    void init();
    void updateSelectedRows();

    bool m_selectRightClickedRow = false;
    TableItemDelegate *m_delegate = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API TreeView : public QTreeView
{
    Q_OBJECT
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible)
    Q_PROPERTY(FluentQt::TreeItemDelegate *delegate READ delegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelagate READ scrollDelagate)

  public:
    explicit TreeView(QWidget *parent = nullptr);

    bool isBorderVisible() const;
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor &light, const QColor &dark);
    TreeItemDelegate *fluentItemDelegate() const;
    TreeItemDelegate *delegate() const;
    SmoothScrollDelegate *scrollDelegate() const;
    SmoothScrollDelegate *scrollDelagate() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  protected:
    void leaveEvent(QEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;

  private:
    void init();

    TreeItemDelegate *m_delegate = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API TreeWidget : public QTreeWidget
{
    Q_OBJECT
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible)
    Q_PROPERTY(FluentQt::TreeItemDelegate *delegate READ delegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelagate READ scrollDelagate)

  public:
    explicit TreeWidget(QWidget *parent = nullptr);

    bool isBorderVisible() const;
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor &light, const QColor &dark);
    TreeItemDelegate *fluentItemDelegate() const;
    TreeItemDelegate *delegate() const;
    SmoothScrollDelegate *scrollDelegate() const;
    SmoothScrollDelegate *scrollDelagate() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  protected:
    void leaveEvent(QEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;

  private:
    void init();

    TreeItemDelegate *m_delegate = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

} // namespace FluentQt
