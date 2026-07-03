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
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "windows/cwindowbase.h"
#include "cascapplicationmanagerwrapper.h"
#include "components/ctooltip.h"
#include "components/cpushbutton.h"
#include "utils.h"
#include "defines.h"
#ifdef _WIN32
# include "windows/platform_win/caption.h"
#endif
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QScreen>
#include <functional>


class CWindowBase::CWindowBasePrivate
{
public:
    CWindowBasePrivate() {
#ifdef Q_OS_LINUX
        GET_REGISTRY_SYSTEM(reg_system)
        GET_REGISTRY_USER(reg_user)
        if ( reg_user.value("titlebar") == "custom" ||
                reg_system.value("titlebar") == "custom" )
        {
            is_custom_window_ = true;
        }
#else
        is_custom_window_ = true;
#endif
    }

    bool is_custom_window_ = false;
};


CWindowBase::CWindowBase(const QRect& rect)
    : QMainWindow(nullptr)
    , m_pTopButtons(3, nullptr)
    , pimpl{new CWindowBasePrivate}
    , m_windowActivated(false)
{
    setWindowIcon(Utils::appIcon());
    double tempDpi = 1.0;
    if (QGuiApplication::platformName() == "wayland") {
        m_dpiRatio = 1.0;
        m_window_rect = startRect(rect, tempDpi);
        m_dpiRatio = 1.0;
    } else {
        m_window_rect = startRect(rect, m_dpiRatio);
    }

    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
#ifdef __linux__
    setGeometry(m_window_rect); // for Windows is set in CWindowPlatform
#endif
}

CWindowBase::~CWindowBase()
{

}

/** Public **/

QRect CWindowBase::startRect(const QRect &rc, double &dpi)
{
    QRect prim_scr_rc = qApp->primaryScreen()->availableGeometry();
    if (QGuiApplication::platformName() == "wayland") {
        dpi = 1.0;
    } else {
        dpi = Utils::getScreenDpiRatio(rc.isEmpty() ? prim_scr_rc.topLeft() : rc.topLeft());
    }
    QSize def_size = MAIN_WINDOW_DEFAULT_SIZE * dpi;
    QRect def_rc = QRect(prim_scr_rc.center() - QPoint(def_size.width()/2, def_size.height()/2), def_size),
          out_rc = rc.isEmpty() ? def_rc : rc,
          scr_rc = Utils::getScreenGeometry(out_rc.topLeft());
    return scr_rc.intersects(out_rc) ? scr_rc.intersected(out_rc) : def_rc;
}

QSize CWindowBase::expectedContentSize(const QRect &rc, bool extended)
{
    double dpi = 1.0;
    QRect win_rc = startRect(rc, dpi);
#ifdef _WIN32
    int brd = Utils::getWinVersion() < Utils::WinVer::Win10 ? MAIN_WINDOW_BORDER_WIDTH * dpi : 0;
#else
    int brd = MAIN_WINDOW_BORDER_WIDTH * dpi;
#endif
    return win_rc.adjusted(brd, extended ? brd : TITLE_HEIGHT * dpi + brd, -brd, -brd).size();
}

QWidget * CWindowBase::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CWindowBase*>(this));
}

QWidget * CWindowBase::mainPanel() const
{
    return m_pMainPanel;
}

bool CWindowBase::isCustomWindowStyle()
{
    return pimpl->is_custom_window_;
}

void CWindowBase::updateScaling(bool resize)
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio ) {
        setScreenScalingFactor(dpi_ratio, resize);
        adjustGeometry();
    }
}

void CWindowBase::applyTheme(const std::wstring& theme)
{
    Q_UNUSED(theme)
    QColor background = GetColorByRole(ecrWindowBackground);
    QColor border = GetColorByRole(ecrWindowBorder);
    setWindowColors(background, border, isActiveWindow());
}

/** Protected **/

CPushButton* CWindowBase::createToolButton(QWidget * parent, const QString& name)
{
    CPushButton * btn = new CPushButton(parent);
    btn->setObjectName(name);
    btn->setProperty("class", "normal");
    btn->setProperty("act", "tool");
    btn->setFixedSize(int(TITLEBTN_WIDTH*m_dpiRatio), int(m_toolbtn_height * m_dpiRatio));
#ifdef __linux__
    btn->setMouseTracking(true);
    btn->setProperty("unix", true);
    if (WindowHelper::getEnvInfo() == WindowHelper::KDE)
        btn->setProperty("kde", true);
#else
    btn->setProperty("unix", false);
#endif
    return btn;
}

