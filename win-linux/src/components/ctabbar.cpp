/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "components/ctabbar.h"
#include "components/canimatedicon.h"
#include "components/cmenu.h"
#include "cascapplicationmanagerwrapper.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QResizeEvent>
#include <QStylePainter>
#include <QStyle>
#include <QTimer>


#define ANIMATION_DEFAULT_MS  0
#define ANIMATION_SCROLL_MS   60
#define ANIMATION_MOVE_TAB_MS 350
#define TABSPACING 0
#define DEFAULT_ICON_SIZE QSize(16,16)
#define _tabRect(i) tabList[i]->geometry()
#define tabIndex(i) tabList[i]->index
#define signum(a) (a ? 1 : -1);
#define PROCESSEVENTS() AscAppManager::getInstance().processEvents()
#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, this, callback)


class Tab : public QFrame
{
    Q_OBJECT
public:
    Tab(QWidget *parent = nullptr);
    ~Tab();

    void setText(const QString &text, Qt::TextElideMode mode = Qt::ElideRight);
    void elideText(Qt::TextElideMode mode = Qt::ElideRight);
    void setIcon(const QIcon &icon);
    void setIcon(const QString &path);
    void setThemeIcons(const std::pair<QString, QString> &);
    void setColorThemeType(const QString&);
    void setActive(bool state);
    void polish();
    void refreshIcon(const QString& themetype);
    void refreshTextColor();
    void refreshCloseButton();

    QIcon  *tab_icon = nullptr;
    CAnimatedIcon *icon_label = nullptr;
    QLabel *text_label = nullptr;
    QToolButton *close_btn = nullptr;
    CMenu *menu = nullptr;
    QString text;
    QString tabcolor;
    int tab_width = -1;
    int index = -1;

    std::pair<QString, QString> theme_icons;
    QString tab_theme_type;
    bool is_active = false;
signals:
    void onTabWidthChanged(int width);

protected:
    virtual void resizeEvent(QResizeEvent*) final;
    virtual void paintEvent(QPaintEvent*) override;
    virtual bool eventFilter(QObject*, QEvent*) override;

private:
    CTabBar *tabBar = nullptr;
};

Tab::Tab(QWidget *parent) :
    QFrame(parent),
    tabcolor("none")
{
    setAttribute(Qt::WA_Hover);
    installEventFilter(this);
    setMinimumWidth(20);
    tabBar = dynamic_cast<CTabBar*>(parent->parent());

    QHBoxLayout *lut = new QHBoxLayout(this);
    lut->setContentsMargins(6, 6, 6, 6);
    lut->setSpacing(6);
    setLayout(lut);

    icon_label = new CAnimatedIcon(this);
    icon_label->setObjectName("tabIcon");
    icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    icon_label->setMaximumSize(DEFAULT_ICON_SIZE);
    lut->addWidget(icon_label);

    text_label = new QLabel(this);
    text_label->setObjectName("tabText");
    text_label->setTextFormat(Qt::PlainText);
    text_label->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter | Qt::AlignAbsolute);
    text_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lut->addWidget(text_label);

    close_btn = new QToolButton(this);
    close_btn->setObjectName("tabButton");
    close_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    close_btn->setMaximumSize(DEFAULT_ICON_SIZE);
    lut->addWidget(close_btn);
    show();
}

Tab::~Tab()
{
    if (tab_icon)
        delete tab_icon, tab_icon = nullptr;
    if (menu)
        delete menu, menu = nullptr;
}

void Tab::setText(const QString &text, Qt::TextElideMode mode)
{
    this->text = text;
    elideText(mode);
}

void Tab::elideText(Qt::TextElideMode mode)
{
    const QMargins mrg = text_label->contentsMargins();
    const int padding = mrg.left() + mrg.right();
    const int width = text_label->maximumWidth() != QWIDGETSIZE_MAX ? text_label->maximumWidth() : text_label->width();
    QFontMetrics mtr(text_label->font());
    text_label->setText(mtr.elidedText(text, mode, width - padding - 1));
}

void Tab::setIcon(const QIcon &icon)
{
    if (tab_icon)
        delete tab_icon, tab_icon = nullptr;
    tab_icon = new QIcon(icon);
    icon_label->setPixmap(tab_icon->pixmap(icon_label->size()));
}

void Tab::setIcon(const QString &path)
{
    if ( !path.isEmpty() ) {
        if ( tab_icon )
            delete tab_icon, tab_icon = nullptr;

        tab_icon = new QIcon(path);
        icon_label->setPixmap(tab_icon->pixmap(icon_label->size()));
    }
}

void Tab::setThemeIcons(const std::pair<QString, QString> & icons)
{
    theme_icons = icons;

    if ( is_active )
        setIcon(tab_theme_type == "dark" ? theme_icons.second : theme_icons.first);
}

void Tab::setColorThemeType(const QString& type)
{
    if ( tab_theme_type != type ) {
        tab_theme_type = type;
        setProperty("uithemetype", tab_theme_type);

        if ( is_active ) {
            refreshIcon(tab_theme_type);
            refreshTextColor();
        }

        refreshCloseButton();
    }
}

void Tab::setActive(bool state)
{
    if ( state != is_active ) {
        is_active = state;

        if ( is_active ) {
            refreshIcon(tab_theme_type);
        } else {
            refreshIcon(AscAppManager::themes().current().isDark() ? "dark" : "light");
        }

        refreshTextColor();
    }
}

void Tab::refreshIcon(const QString& themetype)
{
    setIcon(themetype == "dark" ? theme_icons.second : theme_icons.first);
    icon_label->setSvgElement(themetype == "dark" ? "light" : "dark");
}

