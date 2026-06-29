#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtWidgets/QWidget>

class QHBoxLayout;

namespace FluentQt {

class FQW_API BreadcrumbItem
{
  public:
    BreadcrumbItem() = default;
    BreadcrumbItem(QString key, QString text);

    QString key;
    QString text;
};

class FQW_API BreadcrumbBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentItem READ currentItem WRITE setCurrentItem NOTIFY currentItemChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)

  public:
    explicit BreadcrumbBar(QWidget *parent = nullptr);

    void addItem(const QString &key, const QString &text);
    void setItems(const QList<BreadcrumbItem> &items);
    void clear();
    void popItem();
    QList<BreadcrumbItem> items() const;
    BreadcrumbItem itemAt(int index) const;
    BreadcrumbItem item(const QString &key) const;
    void setItemText(const QString &key, const QString &text);
    int count() const;
    QString currentItem() const;
    int currentIndex() const;
    int spacing() const;

  public slots:
    void setCurrentItem(const QString &key);
    void setCurrentIndex(int index);
    void setSpacing(int spacing);

  signals:
    void itemClicked(const QString &key, int index);
    void currentItemChanged(const QString &key);
    void currentIndexChanged(int index);

  private:
    void rebuild();

    QHBoxLayout *m_layout = nullptr;
    QList<BreadcrumbItem> m_items;
    int m_currentIndex = -1;
    int m_spacing = 10;
};

} // namespace FluentQt
