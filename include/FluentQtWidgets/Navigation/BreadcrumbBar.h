#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtWidgets/QWidget>

namespace FluentQt {

// Base of the breadcrumb items and the ellipsis button: tracks hover and press and emits
// clicked() on release, as Python's BreadcrumbWidget does.
class FQW_API BreadcrumbWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit BreadcrumbWidget(QWidget *parent = nullptr);

    bool isHover() const;
    bool isPressed() const;

  signals:
    void clicked();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

    bool m_isHover = false;
    bool m_isPressed = false;
};

// The "…" button that stands for the items that do not fit; 16 × 16 for a 14 px font.
class FQW_API ElideButton : public BreadcrumbWidget
{
    Q_OBJECT

  public:
    explicit ElideButton(QWidget *parent = nullptr);

    void clearState();

  protected:
    void paintEvent(QPaintEvent *event) override;
};

// One item of a BreadcrumbBar. Every item but the root paints its chevron in front of the text,
// spacing() px either side of it; the height is the text's.
class FQW_API BreadcrumbItem : public BreadcrumbWidget
{
    Q_OBJECT

  public:
    BreadcrumbItem(const QString &routeKey, const QString &text, int index, QWidget *parent = nullptr);

    QString routeKey() const;
    QString text() const;
    void setText(const QString &text);
    int index() const;
    bool isRoot() const;
    bool isSelected() const;
    void setSelected(bool isSelected);
    int spacing() const;
    void setSpacing(int spacing);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

  private:
    QString m_routeKey;
    QString m_text;
    int m_index = 0;
    int m_spacing = 5;
    bool m_isSelected = false;
};

class FQW_API BreadcrumbBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)

  public:
    explicit BreadcrumbBar(QWidget *parent = nullptr);

    void addItem(const QString &routeKey, const QString &text);
    void setItemText(const QString &routeKey, const QString &text);
    BreadcrumbItem *item(const QString &routeKey) const;
    BreadcrumbItem *itemAt(int index) const;
    QList<BreadcrumbItem *> items() const;
    QList<BreadcrumbItem *> hiddenItems() const;
    ElideButton *elideButton() const;
    int currentIndex() const;
    BreadcrumbItem *currentItem() const;
    void clear();
    void popItem();
    int count() const;
    bool isElideVisible() const;
    int spacing() const;

    // Places the items from the left, and when they do not fit, hides the leading ones behind
    // the ellipsis button (the last item always stays). Hides QWidget::updateGeometry(), which
    // it also calls, as Python's override does.
    void updateGeometry();

  public slots:
    void setCurrentIndex(int index);
    void setCurrentItem(const QString &routeKey);
    void setSpacing(int spacing);

  signals:
    void currentItemChanged(const QString &routeKey);
    void currentIndexChanged(int index);

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

  private:
    void showHiddenItemsMenu();
    void removeItem(BreadcrumbItem *item);

    QHash<QString, BreadcrumbItem *> m_itemMap;
    QList<BreadcrumbItem *> m_items;
    QList<BreadcrumbItem *> m_hiddenItems;
    ElideButton *m_elideButton = nullptr;
    int m_spacing = 10;
    int m_currentIndex = -1;
};

} // namespace FluentQt
