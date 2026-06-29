#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QObject>
#include <QtWidgets/QAbstractScrollArea>

class QEvent;
class QWheelEvent;

namespace FluentQt {

enum class SmoothMode
{
    NoSmooth = 0,
    Constant,
    Linear,
    Quadrati,
    Cosine
};

class FQW_API SmoothScrollDelegate : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::SmoothScrollBar *vScrollBar READ vScrollBar)
    Q_PROPERTY(FluentQt::SmoothScrollBar *hScrollBar READ hScrollBar)
    Q_PROPERTY(FluentQt::SmoothMode smoothMode READ smoothMode WRITE setSmoothMode)

  public:
    explicit SmoothScrollDelegate(QAbstractScrollArea *parent, bool useAnimation = false);

    SmoothScrollBar *verticalScrollBar() const;
    SmoothScrollBar *horizontalScrollBar() const;
    SmoothScrollBar *vScrollBar() const;
    SmoothScrollBar *hScrollBar() const;
    bool usesAnimation() const;
    SmoothMode smoothMode() const;
    void setSmoothMode(SmoothMode mode);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    void setupScrollArea();
    void setupItemView();
    bool handleWheelEvent(QWheelEvent *event);

    QAbstractScrollArea *m_area = nullptr;
    bool m_useAnimation = false;
    SmoothScrollBar *m_verticalScrollBar = nullptr;
    SmoothScrollBar *m_horizontalScrollBar = nullptr;
    QObject *m_verticalSmoothScroll = nullptr;
    QObject *m_horizontalSmoothScroll = nullptr;
    SmoothMode m_smoothMode = SmoothMode::Linear;
};

} // namespace FluentQt
