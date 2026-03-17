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

#include "windows/cmainwindow.h"
#include "ceditortools.h"
#include "iconfactory.h"
#include "clangater.h"
#include "defines.h"
#include "utils.h"
#include "components/cfiledialog.h"
#include "components/cmenu.h"
#include "common/Types.h"
#include "version.h"
#include "components/cmessage.h"
#include "ctabundockevent.h"
#include <qtcomp/qdesktopwidget.h>
#include <QGridLayout>
#include <QTimer>
#include <QApplication>
#include <QAction>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>

#include <qtcomp/qpalette.h>

#ifdef _WIN32
# include "shlobj.h"
#endif


#ifdef _UPDMODULE
  #include "../version.h"
#endif

using namespace std::placeholders;
using namespace NSEditorApi;

CMainWindow::CMainWindow(const QRect &rect) :
    CWindowPlatform(rect),
    CScalingWrapper(m_dpiRatio),
    m_savePortal(QString())
{
    setObjectName("MainWindow");
#ifdef _WIN32
    if (Utils::getWinVersion() >= Utils::WinVer::Win10 && isCustomWindowStyle())
        m_toolbtn_height = TOOLBTN_HEIGHT_WIN10;
#endif
    m_pMainPanel = createMainPanel(this);
    setCentralWidget(m_pMainPanel);
    QString css{AscAppManager::getWindowStylesheets(m_dpiRatio)};
#ifdef __linux__
    if (WindowHelper::getEnvInfo() == WindowHelper::KDE)
        setWindowTitle(tr("Main Window"));
    setAcceptDrops(true);
    if (isCustomWindowStyle()) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
    QMetaObject::connectSlotsByName(this);
    css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
    m_pMainPanel->setStyleSheet(css);
    QString tab_css = Utils::readStylesheets(":/styles/tabbar.qss");
    m_pTabs->tabBar()->setStyleSheet(tab_css.arg(GetColorQValueByRole(ecrWindowBackground),
                                                 GetColorQValueByRole(ecrButtonBackground),
                                                 GetColorQValueByRole(ecrButtonHoverBackground),
                                                 GetColorQValueByRole(ecrButtonPressedBackground),
                                                 GetColorQValueByRole(ecrTabDivider),
                                                 GetColorQValueByRole(ecrTabWordActive)));
    updateScalingFactor(m_dpiRatio);
    goStart();
}

CMainWindow::~CMainWindow()
{

}

/** Public **/

QWidget * CMainWindow::editor(int index)
{
    return tabWidget()->panel(index);
}

QRect CMainWindow::windowRect() const
{
    return normalGeometry();
}

QString CMainWindow::documentName(int vid)
{
    int i = tabWidget()->tabIndexByView(vid);
    return (i < 0) ? "" : tabWidget()->panel(i)->data()->title();
}

void CMainWindow::selectView(int viewid)
{
    int _index = tabWidget()->tabIndexByView(viewid);
    if ( !(_index < 0) ) {
        tabWidget()->setCurrentIndex(_index);
        toggleButtonMain(false);
    }
}

void CMainWindow::selectView(const QString& url)
{
    int _index = tabWidget()->tabIndexByUrl(url);
    if ( !(_index < 0) ) {
        tabWidget()->setCurrentIndex(_index);
        toggleButtonMain(false);
    }
}

int CMainWindow::attachEditor(QWidget * panel, int index)
{
    if (!QCefView::IsSupportLayers()) {
        CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
        if (_panel)
            _panel->view()->SetCaptionMaskSize(0);
    }
    int _index = tabWidget()->insertPanel(panel, index);
    if ( !(_index < 0) ) {
        tabWidget()->setCurrentIndex(_index);
        toggleButtonMain(false);
    }
    return _index;
}

int CMainWindow::attachEditor(QWidget * panel, const QPoint& pt)
{
    QPoint _pt_local = tabWidget()->tabBar()->mapFromGlobal(pt);
    if (AscAppManager::isRtlEnabled())
        _pt_local -= QPoint(32 * m_dpiRatio, 0); // Minus tabScroll width
#ifdef Q_OS_WIN
# if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QPoint _tl = windowRect().topLeft();
    if ( _tl.x() < _pt_local.x() && _tl.y() < _pt_local.y() )
        _pt_local -= windowRect().topLeft();
# endif
#endif
    int _index = tabWidget()->tabBar()->tabIndexAt(_pt_local);
    if ( !(_index < 0) ) {
        QRect _rc_tab = tabWidget()->tabBar()->tabRect(_index);
        if ( _pt_local.x() > _rc_tab.left() + (_rc_tab.width() / 2) ) ++_index;
    }
    return attachEditor(panel, _index);
}

int CMainWindow::editorsCount()
{
    return tabWidget()->count(cvwtEditor);
}

int CMainWindow::editorsCount(const std::wstring& portal)
{
    return tabWidget()->count(portal, true);
}

bool CMainWindow::canPinTabAtPoint(const QPoint& pt)
{
    QRect _rc_title(m_pMainPanel->geometry());
    _rc_title.setHeight(tabWidget()->tabBar()->height());
    int dx1 = (AscAppManager::isRtlEnabled()) ? 3 * int(TITLEBTN_WIDTH * m_dpiRatio) : m_pButtonMain->width();
    int dx2 = (AscAppManager::isRtlEnabled()) ? -1 * m_pButtonMain->width() : -3 * int(TITLEBTN_WIDTH * m_dpiRatio);
    _rc_title.adjust(dx1, 1, dx2, 0);
    bool containsPoint = _rc_title.contains(mapFromGlobal(pt));
    bool pinAllowed = m_pTabs->isTabPinAllowed();
    if (!containsPoint && !pinAllowed) {
        m_pTabs->setTabPinAllowed();
        return false;
    }
    return containsPoint && pinAllowed;
}

bool CMainWindow::holdView(int id) const
{
    return holdUid(id);
}

bool CMainWindow::slideshowHoldView(int id) const
{
    return m_pTabs->slideshowHoldView(id);
}

bool CMainWindow::isSlideshowMode() const
{
    return m_pTabs->fullScreenWidget() != nullptr;
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    CWindowPlatform::applyTheme(theme);
    m_pMainPanel->setProperty("uitheme", QString::fromStdWString(GetActualTheme(theme)));
    m_pMainPanel->setProperty("uithemetype", GetCurrentTheme().stype());
    for (int i(m_pTabs->count()); !(--i < 0);) {
        CAscTabData& _doc = *m_pTabs->panel(i)->data();
        if ( _doc.isViewType(cvwtSimple) ) {
            QJsonObject json{{"theme", QString::fromStdWString(theme)}, {"type", GetCurrentTheme().stype()}};
            AscAppManager::sendCommandTo(m_pTabs->panel(i)->cef(), L"uitheme:changed", Utils::stringifyJson(json).toStdWString());
        } else
        if ( _doc.isViewType(cvwtEditor) && !_doc.closed() ) {
            AscAppManager::sendCommandTo(m_pTabs->panel(i)->cef(), L"uitheme:changed", theme);
        }
        if (CMenu *menu = m_pTabs->tabBar()->tabMenu(i)) {
            menu->setSectionIcon(CMenu::ActionCreateNew, IconFactory::icon(IconFactory::CreateNew, SMALL_ICON * m_dpiRatio));
            menu->setSectionIcon(CMenu::ActionShowInFolder, IconFactory::icon(IconFactory::Browse, SMALL_ICON * m_dpiRatio));
            // menu->setSectionIcon(CMenu::ActionUnpinTab, IconFactory::icon(IconFactory::Unpin, SMALL_ICON * m_dpiRatio));
        }
    }
    // m_boxTitleBtns->style()->polish(m_boxTitleBtns);
    // m_pButtonMain->style()->polish(m_pButtonMain);
    // if (m_pTopButtons[BtnType::Btn_Minimize]) {
    //     foreach (auto btn, m_pTopButtons)
    //         btn->style()->polish(btn);
    // }

    QString css{AscAppManager::getWindowStylesheets(m_dpiRatio)};
#ifdef __linux__
    css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
    m_pMainPanel->setStyleSheet(css);
    QString tab_css = Utils::readStylesheets(":/styles/tabbar.qss");
    m_pTabs->tabBar()->setStyleSheet(tab_css.arg(GetColorQValueByRole(ecrWindowBackground),
                                                 GetColorQValueByRole(ecrButtonBackground),
                                                 GetColorQValueByRole(ecrButtonHoverBackground),
                                                 GetColorQValueByRole(ecrButtonPressedBackground),
                                                 GetColorQValueByRole(ecrTabDivider),
                                                 GetColorQValueByRole(ecrTabWordActive)));
    m_pTabs->applyUITheme(GetActualTheme(theme));
    // m_pButtonMain->setIcon(MAIN_ICON_PATH, GetCurrentTheme().isDark() ? "logo-light" : "logo-dark");
    m_pButtonMain->setIconSize(MAIN_ICON_SIZE * m_dpiRatio);
    if (m_pWidgetDownload && m_pWidgetDownload->toolButton()) {
        m_pWidgetDownload->applyTheme();
    }
}