void Tab::refreshTextColor()
{
    const CTheme & _app_theme = AscAppManager::themes().current();
    std::wstring text_color;
    if (tab_theme_type == "dark") {
        text_color = _app_theme.isDark() ? _app_theme.value(CTheme::ColorRole::ecrTabSimpleActiveText) :
                         _app_theme.value(CTheme::ColorRole::ecrTextInverse);
    } else {
        text_color = _app_theme.isDark() ? _app_theme.value(CTheme::ColorRole::ecrTextInverse) :
                         _app_theme.value(CTheme::ColorRole::ecrTabSimpleActiveText);
    }

    QString _styles = "#tabText{color:" + QString::fromStdWString(_app_theme.value(CTheme::ColorRole::ecrTabSimpleActiveText)) + ";}"
                      "[selected=true] #tabText{color:" + QString::fromStdWString(text_color) + ";}";
    text_label->setStyleSheet(_styles);
}

void Tab::refreshCloseButton()
{
    bool _is_app_theme_dark = AscAppManager::themes().current().isDark(),
         _is_tab_theme_dark = tab_theme_type == "dark";

    QString _image_normal = _is_app_theme_dark ? ":/tabbar/icons/close_active_normal.svg" : ":/tabbar/icons/close_normal.svg";
    QString _image_hover = _is_app_theme_dark ? ":/tabbar/icons/close_active_hover.svg" : ":/tabbar/icons/close_hover.svg";
    QString _image_pressed = _is_app_theme_dark ? ":/tabbar/icons/close_active_pressed.svg" : ":/tabbar/icons/close_pressed.svg";
    QString _image_active_normal = _is_tab_theme_dark ? ":/tabbar/icons/close_active_normal.svg" : ":/tabbar/icons/close_normal.svg";
    QString _image_active_hover = _is_tab_theme_dark ? ":/tabbar/icons/close_active_hover.svg" : ":/tabbar/icons/close_hover.svg";
    QString _image_active_pressed = _is_tab_theme_dark ? ":/tabbar/icons/close_active_pressed.svg" : ":/tabbar/icons/close_pressed.svg";

    QString _styles = "[hovered=true] #tabButton{image:url(" + _image_normal + ");}"
                        "[hovered=true] #tabButton:hover{image:url(" + _image_hover + ");}"
                        "[hovered=true] #tabButton:pressed{image:url(" + _image_pressed + ");}"
                        "[selected=true] #tabButton{image:url(" + _image_active_normal + ");}"
                        "[selected=true] #tabButton:hover{image:url(" + _image_active_hover + ");}"
                        "[selected=true] #tabButton:pressed{image:url(" + _image_active_pressed + ");}";
//                        "CTabBar[active=false] Tab[selected=true] #tabButton{image:url(" + _image_normal + ");}"
//                        "CTabBar[active=false] Tab[selected=true] #tabButton:hover{image:url(" + _image_hover + ");}"
//                        "CTabBar[active=false] Tab[selected=true] #tabButton:pressed{image:url(" + _image_pressed + ");}"
        ;
    close_btn->setStyleSheet(_styles);
}

void Tab::polish()
{
    style()->polish(this);
    icon_label->style()->polish(icon_label);
    text_label->style()->polish(text_label);
    close_btn->style()->polish(close_btn);
}

void Tab::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    int new_width = event->size().width();
    if (tab_width < 0) {
        tab_width = new_width;
    } else {
        if (tab_width != new_width) {
            tab_width = new_width;
            SKIP_EVENTS_QUEUE([=]() {
                emit onTabWidthChanged(tab_width);
            });
        }
    }
}

void Tab::paintEvent(QPaintEvent *ev)
{
    if (!tabcolor.isEmpty() && tabcolor != "none" && property("selected").toBool()) {
//        if (tabBar && tabBar->property("active").toBool())
        {
            QStylePainter p(this);
            int left = AscAppManager::isRtlEnabled() ? frameWidth() : 0;
            p.fillRect(rect().adjusted(left, 0, left ? 0 : -frameWidth(), 0), QBrush(QColor(tabcolor)));
        }
    } else {
        QFrame::paintEvent(ev);
    }
}

bool Tab::eventFilter(QObject *obj, QEvent *ev)
{
    if (!isEnabled())
        return true;

    switch (ev->type()) {
    case QEvent::HoverEnter:
        setProperty("hovered", true);
        polish();
        break;
    case QEvent::HoverLeave:
        setProperty("hovered", false);
        polish();
        break;
    default:
        break;
    }
    return QFrame::eventFilter(obj, ev);
}


class CTabBar::CTabBarPrivate
{
public:
    CTabBarPrivate(CTabBar* owner);
    ~CTabBarPrivate();

    enum Direction {
        Left, Right
    };

    Tab* createTab(int posX, const QString &text);
    int getIntersectedOffset(int index);
    int getIntersectedIndex(int direction, int &offsetX);
    int getLayoutsIntersectedIndex(Tab *tab, int &offsetX);
    int cellWidth();
    int nextTabPosByPrev(int index);
//    int prevTabPosByNext(int index);
    void slide(int from, int to, int offset, int animation_ms);
    void scrollToDirection(int direction);
    void scrollTo(int index);
    void onCurrentChanged(int index);
    void onTabWidthChanged(int width);
    void changeScrollerState();
    void reorderIndexes();
    void recalcWidth();
    void setActive(int index);
    bool indexIsValid(int index);