QWidget* CWindowBase::createTopPanel(QWidget *parent)
{
    QWidget *_boxTitleBtns;
#ifdef __linux__
    _boxTitleBtns = new QWidget(parent);
#else
    _boxTitleBtns = static_cast<QWidget*>(new Caption(parent));
#endif
    _boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *layoutBtns = new QHBoxLayout(_boxTitleBtns);
    layoutBtns->setContentsMargins(0, 0, 0, 0);
    layoutBtns->setSpacing(int(1*m_dpiRatio));
    layoutBtns->addStretch();
    layoutBtns->setAlignment(Qt::AlignTop);
    _boxTitleBtns->setLayout(layoutBtns);
    if (isCustomWindowStyle()) {
        const QString names[3] = {"toolButtonMinimize", "toolButtonMaximize", "toolButtonClose"};
        std::function<void(void)> btn_methods[3] = {
            [=]{onMinimizeEvent();}, [=]{onMaximizeEvent();}, [=]{onCloseEvent();}};
        m_pTopButtons.clear();
        for (int i = 0; i < 3; i++) {
            CPushButton *btn = createToolButton(_boxTitleBtns, names[i]);
            QObject::connect(btn, &QPushButton::clicked, btn_methods[i]);
            m_pTopButtons.push_back(btn);
            layoutBtns->addWidget(btn);
        }
    }
    return _boxTitleBtns;
}

void CWindowBase::saveWindowState(const QString &baseKey)
{
    if (!windowState().testFlag(Qt::WindowFullScreen)) {
        GET_REGISTRY_USER(reg_user)
        if (QGuiApplication::platformName() == "wayland") {
            QRect rect = normalGeometry();
            rect.moveTo(0, 0);
            reg_user.setValue(baseKey + "position", rect);
        } else {
            reg_user.setValue(baseKey + "position", normalGeometry());
        }
        if (windowState().testFlag(Qt::WindowMaximized)) {
            reg_user.setValue(baseKey + "maximized", true);
        } else {
            reg_user.remove(baseKey + "maximized");
        }
    }
}

void CWindowBase::moveToPrimaryScreen()
{
    QMainWindow::showNormal();
    QRect rect = QApplication::primaryScreen()->availableGeometry();
    double dpiRatio = Utils::getScreenDpiRatio(rect.topLeft());
    m_window_rect = QRect(rect.translated(100, 100).topLeft() * dpiRatio,
                          MAIN_WINDOW_DEFAULT_SIZE * dpiRatio);
    setGeometry(m_window_rect);
}

void CWindowBase::setIsCustomWindowStyle(bool custom)
{
    pimpl->is_custom_window_ = custom;
}

bool CWindowBase::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
       QHelpEvent *hlp = static_cast<QHelpEvent*>(event);
       QWidget *wgt = qApp->widgetAt(hlp->globalPos());
       if (wgt && !findChild<CToolTip*>()) {
           QString text("");
           if (wgt->property("ToolTip").isValid())
               text = wgt->property("ToolTip").toString();
           if (m_pMainPanel && !text.isEmpty()) {
               CToolTip *tool = new CToolTip(m_pMainPanel, text, hlp->globalPos());
               connect(wgt, &QWidget::destroyed, tool, &CToolTip::deleteLater);
           }
       }
    }
    return QMainWindow::event(event);
}

void CWindowBase::setScreenScalingFactor(double factor, bool resize)
{
    if (QGuiApplication::platformName() == "wayland") {
        factor = 1.0;
    }
    if (resize && !isMaximized()) {
        setMinimumSize(WINDOW_MIN_WIDTH * factor, WINDOW_MIN_HEIGHT * factor);
        double change_factor = factor / m_dpiRatio;
        QRect _src_rect = geometry();
        double dest_width_change = _src_rect.width() * (1 - change_factor);
        QRect _dest_rect = QRect{_src_rect.translated(int(dest_width_change/2), 0).topLeft(), _src_rect.size() * change_factor};
        setGeometry(_dest_rect);
    }
    m_dpiRatio = factor;
    if (m_boxTitleBtns) {
        QLayout *pLayoutBtns = m_boxTitleBtns->layout();
        pLayoutBtns->setSpacing(int(1 * m_dpiRatio));
        if (isCustomWindowStyle()) {
            pLayoutBtns->setContentsMargins(0, 0, 0, 0);
            QSize small_btn_size(int(TITLEBTN_WIDTH*m_dpiRatio), int(m_toolbtn_height * m_dpiRatio));
            foreach (auto pBtn, m_pTopButtons)
                pBtn->setFixedSize(small_btn_size);
        }
    }
}

void CWindowBase::applyWindowState()
{
    if (isCustomWindowStyle() && m_pTopButtons[BtnType::Btn_Maximize]) {
        m_pTopButtons[BtnType::Btn_Maximize]->setProperty("class", isMaximized() ? "min" : "normal") ;
        m_pTopButtons[BtnType::Btn_Maximize]->style()->polish(m_pTopButtons[BtnType::Btn_Maximize]);
    }
}

void CWindowBase::setWindowTitle(const QString& title)
{
    QMainWindow::setWindowTitle(title);
    if (m_labelTitle)
        m_labelTitle->setText(title);
}

void CWindowBase::onMinimizeEvent()
{
    QMainWindow::showMinimized();
}

void CWindowBase::onMaximizeEvent()
{
    isMaximized() ? QMainWindow::showNormal() : QMainWindow::showMaximized();
}

void CWindowBase::onCloseEvent()
{
    deleteLater();
}

void CWindowBase::focus()
{
    setFocus();
}

/** Private **/

void CWindowBase::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        adjustGeometry();
        applyTheme(GetCurrentTheme().id());
    }
}