/** Private **/

void CMainWindow::focus()
{
    if ( m_pTabs->isActiveWidget() ) {
        m_pTabs->setFocusedView();
    } else {
        ((QCefView *)m_pMainWidget)->setFocusToCef();
    }
}

void CMainWindow::onCloseEvent()
{
    close();
}

void CMainWindow::closeEvent(QCloseEvent * e)
{
    if (isEnabled())
        AscAppManager::getInstance().closeQueue().enter(sWinTag{CLOSE_QUEUE_WIN_TYPE_MAIN, size_t(this)});
    e->ignore();
}

void CMainWindow::close()
{
    CWindowBase::saveWindowState();
    m_isCloseAll = true;

    if ( m_pTabs->count() == 0 ) {
        hide();
        emit aboutToClose();
    } else {
        onFullScreen(-1, false);

#ifdef _WIN32
        if (isSessionInProgress() && m_pTabs->count(cvwtEditor) > 1) {
#else
        if (m_pTabs->count(cvwtEditor) > 1) {
#endif
            GET_REGISTRY_USER(reg_user);
            if (!reg_user.value("ignoreMsgAboutOpenTabs", false).toBool()) {
                for (int i = 0; i < m_pTabs->count(); i++) {
                    if (!m_pTabs->modifiedByIndex(i)) {
                        bool dontAskAgain = false;
                        CMessageOpts opts;
                        opts.checkBoxState = &dontAskAgain;
                        opts.chekBoxText = tr("Don't ask again.");
                        int res = CMessage::showMessage(this, tr("More than one document is open.<br>Close the window anyway?"),
                                                           MsgType::MSG_WARN, MsgBtns::mbYesNo, opts);
                        if (dontAskAgain)
                            reg_user.setValue("ignoreMsgAboutOpenTabs", true);
                        if (res != MODAL_RESULT_YES) {
                            AscAppManager::cancelClose();
                            return;
                        }
                        break;
                    }
                }
            }
        }

        for (int i(m_pTabs->count()); i-- > 0;) {
            if ( !m_pTabs->closedByIndex(i) ) {
                if ( !m_pTabs->isProcessed(i) ) {
                    int _result = trySaveDocument(i);
                    if ( _result == MODAL_RESULT_NO ) {
                        if (i == 0)
                            hide();
                        m_pTabs->editorCloseRequest(i);
                        onDocumentSave(m_pTabs->panel(i)->cef()->GetId());
                    } else
                    if ( _result == MODAL_RESULT_CANCEL ) {
                        AscAppManager::cancelClose();
                        return;
                    }
                } else {
                    if (i == 0)
                        hide();
                    m_pTabs->editorCloseRequest(i);
                }
            }

            PROCESSEVENTS();
        }
    }
}

//void CMainWindow::captureMouse(int tabindex)
//{
//    if (tabindex >= 0 && tabindex < tabWidget()->count()) {
//        QPoint spt = tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
//        QTimer::singleShot(0, this, [=] {
//            QMouseEvent event(QEvent::MouseButtonPress, spt, Qt::LeftButton, Qt::MouseButton::NoButton, Qt::NoModifier);
//            QCoreApplication::sendEvent((QWidget *)tabWidget()->tabBar(), &event);
//            tabWidget()->tabBar()->grabMouse();
//        });
//    }
//}

#ifdef __linux__
void CMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QSet<QString> _exts;
    _exts << "docx" << "doc" << "odt" << "rtf" << "txt" << "doct" << "dotx" << "ott";
#ifndef __LOCK_OFORM_FORMATS
    _exts << "docxf" << "oform";
#endif
    _exts << "html" << "mht" << "epub";
    _exts << "pptx" << "ppt" << "odp" << "ppsx" << "pptt" << "potx" << "otp";
    _exts << "xlsx" << "xls" << "ods" << "csv" << "xlst" << "xltx" << "ots";
    _exts << "pdf" << "djvu" << "xps";
    _exts << "plugin";

    QFileInfo oInfo(urls[0].toString());

    if (_exts.contains(oInfo.suffix()))
        event->acceptProposedAction();
}

void CMainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QString _path = urls[0].path();

    Utils::keepLastPath(LOCAL_PATH_OPEN, _path);
    COpenOptions opts = {"", etLocalFile, _path};
    opts.wurl = _path.toStdWString();

    std::wstring::size_type nPosPluginExt = opts.wurl.rfind(L".plugin");
    std::wstring::size_type nUrlLen = opts.wurl.length();
    if ((nPosPluginExt != std::wstring::npos) && ((nPosPluginExt + 7) == nUrlLen))
    {
        // register plugin
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN;
        NSEditorApi::CAscAddPlugin* pData = new NSEditorApi::CAscAddPlugin();
        pData->put_Path(opts.wurl);
        pEvent->m_pData = pData;

        AscAppManager::getInstance().Apply(pEvent);
    }
    else
    {
        doOpenLocalFile(opts);
    }
    event->acceptProposedAction();
}
#endif

/** MainPanel **/