    CTabBar*     owner = nullptr;
    QFrame*      tabArea = nullptr;
    QFrame*      scrollFrame = nullptr;
    QToolButton* leftButton = nullptr;
    QToolButton* rightButton = nullptr;
    QVector<Tab*>  tabList;
    QVector<QRect> tabLayouts;
    Qt::TextElideMode elideMode;
    Tab* movedTab = nullptr;
    bool lock = false;
    bool isActive = false;
    int animationInProgress = 0;
    int movedTabPosX = 0;
    int movedTabPressPosX = 0;
    int movedTabIndex = -1;
    int tab_width = -1;
    int currentIndex = -1;
    QSize iconSize;
};

CTabBar::CTabBarPrivate::CTabBarPrivate(CTabBar* owner) :
    owner(owner),
    elideMode(Qt::ElideRight),
    iconSize(DEFAULT_ICON_SIZE)
{

}

CTabBar::CTabBarPrivate::~CTabBarPrivate()
{}

Tab* CTabBar::CTabBarPrivate::createTab(int posX, const QString &text)
{
    Tab *tab = new Tab(tabArea);
    tab->move(posX, 0);
    tab->setFixedHeight(tabArea->height());
    tab->setText(text, elideMode);
    if (tab->icon_label->minimumSize().isNull()) {
        tab->icon_label->setBaseSize(iconSize);
        tab->icon_label->setFixedSize(iconSize);
    }
    connect(tab->close_btn, &QToolButton::clicked, tab, [=]() {
        emit owner->tabCloseRequested(tab->index);
    });
    connect(tab, &Tab::onTabWidthChanged, tab, [=](int width) {
        if (tab_width != width) {
            tab_width = width;
            onTabWidthChanged(width);
        }
    });
    return tab;
}

int CTabBar::CTabBarPrivate::getIntersectedOffset(int index)
{
    if (indexIsValid(index)) {
        QRect tabRect = _tabRect(index);
        if (!tabArea->rect().contains(tabRect) && tabArea->rect().intersects(tabRect)) {
            QRect interRect = tabArea->rect().intersected(tabRect);
            return (cellWidth() - interRect.width()) * signum(interRect.x() != 0);
        }
    }
    return 0;
}

int CTabBar::CTabBarPrivate::getIntersectedIndex(int direction, int &offsetX)
{
    offsetX = 0;
    for (int i = 0; i < tabList.size(); i++) {
        QRect tabRect = _tabRect(i);
        if (!tabArea->rect().contains(tabRect) && tabArea->rect().intersects(tabRect)) {
            QRect interRect = tabArea->rect().intersected(tabRect);
            if ((direction == Direction::Left && interRect.x() != 0)
                    || (direction == Direction::Right && interRect.x() == 0)) {
                offsetX = cellWidth() - interRect.width();
                return i;
            }
        }
    }
    return -1;
}

int CTabBar::CTabBarPrivate::getLayoutsIntersectedIndex(Tab *tab, int &offsetX)
{
    offsetX = 0;
    QRect tabRect = tab->geometry();
    for (int i = 0; i < tabLayouts.size(); i++) {
        if (i != tab->index && tabRect.intersects(tabLayouts[i])) {
            QRect interRect = tabRect.intersected(tabLayouts[i]);
            offsetX = interRect.width() * signum(tabRect.x() >= tabLayouts[i].x());
            return i;
        }
    }
    return -1;
}

int CTabBar::CTabBarPrivate::cellWidth()
{
    Q_ASSERT(!tabList.isEmpty());
    return tabList[0]->width() + TABSPACING;
}

//int CTabBar::CTabBarPrivate::prevTabPosByNext(int index)
//{
//    Q_ASSERT(index > -1 && index < tabList.size());
//    return tabRect(index).left() - cellWidth() - 1;
//}

int CTabBar::CTabBarPrivate::nextTabPosByPrev(int index)
{
    Q_ASSERT(index > -1 && index < tabList.size());
    return _tabRect(index).right() + TABSPACING + 1;
}

void CTabBar::CTabBarPrivate::slide(int from, int to, int offset, int animation_ms)
{
    animationInProgress++;
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    for (int i = from; i <= to; i++) {
        QPropertyAnimation* animation = new QPropertyAnimation(tabList[i], "pos");
        animation->setDuration(animation_ms);
        animation->setStartValue(tabList[i]->pos());
        animation->setEndValue(tabList[i]->pos() + QPoint(offset, 0));
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        group->addAnimation(animation);
    }
    QObject::connect(group, &QParallelAnimationGroup::finished, qApp, [=]() {
        changeScrollerState();
    });
    QObject::connect(group, &QParallelAnimationGroup::stateChanged, qApp,
                     [=](QAbstractAnimation::State newState, QAbstractAnimation::State) {
        if (newState == QAbstractAnimation::Stopped)
            animationInProgress--;
    });
    group->start(QParallelAnimationGroup::DeleteWhenStopped);
}

void CTabBar::CTabBarPrivate::scrollToDirection(int direction)
{
    if (AscAppManager::isRtlEnabled())
        direction = (direction == Direction::Left) ? Direction::Right : Direction::Left;
    while (animationInProgress)
        PROCESSEVENTS();

    if (tabList.isEmpty())
        return;

    const int supIndex = (direction == Direction::Left) ? tabList.size() - 1 : 0;
    if (!tabArea->rect().contains(_tabRect(supIndex))) {
        int offsetX;
        int ind = getIntersectedIndex(direction, offsetX);
        int offset = (ind != -1) ? offsetX : cellWidth();
        if (direction == Direction::Left)
            offset *= -1;
        slide(0, tabList.size() - 1, offset, ANIMATION_SCROLL_MS);
    }
}

