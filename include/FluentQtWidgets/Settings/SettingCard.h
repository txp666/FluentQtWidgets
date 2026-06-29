#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/Dialogs/FolderListDialog.h>

#include <QtCore/QList>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

class QButtonGroup;
class QEnterEvent;
class QEvent;
class QHBoxLayout;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;
class QPushButton;
class QResizeEvent;
class QSpacerItem;
class QVBoxLayout;
class QWheelEvent;

namespace FluentQt {

class ClickableSlider;
class ComboBox;
class HyperlinkButton;
class IconWidget;
class RadioButton;
class ScrollArea;
class SwitchButton;

class FQW_API SettingCard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)

  public:
    explicit SettingCard(QWidget *parent = nullptr);
    explicit SettingCard(const QString &title, const QString &content = QString(), QWidget *parent = nullptr);
    SettingCard(const QIcon &icon, const QString &title, const QString &content = QString(), QWidget *parent = nullptr);
    SettingCard(FluentIcon icon, const QString &title, const QString &content = QString(), QWidget *parent = nullptr);

    QString title() const;
    QString content() const;
    QIcon icon() const;
    QSize iconSize() const;

    QLabel *iconLabel() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    QHBoxLayout *mainLayout() const;
    QVBoxLayout *textLayout() const;
    QHBoxLayout *actionLayout() const;

  public slots:
    void setTitle(const QString &title);
    void setContent(const QString &content);
    void setIcon(const QIcon &icon);
    void setIcon(FluentQt::FluentIcon icon);
    void setIconSize(const QSize &size);

  signals:
    void titleChanged(const QString &title);
    void contentChanged(const QString &content);

  protected:
    void changeEvent(QEvent *event) override;
    QVBoxLayout *outerLayout() const;

  private:
    void init();
    void updateContentVisibility();
    void refreshIcon();

    QIcon m_icon;
    QSize m_iconSize = QSize(16, 16);
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    QSpacerItem *m_iconSpacer = nullptr;
    QVBoxLayout *m_outerLayout = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_textLayout = nullptr;
    QHBoxLayout *m_actionLayout = nullptr;
};

class FQW_API SettingCardGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

  public:
    explicit SettingCardGroup(const QString &title, QWidget *parent = nullptr);

    QString title() const;
    QLabel *titleLabel() const;
    QVBoxLayout *cardLayout() const;
    QList<QWidget *> cards() const;

  public slots:
    void setTitle(const QString &title);
    void addSettingCard(QWidget *card);
    void addSettingCards(const QList<QWidget *> &cards);

  signals:
    void titleChanged(const QString &title);

  private:
    QLabel *m_titleLabel = nullptr;
    QVBoxLayout *m_cardLayout = nullptr;
};

class FQW_API PushSettingCard : public SettingCard
{
    Q_OBJECT

  public:
    PushSettingCard(const QString &buttonText, const QIcon &icon, const QString &title,
                    const QString &content = QString(), QWidget *parent = nullptr);
    PushSettingCard(const QString &buttonText, FluentIcon icon, const QString &title,
                    const QString &content = QString(), QWidget *parent = nullptr);

    QPushButton *button() const;

  signals:
    void clicked();

  protected:
    enum class ButtonStyle
    {
        Default,
        Primary
    };

    PushSettingCard(ButtonStyle style, const QString &buttonText, const QIcon &icon, const QString &title,
                    const QString &content, QWidget *parent);

  private:
    QPushButton *m_button = nullptr;
};

class FQW_API PrimaryPushSettingCard : public PushSettingCard
{
    Q_OBJECT

  public:
    PrimaryPushSettingCard(const QString &buttonText, const QIcon &icon, const QString &title,
                           const QString &content = QString(), QWidget *parent = nullptr);
    PrimaryPushSettingCard(const QString &buttonText, FluentIcon icon, const QString &title,
                           const QString &content = QString(), QWidget *parent = nullptr);
};

class FQW_API HyperlinkCard : public SettingCard
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

  public:
    HyperlinkCard(const QUrl &url, const QString &buttonText, const QIcon &icon, const QString &title,
                  const QString &content = QString(), QWidget *parent = nullptr);
    HyperlinkCard(const QString &url, const QString &buttonText, const QIcon &icon, const QString &title,
                  const QString &content = QString(), QWidget *parent = nullptr);
    HyperlinkCard(const QUrl &url, const QString &buttonText, FluentIcon icon, const QString &title,
                  const QString &content = QString(), QWidget *parent = nullptr);
    HyperlinkCard(const QString &url, const QString &buttonText, FluentIcon icon, const QString &title,
                  const QString &content = QString(), QWidget *parent = nullptr);

    HyperlinkButton *linkButton() const;
    QUrl url() const;

  public slots:
    void setUrl(const QUrl &url);
    void setUrl(const QString &url);

  private:
    HyperlinkButton *m_linkButton = nullptr;
};