QWidget* CMainWindow::createMainPanel(QWidget *parent)
{
    QWidget *mainPanel = new QWidget(parent);
    mainPanel->setObjectName("mainPanel");
    mainPanel->setProperty("rtl", AscAppManager::isRtlEnabled());
    mainPanel->setProperty("rtl-font", CLangater::isRtlLanguage(CLangater::getCurrentLangCode()));
#ifdef _WIN32
    mainPanel->setProperty("unix", false);
    if (Utils::getWinVersion() >= Utils::WinVer::Win10 && isCustomWindowStyle())
        mainPanel->setProperty("win10", true);
#else
    mainPanel->setProperty("unix", true);
#endif
    QGridLayout *_pMainGridLayout = new QGridLayout(mainPanel);
    _pMainGridLayout->setSpacing(0);
    _pMainGridLayout->setObjectName(QString::fromUtf8("mainGridLayout"));
    _pMainGridLayout->setContentsMargins(0, 0, 0, 0);
    mainPanel->setLayout(_pMainGridLayout);

    // Set custom TabBar
    CTabBar *pTabBar = new CTabBar(mainPanel);
    _pMainGridLayout->addWidget(pTabBar, 0, 1, 1, 1);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(1);
    pTabBar->setSizePolicy(sizePolicy);

    m_boxTitleBtns = createTopPanel(mainPanel);
    m_boxTitleBtns->setObjectName("CX11Caption");
    _pMainGridLayout->addWidget(m_boxTitleBtns, 0, 2, 1, 1);
    m_boxTitleBtns->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

#ifdef _WIN32
    if (Utils::getWinVersion() >= Utils::WinVer::Win10 && isCustomWindowStyle()) {
        foreach (auto *btn, m_pTopButtons) {
            btn->setProperty("win10", true);
        }
    }
#endif

#ifdef __DONT_WRITE_IN_APP_TITLE
    QLabel * label = new QLabel(m_boxTitleBtns);
#else
    QLabel * label = new QLabel(APP_TITLE, m_boxTitleBtns);
#endif
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    static_cast<QHBoxLayout*>(m_boxTitleBtns->layout())->insertWidget(1,label);

    // Main
    m_pButtonMain = new CSVGPushButton(mainPanel);
    m_pButtonMain->setObjectName( "toolButtonMain" );
    m_pButtonMain->setProperty("class", "active");
    m_pButtonMain->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    _pMainGridLayout->addWidget(m_pButtonMain, 0, 0, 1, 1);
    QObject::connect(m_pButtonMain, SIGNAL(clicked()), this, SLOT(pushButtonMainClicked()));

    QPalette palette;
    if (!isCustomWindowStyle()) {
//        m_pButtonMain->setProperty("theme", "light");
        QLinearGradient gradient(mainPanel->rect().topLeft(), QPoint(mainPanel->rect().left(), 29));
        gradient.setColorAt(0, QColor(0xeee));
        gradient.setColorAt(1, QColor(0xe4e4e4));
        palette.setBrush(QtComp::Palette::Background, QBrush(gradient));
        label->setFixedHeight(0);
    }

    // Set TabWidget
    _pMainGridLayout->addItem(new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0, 1, 1);
    m_pTabs = new CAscTabWidget(mainPanel, pTabBar);
    m_pTabs->setObjectName(QString::fromUtf8("ascTabWidget"));
    _pMainGridLayout->addWidget(m_pTabs, 1, 0, 1, 3);
    m_pTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pTabs->activate(false);
    m_pTabs->applyUITheme(GetCurrentTheme().id());

    connect(m_pTabs, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(pTabBar, SIGNAL(tabBarClicked(int)), this, SLOT(onTabClicked(int)), Qt::QueuedConnection);
    connect(pTabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequest(int)));
    connect(pTabBar, &CTabBar::tabMenuRequested, this, &CMainWindow::setTabMenu);
    connect(m_pTabs, &CAscTabWidget::editorInserted, bind(&CMainWindow::onTabsCountChanged, this, _2, _1, 1));
    connect(m_pTabs, &CAscTabWidget::editorRemoved, bind(&CMainWindow::onTabsCountChanged, this, _2, _1, -1));
    m_pTabs->setPalette(palette);
    m_pTabs->setCustomWindowParams(isCustomWindowStyle());
    return mainPanel;
}

void CMainWindow::attachStartPanel(QCefView * const view)
{
    m_pMainWidget = qobject_cast<QWidget *>(view);
#ifdef __linux
    view->setMouseTracking(m_pButtonMain->hasMouseTracking());
#endif
    m_pMainWidget->setParent(m_pMainPanel);
    QGridLayout *_pMainGridLayout = dynamic_cast<QGridLayout*>(m_pMainPanel->layout());
    Q_ASSERT(_pMainGridLayout != nullptr);
    if (_pMainGridLayout)
        _pMainGridLayout->addWidget(m_pMainWidget, 1, 0, 1, 3);
    m_pMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (!m_pTabs->isActiveWidget())
        m_pMainWidget->show();
}

#ifdef __linux
void CMainWindow::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
    m_pMainPanel->findChild<QLabel *>("labelAppTitle")->setMouseTracking(enable);

    m_boxTitleBtns->setMouseTracking(enable);
    m_pTabs->setMouseTracking(enable);
    m_pTabs->tabBar()->setMouseTracking(enable);
    m_pButtonMain->setMouseTracking(enable);
    if (m_pTopButtons[BtnType::Btn_Minimize]) {
        foreach (auto btn, m_pTopButtons)
            btn->setMouseTracking(enable);
    }
    if ( m_pMainWidget )
        m_pMainWidget->setMouseTracking(enable);
}
#endif

void CMainWindow::pushButtonMainClicked()
{
    if ( m_pTabs->isActiveWidget() ) {
        m_pTabs->activate(false);
        m_pMainWidget->setHidden(false);
        m_pTabs->setFocusedView();
        m_pButtonMain->setProperty("class", "active");
        ((QCefView *)m_pMainWidget)->setFocusToCef();
        onTabChanged(m_pTabs->currentIndex());
    }
}

void CMainWindow::toggleButtonMain(bool toggle, bool delay)
{
    auto _toggle = [=] (bool state) {
        if (m_pTabs->isActiveWidget() == state) {
            m_pButtonMain->setProperty("class", state ? "active" : "normal");
            m_pTabs->activate(!state);
            m_pMainWidget->setHidden(!state);
            if (!state)
                m_pTabs->setFocusedView();
            onTabChanged(m_pTabs->currentIndex());
        }
    };

    if ( delay ) {
        QTimer::singleShot(200, this, [=]{ _toggle(toggle); });
    } else {
        QTimer::singleShot(0, this, [=]{ _toggle(toggle); });
    }
}

void CMainWindow::onTabClicked(int index)
{
    Q_UNUSED(index)
    if (!m_pTabs->isActiveWidget()) {
        toggleButtonMain(false);
    }
}

void CMainWindow::onTabsCountChanged(int count, int i, int d)
{
    Q_UNUSED(i)
    Q_UNUSED(d)
    if ( count == 0 ) {
        toggleButtonMain(true);
    }
}

void CMainWindow::onEditorAllowedClose(int uid)
{
    if ( ((QCefView *)m_pMainWidget)->GetCefView()->GetId() == uid ) {
//        if ( m_pTabs->count() ) {
//            m_pMainWidget->setProperty("removed", true);
//        }
    } else {
        int _index = m_pTabs->tabIndexByView(uid);
        if ( !(_index < 0) ) {
            QWidget * _view = m_pTabs->widget(_index);
            m_pTabs->removeWidget(_view);
            _view->deleteLater();

            m_pTabs->tabBar()->removeTab(_index);
            //m_pTabs->adjustTabsSize();

            onTabChanged(m_pTabs->currentIndex());
            CInAppEventBase _event{CInAppEventBase::CEventType::etEditorClosed};
            AscAppManager::getInstance().commonEvents().signal(&_event);

            if ( !m_pTabs->count() ) {
                // m_pTabs->setProperty("empty", true);
                // m_pTabs->style()->polish(m_pTabs);
                // toggleButtonMain(true);

                if ( m_isCloseAll ) {
                    emit aboutToClose();
                }
            }
        }
    }
}

void CMainWindow::onTabChanged(int index)
{
    QString title("");
#ifdef __linux__
    if (WindowHelper::getEnvInfo() == WindowHelper::KDE)
        title = tr("Main Window");
#endif
    if (index > -1) {
        auto _panel = m_pTabs->panel(index);
        if (_panel)
            title = _panel->data()->title();
    } else {
        ((QCefView *)m_pMainWidget)->setFocusToCef();
    }

    if (title != windowTitle()) {
        QTimer::singleShot(100, this, [=]() {
            setWindowTitle(title);
        });
    }

#ifndef __DONT_WRITE_IN_APP_TITLE
    QLabel * title = (QLabel *)m_boxTitleBtns->layout()->itemAt(0)->widget();

    if (m_pTabs->isActive() && !(index < 0) && index < m_pTabs->count()) {
        QString docName = m_pTabs->titleByIndex(index, false);
        if (!docName.length())
            docName = m_pTabs->tabBar()->tabText(index);

        title->setText(QString(APP_TITLE) + " - " + docName);
    } else {
        title->setText(APP_TITLE);
    }
#endif
}

