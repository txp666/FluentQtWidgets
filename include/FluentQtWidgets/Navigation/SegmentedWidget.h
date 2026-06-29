#pragma once

#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtCore/QHash>
#include <QtCore/QList>

class QButtonGroup;
class QHBoxLayout;

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
};

class FQW_API SegmentedToggleToolWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentItem READ currentItem WRITE setCurrentItem NOTIFY currentItemChanged)

  public:
    explicit SegmentedToggleToolWidget(QWidget *parent = nullptr);

    TransparentToggleToolButton *addItem(const QString &routeKey, const QIcon &icon);
    TransparentToggleToolButton *insertItem(int index, const QString &routeKey, const QIcon &icon);
    void removeItem(const QString &routeKey);
    bool contains(const QString &routeKey) const;
    QString currentItem() const;
    TransparentToggleToolButton *item(const QString &routeKey) const;
    QList<QString> routeKeys() const;

  public slots:
    void setCurrentItem(const QString &routeKey);

  signals:
    void currentItemChanged(const QString &routeKey);
    void itemClicked(const QString &routeKey);

  private:
    void updateCheckedButton();

    QButtonGroup *m_buttonGroup = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QList<TransparentToggleToolButton *> m_items;
    QHash<QString, TransparentToggleToolButton *> m_itemMap;
    QString m_currentItem;
};

} // namespace FluentQt
