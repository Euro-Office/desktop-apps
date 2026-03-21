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


#include "windows/cpresenterwindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QGridLayout>
#include <clangater.h>
#include <qtcomp/qnativeevent.h>

using namespace std::placeholders;


CPresenterWindow::CPresenterWindow(const QRect &rect, const QString &title, QCefView *view) :
    CWindowPlatform(rect)
{    
    m_pMainPanel = createMainPanel(this, title, static_cast<QWidget*>(view));
    setCentralWidget(m_pMainPanel);
#ifdef __linux__
    if (isCustomWindowStyle()) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
    updateGeometry();
#endif
}

CPresenterWindow::~CPresenterWindow()
{

}

/** Public **/

void CPresenterWindow::applyTheme(const std::wstring& theme)
{
    CWindowPlatform::applyTheme(theme);
    m_pMainPanel->setProperty("uitheme", QString::fromStdWString(GetActualTheme(theme)));
    m_pMainPanel->setProperty("uithemetype", GetCurrentTheme().stype());
    if (m_boxTitleBtns) {
        m_labelTitle->style()->polish(m_labelTitle);
        if (m_pTopButtons[BtnType::Btn_Minimize]) {
            foreach (auto btn, m_pTopButtons)
                btn->style()->polish(btn);
        }
        m_boxTitleBtns->style()->polish(m_boxTitleBtns);
    }
    m_pMainPanel->style()->polish(m_pMainPanel);
    update();
}

bool CPresenterWindow::holdView(int id) const
{
    return ((QCefView *)m_pMainView)->GetCefView()->GetId() == id;
}

void CPresenterWindow::closeEvent(QCloseEvent *e)
{
    if (isEnabled())
        onCloseEvent();
    e->ignore();
}

void CPresenterWindow::onLayoutDirectionChanged()
{

}

/** Private **/

QWidget * CPresenterWindow::createMainPanel(QWidget * parent, const QString& title, QWidget * view)
{
    QWidget * mainPanel = new QWidget(parent);
    mainPanel->setObjectName("mainPanel");
    mainPanel->setProperty("rtl-font", CLangater::isRtlLanguage(CLangater::getCurrentLangCode()));
    mainPanel->setProperty("uitheme", QString::fromStdWString(GetCurrentTheme().id()));
    QString css(AscAppManager::getWindowStylesheets(m_dpiRatio));
#ifdef __linux__
    css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
    mainPanel->setStyleSheet(css);

    QGridLayout * mainGridLayout = new QGridLayout(mainPanel);
    mainGridLayout->setSpacing(0);
    QtComp::Widget::setLayoutMargin(mainGridLayout, 0);
    mainPanel->setLayout(mainGridLayout);

    m_boxTitleBtns = createTopPanel(mainPanel);
    m_boxTitleBtns->setObjectName("box-title-tools");

    m_labelTitle = new CElipsisLabel(title, m_boxTitleBtns);
    m_labelTitle->setObjectName("labelAppTitle");
    m_labelTitle->setMinimumWidth(100);
    m_labelTitle->setEllipsisMode(Qt::ElideMiddle);
    m_labelTitle->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    m_labelTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *topLayout = static_cast<QHBoxLayout*>(m_boxTitleBtns->layout());
    topLayout->insertWidget(0, m_labelTitle);
    QLayoutItem *stretch = topLayout->takeAt(1);
    if (stretch)
        delete stretch;

    if (isCustomWindowStyle()) {
#ifdef __linux__
        m_labelTitle->setMouseTracking(true);
        //mainGridLayout->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
        /*QPalette _palette(palette());
        _palette.setColor(QPalette::Background, QColor("#f1f1f1"));
        setAutoFillBackground(true);
        setPalette(_palette);
        setStyleSheet("QMainWindow{border:1px solid #888;}");*/
#endif
        QSize small_btn_size(int(TITLEBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));
        QWidget * _lb = new QWidget(m_boxTitleBtns);
        _lb->setFixedWidth( (small_btn_size.width() + static_cast<QHBoxLayout*>(m_boxTitleBtns->layout())->spacing()) * 3 );
        static_cast<QHBoxLayout*>(m_boxTitleBtns->layout())->insertWidget(0, _lb);
    } else {
        QLinearGradient gradient(mainPanel->rect().topLeft(), QPoint(mainPanel->rect().left(), 29));
        gradient.setColorAt(0, QColor(0xeee));
        gradient.setColorAt(1, QColor(0xe4e4e4));
        m_labelTitle->setFixedHeight(0);
        m_boxTitleBtns->setFixedHeight(16*m_dpiRatio);
    }
    mainGridLayout->addWidget(m_boxTitleBtns, 0, 0, Qt::AlignTop);
    if (!view) {
        QCefView * pMainWidget = AscAppManager::createViewer(mainPanel, mainPanel->size());
        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
        pMainWidget->setObjectName("mainPanel");
        pMainWidget->setHidden(false);
        m_pMainView = (QWidget*)pMainWidget;
    } else {
        m_pMainView = view;
        m_pMainView->setParent(mainPanel);
        m_pMainView->show();
    }
    m_pMainView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainGridLayout->addWidget(m_pMainView, 1, 0);

    return mainPanel;
}

void CPresenterWindow::setScreenScalingFactor(double factor, bool resize)
{
    CWindowPlatform::setScreenScalingFactor(factor, resize);
    QString css(AscAppManager::getWindowStylesheets(factor));
#ifdef __linux__
    css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
    if (!css.isEmpty()) {                
        m_pMainPanel->setStyleSheet(css);
    }
}

void CPresenterWindow::onCloseEvent() // Reporter mode
{
    if (m_pMainView) {
        m_pMainView->setObjectName("destroyed");
        AscAppManager::getInstance().DestroyCefView(((QCefView *)m_pMainView)->GetCefView()->GetId() );
    }
}

void CPresenterWindow::focus()
{
    if (m_pMainView)
        ((QCefView *)m_pMainView)->setFocusToCef();
}