void CMainWindow::onTabCloseRequest(int index)
{
    onFullScreen(-1, false);
    if ( m_pTabs->isProcessed(index) ) {
        return;
    } else {
        if ( trySaveDocument(index) == MODAL_RESULT_NO ) {
            m_pTabs->editorCloseRequest(index);
            onDocumentSave(m_pTabs->panel(index)->cef()->GetId());
        }
    }
}

int CMainWindow::tabCloseRequest(int index)
{
    if ( m_pTabs->count() ) {
        if ( index == -1 ) {
            if ( !m_pTabs->closedByIndex(m_pTabs->currentIndex()) )
                index = m_pTabs->currentIndex();
            else {
                for (int i(0); i < m_pTabs->count(); ++i) {
                    if ( !m_pTabs->closedByIndex(i) ) {
                        index = i;
                        break;
                    }
                }
            }
        }
    }

    if ( !(index < 0) && index < m_pTabs->count() ) {
        onFullScreen(-1, false);
        if ( !m_pTabs->isProcessed(index) ) {
            int _result = trySaveDocument(index);
            if ( _result == MODAL_RESULT_NO ) {
                m_pTabs->editorCloseRequest(index);
                onDocumentSave(m_pTabs->panel(index)->cef()->GetId());
            }

            return _result;
        } else {
            m_pTabs->editorCloseRequest(index);
        }
    }

    return MODAL_RESULT_YES;
}

int CMainWindow::trySaveDocument(int index)
{
    if (m_pTabs->closedByIndex(index)) return MODAL_RESULT_YES;

    int modal_res = MODAL_RESULT_NO;
    if ( m_pTabs->modifiedByIndex(index) ) {
        toggleButtonMain(false);
        m_pTabs->setCurrentIndex(index);

        modal_res = CMessage::showMessage(this, getSaveMessage().arg(m_pTabs->titleByIndex(index).toHtmlEscaped()),
                                          MsgType::MSG_WARN, MsgBtns::mbYesDefNoCancel);
        switch (modal_res) {
        case MODAL_RESULT_NO: break;
        case MODAL_RESULT_CANCEL: modal_res = MODAL_RESULT_CANCEL; break;
        case MODAL_RESULT_YES:
        default:{
            m_pTabs->editorCloseRequest(index);
            m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));
            Utils::processMoreEvents(100);

            modal_res = MODAL_RESULT_YES;
            break;}
        }
    }

    return modal_res;
}

void CMainWindow::setTabMenu(int index, const QPoint &pos)
{
    CTabPanel *panel = m_pTabs->panel(index);
    CMenu *menu = new CMenu(m_pTabs->tabBar()->tabAtIndex(index));
    connect(menu, &CMenu::wasHidden, this, [=]() {
        m_pTabs->tabBar()->setTabMenu(index, nullptr);
    });
    QAction* actClose = menu->addSection(CMenu::ActionClose);
    connect(actClose, &QAction::triggered, this, [=]() {
            onTabCloseRequest(index);
            Utils::processMoreEvents();
        }, Qt::QueuedConnection);

    QAction* actCloseSaved = menu->addSection(CMenu::ActionCloseSaved);
    connect(actCloseSaved, &QAction::triggered, this, [=]() {
            for (int i(m_pTabs->count()); !(--i < 0);) {
                CAscTabData *doc = m_pTabs->panel(i)->data();
                if (doc->isViewType(cvwtEditor) && !doc->closed() && !doc->hasChanges() && !m_pTabs->panel(i)->hasUncommittedChanges() && !doc->url().empty()) {
                    onTabCloseRequest(i);
                    Utils::processMoreEvents();
                }
            }
        }, Qt::QueuedConnection);

    QAction* actCloseAll = menu->addSection(CMenu::ActionCloseAll);
    connect(actCloseAll, &QAction::triggered, this, [=]() {
            for (int i(m_pTabs->count()); !(--i < 0);) {
                CAscTabData *doc = m_pTabs->panel(i)->data();
                if (/*doc->isViewType(cvwtEditor) &&*/ !doc->closed()) {
                    onTabCloseRequest(i);
                    Utils::processMoreEvents();
                }
            }
        }, Qt::QueuedConnection);

    if (panel && panel->data()->isViewType(cvwtEditor)) {
        menu->addSeparator();
        QAction *actShowInFolder = menu->addSection(CMenu::ActionShowInFolder);
        actShowInFolder->setIcon(IconFactory::icon(IconFactory::Browse, SMALL_ICON * m_dpiRatio));
        actShowInFolder->setEnabled(panel->data()->isLocal() && !panel->data()->url().empty());
        connect(actShowInFolder, &QAction::triggered, this, [=]() {
                    Utils::openFileLocation(QString::fromStdWString(panel->data()->url()));
            }, Qt::QueuedConnection);
    }
    menu->addSeparator();

    QAction *actMoveToStart = menu->addSection(CMenu::ActionMoveToStart);
    // actUnpinTab->setIcon(IconFactory::icon(IconFactory::Unpin, SMALL_ICON * m_dpiRatio));
    connect(actMoveToStart, &QAction::triggered, this, [=]() {
            int destIndex = AscAppManager::isRtlEnabled() ? m_pTabs->count() - 1 : 0;
            if (m_pTabs->count() > 1 && index != destIndex)
                m_pTabs->tabBar()->moveTab(index, destIndex);
        }, Qt::QueuedConnection);

    QAction *actMoveToEnd = menu->addSection(CMenu::ActionMoveToEnd);
    // actUnpinTab->setIcon(IconFactory::icon(IconFactory::Unpin, SMALL_ICON * m_dpiRatio));
    connect(actMoveToEnd, &QAction::triggered, this, [=]() {
            int destIndex = AscAppManager::isRtlEnabled() ? 0 : m_pTabs->count() - 1;
            if (m_pTabs->count() > 1 && index != destIndex)
                m_pTabs->tabBar()->moveTab(index, destIndex);
        }, Qt::QueuedConnection);

    if (panel && panel->data()->isViewType(cvwtEditor)) {
        QAction *actUnpinTab = menu->addSection(CMenu::ActionUnpinTab);
        // actUnpinTab->setIcon(IconFactory::icon(IconFactory::Unpin, SMALL_ICON * m_dpiRatio));
        connect(actUnpinTab, &QAction::triggered, this, [=]() {
                    CTabUndockEvent event(index);
                    QObject *obj = qobject_cast<QObject*>(&AscAppManager::getInstance());
                    if (QApplication::sendEvent(obj, &event) && event.isAccepted()) {
                        QTimer::singleShot(0, this, [=]() {
                            QWidget *view = m_pTabs->widget(index);
                            m_pTabs->removeWidget(view);
                            view->deleteLater();
                            m_pTabs->tabBar()->removeTab(index);
                        });
                    }
            }, Qt::QueuedConnection);
        menu->addSeparator();

        QAction *actCreateNew = menu->addSection(CMenu::ActionCreateNew);
        actCreateNew->setIcon(IconFactory::icon(IconFactory::CreateNew, SMALL_ICON * m_dpiRatio));
        AscEditorType etype = panel->data()->contentType();
        actCreateNew->setEnabled(panel->isReady() && (etype == AscEditorType::etDocument || etype == AscEditorType::etPresentation ||
                                                      etype == AscEditorType::etSpreadsheet || etype == AscEditorType::etPdf /*||
                                                      etype == AscEditorType::etDraw*/));
        connect(actCreateNew, &QAction::triggered, this, [=]() {
                AscEditorType etype = panel->data()->contentType();
                std::wstring cmd = etype == AscEditorType::etDocument ? L"--new:word" :
                                       etype == AscEditorType::etPresentation ? L"--new:slide" :
                                       etype == AscEditorType::etSpreadsheet ? L"--new:cell" :
                                       // etype == AscEditorType::etDraw ? L"--new:draw" :
                                       etype == AscEditorType::etPdf ? L"--new:form" : L"";
                if (!cmd.empty())
                    AscAppManager::handleInputCmd({cmd});
            }, Qt::QueuedConnection);
    }
    m_pTabs->tabBar()->setTabMenu(index, menu);
    menu->exec(pos);
}