void CTabBar::CTabBarPrivate::scrollTo(int index)
{
    while (animationInProgress)
        PROCESSEVENTS();

    if (!indexIsValid(index))
        return;

    if (!tabArea->rect().contains(_tabRect(index))) {
        int x = _tabRect(index).x();
        int offsetX = (x > 0) ? x - tabArea->width() + cellWidth() : x;
        slide(0, tabList.size() - 1, -1 * offsetX, ANIMATION_DEFAULT_MS);
    } else {
        changeScrollerState();
    }
}

void CTabBar::CTabBarPrivate::onCurrentChanged(int index)
{
    while (animationInProgress)
        PROCESSEVENTS();

    recalcWidth();
    scrollTo(index);

    currentIndex = index;

    setActive(index);

    emit owner->currentChanged(index);
}

void CTabBar::CTabBarPrivate::onTabWidthChanged(int width)
{
    Q_UNUSED(width)
    while (animationInProgress)
        PROCESSEVENTS();

    if (!tabList.isEmpty())
        iconSize = tabList[0]->icon_label->size();

    for (int i = 0; i < tabList.size(); i++) {
        int posX = i * cellWidth();
        tabList[i]->move(posX, 0);

        if (QIcon *icon = tabList[i]->tab_icon)
            tabList[i]->icon_label->setPixmap(icon->pixmap(iconSize));

        tabList[i]->elideText(elideMode);
    }
    recalcWidth();
    scrollTo(currentIndex);
}

void CTabBar::CTabBarPrivate::changeScrollerState()
{
    bool allowScroll = false;
    for (int i = 0; i < tabList.size(); i++) {
        if (_tabRect(i).x() + 1 < 0) {
            allowScroll = true;
            break;
        }
    }
    if (AscAppManager::isRtlEnabled())
        rightButton->setEnabled(allowScroll);
    else
        leftButton->setEnabled(allowScroll);

    allowScroll = false;
    for (int i = 0; i < tabList.size(); i++) {
        if (_tabRect(i).right() + TABSPACING > tabArea->rect().right()) {
            allowScroll = true;
            break;
        }
    }
    if (AscAppManager::isRtlEnabled())
        leftButton->setEnabled(allowScroll);
    else
        rightButton->setEnabled(allowScroll);
}

void CTabBar::CTabBarPrivate::reorderIndexes()
{
    const int size = tabList.size();
    QVector<Tab*> dupTabList(size, nullptr);
    for (int i = 0; i < size; i++) {
        Q_ASSERT(tabIndex(i) > -1 && tabIndex(i) < size);
        dupTabList[tabIndex(i)] = tabList[i];
    }
    tabList = std::move(dupTabList);
}

void CTabBar::CTabBarPrivate::recalcWidth()
{
    int minWidth = (tabList.isEmpty()) ? 0 : cellWidth();
    tabArea->setMinimumWidth(minWidth);
    tabArea->setMaximumWidth(minWidth * tabList.size());
    owner->setMaximumWidth(tabArea->maximumWidth() + scrollFrame->maximumWidth());
    PROCESSEVENTS();
}

void CTabBar::CTabBarPrivate::setActive(int index)
{
    for (int i = 0; i < tabList.size(); i++) {
        bool state = (i == index);
        if (tabList[i]->property("selected").toBool() != (state && isActive)) {
            tabList[i]->setProperty("selected", state && isActive);
            tabList[i]->polish();
        }
        tabList[i]->setActive(state);
    }
}

bool CTabBar::CTabBarPrivate::indexIsValid(int index)
{
    return index > -1 && index < tabList.size();
}


CTabBar::CTabBar(QWidget *parent) :
    QFrame(parent),
    d(new CTabBarPrivate(this))
{
    installEventFilter(this);
    QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->setAlignment(Qt::AlignLeft);
    main_layout->setContentsMargins(0,0,0,0);
    main_layout->setSpacing(0);
    d->tabArea = new QFrame(this);
    d->tabArea->setObjectName("tabArea");
    d->tabArea->installEventFilter(this);
    d->tabArea->setMaximumWidth(0);
    d->tabArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    main_layout->addWidget(d->tabArea);
    setMinimumWidth(0);
    setMaximumWidth(0);

    d->scrollFrame = new QFrame(this);
    d->scrollFrame->setObjectName("tabScroll");
    QHBoxLayout *scrollLayout = new QHBoxLayout(d->scrollFrame);
    scrollLayout->setSpacing(0);
    scrollLayout->setContentsMargins(0,0,0,0);
    d->scrollFrame->setLayout(scrollLayout);

    d->leftButton = new QToolButton(d->scrollFrame);
    d->rightButton = new QToolButton(d->scrollFrame);
    d->leftButton->setObjectName(AscAppManager::isRtlEnabled() ? "rightButton" : "leftButton");
    d->rightButton->setObjectName(AscAppManager::isRtlEnabled() ? "leftButton" : "rightButton");

    scrollLayout->addWidget(d->leftButton);
    scrollLayout->addWidget(d->rightButton);
    d->leftButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->rightButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    main_layout->addWidget(d->scrollFrame);
    connect(d->leftButton, &QToolButton::clicked, this, [=]() {
        d->scrollToDirection(d->Direction::Right);
    });
    connect(d->rightButton, &QToolButton::clicked, this, [=]() {
        d->scrollToDirection(d->Direction::Left);
    });

    show();
    d->leftButton->hide();
    d->rightButton->hide();
    d->leftButton->setEnabled(false);
    d->rightButton->setEnabled(false);
}

CTabBar::~CTabBar()
{
    delete d, d = nullptr;
}

