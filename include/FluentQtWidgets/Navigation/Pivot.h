#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtWidgets/QWidget>

class QButtonGroup;
class QHBoxLayout;
class QPropertyAnimation;

namespace FluentQt {

class FQW_API PivotItem : public PushButton
{
    Q_OBJECT
    Q_PROPERTY(QString routeKey READ routeKey WRITE setRouteKey)

  public:
    explicit PivotItem(QWidget *parent = nullptr);
    PivotItem(const QString &routeKey, const QString &text, QWidget *parent = nullptr);

    QString routeKey() const;
    void setRouteKey(const QString &routeKey);

  private:
    QString m_routeKey;
};

class FQW_API Pivot : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentItem READ currentItem WRITE setCurrentItem NOTIFY currentItemChanged)
    Q_PROPERTY(QRect indicatorGeometry READ indicatorGeometry WRITE setIndicatorGeometry)

  public:
    explicit Pivot(QWidget *parent = nullptr);

    PivotItem *addItem(const QString &routeKey, const QString &text);
    PivotItem *insertItem(int index, const QString &routeKey, const QString &text);
    void removeItem(const QString &routeKey);
    bool contains(const QString &routeKey) const;
    QString currentItem() const;
    PivotItem *item(const QString &routeKey) const;
    QList<QString> routeKeys() const;
    QRect indicatorGeometry() const;
    void setIndicatorGeometry(const QRect &geometry);

  public slots:
    void setCurrentItem(const QString &routeKey);

  signals:
    void currentItemChanged(const QString &routeKey);
    void itemClicked(const QString &routeKey);

  protected:
    virtual PivotItem *createItem(const QString &routeKey, const QString &text);
    QHBoxLayout *layoutHandle() const;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QRect computeIndicatorGeometry() const;

  private:
    void adjustIndicatorPosition();

    QButtonGroup *m_buttonGroup = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QHash<QString, PivotItem *> m_items;
    QString m_currentItem;

    QPropertyAnimation *m_slideAnimation = nullptr;
    QRect m_indicatorGeometry;
};

} // namespace FluentQt