void CMainWindow::onPortalLogout(std::wstring wjson)
{
    const auto _is_url_starts_with = [](const QString& url, const std::vector<QString>& v) -> bool {
        for (auto& i: v) {
            if ( url.startsWith(i) )
                return true;
        }

        return false;
    };


    if ( m_pTabs->count() ) {
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(wjson).toUtf8(), &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            std::vector<QString> _portals{objRoot["domain"].toString()};

            if ( objRoot.contains("extra") && objRoot["extra"].isArray() ) {
                QJsonArray a = objRoot["extra"].toArray();
                for (auto&& v: a) {
                    _portals.push_back(v.toString());
                }
            }

            for (int i(m_pTabs->count()); !(--i < 0);) {
                int _answer = MODAL_RESULT_NO;

                CAscTabData& _doc = *m_pTabs->panel(i)->data();
                if ( _doc.isViewType(cvwtEditor) && !_doc.closed() &&
                        _is_url_starts_with(QString::fromStdWString(_doc.url()), _portals) )
                {
                    if ( _doc.hasChanges() || m_pTabs->panel(i)->hasUncommittedChanges() ) {
                        _answer = trySaveDocument(i);
                        if ( _answer == MODAL_RESULT_CANCEL) {
                            AscAppManager::cancelClose();
                            return;
                        }
                    }

                    if ( _answer != MODAL_RESULT_YES ) {
                        m_pTabs->editorCloseRequest(i);
                        onDocumentSave(m_pTabs->panel(i)->cef()->GetId());
                    }
                }
            }
        }
    }
}

void CMainWindow::onPortalLogin(int viewid, const std::wstring &json)
{
    if ( !(json.find(L"uiTheme") == std::wstring::npos) ) {
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(json).toLatin1(), &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            QJsonValue value = objRoot["uiTheme"];

            if ( value.isString() )
                onPortalUITheme(viewid, value.toString().toStdWString());
            else
            if ( value.isObject() )
                onPortalUITheme(viewid, QString(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact)).toStdWString());

            }
        }
    }

void CMainWindow::onPortalUITheme(int viewid, const std::wstring& json)
{
    if ( !json.empty() ) {
        int index = m_pTabs->tabIndexByView(viewid);
        if (index < 0 || m_pTabs->panel(index)->data()->isViewType(cvwtEditor))
            return;

        const QString id = QString::fromStdWString(json);
        CTheme tm = AscAppManager::themes().localFromId(id);
        if (tm.isValid()) {
            const QString color = QString::fromStdWString(tm.value(CTheme::ColorRole::ecrTabSimpleActiveBackground));
            m_pTabs->setTabTheme(index, tm.isDark() ? "dark" : "light", color);
        } else
        if ( json.rfind(L"default-", 0) == 0 ) {
            if ( json.compare(L"default-dark") == 0 )
                m_pTabs->setTabTheme(index, "dark", "#333");
            else m_pTabs->setTabTheme(index, "light", "#fff");
        } else {
            QJsonParseError jerror;
            QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(json).toUtf8(), &jerror);

            if( jerror.error == QJsonParseError::NoError ) {
                QJsonObject objRoot = jdoc.object();
                m_pTabs->setTabTheme(index, objRoot["type"].toString(), objRoot["color"].toString());
            }
        }
    }
}

void CMainWindow::doOpenLocalFile(COpenOptions& opts)
{
    QFileInfo info(opts.url);
    if (!info.exists()) { return; }
    if (!info.isFile()) { return; }
    if (!info.isReadable()) {
        QTimer::singleShot(0, this, [=] {
            CMessage::error(this, QObject::tr("Access to file '%1' is denied!").arg(opts.url));
        });
        return;
    }

    int result = m_pTabs->openLocalDocument(opts, true);
    if ( !(result < 0) ) {
        toggleButtonMain(false, true);
    } else
    if (result == -255) {
        QTimer::singleShot(0, this, [=] {
            CMessage::error(this, tr("File format not supported."));
        });
    }
    bringToTop();
}

void CMainWindow::onLocalFileRecent(void * d)
{
    CAscLocalOpenFileRecent_Recover * pData = static_cast<CAscLocalOpenFileRecent_Recover *>(d);
    COpenOptions opts = { pData->get_Path(),
          pData->get_IsRecover() ? etRecoveryFile : etRecentFile, pData->get_Id() };
    RELEASEINTERFACE(pData);
    onLocalFileRecent(opts);
}

void CMainWindow::onLocalFileRecent(const COpenOptions& opts)
{
    static QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(opts.url);

    bool forcenew = false;
    if ( !match.hasMatch() ) {
        QFileInfo _info(opts.url);
        if ( opts.srctype != etRecoveryFile && !_info.exists() ) {
            int modal_res = CMessage::showMessage(this, tr("%1 doesn't exists!<br>Remove file from the list?").arg(_info.fileName().toHtmlEscaped()),
                                                  MsgType::MSG_WARN, MsgBtns::mbYesDefNo);
            if (modal_res == MODAL_RESULT_YES) {
                AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "file:skip", QString::number(opts.id));
            }

            return;
        }
    } else forcenew = true;

//    openLocalFile(opts);
    int result = m_pTabs->openLocalDocument(opts, true, forcenew);
    if ( !(result < 0) ) {
        toggleButtonMain(false);
    } else
    if (result == -255) {
        CMessage::error(this, tr("File format not supported."));
    }
}

void CMainWindow::createLocalFile(const QString& name, int format)
{
    COpenOptions opts{name, etNewFile};
    opts.format = format;

    int tabIndex = m_pTabs->addEditor(opts);

    if ( !(tabIndex < 0) ) {
        m_pTabs->setCurrentIndex(tabIndex);

        toggleButtonMain(false, true);
    }
}

void CMainWindow::onLocalFilesOpen(void * data)
{
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    std::vector<std::wstring> vctFiles = pData->get_Files();

    doOpenLocalFiles(&vctFiles);

    RELEASEINTERFACE(pData);
}

void CMainWindow::onLocalFileLocation(QString path)
{
    QJsonObject objRoot = Utils::parseJsonString(path.toStdWString());
    if ( !objRoot.isEmpty() ) {
        QString _path = objRoot["path"].toString();
        int id = objRoot["id"].toInt();

        QFileInfo _info(_path);
        if ( _info.exists() ) {
            Utils::openFileLocation(_path);
        } else {
            int res = CMessage::showMessage(this, QObject::tr("%1 doesn't exists!<br>Remove file from the list?").arg(_info.fileName().toHtmlEscaped()),
                                                MsgType::MSG_WARN, MsgBtns::mbYesDefNo);
            if ( res == MODAL_RESULT_YES )
                AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "file:skip", QString::number(id));
            else
            if ( res == MODAL_RESULT_NO ) {
                int uid = objRoot["hash"].toInt();
                AscAppManager::getInstance().onFileChecked(_info.fileName(), uid, false);
            }
        }
    }
}