int CTabBar::addTab(const QString &text)
{
    while (d->animationInProgress)
        PROCESSEVENTS();

    d->movedTab = nullptr;
    d->movedTabIndex = -1;
    const int lastIndex = d->tabList.size() - 1;
    const int posX = (lastIndex == -1) ? 0 : d->nextTabPosByPrev(lastIndex);
    Tab *tab = d->createTab(posX, text);
    tab->index = lastIndex + 1;
    d->tabList.append(tab);
    tabInserted(lastIndex + 1);
    d->onCurrentChanged(lastIndex + 1);
    return d->currentIndex;
}

int CTabBar::addTab(const QIcon &icon, const QString &text)
{
    const int index = addTab(text);
    setTabIcon(index, icon);
    return index;
}

int CTabBar::count() const
{
    return d->tabList.size();
}

int CTabBar::currentIndex() const
{
    return d->currentIndex;
}

Qt::TextElideMode CTabBar::elideMode() const
{
    return d->elideMode;
}

QSize CTabBar::iconSize() const
{
    return d->iconSize;
}

int CTabBar::insertTab(int index, const QString &text)
{
    while (d->animationInProgress)
        PROCESSEVENTS();

    if (!d->indexIsValid(index))
        return addTab(text);

    d->movedTab = nullptr;
    d->movedTabIndex = -1;
    int posX = d->_tabRect(index).left();
    d->slide(index, d->tabList.size() - 1, d->cellWidth(), ANIMATION_DEFAULT_MS);
    while (d->animationInProgress)
        PROCESSEVENTS();

    Tab *tab = d->createTab(posX, text);
    d->tabList.insert(index, tab);
    for (int i = index; i < d->tabList.size(); i++)
        d->tabIndex(i) = i;
    tabInserted(index);
    d->onCurrentChanged(index);
    return d->currentIndex;
}

int CTabBar::insertTab(int index, const QIcon &icon, const QString &text)
{
    const int actual_index = insertTab(index, text);
    setTabIcon(actual_index, icon);
    return actual_index;
}

void CTabBar::swapTabs(int from, int to)
{
    while (d->animationInProgress)
        PROCESSEVENTS();
    if (from == to || !d->indexIsValid(from) || !d->indexIsValid(to))
        return;
    d->movedTab = nullptr;
    d->movedTabIndex = -1;
    int posX = d->_tabRect(from).x();
    d->tabList[from]->move(d->_tabRect(to).x(), 0);
    d->tabList[to]->move(posX, 0);
    int from_index = d->tabIndex(from);
    d->tabIndex(from) = d->tabIndex(to);
    d->tabIndex(to) = from_index;
    std::swap(d->tabList[from], d->tabList[to]);
    emit tabsSwapped(from, to);
    if (from == d->currentIndex)
        d->onCurrentChanged(to);
    else
    if (to == d->currentIndex)
        d->onCurrentChanged(from);
}

void CTabBar::moveTab(int from, int to)
{
    while (d->animationInProgress)
        PROCESSEVENTS();
    if (from == to || !d->indexIsValid(from) || !d->indexIsValid(to))
        return;
    d->movedTab = nullptr;
    d->movedTabIndex = -1;
    d->tabList[from]->move(d->_tabRect(to).x(), 0);
    d->tabIndex(from) = d->tabIndex(to);
    int start = (from < to) ? from + 1 : to;
    int end = (from < to) ? to : from - 1;
    int sign = signum(from < to);
    for (int i = start; i <= end; i++) {
        d->tabList[i]->move(d->tabList[i]->x() - sign * d->cellWidth(), 0);
        d->tabIndex(i) = i - sign;
    }
    d->reorderIndexes();
    emit tabMoved(from, to);
    if (from < d->currentIndex) {
        if (to >= d->currentIndex)
            d->onCurrentChanged(d->currentIndex - 1);
    } else
    if (from == d->currentIndex) {
        d->onCurrentChanged(to);
    } else {
        if (to <= d->currentIndex)
            d->onCurrentChanged(d->currentIndex + 1);
    }
}

void CTabBar::removeTab(int index)
{
    while (d->animationInProgress)
        PROCESSEVENTS();

    if (!d->indexIsValid(index))
        return;

    d->movedTab = nullptr;
    d->movedTabIndex = -1;
    d->tabList[index]->hide();
    if (d->_tabRect(0).x() <= d->tabArea->x() - d->cellWidth()) {
        const int prevIndex = index - 1;
        if (prevIndex > -1) {
            d->slide(0, prevIndex, d->cellWidth(), ANIMATION_DEFAULT_MS);
            while (d->animationInProgress)
                PROCESSEVENTS();
        }
    } else {
        const int nextIndex = index + 1;
        if (nextIndex < d->tabList.size()) {
            d->slide(nextIndex, d->tabList.size() - 1, -1 * d->cellWidth(), ANIMATION_DEFAULT_MS);
            while (d->animationInProgress)
                PROCESSEVENTS();
        }
    }

    delete d->tabList[index];
    d->tabList.remove(index);
    for (int i = index; i < d->tabList.size(); i++)
        d->tabIndex(i) = i;

    if (d->currentIndex > 0) {
        if (index > 0) {
            if (d->currentIndex > index) {
                d->onCurrentChanged(d->currentIndex - 1);
            } else
            if (d->currentIndex < index) {
                d->recalcWidth();
            } else {
                const int initialMaxIndex = d->tabList.size(); // max index before deletion
                d->onCurrentChanged(index < initialMaxIndex ? index : initialMaxIndex - 1);
            }
        } else {
            d->onCurrentChanged(d->currentIndex - 1);
        }
    } else {
        if (index > 0) {
            d->recalcWidth();
        } else {
            const int initialMaxIndex = d->tabList.size(); // max index before deletion
            d->onCurrentChanged(index < initialMaxIndex ? index : -1);
        }
    }
}

