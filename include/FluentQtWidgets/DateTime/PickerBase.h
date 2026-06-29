#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtGui/QColor>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class QBoxLayout;
class QFrame;
class QHBoxLayout;
class QPaintEvent;
class QVBoxLayout;

namespace FluentQt {

class CycleListWidget;
class TransparentToolButton;

class FQW_API SeparatorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation)

  public:
    explicit SeparatorWidget(Qt::Orientation orientation, QWidget *parent = nullptr);

    Qt::Orientation orientation() const;

  private:
    Qt::Orientation m_orientation = Qt::Horizontal;
};

class FQW_API ItemMaskWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor WRITE setLightBackgroundColor)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor WRITE setDarkBackgroundColor)

  public:
    explicit ItemMaskWidget(const QList<CycleListWidget *> &listWidgets = {}, QWidget *parent = nullptr);

    QList<CycleListWidget *> listWidgets() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;

  public slots:
    void setListWidgets(const QList<CycleListWidget *> &widgets);
    void setLightBackgroundColor(const QColor &color);
    void setDarkBackgroundColor(const QColor &color);
    void setCustomBackgroundColor(const QColor &light, const QColor &dark);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QColor backgroundColor() const;

    QList<CycleListWidget *> m_listWidgets;
    QColor m_lightBackgroundColor;
    QColor m_darkBackgroundColor;
};

class FQW_API PickerColumnFormatter : public QObject
{
    Q_OBJECT

  public:
    explicit PickerColumnFormatter(QObject *parent = nullptr);

    Q_INVOKABLE virtual QString encode(const QVariant &value) const;
    Q_INVOKABLE virtual QVariant decode(const QString &value) const;
};

class FQW_API DigitFormatter : public PickerColumnFormatter
{
    Q_OBJECT

  public:
    using PickerColumnFormatter::PickerColumnFormatter;

    QString encode(const QVariant &value) const override;
    QVariant decode(const QString &value) const override;
};

class FQW_API PickerColumnButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QVariant value READ value WRITE setValue)
    Q_PROPERTY(QVariant initialValue READ initialValue WRITE setInitialValue)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

  public:
    PickerColumnButton(const QString &name, const QVariantList &items, int width,
                       Qt::Alignment alignment = Qt::AlignLeft, PickerColumnFormatter *formatter = nullptr,
                       QWidget *parent = nullptr);

    QString name() const;
    QVariantList rawItems() const;
    QStringList items() const;
    QVariant value() const;
    QVariant initialValue() const;
    QString displayValue() const;
    Qt::Alignment alignment() const;
    PickerColumnFormatter *formatter() const;

  public slots:
    void setName(const QString &name);
    void setItems(const QVariantList &items);
    void setValue(const QVariant &value);
    void setInitialValue(const QVariant &value);
    void setAlignment(Qt::Alignment alignment);
    void setFormatter(PickerColumnFormatter *formatter);

  private:
    void syncDisplayText();

    QString m_name;
    QVariantList m_items;
    QVariant m_value;
    QVariant m_initialValue;
    Qt::Alignment m_alignment = Qt::AlignLeft;
    PickerColumnFormatter *m_formatter = nullptr;
    PickerColumnFormatter *m_defaultFormatter = nullptr;
};

class FQW_API PickerPanel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool resetEnabled READ isResetEnabled WRITE setResetEnabled)
    Q_PROPERTY(bool scrollButtonRepeatEnabled READ isScrollButtonRepeatEnabled WRITE setScrollButtonRepeatEnabled)

  public:
    explicit PickerPanel(QWidget *parent = nullptr);

    QFrame *view() const;
    ItemMaskWidget *itemMaskWidget() const;
    SeparatorWidget *horizontalSeparatorWidget() const;
    QHBoxLayout *listLayout() const;
    QHBoxLayout *buttonLayout() const;
    QVBoxLayout *viewLayout() const;
    QList<CycleListWidget *> listWidgets() const;
    TransparentToolButton *yesButton() const;
    TransparentToolButton *resetButton() const;
    TransparentToolButton *cancelButton() const;
    QStringList value() const;
    QStringList columnItems(int index) const;
    QString columnValue(int index) const;
    CycleListWidget *column(int index) const;
    bool isResetEnabled() const;
    bool isScrollButtonRepeatEnabled() const;

    void exec(const QPoint &pos, bool animated = true);

  public slots:
    void addColumn(const QStringList &items, int width, Qt::Alignment alignment = Qt::AlignCenter);
    void setPanelRole(const QString &role);
    void setViewRole(const QString &role);
    void setValue(const QStringList &value);
    void setColumnItems(int index, const QStringList &items);
    void setColumnValue(int index, const QString &value);
    void setResetEnabled(bool enabled);
    void setScrollButtonRepeatEnabled(bool enabled);
    void setSelectedBackgroundColor(const QColor &light, const QColor &dark);

  signals:
    void confirmed(const QStringList &value);
    void resetted();
    void columnValueChanged(int index, const QString &value);

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    QPoint clampedPosition(const QPoint &preferred) const;
    void updateAnimationMask(qreal opacity);
    void fadeOut();

    QFrame *m_view = nullptr;
    ItemMaskWidget *m_itemMaskWidget = nullptr;
    SeparatorWidget *m_hSeparatorWidget = nullptr;
    QHBoxLayout *m_hBoxLayout = nullptr;
    QHBoxLayout *m_listLayout = nullptr;
    QHBoxLayout *m_buttonLayout = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QList<CycleListWidget *> m_listWidgets;
    TransparentToolButton *m_yesButton = nullptr;
    TransparentToolButton *m_resetButton = nullptr;
    TransparentToolButton *m_cancelButton = nullptr;
    bool m_scrollButtonRepeatEnabled = true;
    bool m_isExpanded = false;
};

} // namespace FluentQt