void CMainWindow::onFileLocation(int uid, QString param)
{
    if ( param == "offline" ) {
        QString path = m_pTabs->urlByView(uid);
        if ( !path.isEmpty() ) {
//            if ( Utils::isFileLocal(path) )
                Utils::openFileLocation(path);
//            else {
//            }
        } else {
            CMessage::info(this, tr("Document must be saved firstly."));
        }
    } else {
        static QRegularExpression _re("^((?:https?:\\/{2})?[^\\s\\/]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch _re_match = _re.match(param);

        if ( _re_match.hasMatch() ) {
            QString _domain = _re_match.captured(1);
            QString _folder = param;

            if ( !_folder.contains("desktop=true") ) {
                if ( _folder.contains("?") )
                    _folder.append("&desktop=true");
                else {
                    static QRegularExpression _re_dig("#\\d+");
                    int pos = _folder.indexOf(_re_dig);
                    !(pos < 0) ? _folder.insert(pos, "?desktop=true&") : _folder.append("?desktop=true");
                }
            }

            int _tab_index = m_pTabs->tabIndexByTitle(Utils::getPortalName(_domain), etPortal);
            if ( !(_tab_index < 0)) {
                ((CAscTabWidget *)m_pTabs)->updatePortal(_tab_index, _folder);
            } else {
                _tab_index = m_pTabs->addPortal(_folder, "", "");
            }

            if ( !(_tab_index < 0) ) {
                if (windowState().testFlag(Qt::WindowMinimized))
                    QMainWindow::setWindowState(Qt::WindowNoState);

                toggleButtonMain(false, true);
                m_pTabs->setCurrentIndex(_tab_index);
            }
        }
    }
}

void CMainWindow::doOpenLocalFiles(const std::vector<std::wstring> * vec)
{
    if (qApp->activeModalWidget()) return;

    for (const auto& wstr : (*vec)) {
        COpenOptions opts = {wstr, etLocalFile};
        doOpenLocalFile(opts);
    }

    if (vec->size())
        Utils::keepLastPath(LOCAL_PATH_OPEN,
                    QFileInfo(QString::fromStdWString(vec->back())).absolutePath());
}

void CMainWindow::doOpenLocalFiles(const QStringList& list)
{
    if (qApp->activeModalWidget()) return;

    QStringListIterator i(list);
    while (i.hasNext()) {
        QString n = i.next();
        if ( n.startsWith("--new:") ) {
        } else {
            COpenOptions opts = {n.toStdWString(), etLocalFile};
            doOpenLocalFile(opts);
        }
    }

    i.toBack();
    if (i.hasPrevious()) {
        Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(i.peekPrevious()).absolutePath());
    }
}

void CMainWindow::onDocumentType(int id, int type)
{
    m_pTabs->applyDocumentChanging(id, type);
}

void CMainWindow::onDocumentName(void * data)
{
    CAscDocumentName * pData = static_cast<CAscDocumentName *>(data);
    QString name = QString::fromStdWString(pData->get_Name());
    QString path = !pData->get_Url().empty() ? QString() : QString::fromStdWString(pData->get_Path());
    m_pTabs->applyDocumentChanging(pData->get_Id(), name, path);
    onTabChanged(m_pTabs->currentIndex());
    RELEASEINTERFACE(pData);
}

void CMainWindow::onEditorConfig(int, std::wstring cfg)
{
    Q_UNUSED(cfg)
}

void CMainWindow::onWebAppsFeatures(int id, std::wstring opts)
{
    m_pTabs->setEditorOptions(id, opts);
}

void CMainWindow::onDocumentReady(int uid)
{
    if ( uid < 0 ) {
        QTimer::singleShot(20, this, [=]{
            m_isStartPageReady = true;
            if ( !m_keepedAction.empty() ) {
                handleWindowAction(m_keepedAction);
                m_keepedAction.clear();
            }

            refreshAboutVersion();
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"app:ready");
            focus(); // TODO: move to app manager
        });
    } else {
        m_pTabs->applyPageLoadingStatus(uid, DOCUMENT_CHANGED_LOADING_FINISH);

        int index = m_pTabs->tabIndexByView(uid);
        if (CMenu *menu = m_pTabs->tabBar()->tabMenu(index)) {
            AscEditorType etype = m_pTabs->panel(index)->data()->contentType();
            if (etype == AscEditorType::etDocument || etype == AscEditorType::etPresentation ||
                    etype == AscEditorType::etSpreadsheet || etype == AscEditorType::etPdf /*|| etype == AscEditorType::etDraw*/) {
                menu->setSectionEnabled(CMenu::ActionCreateNew, true);
            }
        }
        if (m_pTabs->isActiveWidget())
            m_pTabs->setFocusedView();
    }
    AscAppManager::getInstance().onDocumentReady(uid);
}

void CMainWindow::onDocumentLoadFinished(int uid)
{
    m_pTabs->applyPageLoadingStatus(uid, DOCUMENT_CHANGED_PAGE_LOAD_FINISH);
}

void CMainWindow::onDocumentChanged(int id, bool changed)
{
    m_pTabs->applyDocumentChanging(id, changed);
    onTabChanged(m_pTabs->currentIndex());
}

void CMainWindow::onDocumentSave(int id, bool cancel)
{
    int _i = m_pTabs->tabIndexByView(id);
    if ( !(_i < 0) ) {
        if ( !cancel ) {
            if ( m_pTabs->closedByIndex(_i) &&
                    !m_pTabs->isProcessed(_i) &&
                        !m_pTabs->isFragmented(_i) )
                {
                    m_pTabs->closeEditorByIndex(_i);
                }
            else {
                if (CMenu *menu = m_pTabs->tabBar()->tabMenu(_i))
                    menu->setSectionEnabled(CMenu::ActionShowInFolder, true);
            }
        } else {
            m_pTabs->cancelDocumentSaving(_i);

//            AscAppManager::cancelClose();
        }
    }
}

void CMainWindow::onDocumentSaveInnerRequest(int id)
{
    int modal_res = CMessage::showMessage(this, tr("Document must be saved to continue.<br>Save the document?"),
                                          MsgType::MSG_CONFIRM, MsgBtns::mbYesDefNo);
    CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
    pData->put_Value(modal_res == MODAL_RESULT_YES);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
    pEvent->m_pData = pData;

    AscAppManager::getInstance().GetViewById(id)->Apply(pEvent);
}

void CMainWindow::onDocumentDownload(void * info)
{
    CAscDownloadFileInfo *pData = reinterpret_cast<CAscDownloadFileInfo*>(info);
    if (!m_pWidgetDownload && !pData->get_IsCanceled() && !pData->get_FilePath().empty()) {
        m_pWidgetDownload = new CDownloadWidget(this);
        connect(m_pWidgetDownload, &QWidget::destroyed, this, [=]() {
            m_pWidgetDownload = nullptr;
        });
        QHBoxLayout * layoutBtns = qobject_cast<QHBoxLayout *>(m_boxTitleBtns->layout());
        layoutBtns->insertWidget(1, m_pWidgetDownload->toolButton());
        m_pWidgetDownload->setLayoutDirection(AscAppManager::isRtlEnabled() ? Qt::RightToLeft : Qt::LeftToRight);
        m_pWidgetDownload->applyTheme();
        m_pWidgetDownload->updateScalingFactor(m_dpiRatio);
    }
    if (m_pWidgetDownload && !pData->get_FilePath().empty())
        m_pWidgetDownload->downloadProcess(info);
}

void CMainWindow::onDocumentFragmented(int id, bool isfragmented)
{
    int index = m_pTabs->tabIndexByView(id);
    if ( !(index < 0) ) {
            int _answer = MODAL_RESULT_NO;
            if ( isfragmented ) {
                static const bool _skip_user_warning = !InputArgs::contains(L"--warning-doc-fragmented");
                if ( _skip_user_warning ) {
                    m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
                    return;
                } else {
//                    CMessage mess(TOP_NATIVE_WINDOW_HANDLE);
//                    mess.setButtons(CMessageOpts::cmButtons::cmbYesDefNoCancel);
//                    _answer = mess.warning(tr("%1 must be built. Continue?").arg(m_pTabs->titleByIndex(index)));
//                    switch (_answer) {
//                    case MODAL_RESULT_CUSTOM + 0:
//                        m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
//                        return;
//                    case MODAL_RESULT_CUSTOM + 1: _answer == MODAL_RESULT_NO; break;
//                    case MODAL_RESULT_CUSTOM + 2:
//                    default:                      _answer = MODAL_RESULT_CANCEL; break;
//                    }
                }
            } else {
                onDocumentFragmentedBuild(id, 0);
            }

            if ( _answer == MODAL_RESULT_CANCEL ) {
                AscAppManager::cancelClose();
            }
    }
}