void CTabBar::setElideMode(Qt::TextElideMode mode)
{
    d->elideMode = mode;
}

void CTabBar::setIconSize(const QSize &size)
{
    d->iconSize = size;
    for (int i = 0; i < d->tabList.size(); i++) {
        if (d->tabList[i]->icon_label->baseSize().isNull())
            break;
        d->tabList[i]->icon_label->setBaseSize(size);
        d->tabList[i]->icon_label->setFixedSize(size);
        if (QIcon *icon = d->tabList[i]->tab_icon)
            d->tabList[i]->icon_label->setPixmap(icon->pixmap(d->iconSize));
    }
}

void CTabBar::setTabIconLabel(int index, QWidget *widget)
{
    if (!d->indexIsValid(index))
        return;
    if (CAnimatedIcon *icon_label = dynamic_cast<CAnimatedIcon*>(widget)) {
        Tab *tab = d->tabList[index];
        if (tab->icon_label)
            delete tab->icon_label;
        tab->icon_label = icon_label;
        icon_label->setParent(tab);
        icon_label->setObjectName("tabIcon");
        icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        icon_label->setMaximumSize(DEFAULT_ICON_SIZE);
        if (QHBoxLayout *lut = dynamic_cast<QHBoxLayout*>(tab->layout()))
            lut->insertWidget(0, icon_label);
        icon_label->style()->polish(icon_label);
    }
}

void CTabBar::setTabButton(int index, QWidget *widget)
{
    if (!d->indexIsValid(index))
        return;
    if (QToolButton *close_btn = dynamic_cast<QToolButton*>(widget)) {
        Tab *tab = d->tabList[index];
        if (tab->close_btn)
            delete tab->close_btn;
        tab->close_btn = close_btn;
        close_btn->setParent(tab);
        close_btn->setObjectName("tabButton");
        close_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        close_btn->setMaximumSize(DEFAULT_ICON_SIZE);
        connect(close_btn, &QToolButton::clicked, this, [=]() {
            emit tabCloseRequested(tab->index);
        });
        if (QHBoxLayout *lut = dynamic_cast<QHBoxLayout*>(tab->layout()))
            lut->insertWidget(2, close_btn);
        close_btn->style()->polish(close_btn);
    }
}

void CTabBar::setTabMenu(int index, CMenu *menu)
{
    if (!d->indexIsValid(index))
        return;
    Tab *tab = d->tabList[index];
    if (tab->menu)
        delete tab->menu;
    tab->menu = menu;
    if (menu)
        menu->setObjectName("tabMenu");
}

//void CTabBar::setTabData(int index, const QVariant &data)
//{
//    if (d->indexIsValid(index))
//        d->tabList[index]->setProperty("TabData", data);
//}

void CTabBar::setTabIcon(int index, const QIcon &icon)
{
    if (d->indexIsValid(index))
        d->tabList[index]->setIcon(icon);
}

void CTabBar::setTabText(int index, const QString &text)
{
    if (d->indexIsValid(index))
        d->tabList[index]->setText(text, d->elideMode);
}

void CTabBar::setTabToolTip(int index, const QString &text)
{
    if (d->indexIsValid(index)) {
        d->tabList[index]->setProperty("ToolTip", text);
        d->tabList[index]->icon_label->setProperty("ToolTip", text);
        d->tabList[index]->text_label->setProperty("ToolTip", text);
        d->tabList[index]->close_btn->setProperty("ToolTip", text);
    }
}

void CTabBar::setCurrentIndex(int index)
{
    while (d->animationInProgress)
        PROCESSEVENTS();

    if (!d->indexIsValid(index) || index == d->currentIndex)
        return;

    d->onCurrentChanged(index);
}

void CTabBar::setActiveTabColor(int index, const QString& color)
{
    if (!d->indexIsValid(index) || color == "none")
        return;
    if (d->tabList[index]->tabcolor != color) {
        d->tabList[index]->tabcolor = color;
        d->tabList[index]->polish();
    }
}

void CTabBar::setUseTabCustomPalette(int index, bool use)
{
    if (d->indexIsValid(index)) {
        if (d->tabList[index]->property("custom").toBool() != use) {
            d->tabList[index]->setProperty("custom", use);
            polish();
        }
    }
}

void CTabBar::setTabLoading(int index, bool start, const QString& theme)
{
    if (CAnimatedIcon * icon = (CAnimatedIcon*)tabIconLabel(index)) {
        if (start) {
            if (!icon->isStarted() )
                icon->startSvg(":/tabbar/icons/loader.svg", theme);
        } else
            icon->stop();
    }
}

void CTabBar::setTabThemeType(int index, TabTheme theme)
{
    if ( d->indexIsValid(index) ) {
        d->tabList[index]->setColorThemeType(TabTheme::LightTab == theme ? "light" : "dark");
    }
}

void CTabBar::setTabThemeIcons(int index, const std::pair<QString, QString> & icons)
{
    if ( d->indexIsValid(index) ) {
        d->tabList[index]->setThemeIcons(icons);
    }
}

void CTabBar::polish()
{
    for (int i = 0; i < d->tabList.size(); i++)
        d->tabList[i]->polish();
    d->tabArea->style()->polish(d->tabArea);
    d->scrollFrame->style()->polish(d->scrollFrame);
    d->leftButton->style()->polish(d->leftButton);
    d->rightButton->style()->polish(d->rightButton);
}