class FQW_API SwitchSettingCard : public SettingCard
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)

  public:
    SwitchSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                      QWidget *parent = nullptr);
    SwitchSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                      QWidget *parent = nullptr);

    SwitchButton *switchButton() const;
    bool isChecked() const;

  public slots:
    void setChecked(bool checked);

  signals:
    void checkedChanged(bool checked);

  private:
    SwitchButton *m_switchButton = nullptr;
};

class FQW_API RangeSettingCard : public SettingCard
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

  public:
    RangeSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                     QWidget *parent = nullptr);
    RangeSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                     QWidget *parent = nullptr);
    RangeSettingCard(int minimum, int maximum, int value, const QIcon &icon, const QString &title,
                     const QString &content = QString(), QWidget *parent = nullptr);
    RangeSettingCard(int minimum, int maximum, int value, FluentIcon icon, const QString &title,
                     const QString &content = QString(), QWidget *parent = nullptr);

    ClickableSlider *slider() const;
    QLabel *valueLabel() const;
    int value() const;
    int minimum() const;
    int maximum() const;

  public slots:
    void setValue(int value);
    void setRange(int minimum, int maximum);

  signals:
    void valueChanged(int value);

  private:
    void updateValueLabel(int value);

    ClickableSlider *m_slider = nullptr;
    QLabel *m_valueLabel = nullptr;
};

class FQW_API ComboBoxSettingCard : public SettingCard
{
    Q_OBJECT

  public:
    ComboBoxSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                        QWidget *parent = nullptr);
    ComboBoxSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                        QWidget *parent = nullptr);
    ComboBoxSettingCard(const QStringList &items, const QIcon &icon, const QString &title,
                        const QString &content = QString(), QWidget *parent = nullptr);
    ComboBoxSettingCard(const QStringList &items, FluentIcon icon, const QString &title,
                        const QString &content = QString(), QWidget *parent = nullptr);

    ComboBox *comboBox() const;
    int currentIndex() const;
    QString currentText() const;

  public slots:
    void addItem(const QString &text, const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void setCurrentIndex(int index);

  signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString &text);

  private:
    ComboBox *m_comboBox = nullptr;
};

class FQW_API ColorPickerButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool alphaEnabled READ isAlphaEnabled WRITE setAlphaEnabled)

  public:
    explicit ColorPickerButton(const QColor &color = QColor(), QWidget *parent = nullptr);
    ColorPickerButton(const QColor &color, const QString &title, QWidget *parent = nullptr, bool enableAlpha = false);

    QColor color() const;
    QString title() const;
    bool isAlphaEnabled() const;

  public slots:
    void setColor(const QColor &color);
    void setTitle(const QString &title);
    void setAlphaEnabled(bool enabled);

  signals:
    void colorChanged(const QColor &color);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void showColorDialog();

    QColor m_color;
    QString m_title;
    bool m_alphaEnabled = false;
};

class FQW_API ColorSettingCard : public SettingCard
{
    Q_OBJECT

  public:
    ColorSettingCard(const QColor &color, const QIcon &icon, const QString &title, const QString &content = QString(),
                     QWidget *parent = nullptr);
    ColorSettingCard(const QColor &color, FluentIcon icon, const QString &title, const QString &content = QString(),
                     QWidget *parent = nullptr);

    ColorPickerButton *colorPicker() const;
    QColor color() const;

  public slots:
    void setColor(const QColor &color);

  signals:
    void colorChanged(const QColor &color);

  private:
    ColorPickerButton *m_colorPicker = nullptr;
};

class FQW_API ExpandButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)
    Q_PROPERTY(bool hover READ isHover WRITE setHover)
    Q_PROPERTY(bool pressed READ isPressed WRITE setPressed)

  public:
    explicit ExpandButton(QWidget *parent = nullptr);

    qreal angle() const;
    bool isHover() const;
    bool isPressed() const;

  public slots:
    void setAngle(qreal angle);
    void setHover(bool hover);
    void setPressed(bool pressed);
    void setExpand(bool expanded);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    QPropertyAnimation *m_rotateAnimation = nullptr;
    qreal m_angle = 0;
    bool m_hover = false;
    bool m_pressed = false;
};

class FQW_API HeaderSettingCard : public SettingCard
{
    Q_OBJECT

