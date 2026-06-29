#pragma once

#include <FluentQtWidgets/FluentQtWidgets.h>

class GalleryWindow : public FluentQt::FluentWindow
{
    Q_OBJECT

  public:
    explicit GalleryWindow(QWidget *parent = nullptr, bool deferPageLoad = false);
    ~GalleryWindow() override;

  protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

  private:
    bool addGalleryInterface(int index);
    void populateInterfaces();
    void addNextDeferredInterface();
    void scheduleSplashFinish();
    void reloadForLanguageChange();
    void switchToSample(const QString &routeKey, int index);

    QWidget *createBasicInputPage();
    QWidget *createNavigationPage();
    QWidget *createScrollPage();
    QWidget *createViewsPage();
    QWidget *createSettingsPage();
    QWidget *createDateTimePage();
    QWidget *createDialogPage();
    QWidget *createMenuPage();
    QWidget *createStatusInfoPage();
    QWidget *createTextPage();
    QWidget *createIconPage();
    QWidget *createLayoutPage();
    QWidget *createMaterialPage();

    bool m_reloadingLanguage = false;
    bool m_splashFinishScheduled = false;
    bool m_interfacesPopulated = false;
    int m_nextDeferredInterface = 0;
};