void CMainWindow::onDocumentFragmentedBuild(int vid, int error)
{
    int index = m_pTabs->tabIndexByView(vid);
    if ( error == 0 ) {
        m_pTabs->closeEditorByIndex(index, false);
    } else {
        m_pTabs->cancelDocumentSaving(index);
        AscAppManager::cancelClose();
    }
}

void CMainWindow::onEditorActionRequest(int vid, const QString& args)
{
    int index = m_pTabs->tabIndexByView(vid);
    if (!(index < 0)) {
        if (args.contains(QRegularExpression("action\\\":\\\"file:close"))) {
            bool _is_local = m_pTabs->isLocalByIndex(index);
            onTabCloseRequest(index);
            if (!_is_local) {
                QJsonParseError jerror;
                QJsonDocument jdoc = QJsonDocument::fromJson(args.toLatin1(), &jerror);
                if(jerror.error == QJsonParseError::NoError) {
                    QJsonObject objRoot = jdoc.object();
                    QString _url = objRoot["url"].toString();
                    if (!_url.isEmpty())
                        onFileLocation(vid, _url);
                    else _is_local = true;
                }
            }
        }
    }
}

void CMainWindow::goStart()
{
//    loadStartPage();
    toggleButtonMain(true);
}

void CMainWindow::onDocumentPrint(void * opts)
{
    Q_UNUSED(opts)
    static bool printInProcess = false;
    if (!printInProcess)
        printInProcess = true; else
        return;

    int viewId = AscAppManager::printData().viewId();
    int pagesCount = AscAppManager::printData().pagesCount(),
        currentPage = AscAppManager::printData().pageCurrent();
    CCefView * pView = AscAppManager::getInstance().GetViewById(viewId);
    QString documentName = m_pTabs->titleByIndex(m_pTabs->tabIndexByView(viewId), true);

    CEditorTools::onDocumentPrint(this, pView, documentName, currentPage, pagesCount);
    printInProcess = false;
}

void CMainWindow::onFullScreen(int id, bool apply)
{
    if (apply) {
        if (isVisible()) {
            m_isMaximized = windowState().testFlag(Qt::WindowMaximized);
            m_pTabs->setFullScreen(apply, id);
            QTimer::singleShot(0, this, [=] {
                CCefView* pView = AscAppManager::getInstance().GetViewById(id);
                if (pView)
                    pView->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER));
            });
        }
    } else
    if (isHidden()) {
        m_pTabs->setFullScreen(apply);
        toggleButtonMain(false);
        PROCESSEVENTS();
        focus();
    }
}

void CMainWindow::onKeyDown(void * eventData)
{
    NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)eventData;
    int key = pData->get_KeyCode();
    bool _is_ctrl = pData->get_IsCtrl();
    bool _is_shift = pData->get_IsShift();

    RELEASEINTERFACE(pData)
    switch (key) {
    case 'W':
    case VK_F4:
        if (_is_ctrl && m_pTabs->isActiveWidget()) {
            onTabCloseRequest(m_pTabs->currentIndex());
        }
        break;
    case VK_TAB:
        if (m_pTabs->count()) {
            if (_is_ctrl) {
                int _new_index = 0;
                if (_is_shift) {
                    if ( m_pTabs->isActiveWidget() )
                        _new_index = m_pTabs->currentIndex() - 1; else
                        _new_index = m_pTabs->count() - 1;
                } else {
                    if (m_pTabs->isActiveWidget())
                        _new_index = m_pTabs->currentIndex() + 1;
                }

                if (_new_index < 0 || !(_new_index < m_pTabs->count()))
                    toggleButtonMain(true);
                else {
                    toggleButtonMain(false);
                    m_pTabs->setCurrentIndex(_new_index);
                }
            }
        }
        break;
    case VK_F1:
        if ( _is_ctrl && _is_shift ) {
            GET_REGISTRY_USER(reg_user)

            CFileDialogWrapper _dlg(this);
            QString _dir = _dlg.selectFolder(reg_user.contains("helpUrl") ?
                        reg_user.value("helpUrl").toString() : Utils::lastPath(LOCAL_PATH_OPEN));

            if ( !_dir.isEmpty() ) {
                QString _path = _dir + "/apps/documenteditor/main/resources/help/en/Contents.json";
                if( QFileInfo::exists(_path) ) {
                    reg_user.setValue("helpUrl", _dir + "/apps");
                    EditorJSVariables::setVariable("helpUrl", _dir + "/apps");
                    EditorJSVariables::apply();

                    CMessage::error(this, "Successfully");
                } else {
                    CMessage::error(this, "Failed");
                }
            }
        }
        break;
    }
}

void CMainWindow::onPortalOpen(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);
    if (jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();
        QString _portal = objRoot["portal"].toString(),
                _entry = objRoot["entrypage"].toString();
        if (!_portal.isEmpty()) {
            int res = m_pTabs->openPortal(_portal, objRoot["provider"].toString("onlyoffice"), _entry);
            if (!(res < 0)) {
                toggleButtonMain(false, true);
                m_pTabs->setCurrentIndex(res);
            }

            QString _title = objRoot["title"].toString();
            if ( !_title.isEmpty() ) {
                m_pTabs->applyDocumentChanging(m_pTabs->viewByIndex(res), _title, "");
            }
        }
    }
}

void CMainWindow::onPortalNew(QString in)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(in.toLatin1(), &jerror);
    if (jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();
        QString _domain = objRoot["domain"].toString();
        QString _name = Utils::getPortalName(_domain);
        int _tab_index = m_pTabs->tabIndexByEditorType(etNewPortal);
        if (!(_tab_index < 0)) {
            int _uid = m_pTabs->viewByIndex(_tab_index);
            m_pTabs->applyDocumentChanging(_uid, _name, _domain);
            m_pTabs->applyDocumentChanging(_uid, int(etPortal));
            onTabChanged(m_pTabs->currentIndex());
        }
    }
}

void CMainWindow::onPortalCreate()
{
    QString _url = URL_SIGNUP;
    GET_REGISTRY_SYSTEM(reg_system)
    if (reg_system.contains("Store"))
            _url += "&store=" + reg_system.value("Store").toString();
    int res = m_pTabs->newPortal(_url, tr("Sign Up"));
    if (!(res < 0)) {
        toggleButtonMain(false, true);
        m_pTabs->setCurrentIndex(res);
    }
}

void CMainWindow::onOutsideAuth(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);
    if( jerror.error == QJsonParseError::NoError ) {
        QJsonObject objRoot = jdoc.object();
        QString _domain = objRoot["portal"].toString();
        int _tab_index = m_pTabs->tabIndexByTitle(Utils::getPortalName(_domain), etPortal);
        if (_tab_index < 0) {
            _tab_index = m_pTabs->addOAuthPortal(_domain,
                    objRoot["type"].toString(), objRoot["provider"].toString(),
                    objRoot["entrypage"].toString());
        }
        if (!(_tab_index < 0)) {
            m_pTabs->setCurrentIndex(_tab_index);
            toggleButtonMain(false, true);
        }
    }
}

void CMainWindow::onReporterMode(int id, bool status)
{
    if ( m_pTabs->fullScreenWidget() ) {
        CTabPanel * _widget = qobject_cast<CTabPanel *>(m_pTabs->fullScreenWidget());
        if (_widget && _widget->cef()->GetId() == id) {
            _widget->setReporterMode(status);
            return;
        }
    }

    int _i{m_pTabs->tabIndexByView(id)};
    if ( !(_i < 0) ) {
        m_pTabs->panel(_i)->setReporterMode(status);
    }
}

void CMainWindow::onErrorPage(int id, const std::wstring& action)
{
    CCefView * view = AscAppManager::getInstance().GetViewById(id);
    if ( view && cvwtEditor == view->GetType() && action.compare(L"open") == 0 ) {
        int ind = m_pTabs->tabIndexByView(id);
        m_pTabs->panel(ind)->data()->setHasError();
        m_pTabs->tabBar()->setTabLoading(ind, false);
    }
}

