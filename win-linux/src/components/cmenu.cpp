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

#include "cmenu.h"
#include "cascapplicationmanagerwrapper.h"
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QLayout>
#include <QAction>
#include <qtcomp/qnativeevent.h>

#ifdef __linux__
# include <QX11Info>
#endif

#define ICON_SIZE (QSizeF(20,20) * m_dpiRatio)
// #define SMALL_ICON_SIZE (QSizeF(12,12) * m_dpiRatio)
#define ITEM_MAX_HEIGHT   26 * m_dpiRatio
// #define MARGINS 6 * m_dpiRatio
#define SPACING 2 * m_dpiRatio
#define SHADOW  12 * m_dpiRatio
#define RADIUS  3 * m_dpiRatio
#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, this, callback)


static bool isCompositingEnabled()
{
#ifdef __linux__
    return QX11Info::isCompositingManagerRunning();
#else
    return true;
#endif
}

class MenuItem : public QLabel
{
    Q_OBJECT
public:
    explicit MenuItem(QWidget *parent, const QString &text = QString()) : QLabel(parent) {
        setText(text);
    }
    ~MenuItem()
    {}
    void setIcon(const QIcon &icon) {
        m_icon = icon;
        if (isVisible())
            update();
    }
    void setIconSize(const QSize &size) {
        m_icon_size = size;
        if (isVisible())
            update();
    }
    void setInactive(bool disabled) {
        m_disabled = disabled;
        setProperty("disabled", disabled);
        style()->polish(this);
    }
    bool isInactive() {
        return m_disabled;
    }

protected:
    virtual void paintEvent(QPaintEvent *ev) final {
        if (!m_icon.isNull()) {
            QSize icon_size(m_icon_size);
            if (m_icon_size.isEmpty()) {
                QStyleOption opt;
                QtComp::Widget::initStyleOption(&opt, this);
                int icSize = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
                icon_size.setWidth(icSize);
                icon_size.setHeight(icSize);
            }
            QPixmap pix = m_icon.pixmap(icon_size, QIcon::Normal);
            int margin = (height() - icon_size.height())/2;
            int posX = AscAppManager::isRtlEnabled() ? width() - icon_size.width() - 1.5*margin : 1.5*margin;
            QRect rc = QRect(QPoint(posX, margin), icon_size);
            if (m_disabled) {
                QPixmap pix_out(icon_size);
                pix_out.fill(Qt::transparent);
                QPainter p(&pix_out);
                p.setOpacity(0.35);
                p.drawPixmap(0, 0, pix);
                p.end();
                pix = pix_out;
            }
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            p.setRenderHints(QPainter::RenderHint::Antialiasing);
            p.drawPixmap(rc, pix);
            p.end();
        }
        QLabel::paintEvent(ev);
    }

private:
    QIcon m_icon;
    QSize m_icon_size;
    bool  m_disabled = false;
};

class CMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CMenuWidget(QWidget * parent = nullptr) :
        QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    {
        m_dpiRatio = CScalingWrapper::parentScalingFactor(topLevelWidget());
        if (isCompositingEnabled()) {
            setAttribute(Qt::WA_TranslucentBackground);
            m_shadow = qRound(SHADOW);
        }
        setLayoutDirection(AscAppManager::isRtlEnabled() ? Qt::RightToLeft : Qt::LeftToRight);
        installEventFilter(this);
        setLayout(new QVBoxLayout);
        layout()->setContentsMargins(m_shadow, m_shadow, m_shadow, 1.4*m_shadow);
        layout()->setSpacing(0);

        m_mainFrame = new QFrame(this);
        m_mainFrame->setObjectName("menuFrame");
        m_mainFrame->setLayout(new QVBoxLayout);
        m_mainFrame->layout()->setContentsMargins(0, 2*SPACING, 0, 2*SPACING);
        m_mainFrame->layout()->setSpacing(0);
        layout()->addWidget(m_mainFrame);

        if (isCompositingEnabled()) {
            QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(m_mainFrame);
            shadow->setBlurRadius(21.0*m_dpiRatio);
            shadow->setColor(QColor(0, 0, 0, 65));
            shadow->setXOffset(0);
            shadow->setYOffset(3.5*m_dpiRatio);
            m_mainFrame->setGraphicsEffect(shadow);
        }
    }
    ~CMenuWidget()
    {}

    MenuItem* addSection(const QString &text, const QIcon &icon)
    {
        MenuItem *item = new MenuItem(m_mainFrame, text);
        item->setObjectName("menuItem");
        item->setAttribute(Qt::WA_Hover);
        item->installEventFilter(this);
        item->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft)  | Qt::AlignAbsolute);