void CTabBar::activate(bool isActive)
{
    if (d->isActive != isActive) {
        d->isActive = isActive;
        d->setActive(isActive ? d->currentIndex : -1);
    }
}

void CTabBar::refreshTheme()
{
    for (int i = 0; i < d->tabList.size(); i++) {
        d->tabList[i]->refreshCloseButton();
        d->tabList[i]->refreshTextColor();

        if ( i != currentIndex() )
            d->tabList[i]->refreshIcon(AscAppManager::themes().current().isDark() ? "dark" : "light");
        else d->tabList[i]->refreshIcon(d->tabList[i]->tab_theme_type);

    }

    polish();
}

int CTabBar::tabIndexAt(const QPoint &pos) const
{
    QPoint rel_pos = d->tabArea->mapFromParent(pos);
    if (d->tabArea->rect().contains(rel_pos)) {
        for (int i = 0; i < d->tabList.size(); i++) {
            if (d->_tabRect(i).contains(rel_pos))
                return i;
        }
    }
    return -1;
}

QWidget *CTabBar::tabAtIndex(int index) const
{
    return d->indexIsValid(index) ? d->tabList[index] : nullptr;
}

QWidget *CTabBar::tabIconLabel(int index) const
{
    return d->indexIsValid(index) ? d->tabList[index]->icon_label : nullptr;
}

QWidget *CTabBar::tabButton(int index) const
{
    return d->indexIsValid(index) ? d->tabList[index]->close_btn : nullptr;
}

CMenu *CTabBar::tabMenu(int index) const
{
    return d->indexIsValid(index) ? d->tabList[index]->menu : nullptr;
}

//QVariant CTabBar::tabData(int index) const
//{
//    return d->indexIsValid(index) ? d->tabList[index]->property("TabData") : QVariant();
//}

QIcon CTabBar::tabIcon(int index) const
{
    if (d->indexIsValid(index) && d->tabList[index]->tab_icon)
        return *d->tabList[index]->tab_icon;
    return QIcon();
}

QRect CTabBar::tabRect(int index) const
{
    return d->indexIsValid(index) ? d->_tabRect(index) : QRect();
}

QString CTabBar::tabText(int index) const
{
    return (d->indexIsValid(index)) ? d->tabList[index]->text : QString();
}

QVariant CTabBar::tabProperty(int index, const char *name)
{
    return d->indexIsValid(index) ? d->tabList[index]->property(name) : QVariant();
}

void CTabBar::tabInserted(int index)
{
    Q_UNUSED(index)
}

void CTabBar::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    for (int i = 0; i < d->tabList.size(); i++)
        d->tabList[i]->setFixedHeight(d->tabArea->height());

    int tabsWidth = d->tabList.isEmpty() ? 0 : count() * d->cellWidth();
    bool visible = (tabsWidth > d->tabArea->width());
    d->leftButton->setVisible(visible);
    d->rightButton->setVisible(visible);

    const int lastIndex = d->tabList.size() - 1;
    const int offsetX = (lastIndex == -1) ? 0 : d->tabArea->rect().right() - d->_tabRect(lastIndex).right();
    if (offsetX > TABSPACING) {
        for (int i = 0; i < d->tabList.size(); i++)
            d->tabList[i]->move(d->_tabRect(i).x() + offsetX - TABSPACING, 0);
    }
    d->changeScrollerState();
}

void CTabBar::wheelEvent(QWheelEvent *event)
{
    if (!isEnabled())
        return;

    QFrame::wheelEvent(event);
    if (!d->animationInProgress && d->tabArea->underMouse()) {
#ifdef DONT_USE_SIMPLE_WHEEL_SCROLL
        if (d->currentIndex > 0 && event->angleDelta().y() > 0) {
            emit onCurrentChangedByWhell(d->currentIndex - 1);
            d->onCurrentChanged(d->currentIndex - 1);
        } else
        if (d->currentIndex < count() - 1 && event->angleDelta().y() < 0) {
            emit onCurrentChangedByWhell(d->currentIndex + 1);
            d->onCurrentChanged(d->currentIndex + 1);
        }
#else
        if (event->angleDelta().y() != 0)
            d->scrollToDirection(event->angleDelta().y() > 0 ? d->Direction::Right : d->Direction::Left);
#endif
    }
}

