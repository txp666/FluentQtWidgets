#pragma once

#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QRect>

class QButtonGroup;
class QHBoxLayout;
class QPaintEvent;
class QPropertyAnimation;
class QResizeEvent;
class QShowEvent;

namespace FluentQt {

class FQW_API SegmentedItem : public PivotItem
{
    Q_OBJECT

  public:
    explicit SegmentedItem(QWidget *parent = nullptr);
    SegmentedItem(const QString &routeKey, const QString &text, QWidget *parent = nullptr);
};

class FQW_API SegmentedWidget : public Pivot
{
    Q_OBJECT

  public:
    explicit SegmentedWidget(QWidget *parent = nullptr);

  protected:
    PivotItem *createItem(const QString &routeKey, const QString &text) override;
    void paintEvent(QPaintEvent *event) override;
};

// An icon-only item of a SegmentedToolWidget: 38 x 33, its 16 px icon centred, as Python's
// SegmentedToolItem (a ToolButton) draws it.
class FQW_API SegmentedToolItem : public PivotItem
{
    Q_OBJECT

  public:
    explicit SegmentedToolItem(const QIcon &icon = QIcon(), QWidget *parent = nullptr);
    SegmentedToolItem(const QString &routeKey, const QIcon &icon, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

// A segmented control of icons; Python's SegmentedToolWidget.
class FQW_API SegmentedToolWidget : public SegmentedWidget
{
    Q_OBJECT

  public:
    explicit SegmentedToolWidget(QWidget *parent = nullptr);

    SegmentedToolItem *addItem(const QString &routeKey, const QIcon &icon);
    SegmentedToolItem *insertItem(int index, const QString &routeKey, const QIcon &icon);

  protected:
    PivotItem *createItem(const QString &routeKey, const QString &text) override;
};

class FQW_API SegmentedToggleToolWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentItem READ currentItem WRITE setCurrentItem NOTIFY currentItemChanged)
    Q_PROPERTY(QRect selectionGeometry READ selectionGeometry WRITE setSelectionGeometry)

  public:
    explicit SegmentedToggleToolWidget(QWidget *parent = nullptr);

    TransparentToggleToolButton *addItem(const QString &routeKey, const QIcon &icon);
    TransparentToggleToolButton *insertItem(int index, const QString &routeKey, const QIcon &icon);
    void removeItem(const QString &routeKey);
    bool contains(const QString &routeKey) const;
    QString currentItem() const;
    TransparentToggleToolButton *item(const QString &routeKey) const;
    QList<QString> routeKeys() const;
    QRect selectionGeometry() const;
    void setSelectionGeometry(const QRect &geometry);

  public slots:
    void setCurrentItem(const QString &routeKey);

  signals:
    void currentItemChanged(const QString &routeKey);
    void itemClicked(const QString &routeKey);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

  private:
    void updateCheckedButton();
    QRect computeSelectionGeometry() const;

    QButtonGroup *m_buttonGroup = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QPropertyAnimation *m_selectionAnimation = nullptr;
    QList<TransparentToggleToolButton *> m_items;
    QHash<QString, TransparentToggleToolButton *> m_itemMap;
    QString m_currentItem;
    QRect m_selectionGeometry;
};

} // namespace FluentQt