void CMainWindow::updateScalingFactor(double dpiratio)
{
    CScalingWrapper::updateScalingFactor(dpiratio);
    /*QLayout * layoutBtns = m_boxTitleBtns->layout();
    layoutBtns->setSpacing(int(1 * dpiratio));
    if (isCustomWindowStyle()) {
        layoutBtns->setContentsMargins(0,0,0,0);
        QSize small_btn_size(int(TITLEBTN_WIDTH*dpiratio), int(TOOLBTN_HEIGHT*dpiratio));
        foreach (auto btn, m_pTopButtons)
            btn->setFixedSize(small_btn_size);
    }*/
    m_pButtonMain->setFixedSize(int(BUTTON_MAIN_WIDTH * dpiratio), int(m_toolbtn_height * dpiratio));
    m_pMainPanel->setProperty("zoom", QString::number(dpiratio) + "x");
    QString tab_css = Utils::readStylesheets(":/styles/tabbar.qss");
    m_pTabs->tabBar()->setStyleSheet(tab_css.arg(GetColorQValueByRole(ecrWindowBackground),
                                                 GetColorQValueByRole(ecrButtonBackground),
                                                 GetColorQValueByRole(ecrButtonHoverBackground),
                                                 GetColorQValueByRole(ecrButtonPressedBackground),
                                                 GetColorQValueByRole(ecrTabDivider),
                                                 GetColorQValueByRole(ecrTabWordActive)));
    // m_pTabs->setStyleSheet(_style);
//    m_pTabs->updateScalingFactor(dpiratio);
    m_pTabs->reloadTabIcons();
    // m_pButtonMain->setIcon(MAIN_ICON_PATH, GetCurrentTheme().isDark() ? "logo-light" : "logo-dark");
    m_pButtonMain->setIconSize(MAIN_ICON_SIZE * dpiratio);
    if (m_pWidgetDownload && m_pWidgetDownload->toolButton()) {
        m_pWidgetDownload->updateScalingFactor(dpiratio);
        m_pWidgetDownload->toolButton()->style()->polish(m_pWidgetDownload->toolButton());
    }
    for (int i(m_pTabs->count()); !(--i < 0);) {
        if (CMenu *menu = m_pTabs->tabBar()->tabMenu(i)) {
            menu->setSectionIcon(CMenu::ActionCreateNew, IconFactory::icon(IconFactory::CreateNew, SMALL_ICON * m_dpiRatio));
            menu->setSectionIcon(CMenu::ActionShowInFolder, IconFactory::icon(IconFactory::Browse, SMALL_ICON * m_dpiRatio));
            // menu->setSectionIcon(CMenu::ActionUnpinTab, IconFactory::icon(IconFactory::Unpin, SMALL_ICON * m_dpiRatio));
        }
    }
}

void CMainWindow::setScreenScalingFactor(double factor, bool resize)
{
    CWindowPlatform::setScreenScalingFactor(factor, resize);
    QString css(AscAppManager::getWindowStylesheets(factor));
#ifdef __linux__
    css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
    if (!css.isEmpty()) {
        m_pMainPanel->setStyleSheet(css);
    }
    updateScalingFactor(factor);
    CScalingWrapper::updateChildScaling(m_pMainPanel, factor);
}

QString CMainWindow::getSaveMessage() const
{
    return tr("%1 is modified.<br>Do you want to keep changes?");
}

bool CMainWindow::holdUid(int uid) const
{
    CCefView * _view = (qobject_cast<QCefView *>(m_pMainWidget))->GetCefView();
    bool _res_out = _view->GetId() == uid;
    if (!_res_out) {
        CTabPanel * _widget = qobject_cast<CTabPanel *>(m_pTabs->fullScreenWidget());
        if (_widget) {
            _res_out = _widget->cef()->GetId() == uid;
        }
    }
    return _res_out ? true : !(m_pTabs->tabIndexByView(uid) < 0);
}

bool CMainWindow::holdUrl(const QString& url, AscEditorType type) const
{
    if (type == etPortal) {
        return !(m_pTabs->tabIndexByTitle(Utils::getPortalName(url), etPortal) < 0);
    } else
    if (type == etLocalFile) {
        return !(m_pTabs->tabIndexByUrl(url) < 0);
    }
    return false;
}

bool CMainWindow::slideshowHoldUrl(const QString &url, AscEditorType type) const
{
    if (type == etPortal) {
        return m_pTabs->slideshowHoldViewByTitle(Utils::getPortalName(url), etPortal);
    } else
    if (type == etLocalFile) {
        return m_pTabs->slideshowHoldViewByUrl(url);
    }
    return false;
}

int CMainWindow::startPanelId()
{
    return ((QCefView *)m_pMainWidget)->GetCefView()->GetId();
}

CAscTabWidget * CMainWindow::tabWidget()
{
    return m_pTabs;
}

void CMainWindow::showEvent(QShowEvent * e)
{
    CWindowPlatform::showEvent(e);
    cancelClose();
}

bool CMainWindow::isAboutToClose() const
{
    return m_isCloseAll;
}

void CMainWindow::cancelClose()
{
    m_isCloseAll && (m_isCloseAll = false);
}

QSize CMainWindow::contentSize()
{
    return m_pMainWidget ? m_pMainWidget->size() : QSize();
}

void CMainWindow::onLayoutDirectionChanged()
{
    m_pButtonMain->style()->polish(m_pButtonMain);
    if (m_pWidgetDownload && m_pWidgetDownload->toolButton()) {
        m_pWidgetDownload->onLayoutDirectionChanged();
        m_pWidgetDownload->toolButton()->style()->polish(m_pWidgetDownload->toolButton());
    }
}

#ifdef _WIN32
void CMainWindow::applyWindowState()
{
    if (Utils::getWinVersion() >= Utils::WinVer::Win10 && isCustomWindowStyle()) {
        m_toolbtn_height = isMaximized() ? TOOLBTN_HEIGHT : TOOLBTN_HEIGHT_WIN10;
        m_pMainPanel->setProperty("win10", !isMaximized());
        m_pMainPanel->style()->polish(m_pMainPanel);
        m_pButtonMain->style()->polish(m_pButtonMain);
        m_pButtonMain->setFixedHeight(int(m_toolbtn_height * m_dpiRatio));
        if (m_pWidgetDownload && m_pWidgetDownload->toolButton())
            m_pWidgetDownload->toolButton()->style()->polish(m_pWidgetDownload->toolButton());

        QString tab_css = Utils::readStylesheets(":/styles/tabbar.qss");
        m_pTabs->tabBar()->setStyleSheet(tab_css.arg(GetColorQValueByRole(ecrWindowBackground),
                                                     GetColorQValueByRole(ecrButtonBackground),
                                                     GetColorQValueByRole(ecrButtonHoverBackground),
                                                     GetColorQValueByRole(ecrButtonPressedBackground),
                                                     GetColorQValueByRole(ecrTabDivider),
                                                     GetColorQValueByRole(ecrTabWordActive)));

        foreach (auto *btn, m_pTopButtons) {
            btn->setFixedHeight(int(m_toolbtn_height * m_dpiRatio));
            btn->setProperty("win10", !isMaximized());
            btn->style()->polish(btn);
        }
    }
    CWindowBase::applyWindowState();
}
#endif

void CMainWindow::handleWindowAction(const std::wstring& action)
{
    if ( !m_isStartPageReady ) {
        m_keepedAction = action;
    } else {
        if ( action.rfind(L"panel|") == 0 ) {
            const std::wstring _panel_to_select = action.substr(std::wstring(L"panel|").size());

            if ( !_panel_to_select.empty() )
                AscAppManager::sendCommandTo(0, L"panel:select", _panel_to_select);
        }
    }
}