  public:
    HeaderSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                      QWidget *parent = nullptr);
    HeaderSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                      QWidget *parent = nullptr);

    ExpandButton *expandButton() const;

  public slots:
    void addWidget(QWidget *widget);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    ExpandButton *m_expandButton = nullptr;
};

class FQW_API ExpandSettingCard : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)

  public:
    ExpandSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                      QWidget *parent = nullptr);
    ExpandSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                      QWidget *parent = nullptr);

    HeaderSettingCard *card() const;
    QWidget *view() const;
    QVBoxLayout *viewLayout() const;
    ExpandButton *expandButton() const;
    bool isExpanded() const;
    QLabel *iconLabel() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    QHBoxLayout *mainLayout() const;
    QVBoxLayout *textLayout() const;
    QHBoxLayout *actionLayout() const;

  public slots:
    void addWidget(QWidget *widget);
    void setExpanded(bool expanded);
    void toggleExpanded();

  signals:
    void expandedChanged(bool expanded);

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void refreshExpandedGeometry();
    virtual int expandedContentHeight() const;

  private:
    void adjustViewSize();
    void onExpandValueChanged();

    QWidget *m_scrollWidget = nullptr;
    QWidget *m_view = nullptr;
    QWidget *m_spaceWidget = nullptr;
    QWidget *m_borderWidget = nullptr;
    HeaderSettingCard *m_card = nullptr;
    QVBoxLayout *m_scrollLayout = nullptr;
    QVBoxLayout *m_viewLayout = nullptr;
    ExpandButton *m_expandButton = nullptr;
    QPropertyAnimation *m_expandAnimation = nullptr;
    bool m_expanded = false;
};

class FQW_API GroupSeparator : public QWidget
{
    Q_OBJECT

  public:
    explicit GroupSeparator(QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API GroupWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)

  public:
    GroupWidget(const QIcon &icon, const QString &title, const QString &content, QWidget *widget,
                int stretch = 0, QWidget *parent = nullptr);
    GroupWidget(FluentIcon icon, const QString &title, const QString &content, QWidget *widget,
                int stretch = 0, QWidget *parent = nullptr);

    QString title() const;
    QString content() const;
    QIcon icon() const;
    IconWidget *iconWidget() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    QWidget *widget() const;
    QHBoxLayout *hBoxLayout() const;
    QVBoxLayout *textLayout() const;

  public slots:
    void setTitle(const QString &title);
    void setContent(const QString &content);
    void setIcon(const QIcon &icon);
    void setIcon(FluentIcon icon);
    void setIconSize(const QSize &size);

  private:
    IconWidget *m_iconWidget = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    QWidget *m_widget = nullptr;
    QHBoxLayout *m_hBoxLayout = nullptr;
    QVBoxLayout *m_textLayout = nullptr;
};

class FQW_API ExpandGroupSettingCard : public ExpandSettingCard
{
    Q_OBJECT

  public:
    ExpandGroupSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                           QWidget *parent = nullptr);
    ExpandGroupSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                           QWidget *parent = nullptr);

    QList<QWidget *> groupWidgets() const;
    int groupCount() const;
    GroupWidget *addGroup(const QIcon &icon, const QString &title, const QString &content, QWidget *widget,
                          int stretch = 0);
    GroupWidget *addGroup(FluentIcon icon, const QString &title, const QString &content, QWidget *widget,
                          int stretch = 0);

  public slots:
    void addGroupWidget(QWidget *widget);
    void removeGroupWidget(QWidget *widget);

  protected:
    virtual int expandedContentHeight() const;

  private:
    void rebuildGroupSeparators();
    QList<QWidget *> m_groupWidgets;
};

class FQW_API SimpleExpandGroupSettingCard : public ExpandGroupSettingCard
{
    Q_OBJECT

  public:
    SimpleExpandGroupSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                                 QWidget *parent = nullptr);
    SimpleExpandGroupSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                                 QWidget *parent = nullptr);

  protected:
    int expandedContentHeight() const override;
};

class FQW_API OptionsSettingCard : public ExpandSettingCard
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

  public:
    OptionsSettingCard(const QIcon &icon, const QString &title, const QString &content = QString(),
                       QWidget *parent = nullptr);
    OptionsSettingCard(FluentIcon icon, const QString &title, const QString &content = QString(),
                       QWidget *parent = nullptr);
    OptionsSettingCard(const QStringList &texts, const QVariantList &values, const QIcon &icon, const QString &title,
                       const QString &content = QString(), QWidget *parent = nullptr);
    OptionsSettingCard(const QStringList &texts, const QVariantList &values, FluentIcon icon, const QString &title,
                       const QString &content = QString(), QWidget *parent = nullptr);

    QVariant value() const;
    QLabel *choiceLabel() const;
    QButtonGroup *buttonGroup() const;

  public slots:
    void addOption(const QString &text, const QVariant &value = QVariant());
    void setValue(const QVariant &value);

  signals:
    void valueChanged(const QVariant &value);

  private:
    void updateChoiceLabel();

    QVariant m_value;
    QLabel *m_choiceLabel = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
};