#ifdef _WIN32
        if (m_dpiRatio == 1.0 && AscAppManager::themes().current().isDark() && !AscAppManager::isRtlEnabled()) {
            QFont fnt = item->font();
            fnt.setStretch(70);
            item->setFont(fnt);
        }
#endif
        if (!icon.isNull()) {
            item->setIconSize(ICON_SIZE.toSize());
            item->setIcon(icon);
        }
        m_mainFrame->layout()->addWidget(item);
        return item;
    }
    void addSeparator()
    {
        qobject_cast<QVBoxLayout*>(m_mainFrame->layout())->addSpacerItem(new QSpacerItem(4, 2*SPACING, QSizePolicy::Fixed, QSizePolicy::Fixed));
        QFrame *line = new QFrame(m_mainFrame);
        line->setObjectName("menuSeparator");
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);
        m_mainFrame->layout()->addWidget(line);
        qobject_cast<QVBoxLayout*>(m_mainFrame->layout())->addSpacerItem(new QSpacerItem(4, 2*SPACING, QSizePolicy::Fixed, QSizePolicy::Fixed));
    }

    int m_shadow = 0;

signals:
    void wasHidden();

protected:
    virtual void showEvent(QShowEvent *ev) final {
        QWidget::showEvent(ev);
        if (ev->type() == QShowEvent::Show) {
            if (AscAppManager::isRtlEnabled())
                move(QCursor::pos() - QPoint(width() - m_shadow, m_shadow));
        }
    }
    virtual void closeEvent(QCloseEvent *ev) final {
        ev->ignore();
        if (isVisible())
            hide();
    }
    virtual bool eventFilter(QObject *obj, QEvent *ev) final {
        switch (ev->type()) {
        case QEvent::WindowDeactivate:
            if (obj == this)
                hide();
            break;
        case QEvent::HoverEnter:
            if (MenuItem *item = qobject_cast<MenuItem*>(obj)) {
                if (!item->isInactive()) {
                    item->setProperty("hovered", true);
                    item->style()->polish(item);
                }
            }
            break;
        case QEvent::HoverLeave:
            if (MenuItem *item = qobject_cast<MenuItem*>(obj)) {
                if (m_current_index == -1 || menuItemAt(m_current_index) != item) {
                    item->setProperty("hovered", false);
                    item->style()->polish(item);
                }
            }
            break;
        case QEvent::MouseButtonPress:
            if (obj == this) {
                QMouseEvent *mev = static_cast<QMouseEvent*>(ev);
                if (!m_mainFrame->geometry().contains(mev->pos()))
                    hide();
            }
            break;
        case QEvent::MouseButtonRelease:
            if (MenuItem *item = qobject_cast<MenuItem*>(obj)) {
                if (!item->isInactive()) {
                    QMouseEvent *mev = static_cast<QMouseEvent*>(ev);
                    if (mev->button() == Qt::LeftButton) {
                        if (!item->actions().empty())
                            emit item->actions().at(0)->triggered();
                        hide();
                    }
                }
            }
            break;
        case QEvent::KeyPress:
            if (obj == this) {
                QKeyEvent *kev = static_cast<QKeyEvent*>(ev);
                switch (kev->key()) {
                case Qt::Key_Up:
                    onKeyUpDownPressed(Qt::Key_Up);
                    break;
                case Qt::Key_Down:
                    onKeyUpDownPressed(Qt::Key_Down);
                    break;
                case Qt::Key_Return:
                case Qt::Key_Enter:
                    if (m_current_index != -1) {
                        MenuItem *item = menuItemAt(m_current_index);
                        if (item && !item->isInactive()) {
                            if (!item->actions().empty())
                                emit item->actions().at(0)->triggered();
                            hide();
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case QEvent::KeyRelease:
            if (obj == this) {
                QKeyEvent *kev = static_cast<QKeyEvent*>(ev);
                switch (kev->key()) {
                case Qt::Key_Escape:
                case Qt::Key_Return:
                case Qt::Key_Enter:
                    hide();
                    break;
                default:
                    break;
                }
            }
            break;
        case QEvent::Hide:
            if (obj == this)
                emit wasHidden();
            break;
        default:
            break;
        }
        return QWidget::eventFilter(obj, ev);
    }

private:    
    MenuItem* menuItemAt(int index) {
        int curr_index = 0;
        for (int i = 0; i < m_mainFrame->layout()->count(); ++i) {
            auto item = m_mainFrame->layout()->itemAt(i);
            if (item && item->widget()) {
                if (MenuItem *menu_item = qobject_cast<MenuItem*>(item->widget())) {
                    if (index == curr_index++)
                        return menu_item;
                }
            }
        }
        return nullptr;
    }
    int menuItemCount() {
        int count = 0;
        for (int i = 0; i < m_mainFrame->layout()->count(); ++i) {
            auto item = m_mainFrame->layout()->itemAt(i);
            if (item && item->widget() && qobject_cast<MenuItem*>(item->widget()))
                ++count;
        }
        return count;
    }
    void onKeyUpDownPressed(Qt::Key key) {
        bool item_exist = false;
        int count = menuItemCount();
        for (int i = 0; i < count; i++) {
            if (key == Qt::Key_Up) {
                --m_current_index;
                if (m_current_index < 0)
                    m_current_index = count - 1;
            } else {
                ++m_current_index;
                if (m_current_index > count - 1)
                    m_current_index = 0;
            }
            MenuItem *item = menuItemAt(m_current_index);
            if (item && !item->isInactive()) {
                item_exist = true;
                break;
            }
        }
        if (!item_exist)
            m_current_index = -1;
        else {
            MenuItem *item = menuItemAt(m_current_index);
            item->setProperty("hovered", true);
            item->style()->polish(item);
        }
        if (m_prev_index > -1) {
            if (MenuItem *prev_item = menuItemAt(m_prev_index)) {
                prev_item->setProperty("hovered", false);
                prev_item->style()->polish(prev_item);
            }
        }
        m_prev_index = m_current_index;
    }

    QFrame *m_mainFrame;
    double m_dpiRatio = 1;
    int m_current_index = -1,
        m_prev_index = -1;
};

const char* CMenu::m_actionText[ACTION_COUNT] = {
    QT_TRANSLATE_NOOP("CMenu", "Close"),
    QT_TRANSLATE_NOOP("CMenu", "Close saved"),
    QT_TRANSLATE_NOOP("CMenu", "Close all"),
    QT_TRANSLATE_NOOP("CMenu", "Show in folder"),
    QT_TRANSLATE_NOOP("CMenu", "Move to start"),
    QT_TRANSLATE_NOOP("CMenu", "Move to end"),
    QT_TRANSLATE_NOOP("CMenu", "Unpin tab to window"),
    QT_TRANSLATE_NOOP("CMenu", "Pin to tab"),
    QT_TRANSLATE_NOOP("CMenu", "Create new")
};

CMenu::CMenu(QWidget * parent) :
    QObject(parent),
    m_menu_widget(nullptr),
    m_parent(parent)
{

}

CMenu::~CMenu()
{

}

QAction *CMenu::addSection(Action action, const QIcon &icon)
{
    QAction *act = addSection(tr(m_actionText[action]), icon);
    act->setProperty("id", action);
    return act;
}

QAction* CMenu::addSection(const QString &text, const QIcon &icon)
{
    QAction *act = new QAction(this);
    act->setText(text);
    act->setIcon(icon);
    m_actions.push_back(act);    
    return act;
}

QAction* CMenu::addSeparator()
{
    QAction *act = new QAction(this);
    act->setSeparator(true);
    m_actions.push_back(act);
    return act;
}

void CMenu::setSectionIcon(Action action, const QIcon &icon)
{
    foreach (auto *act, m_actions) {
        QVariant id = act->property("id");
        if (id.isValid() && id.toInt() == action) {
            act->setIcon(icon);
            break;
        }
    }
}

void CMenu::setSectionEnabled(Action action, bool enabled)
{
    foreach (auto *act, m_actions) {
        QVariant id = act->property("id");
        if (id.isValid() && id.toInt() == action) {
            act->setEnabled(enabled);
            break;
        }
    }
}

void CMenu::exec(const QPoint &pos)
{
    m_menu_widget = new CMenuWidget(m_parent);
    m_menu_widget->setObjectName("CMenuWidget");
    connect(m_menu_widget, &CMenuWidget::wasHidden, this, [=]() {
        m_menu_widget->deleteLater();
        m_menu_widget = nullptr;
        emit wasHidden();
    });
    foreach (auto *act, m_actions) {
        if (act->isSeparator()) {
            m_menu_widget->addSeparator();
        } else {
            MenuItem *label = m_menu_widget->addSection(act->text(), act->icon());
            label->addAction(act);
            if (!act->isEnabled())
                label->setInactive(true);
            connect(act, &QAction::changed, label, [=]() {
                label->setInactive(!act->isEnabled());
            });
        }
    }
    m_menu_widget->move(pos - QPoint(m_menu_widget->m_shadow, m_menu_widget->m_shadow));
    m_menu_widget->show();
    m_menu_widget->activateWindow();
}

#include "cmenu.moc"
