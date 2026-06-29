#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Flyout.h>

#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QToolButton>

class QMouseEvent;

namespace FluentQt {

enum class SpinIconType
{
    Up,
    Down
};

class FQW_API SpinButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::SpinIconType iconType READ iconType WRITE setIconType)
    Q_PROPERTY(bool isPressed READ isPressed)

public:
    explicit SpinButton(SpinIconType icon, QWidget *parent = nullptr);

    SpinIconType iconType() const;
    bool isPressed() const;
    void setIconType(SpinIconType icon);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    SpinIconType m_icon;
    bool m_isPressed = false;
};

class FQW_API CompactSpinButton : public QToolButton
{
    Q_OBJECT

public:
    explicit CompactSpinButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API SpinFlyoutView : public FlyoutViewBase
{
    Q_OBJECT

public:
    explicit SpinFlyoutView(QWidget *parent = nullptr);

    TransparentToolButton *upButtonWidget() const;
    TransparentToolButton *downButtonWidget() const;
    QVBoxLayout *vBoxLayout() const;

    TransparentToolButton *upButton = nullptr;
    TransparentToolButton *downButton = nullptr;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout *m_layout = nullptr;
};

// Internal helper - not exported, used in SpinBox variants
class SpinBoxHelper
{
public:
    enum Mode
    {
        Inline,
        Compact
    };

    SpinBoxHelper();
    ~SpinBoxHelper();

    void init(QAbstractSpinBox *spinBox, Mode mode);
    bool isError() const;
    bool isSymbolVisible() const;
    void setError(bool error);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setReadOnly(bool readOnly);
    void drawBorderBottom(QPainter &painter);
    void onFocusIn();
    void showContextMenu(const QPoint &pos);
    SpinButton *upButton() const;
    SpinButton *downButton() const;
    CompactSpinButton *compactSpinButton() const;
    SpinFlyoutView *spinFlyoutView() const;
    Flyout *spinFlyout() const;
    QHBoxLayout *hBoxLayout() const;

private:
    QColor focusedBorderColor() const;

    QAbstractSpinBox *m_spinBox = nullptr;
    Mode m_mode = Inline;
    bool m_isError = false;
    bool m_symbolVisible = true;
    QColor m_lightFocusedBorderColor;
    QColor m_darkFocusedBorderColor;
    QHBoxLayout *m_layout = nullptr;
    SpinButton *m_upButton = nullptr;
    SpinButton *m_downButton = nullptr;
    CompactSpinButton *m_compactButton = nullptr;
    SpinFlyoutView *m_flyoutView = nullptr;
    Flyout *m_flyout = nullptr;
};

// ---------------------------------------------------------------------------
// Inline SpinBox variants (with up/down buttons inside)
// ---------------------------------------------------------------------------

class FQW_API SpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit SpinBox(QWidget *parent = nullptr);
    ~SpinBox() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    SpinButton *upButton() const;
    SpinButton *downButton() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit DoubleSpinBox(QWidget *parent = nullptr);
    ~DoubleSpinBox() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    SpinButton *upButton() const;
    SpinButton *downButton() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API TimeEdit : public QTimeEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit TimeEdit(QWidget *parent = nullptr);
    ~TimeEdit() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    SpinButton *upButton() const;
    SpinButton *downButton() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API DateEdit : public QDateEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit DateEdit(QWidget *parent = nullptr);
    ~DateEdit() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    SpinButton *upButton() const;
    SpinButton *downButton() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API DateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit DateTimeEdit(QWidget *parent = nullptr);
    ~DateTimeEdit() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    SpinButton *upButton() const;
    SpinButton *downButton() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

// ---------------------------------------------------------------------------
// Compact SpinBox variants (compact button + flyout)
// ---------------------------------------------------------------------------

class FQW_API CompactSpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit CompactSpinBox(QWidget *parent = nullptr);
    ~CompactSpinBox() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    CompactSpinButton *compactSpinButton() const;
    SpinFlyoutView *spinFlyoutView() const;
    Flyout *spinFlyout() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API CompactDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit CompactDoubleSpinBox(QWidget *parent = nullptr);
    ~CompactDoubleSpinBox() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    CompactSpinButton *compactSpinButton() const;
    SpinFlyoutView *spinFlyoutView() const;
    Flyout *spinFlyout() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API CompactTimeEdit : public QTimeEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit CompactTimeEdit(QWidget *parent = nullptr);
    ~CompactTimeEdit() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    CompactSpinButton *compactSpinButton() const;
    SpinFlyoutView *spinFlyoutView() const;
    Flyout *spinFlyout() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API CompactDateEdit : public QDateEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit CompactDateEdit(QWidget *parent = nullptr);
    ~CompactDateEdit() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    CompactSpinButton *compactSpinButton() const;
    SpinFlyoutView *spinFlyoutView() const;
    Flyout *spinFlyout() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

class FQW_API CompactDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isError READ isError WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(bool symbolVisible READ isSymbolVisible WRITE setSymbolVisible)

public:
    explicit CompactDateTimeEdit(QWidget *parent = nullptr);
    ~CompactDateTimeEdit() override;

    bool isError() const;
    bool isSymbolVisible() const;
    QHBoxLayout *hBoxLayout() const;
    CompactSpinButton *compactSpinButton() const;
    SpinFlyoutView *spinFlyoutView() const;
    Flyout *spinFlyout() const;
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setSymbolVisible(bool visible);
    void setAccelerated(bool on);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

signals:
    void errorChanged(bool error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    SpinBoxHelper *m_helper = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::SpinIconType)
