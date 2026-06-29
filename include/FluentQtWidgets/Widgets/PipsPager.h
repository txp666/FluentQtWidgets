#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QList>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/Qt>
#include <QtCore/QtGlobal>
#include <QtWidgets/QWidget>

class QBoxLayout;
class QEvent;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif
class QPropertyAnimation;
class QToolButton;
class QWheelEvent;
class QWidget;

namespace FluentQt {

enum class PipsScrollButtonDisplayMode
{
    Always,
    OnHover,
    Never
};

class FQW_API PipsPager : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int pageNumber READ pageNumber WRITE setPageNumber NOTIFY pageNumberChanged)
    Q_PROPERTY(int visibleNumber READ visibleNumber WRITE setVisibleNumber NOTIFY visibleNumberChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(FluentQt::PipsScrollButtonDisplayMode previousButtonDisplayMode READ previousButtonDisplayMode WRITE
                   setPreviousButtonDisplayMode)
    Q_PROPERTY(FluentQt::PipsScrollButtonDisplayMode nextButtonDisplayMode READ nextButtonDisplayMode WRITE
                   setNextButtonDisplayMode)
    Q_PROPERTY(qreal pipScrollOffset READ pipScrollOffset WRITE setPipScrollOffset)
    Q_PROPERTY(bool isHover READ isHover)
    Q_PROPERTY(QWidget *pipContainer READ pipContainer)
    Q_PROPERTY(QPropertyAnimation *scrollAnimation READ scrollAnimation)

  public:
    explicit PipsPager(QWidget *parent = nullptr);
    explicit PipsPager(Qt::Orientation orientation, QWidget *parent = nullptr);

    int pageNumber() const;
    int getPageNumber() const;
    int visibleNumber() const;
    int getVisibleNumber() const;
    int currentIndex() const;
    Qt::Orientation orientation() const;
    bool isHorizontal() const;
    PipsScrollButtonDisplayMode previousButtonDisplayMode() const;
    PipsScrollButtonDisplayMode nextButtonDisplayMode() const;
    QToolButton *previousButton() const;
    QToolButton *preButton() const;
    QToolButton *nextButton() const;
    QWidget *pipContainer() const;
    QList<QToolButton *> pipButtons() const;
    QPropertyAnimation *scrollAnimation() const;
    qreal pipScrollOffset() const;
    bool isHover() const;
    bool isPreviousButtonVisible() const;
    bool isNextButtonVisible() const;

    QSize sizeHint() const override;

  public slots:
    void setPageNumber(int pageNumber);
    void setVisibleNumber(int visibleNumber);
    void setCurrentIndex(int index);
    void setPipScrollOffset(qreal offset);
    void setOrientation(Qt::Orientation orientation);
    void setPreviousButtonDisplayMode(FluentQt::PipsScrollButtonDisplayMode mode);
    void setNextButtonDisplayMode(FluentQt::PipsScrollButtonDisplayMode mode);
    void scrollNext();
    void scrollPrevious();

  signals:
    void pageNumberChanged(int pageNumber);
    void visibleNumberChanged(int visibleNumber);
    void currentIndexChanged(int index);
    void orientationChanged(Qt::Orientation orientation);

  protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    void init();
    void rebuildPips();
    void updateVisiblePips();
    void updateCurrentPip();
    void updateScrollButtonVisibility();
    void updateLayoutDirection();
    void updatePipContainerOffset();
    int visibleStartIndex() const;
    int visibleStartIndex(int index) const;
    QString displayModeName(PipsScrollButtonDisplayMode mode) const;

    Qt::Orientation m_orientation = Qt::Horizontal;
    int m_pageNumber = 0;
    int m_visibleNumber = 5;
    int m_currentIndex = -1;
    qreal m_pipScrollOffset = 0.0;
    bool m_hovered = false;
    PipsScrollButtonDisplayMode m_previousButtonDisplayMode = PipsScrollButtonDisplayMode::Never;
    PipsScrollButtonDisplayMode m_nextButtonDisplayMode = PipsScrollButtonDisplayMode::Never;
    QBoxLayout *m_layout = nullptr;
    QWidget *m_pipContainer = nullptr;
    QToolButton *m_previousButton = nullptr;
    QToolButton *m_nextButton = nullptr;
    QList<QToolButton *> m_pipButtons;
    QPropertyAnimation *m_scrollAnimation = nullptr;
};

class FQW_API HorizontalPipsPager : public PipsPager
{
    Q_OBJECT

  public:
    explicit HorizontalPipsPager(QWidget *parent = nullptr);
};

class FQW_API VerticalPipsPager : public PipsPager
{
    Q_OBJECT

  public:
    explicit VerticalPipsPager(QWidget *parent = nullptr);
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::PipsScrollButtonDisplayMode)
