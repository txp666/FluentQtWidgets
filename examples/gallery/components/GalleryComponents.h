#pragma once

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QUrl>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

class GalleryInterface : public FluentQt::ScrollArea
{
    Q_OBJECT

  public:
    explicit GalleryInterface(const QString &title, const QString &subtitle, QWidget *parent = nullptr);

    QWidget *contentWidget() const;
    QVBoxLayout *contentLayout() const;
    QWidget *addExampleCard(const QString &title, QWidget *widget, const QString &sourcePath, int stretch = 0);
    void scrollToCard(int index);

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    QWidget *m_toolBar = nullptr;
    QWidget *m_view = nullptr;
    QVBoxLayout *m_viewLayout = nullptr;
};

class HomeInterface : public FluentQt::ScrollArea
{
    Q_OBJECT

  public:
    explicit HomeInterface(QWidget *parent = nullptr);

  signals:
    void sampleCardClicked(const QString &routeKey, int index);

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    QWidget *m_banner = nullptr;
    QWidget *m_view = nullptr;
    QVBoxLayout *m_viewLayout = nullptr;
};