bool CTabBar::eventFilter(QObject *watched, QEvent *event)
{
    if (!isEnabled())
        return true;

    if (watched == d->tabArea) {
        switch (event->type()) {
        case QEvent::MouseMove: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->buttons().testFlag(Qt::LeftButton)) {
                if (d->movedTab && !d->lock) {
                    int currPosX = d->movedTab->x();
                    int diffX = me->x() - currPosX - d->movedTabPressPosX;
                    const int signDiffX = signum(diffX > 0);
                    currPosX += qMin(diffX * signDiffX, d->cellWidth()/3) * signDiffX;
                    if (currPosX >= d->tabLayouts[0].x() && currPosX <= d->tabLayouts[d->tabLayouts.size() - 1].x()) {
                        d->movedTab->move(currPosX, 0);
                        int offsetX;
                        const int interIndex = d->getLayoutsIntersectedIndex(d->movedTab, offsetX);
                        if (/*interIndex != -1 &&*/ d->movedTab->index != interIndex && offsetX != 0) {
                            const int sign = signum(offsetX > 0);
                            if (sign * offsetX > d->movedTab->width()/2) {
                                const int destIndex = interIndex + sign;
                                Q_ASSERT(destIndex > -1 && destIndex < d->tabList.size());

                                int delta = d->tabLayouts[destIndex].x() - d->_tabRect(interIndex).x();
                                d->slide(interIndex, interIndex, delta, ANIMATION_MOVE_TAB_MS);
                                d->movedTab->index = interIndex;
                                d->tabIndex(interIndex) = destIndex;
                                std::swap(d->tabList[interIndex], d->tabList[destIndex]);
                                emit tabMoved(interIndex, destIndex);
                                d->currentIndex = interIndex;
                                emit currentChanged(interIndex);
                            }
                        }
                    }
                    // bool undockDirectionIsValid = AscAppManager::isRtlEnabled() ? d->tabArea->rect().left() <= me->x() : d->tabArea->rect().right() >= me->x();
                    if (!d->tabArea->rect().contains(me->pos()) /*&& undockDirectionIsValid*/) {
                        if (d->currentIndex != d->movedTabIndex)
                            d->reorderIndexes();
                        bool accepted = false;
                        emit tabUndock(d->currentIndex, accepted);
                        if (accepted) {
                            // d->movedTab->hide();
                            d->movedTab = nullptr;
                            d->movedTabIndex = -1;
                            SKIP_EVENTS_QUEUE([=]() {
                                removeTab(d->currentIndex);
                                while (d->animationInProgress)
                                    PROCESSEVENTS();
                                d->changeScrollerState();
                            });
                        }
                    }
                }
            }
            break;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                if (!d->animationInProgress) {
                    for (int i = 0; i < d->tabList.size(); i++) {
                        if (d->_tabRect(i).contains(me->pos())) {
                            d->lock = true;
                            d->movedTab = d->tabList[i];
                            d->movedTabIndex = i;
                            const int offset = d->getIntersectedOffset(i);
                            d->movedTabPosX = d->movedTab->x() - offset;
                            d->movedTabPressPosX = me->x() - d->movedTabPosX;
                            QPoint oldCurPos = QCursor::pos();
                            d->tabLayouts.clear();
                            for (int j = 0; j < d->tabList.size(); j++) {
                                d->tabLayouts.append(d->_tabRect(j).translated(-1 * offset, 0).adjusted(0,0,TABSPACING,0));
                                if (j != i)
                                    d->tabList[j]->stackUnder(d->movedTab);
                            }
                            emit tabBarClicked(i);
                            if (i != d->currentIndex)
                                d->onCurrentChanged(i);
                            else
                                d->scrollTo(i);
                            while (d->animationInProgress)
                                PROCESSEVENTS();
                            QCursor::setPos(oldCurPos);
                            d->lock = false;
                            return true;
                        }
                    }
                }
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton) {
                while (d->animationInProgress)
                    PROCESSEVENTS();
                if (d->movedTab && !d->lock) {
                    if (d->currentIndex != d->movedTabIndex) {
                        d->reorderIndexes();
                        int posX = d->tabLayouts[d->currentIndex].x();
                        d->movedTab->move(posX, 0);
                    } else {
                        d->movedTab->move(d->movedTabPosX, 0);
                    }
                    d->movedTab = nullptr;
                    d->movedTabIndex = -1;
                    d->changeScrollerState();
                    return true;
                }
            } else
            if (mouse_event->button() == Qt::MiddleButton) {
                for (int i = 0; i < d->tabList.size(); i++) {
                    if (d->_tabRect(i).contains(mouse_event->pos())) {
                        emit tabCloseRequested(i);
                        return true;
                    }
                }
            }
            break;
        }
        case QEvent::ContextMenu: {
            QContextMenuEvent* cm_event = static_cast<QContextMenuEvent*>(event);
            for (int i = 0; i < d->tabList.size(); i++) {
                if (d->_tabRect(i).contains(cm_event->pos())) {
                        QPoint pos = d->tabArea->mapToGlobal(cm_event->pos());
                        SKIP_EVENTS_QUEUE([=]() {
                            emit tabMenuRequested(i, pos);
                        });
                        return true;
                }
            }
            break;
        }
//        case QEvent::MouseButtonDblClick: {
//            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
//            if (mouse_event->button() == Qt::LeftButton) {
//                for (int i = 0; i < d->tabList.size(); i++) {
//                    if (d->tabList[i]->underMouse()) {
//                        emit tabBarDoubleClicked(i);
//                        return true;
//                    }
//                }
//            }
//            break;
//        }
        default:
            break;
        }
    } else
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Enter:
            setCursor(QCursor(Qt::ArrowCursor));
            break;
        case QEvent::Leave:
            if (d->movedTab && d->tabList.size() > 0)
                QApplication::postEvent(d->tabArea, new QMouseEvent(QEvent::MouseButtonRelease, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
            break;
        case QEvent::LayoutDirectionChange: {
            SKIP_EVENTS_QUEUE([=]() {
                for (int i = 0; i < d->tabList.size(); i++) {
                    d->tabList[i]->polish();
                    d->tabList[i]->text_label->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter | Qt::AlignAbsolute);
                }
                d->leftButton->setObjectName(AscAppManager::isRtlEnabled() ? "rightButton" : "leftButton");
                d->rightButton->setObjectName(AscAppManager::isRtlEnabled() ? "leftButton" : "rightButton");
                d->leftButton->style()->polish(d->leftButton);
                d->rightButton->style()->polish(d->rightButton);
                int n = count();
                for (int i = 0; i < n/2; i++)
                    swapTabs(i, n - i - 1);
            });
            break;
        }
        default:
            break;
        }
    }
    return QFrame::eventFilter(watched, event);
}

#include "ctabbar.moc"