class FQW_API FolderItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

  public:
    explicit FolderItem(const QString &path, QWidget *parent = nullptr);

    QString path() const;
    QLabel *pathLabel() const;
    QToolButton *removeButton() const;

  public slots:
    void setPath(const QString &path);

  signals:
    void pathChanged(const QString &path);
    void removeRequested(const QString &path);

  private:
    QString m_path;
    QLabel *m_pathLabel = nullptr;
    QToolButton *m_removeButton = nullptr;
};

class FQW_API FolderListSettingCard : public ExpandSettingCard
{
    Q_OBJECT
    Q_PROPERTY(QStringList folders READ folders WRITE setFolders NOTIFY foldersChanged)
    Q_PROPERTY(QString dialogDirectory READ dialogDirectory WRITE setDialogDirectory)

  public:
    FolderListSettingCard(const QStringList &folders, const QIcon &icon, const QString &title,
                          const QString &content = QString(), const QString &dialogDirectory = QString(),
                          QWidget *parent = nullptr);
    FolderListSettingCard(const QStringList &folders, FluentIcon icon, const QString &title,
                          const QString &content = QString(), const QString &dialogDirectory = QString(),
                          QWidget *parent = nullptr);

    QStringList folders() const;
    QString dialogDirectory() const;
    QPushButton *addFolderButton() const;
    QList<FolderItem *> folderItems() const;

  public slots:
    void setFolders(const QStringList &folders);
    void setDialogDirectory(const QString &directory);
    bool addFolder(const QString &folder);
    bool removeFolder(const QString &folder);
    void chooseFolder();

  signals:
    void foldersChanged(const QStringList &folders);
    void folderAdded(const QString &folder);
    void folderRemoved(const QString &folder);

  private:
    void rebuildFolderItems();
    void insertFolderItem(const QString &folder);

    QStringList m_folders;
    QString m_dialogDirectory;
    QPushButton *m_addFolderButton = nullptr;
    QList<FolderItem *> m_folderItems;
};

class FQW_API FolderListDialog; // forward declaration, defined in Dialogs/FolderListDialog.h

class FQW_API CustomColorSettingCard : public ExpandGroupSettingCard
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor defaultColor READ defaultColor WRITE setDefaultColor NOTIFY defaultColorChanged)
    Q_PROPERTY(QColor customColor READ customColor WRITE setCustomColor NOTIFY customColorChanged)
    Q_PROPERTY(bool customColorEnabled READ isCustomColorEnabled WRITE setCustomColorEnabled NOTIFY customColorEnabledChanged)
    Q_PROPERTY(bool alphaEnabled READ isAlphaEnabled WRITE setAlphaEnabled)

  public:
    CustomColorSettingCard(const QColor &defaultColor, const QColor &customColor, const QIcon &icon,
                           const QString &title, const QString &content = QString(), QWidget *parent = nullptr);
    CustomColorSettingCard(const QColor &defaultColor, const QColor &customColor, FluentIcon icon,
                           const QString &title, const QString &content = QString(), QWidget *parent = nullptr);

    QColor color() const;
    QColor defaultColor() const;
    QColor customColor() const;
    bool isCustomColorEnabled() const;
    bool isAlphaEnabled() const;
    QLabel *choiceLabel() const;
    RadioButton *defaultRadioButton() const;
    RadioButton *customRadioButton() const;
    ColorPickerButton *colorPicker() const;

  public slots:
    void setColor(const QColor &color);
    void setDefaultColor(const QColor &color);
    void setCustomColor(const QColor &color);
    void setCustomColorEnabled(bool enabled);
    void setAlphaEnabled(bool enabled);

  signals:
    void colorChanged(const QColor &color);
    void defaultColorChanged(const QColor &color);
    void customColorChanged(const QColor &color);
    void customColorEnabledChanged(bool enabled);

  private:
    void updateColorControls();

    QColor m_defaultColor;
    QColor m_customColor;
    bool m_customColorEnabled = false;
    QLabel *m_choiceLabel = nullptr;
    RadioButton *m_defaultRadioButton = nullptr;
    RadioButton *m_customRadioButton = nullptr;
    ColorPickerButton *m_colorPicker = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
};

} // namespace FluentQt
