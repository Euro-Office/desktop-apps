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

#include "components/asctabwidget.h"
#include <QHBoxLayout>
#include <QApplication>
#include "casctabdata.h"
#include "common/Types.h"
#include "defines.h"
#include "utils.h"
#include "cfilechecker.h"
#include "ceditortools.h"
#include "ctabundockevent.h"

#define indexIsValid(i) (!(i < 0) && i < count())

using namespace std;

/*
 *
 *  Tab data
 *
*/

//template <class T> class VPtr
//{
//public:
//    static T * asPtr(QVariant v) {return  (T *) v.value<void *>();}
//    static QVariant asQVariant(T * ptr){return qVariantFromValue((void *) ptr);}
//};


/*
 *
 * COpenOptions structure definition
 *
*/

COpenOptions::COpenOptions() :
    srctype(AscEditorType::etUndefined), id(-1)
{}

COpenOptions::COpenOptions(wstring _url_) :
    COpenOptions(_url_, AscEditorType::etUndefined, -1)
{}

COpenOptions::COpenOptions(wstring _url_, AscEditorType _srctype_) :
    COpenOptions(_url_, _srctype_, -1)
{}

COpenOptions::COpenOptions(wstring _url_, AscEditorType _srctype_, int _id_) :
    COpenOptions(QString(), _srctype_, QString::fromStdWString(_url_), _id_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_, QString _url_) :
    COpenOptions(_name_, _srctype_, _url_, -1)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_, QString _url_, int _id_) :
    name(_name_)
  , srctype(_srctype_)
  , url(Utils::replaceBackslash(_url_))
  , id(_id_)
  , wurl(url.toStdWString())
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_, std::wstring _url_, int _id_) :
    COpenOptions(_name_, _srctype_, QString::fromStdWString(_url_), _id_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_) :
    COpenOptions(_name_, _srctype_, "")
{}

/*
 *  TabWidget component
 *
*/

auto createTabPanel(QWidget * parent, CTabPanel * panel = nullptr) -> QWidget * {
    QWidget * panelwidget = new QWidget(parent);

    panelwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGridLayout *layout = new QGridLayout(panelwidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    panelwidget->setLayout(layout);
    if (!panel) {
        // workaround for determining the size of a CAscTabWidget that does not contain widgets
        CMainWindow *w = parent ? qobject_cast<CMainWindow*>(parent->topLevelWidget()) : nullptr;
        panel = new CTabPanel(nullptr, w ? w->contentSize() : QSize());
    }
    panelwidget->layout()->addWidget(panel);
    return panelwidget;
}

auto panelfromwidget(QWidget * panelwidget) -> CTabPanel * {
    return panelwidget->children().count() ? static_cast<CTabPanel *>(panelwidget->findChild<CTabPanel*>()) : nullptr;
}

CAscTabWidget::CAscTabWidget(QWidget *parent, CTabBar *_pBar)
    : QStackedWidget(parent)
    , CScalingWrapper(parent)
    , m_dataFullScreen(0)
    , m_widthParams({{100, 135, 9}, 68, 3, 0, WINDOW_TITLE_MIN_WIDTH, 140, 0})
    , m_defWidthParams(m_widthParams)
    , m_isCustomStyle(true)
    , m_isTabPinAllowed(true)
//    , m_tabIconSize(16, 16)
    , m_pBar(_pBar)
{
    m_pBar->setObjectName("asc_editors_tabbar");
    setProperty("active", false);
    // setProperty("empty", true);
    m_pBar->setProperty("active", false);

    static int _dropedindex = -1;
    QObject::connect(this, &CAscTabWidget::currentChanged, this, [=](int index) {
        setFocusedView();
        _dropedindex = -1;
    });
    QObject::connect(this, &CAscTabWidget::widgetRemoved, this, [=](int index) {
        emit editorRemoved(index, count());
    });
    QObject::connect(m_pBar, &CTabBar::tabUndock, this, [=](int index, bool &accept) {
        if (index == _dropedindex) return;

        const CTabPanel * _panel = panel(index);

        if ( _panel && _panel->data()->viewType() == cvwtEditor ) {
            CTabUndockEvent event(index);
            QObject * obj = qobject_cast<QObject*>(&AscAppManager::getInstance());
            if ( QApplication::sendEvent(obj, &event) && event.isAccepted() ) {
                _dropedindex = index;
                accept = true;
                m_isTabPinAllowed = false;

                QTimer::singleShot(0, this, [=]() {
                    if (widget(index)) {
                        widget(index)->deleteLater();
                    }
                });
            }
        }
    });
    auto turnOffAltHints = [=](int old_index, int index) {
        QTimer::singleShot(0, this, [=]() {
            setCurrentIndex(index);
        });
        if (old_index > -1 && panel(old_index))
            AscAppManager::sendCommandTo(panel(old_index)->cef(), L"althints:show", L"false");
    };
    QObject::connect(m_pBar, &CTabBar::tabBarClicked, this, [=](int index) {
        turnOffAltHints(m_pBar->currentIndex(), index);
    });
    QObject::connect(m_pBar, &CTabBar::onCurrentChangedByWhell, this, [=](int index) {
        turnOffAltHints(m_pBar->currentIndex(), index);
    });
    QObject::connect(m_pBar, &CTabBar::tabMoved, this, [=](int from, int to) {
        if (from < 0 || from >= count() || to < 0 || to >= count() || from == to)
            return;
        auto wgt = widget(from);
        blockSignals(true);
        removeWidget(wgt);
        insertWidget(to, wgt);
        if (from < m_pBar->currentIndex()) {
            if (to >= m_pBar->currentIndex()) {
                if (m_pBar->currentIndex() - 1 != currentIndex())
                    QStackedWidget::setCurrentIndex(m_pBar->currentIndex() - 1);
            }
        } else
        if (from == m_pBar->currentIndex()) {
            QStackedWidget::setCurrentIndex(to);
        } else {
            if (to <= m_pBar->currentIndex()) {
                if (m_pBar->currentIndex() + 1 != currentIndex())
                    QStackedWidget::setCurrentIndex(m_pBar->currentIndex() + 1);
            }
        }
        blockSignals(false);
    });
    QObject::connect(m_pBar, &CTabBar::tabsSwapped, this, [=](int from, int to) {
        if (from == to || !indexIsValid(from) || !indexIsValid(to))
            return;
        auto wgt_from = widget(from);
        auto wgt_to = widget(to);
        blockSignals(true);
        removeWidget(wgt_from);
        removeWidget(wgt_to);
        insertWidget(from < to ? from : to, from < to ? wgt_to : wgt_from);
        insertWidget(from < to ? to : from, from < to ? wgt_from : wgt_to);
        if (from == m_pBar->currentIndex())
            QStackedWidget::setCurrentIndex(to);
        else
        if (to == m_pBar->currentIndex())
            QStackedWidget::setCurrentIndex(from);
        blockSignals(false);
    });
}

CTabPanel * CAscTabWidget::panel(int index) const
{
    QWidget * _w = widget(index);
    return _w && _w->children().count() ? static_cast<CTabPanel *>(_w->findChild<CTabPanel*>()) : nullptr;
}

CTabBar *CAscTabWidget::tabBar() const
{
    return m_pBar;
}

int CAscTabWidget::addEditor(const COpenOptions& opts)
{
    if ( opts.url.isEmpty() && opts.srctype != etNewFile )
        return -1;

    // setProperty("empty", false);

    int file_format = 0;
    if (opts.srctype == etLocalFile) {
        file_format = CCefViewEditor::GetFileFormat(opts.wurl);
        if (file_format == 0)
            /* TODO: show error for file format */
            return -255;
    }

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);

    pView->initAsEditor();

    int tab_index = AscAppManager::isRtlEnabled() ? 0 : -1;
    bool res_open = true;
    if (opts.srctype == etLocalFile) {
        pView->openLocalFile(opts.wurl, file_format, L"");
    } else
    if (opts.srctype == etRecoveryFile) {
        res_open = pView->openRecoverFile(opts.id);
    } else
    if (opts.srctype == etRecentFile) {
        res_open = pView->openRecentFile(opts.id);
    } else
    if (opts.srctype == etNewFile) {
        pView->createLocalFile(CEditorTools::editorTypeFromFormat(opts.format), opts.name.toStdWString());
    } else {
        pView->cef()->load(opts.wurl);
    }

    if (res_open) {
        CAscTabData * data = new CAscTabData(opts.name);
        data->setUrl(opts.wurl);
        data->setIsLocal( opts.srctype == etLocalFile || opts.srctype == etNewFile ||
                       (opts.srctype == etRecentFile && !CExistanceController::isFileRemote(opts.url)) );

        data->setContentType(CEditorTools::editorTypeFromFormat(opts.format));
        data->setChanged(opts.srctype == etRecoveryFile);

        pView->setData(data);
        tab_index = insertWidget(tab_index, panelwidget);
        m_pBar->insertTab(tab_index, data->title());
        m_pBar->setTabToolTip(tab_index, data->title());
        m_pBar->setTabLoading(tab_index);

        //TODO: test for safe remove
//        applyDocumentChanging(id_view, opts.type);
    } else {
        tab_index = -255;
        RELEASEOBJECT(pView)
    }

    return tab_index;
}

void CAscTabWidget::closeEditor(int i, bool m, bool r)
{
    if (indexIsValid(i)) {
        CTabPanel * view = panel(i);
        CAscTabData * doc = view->data();

        if (doc && (!m || (!doc->hasChanges() && !view->hasUncommittedChanges()))) {
            doc->close();
            if (i == currentIndex()) {
                int last = count() - 1;
                if (i == last) {
                    if (last == 0) {
                        AscAppManager::getInstance().mainWindow()->toggleButtonMain(true);
                    } else {
                        QStackedWidget::setCurrentIndex(last - 1);
                    }
                } else {
                    QStackedWidget::setCurrentIndex(i + 1);
                }
            }
            AscAppManager::getInstance().DestroyCefView(view->cef()->GetId());

//            RELEASEOBJECT(view)

//            adjustTabsSize();
        }

//        if (r) {
//            setProperty("empty", tabBar()->count()==0);
//            style()->polish(this);
//        }
    }
}

void CAscTabWidget::closeEditorByIndex(int index, bool checkmodified)
{
    closeEditor(index, checkmodified, true);
}

int CAscTabWidget::count(int type) const
{
    if ( type < 0 )
        return QStackedWidget::count();
    else {
        int _out(0);
        for (int i(count()); i-- > 0; ) {
            if ( (panel(i))->data()->viewType() == type )
                ++_out;
        }
        return _out;
    }
}

int CAscTabWidget::count(const wstring& portal, bool exclude)
{
    if ( portal.empty() )
        return QStackedWidget::count();
    else {
        int _out(0);
        for (int i(count()); i-- > 0; ) {
            if ( panel(i)->data()->url().find(portal) != wstring::npos ) {
                if ( exclude && panel(i)->data()->isViewType(cvwtSimple) )
                    continue;

                ++_out;
            }
        }
        return _out;
    }
}

bool CAscTabWidget::hasForPortal(const QString& portal)
{
    const wstring _wsp = portal.toStdWString();
    for (int i(count()); i-- > 0; ) {
        if ( panel(i)->data()->isViewType(cvwtEditor) &&
                panel(i)->data()->url().find(_wsp) != wstring::npos )
        {
            return true;
        }
    }

    return false;
}

int CAscTabWidget::addPortal(const QString& url, const QString& name, const QString& provider, const QString& entrypage)
{
    if ( url.isEmpty() ) return -1;

    // setProperty("empty", false);

    QString args, _url = url;
    if ( provider == "onlyoffice" && !_url.contains(QRegularExpression("desktop=true")) )
        args.append("/?desktop=true");
    else {
        QRegularExpression _re("^((?:https?:\\/{2})?[^\\s\\?]+)(\\?[^\\s]+)?", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch _re_match = _re.match(url);

        if ( _re_match.hasMatch() ) {
            _url = _re_match.captured(1);
            args = _re_match.captured(2);
        }
    }

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);

    pView->initAsSimple();
    pView->cef()->SetExternalCloud(provider.toStdWString());
    pView->cef()->load((_url + entrypage + args).toStdWString());

    QString portal = name.isEmpty() ? Utils::getPortalName(url) : name;

    CAscTabData * data = new CAscTabData(portal, etPortal);
    data->setUrl(_url);
    pView->setData(data);

    int tab_index = AscAppManager::isRtlEnabled() ? 0 : -1;

    tab_index = insertWidget(tab_index, panelwidget);
    m_pBar->insertTab(tab_index, portal);
    m_pBar->setTabToolTip(tab_index, _url);
    m_pBar->setTabThemeType(tab_index, GetCurrentTheme().isDark() ? CTabBar::DarkTab : CTabBar::LightTab);
    m_pBar->setTabThemeIcons(tab_index, std::make_pair(":/tabbar/icons/portal.svg", ":/tabbar/icons/portal_light.svg"));
    m_pBar->setActiveTabColor(tab_index, QString::fromStdWString(GetColorValueByRole(ecrTabSimpleActiveBackground)));
    m_pBar->setTabLoading(tab_index);
//    updateTabIcon(tabIndexByView(id));

    return tab_index;
}

int CAscTabWidget::addOAuthPortal(const QString& portal, const QString& type, const QString& service, const QString& entrypage)
{
    if ( service.isEmpty() || !type.contains(QRegularExpression("sso|outer")) ) return -1;

    // setProperty("empty", false);

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);
    pView->initAsSimple();

    if ( type == "sso" ) {
        pView->cef()->load(("sso:" + service).toStdWString());
    } else
    if ( type == "outer" ) {
        pView->cef()->SetExternalCloud(service.toStdWString());

        QString _postfix;
        if (service == "onlyoffice") _postfix = "/?desktop=true";
        pView->cef()->load((portal + entrypage + _postfix).toStdWString());
    }

    QString _portal = portal.isEmpty() ? Utils::getPortalName(service) : Utils::getPortalName(portal);

    CAscTabData * data = new CAscTabData(_portal, etPortal);
    data->setUrl(portal);
    pView->setData(data);

    int tab_index = AscAppManager::isRtlEnabled() ? 0 : -1;

    tab_index = insertWidget(tab_index, panelwidget);
    m_pBar->insertTab(tab_index, _portal);
    m_pBar->setTabToolTip(tab_index, portal);
    m_pBar->setTabThemeType(tab_index, GetCurrentTheme().isDark() ? CTabBar::DarkTab : CTabBar::LightTab);
    m_pBar->setTabThemeIcons(tab_index, std::make_pair(":/tabbar/icons/portal.svg", ":/tabbar/icons/portal_light.svg"));
    m_pBar->setActiveTabColor(tab_index, QString::fromStdWString(GetColorValueByRole(ecrTabSimpleActiveBackground)));
//    m_pBar->tabStartLoading(tab_index);

    return tab_index;
}

int CAscTabWidget::insertPanel(QWidget * panel, int index)
{
    int tabindex = AscAppManager::isRtlEnabled() ? 0 : -1;

    CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
    Q_ASSERT(_panel != nullptr);
    if ( _panel ) {
        const CAscTabData * tabdata = _panel->data();

        QWidget * panelwidget = createTabPanel(this, _panel);

        if (index < 0 && AscAppManager::isRtlEnabled())
            index = 0;
        tabindex = insertWidget(index, panelwidget);
        m_pBar->insertTab(tabindex, tabdata->title());
        m_pBar->setTabToolTip(tabindex, tabdata->title());

        QString tabcolor = "none";
        const CTheme & ui_theme = AscAppManager::themes().current();
        const AscEditorType tab_type = tabdata->contentType();
        switch ( tab_type ) {
        case AscEditorType::etPresentation:
            tabcolor = QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSlideActive));
            break;
        case AscEditorType::etSpreadsheet:
            tabcolor =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabCellActive));
            break;
        case AscEditorType::etDocument:
            tabcolor =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabWordActive));
            break;
        case AscEditorType::etDocumentMasterForm:
        case AscEditorType::etPdf:
            tabcolor =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabViewerActive));
            break;
        case AscEditorType::etDraw:
            tabcolor =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabDrawActive));
            break;
        case etPortal:
            tabcolor =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSimpleActiveBackground));
            m_pBar->setTabThemeType(tabindex, /*ui_theme.isDark() ? CTabBar::DarkTab :*/ CTabBar::LightTab);
            break;
        default:
            tabcolor =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabDefaultActiveBackground));
            m_pBar->setTabThemeType(tabindex, ui_theme.isDark() ? CTabBar::DarkTab : CTabBar::LightTab);
        }

        switch ( tab_type ) {
        case AscEditorType::etPresentation:
        case AscEditorType::etSpreadsheet:
        case AscEditorType::etDocumentMasterForm:
        case AscEditorType::etPdf:
        case AscEditorType::etDocument:
        case AscEditorType::etDraw:
            m_pBar->setTabThemeType(tabindex,
                ui_theme.value(CTheme::ColorRole::ecrTabThemeType, L"dark") == L"dark" ? CTabBar::DarkTab : CTabBar::LightTab);
            break;
        default:
            if (!tabdata->isLocal())
                m_pBar->setTabLoading(tabindex);
            break;
        }

        m_pBar->setActiveTabColor(tabindex, tabcolor);

        const char *icon_name = tabindex == m_pBar->currentIndex() ? m_mapTabIcons.at(tab_type).second : m_mapTabIcons.at(tab_type).first;
        m_pBar->setTabIcon(tabindex, QIcon(icon_name));
    }

    return tabindex;
}

int CAscTabWidget::insertWidget(int index, QWidget* widget)
{
    int actual_index = QStackedWidget::insertWidget(index, widget);
    emit editorInserted(actual_index, count());
    return actual_index;
}

void CAscTabWidget::setCustomWindowParams(bool iscustom)
{
    m_isCustomStyle = iscustom;
    m_widthParams.tools_width = int((iscustom ? 50 : 0) * scaling());
    m_widthParams.title_width = int((iscustom ? WINDOW_TITLE_MIN_WIDTH : 0) * scaling());
}

void CAscTabWidget::setTabIcons(CTabIconSet& icons)
{
    m_mapTabIcons = icons;
}

void CAscTabWidget::reloadTabIcons()
{
    m_mapTabIcons.clear();
    const char *icons[] = {":/tabbar/icons/newdoc.svg", ":/tabbar/icons/de.svg", ":/tabbar/icons/pe.svg",
                           ":/tabbar/icons/pdf-form.svg",  ":/tabbar/icons/se.svg", ":/tabbar/icons/portal_light.svg",
                           ":/tabbar/icons/portal.svg", ":/tabbar/icons/pdf.svg", ":/tabbar/icons/ve.svg"};
    int portal_icon = GetCurrentTheme().isDark() ? 5 : 6;
    m_mapTabIcons.insert({
        {AscEditorType::etUndefined,          std::make_pair(icons[0], icons[0])},
        {AscEditorType::etDocument,           std::make_pair(icons[1], icons[1])},
        {AscEditorType::etPresentation,       std::make_pair(icons[2], icons[2])},
        {AscEditorType::etDocumentMasterForm, std::make_pair(icons[3], icons[3])},
        {AscEditorType::etSpreadsheet,        std::make_pair(icons[4], icons[4])},
        {AscEditorType::etPdf,                std::make_pair(icons[7], icons[7])},
        {AscEditorType::etDraw,               std::make_pair(icons[8], icons[8])},
        {etPortal,             std::make_pair(icons[portal_icon], icons[6])},
        {etNewPortal,          std::make_pair(icons[portal_icon], icons[6])}
    });
}

void CAscTabWidget::setTabActiveColor(int index, const std::wstring& color)
{
}

void CAscTabWidget::setTabTheme(int index, const QString& type, const QString& color)
{
    if ( !(index < 0) && index < count() ) {
        if ( !type.isEmpty() ) {
            m_pBar->setTabThemeType(index, type == "dark" ? CTabBar::DarkTab : CTabBar::LightTab);
        }

        if ( !color.isEmpty() ) {
            m_pBar->setActiveTabColor(index, color);

            if ( type.isEmpty() ) {
                m_pBar->setTabThemeType(index, CThemes::isColorDark(color) ? CTabBar::DarkTab : CTabBar::LightTab);
            }
        }
    }
}

/*
 *      Slots
*/

void CAscTabWidget::editorCloseRequest(int index)
{
    panel(index)->data()->close();
}

int CAscTabWidget::tabIndexByView(int viewId)
{
    for (int i(count()); i-- > 0; ) {
        if (panel(i) && panel(i)->cef()->GetId() == viewId)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, CefType vt)
{
    const CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->viewType() == vt && doc->title() == t)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, AscEditorType et)
{
    const CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        CTabPanel *cefpanel = panel(i);
        if (!cefpanel)
            continue;
        doc = cefpanel->data();

        if (doc && doc->contentType() == et)
            if (doc->title() == t ||
                    (et == etPortal && doc->title().startsWith(t)))
            {
                return i;
            }
    }

    return -1;
}

int CAscTabWidget::tabIndexByEditorType(AscEditorType et)
{
    const CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->contentType() == et)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByUrl(const QString& url)
{
    return tabIndexByUrl(url.toStdWString());
}

int CAscTabWidget::tabIndexByUrl(const wstring& url)
{
    CCefView * view = AscAppManager::getInstance().GetViewByUrl(url);
    if ( view ) {
        return tabIndexByView(view->GetId());
    } else {       
        const CAscTabData * doc;
        for (int i(count()); !(--i < 0);) {
            CTabPanel *cefpanel = panel(i);
            if (!cefpanel)
                continue;
            doc = cefpanel->data();

            if (doc && Utils::normalizeAppProtocolUrl(doc->url()).compare(Utils::normalizeAppProtocolUrl(url)) == 0)
                return i;
        }
    }

    return -1;
}

int CAscTabWidget::openCloudDocument(COpenOptions& opts, bool select, bool forcenew)
{
    int tabIndex{-1};
    if (opts.id > 0 && !forcenew) {
        tabIndex = tabIndexByView(opts.id);
        if (!(tabIndex < 0))
            m_pBar->setCurrentIndex(tabIndex);
    } else {
        opts.name   = tr("Document");
//        opts.type   = AscEditorType::etUndefined;
        tabIndex    = addEditor(opts);

        if (select && !(tabIndex < 0))
            m_pBar->setCurrentIndex(tabIndex);
    }

    return tabIndex;
}

int CAscTabWidget::openLocalDocument(const COpenOptions& options, bool select, bool forcenew)
{
    int tabIndex = -1;
    if ( !forcenew && options.srctype != etRecoveryFile ) {
        CCefView * view = AscAppManager::getInstance().GetViewByRecentId( options.id );
        tabIndex = view ? tabIndexByView(view->GetId()) : tabIndexByUrl(options.wurl);
    }

    if (tabIndex < 0){
        COpenOptions _opts{options};
        _opts.name = QFileInfo(options.url).fileName();
        tabIndex = addEditor(_opts);
    }

    if (select && !(tabIndex < 0))
        setCurrentIndex(tabIndex);

    /* TODO: rise message if index < 0 */

    return tabIndex;
}

int CAscTabWidget::openPortal(const QString& url, const QString& provider, const QString& entrypage)
{
    QString portal_name = Utils::getPortalName(url);

    int tabIndex = tabIndexByTitle(portal_name, etPortal);
    if (tabIndex < 0) {
        tabIndex = addPortal(url, "", provider, entrypage);
    } else {
//        updatePortal(tabIndex, url);
    }

    return tabIndex;
}

bool CAscTabWidget::updatePortal(int index,const QString& url)
{
    if ( !(index < 0) ) {
        CTabPanel * _panel = panel(index);

        if ( _panel->data()->contentType() == etPortal ) {
            _panel->cef()->load(url.isEmpty() ? _panel->data()->url() : url.toStdWString());
            return true;
        }
    }

    return false;
}

int CAscTabWidget::newPortal(const QString& url, const QString& name)
{
    int tabIndex = tabIndexByEditorType(etNewPortal);
    if ( tabIndex < 0 ) {
        if ( !((tabIndex = addPortal(url, name, "onlyoffice")) < 0) ) {
            panel(tabIndex)->data()->setContentType(etNewPortal);
        }
    }

    return tabIndex;
}

void CAscTabWidget::closePortal(const wstring& url, bool editors)
{
    closeEditorByIndex(tabIndexByUrl(url));

    if (editors) {
        const CAscTabData * doc;
        for (int i = m_pBar->count(); i-- > 0; ) {
            doc = panel(i)->data();

            if (doc->viewType() == cvwtEditor &&
                    doc->url().find(url) != wstring::npos)
            {
                closeEditor(i, false, false);
            }
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, const QString& name, const QString& path)
{
    int tabIndex = tabIndexByView(viewId);

    if (!(tabIndex < 0)) {
        CAscTabData * doc = panel(tabIndex)->data();
        doc->setTitle(name);
        if ( doc->isLocal() && !path.isEmpty() )
            doc->setUrl( Utils::replaceBackslash(path) );

        m_pBar->setTabText(tabIndex, doc->title());
        m_pBar->setTabToolTip(tabIndex, path.isEmpty() ? doc->title() : path);
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, bool state)
{
    int tabIndex = tabIndexByView(viewId);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = panel(tabIndex)->data();

        /* TODO: if exists the saving error, sdk rise the changing event
         * again. maybe not good action.
        */
        if (state && doc->closed()) doc->reuse();
        /**/

        if (doc->hasChanges() != state && (!doc->closed() || state)) {
            doc->setChanged(state);
            m_pBar->setTabText(tabIndex, doc->title());
            m_pBar->setTabToolTip(tabIndex, doc->title());
        }
    }
}

void CAscTabWidget::cancelDocumentSaving(int index)
{
    if ( !(index < 0) ) {
        CAscTabData * doc = panel(index)->data();

        if ( doc->closed() ) {
            doc->reuse();
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int id, int type)
{
    int tabIndex = tabIndexByView(id);
    if ( !(tabIndex < 0) ) {
        if ( AscEditorType(etPortal) == panel(tabIndex)->data()->contentType() )
            return;

        panel(tabIndex)->data()->setContentType(AscEditorType(type));

        const CTheme & ui_theme = AscAppManager::themes().current();
        switch (AscEditorType(type)) {
        case AscEditorType::etDocument:
            panel(tabIndex)->applyLoader("loader:style", "word");
            m_pBar->setActiveTabColor(tabIndex,
                QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabWordActive)));
            break;
        case AscEditorType::etSpreadsheet:
            panel(tabIndex)->applyLoader("loader:style", "cell");
            m_pBar->setActiveTabColor(tabIndex,
                QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabCellActive)));
            break;
        case AscEditorType::etPresentation:
            panel(tabIndex)->applyLoader("loader:style", "slide");
            m_pBar->setActiveTabColor(tabIndex,
                QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSlideActive)));
            break;
        case AscEditorType::etDocumentMasterForm:
        case AscEditorType::etPdf:
            panel(tabIndex)->applyLoader("loader:style", "pdf");
            m_pBar->setActiveTabColor(tabIndex,
                QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabViewerActive)));
            break;
        case AscEditorType::etDraw:
            panel(tabIndex)->applyLoader("loader:style", "draw");
            m_pBar->setActiveTabColor(tabIndex,
                QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabDrawActive)));
            break;
        default: break;
        }

        if (AscEditorType::etUndefined != AscEditorType(type) && !panel(tabIndex)->data()->isLocal())
            m_pBar->setTabLoading(tabIndex, false);

        m_pBar->setTabThemeType(tabIndex,
            ui_theme.value(CTheme::ColorRole::ecrTabThemeType, L"dark") == L"dark" ? CTabBar::DarkTab : CTabBar::LightTab);

        const char *icon_name = tabIndex == m_pBar->currentIndex() ?
                                    m_mapTabIcons.at(AscEditorType(type)).second : m_mapTabIcons.at(AscEditorType(type)).first;
        m_pBar->setTabIcon(tabIndex, QIcon(icon_name));
    }
}

void CAscTabWidget::applyPageLoadingStatus(int id, int state)
{
    int tabIndex = tabIndexByView(id);
    if ( !(tabIndex < 0) ) {
        if ( state == DOCUMENT_CHANGED_LOADING_START ) {
//            ((CTabBar *)tabBar())->setTabLoading(tabIndex, true);
        } else
        if ( state == DOCUMENT_CHANGED_LOADING_FINISH ) {
            m_pBar->setTabLoading(tabIndex, false);
            panel(tabIndex)->applyLoader("hide");
            panel(tabIndex)->setReady();
        } else
        if ( state == DOCUMENT_CHANGED_PAGE_LOAD_FINISH ) {
            if ( !panel(tabIndex)->data()->eventLoadSupported() ) {
                if (panel(tabIndex)->data()->isViewType(cvwtSimple))
                    m_pBar->setTabLoading(tabIndex, false);
                panel(tabIndex)->applyLoader("hide");
            }
        }
    }
}

void CAscTabWidget::setEditorOptions(int id, const wstring& option)
{
    int tabIndex = tabIndexByView(id);
    if ( !(tabIndex < 0) ) {
        CAscTabData * doc = panel(tabIndex)->data();

        doc->setFeatures(option);

        size_t _pos;
        if ((_pos = option.find(L"eventloading:")) != wstring::npos) {
            if (option.find(L"true", _pos + 1) != wstring::npos)
                doc->setEventLoadSupported(true);
        }

        if (option.find(L"readonly\":") != wstring::npos) {
            m_pBar->setTabText(tabIndex, doc->title());
            m_pBar->setTabToolTip(tabIndex, doc->title());
        }

//        if (std::regex_search(option, std::wregex(L"titlebuttons\":\\s?true"))) {
//            panel(tabIndex)->setWindowed(true);
//        }
    }
}

/*
 *      Slots description end
*/

void CAscTabWidget::setFocusedView(int index)
{
    if ( !isActiveWidget() )
    {
        if (!QCefView::IsSupportLayers())
        {
            if (this->currentWidget() && !this->currentWidget()->isHidden())
                this->currentWidget()->hide();
        }
        return;
    }
    int nIndex = !(index < 0) ? index : currentIndex();
    if (!(nIndex < 0 ))
    {
        if (!QCefView::IsSupportLayers())
        {
            if (this->currentWidget()->isHidden())
                this->currentWidget()->show();
        }

        if ( panel(nIndex) )
            panel(nIndex)->view()->setFocusToCef();
    }
}

void CAscTabWidget::activate(bool a)
{
    this->setHidden(!a);
    if (property("active").toBool() != a) {
        this->setProperty("active", a);
        m_pBar->activate(a);
    }
//    m_pBar->polish();
}

bool CAscTabWidget::isActiveWidget()
{
    return property("active").toBool();
}

bool CAscTabWidget::isTabPinAllowed()
{
    return m_isTabPinAllowed;
}

void CAscTabWidget::setTabPinAllowed()
{
    m_isTabPinAllowed = true;
}

int CAscTabWidget::modifiedCount()
{
    int mod_count = 0;
    const CAscTabData * doc;

    for (int i = m_pBar->count(); i-- > 0; ) {
        doc = panel(i)->data();
        (doc->hasChanges() || panel(i)->hasUncommittedChanges()) && mod_count++;
    }

    return mod_count;
}

int CAscTabWidget::viewByIndex(int index)
{
    if (indexIsValid(index)) {
        CCefView * view = panel(index)->cef();
        return view ? view->GetId() : -1;
    }

    return -1;
}

QString CAscTabWidget::titleByIndex(int index, bool mod)
{
    if (indexIsValid(index)) {
        const CAscTabData * doc = panel(index)->data();
        if (doc)
            return doc->title(mod);
    }

    return "";
}

QString CAscTabWidget::urlByView(int id)
{
    for (int i(count()); i-- > 0; ) {
        if (panel(i)->cef()->GetId() == id)
            return QString::fromStdWString(panel(i)->data()->url());
    }

    return "";
}

bool CAscTabWidget::modifiedByIndex(int index)
{
    if (indexIsValid(index)) {
        const CAscTabData * doc = panel(index)->data();
        return (doc->hasChanges() || panel(index)->hasUncommittedChanges()) && !doc->closed();
    }

    return false;
}

bool CAscTabWidget::isLocalByIndex(int index)
{
    return indexIsValid(index) ? panel(index)->data()->isLocal() : true;
}

bool CAscTabWidget::slideshowHoldView(int id) const
{
    CTabPanel *cefpanel = nullptr;
    if (m_dataFullScreen && (cefpanel = qobject_cast<CTabPanel*>(m_dataFullScreen->widget())) != nullptr) {
        if (cefpanel->cef()->GetId() == id)
            return true;
    }
    return false;
}

bool CAscTabWidget::slideshowHoldViewByTitle(const QString &title, AscEditorType type) const
{

    CTabPanel *cefpanel = nullptr;
    if (m_dataFullScreen && (cefpanel = qobject_cast<CTabPanel*>(m_dataFullScreen->widget())) != nullptr) {
        CAscTabData *doc = cefpanel->data();
        if (doc && doc->contentType() == type) {
            if (doc->title() == title || (type == etPortal && doc->title().contains(title)))
                return true;
        }
    }
    return false;
}

bool CAscTabWidget::slideshowHoldViewByUrl(const QString &url) const
{
    std::wstring _url = url.toStdWString();
    CCefView * view = AscAppManager::getInstance().GetViewByUrl(_url);
    if ( view ) {
        return slideshowHoldView(view->GetId());
    } else {
        CTabPanel *cefpanel = nullptr;
        if (m_dataFullScreen && (cefpanel = qobject_cast<CTabPanel*>(m_dataFullScreen->widget())) != nullptr) {
            CAscTabData *doc = cefpanel->data();
            if (doc && Utils::normalizeAppProtocolUrl(doc->url()).compare(Utils::normalizeAppProtocolUrl(_url)) == 0)
                return true;
        }
    }
    return false;
}

bool CAscTabWidget::closedByIndex(int index)
{
    return indexIsValid(index) ? panel(index)->data()->closed() : true;
}

MapEditors CAscTabWidget::modified(const QString& portalname)
{
    QMap<int, QString> mapModified;
    wstring portal = portalname.toStdWString();
    const CAscTabData * doc;
    for (int i(m_pBar->count()); i-- > 0; i++) {
        doc = panel(i)->data();

        if (doc->isViewType(cvwtEditor) &&
            (doc->hasChanges() || panel(i)->hasUncommittedChanges()) && !doc->closed() &&
                (portal.empty() || doc->url().find(portal) != wstring::npos))
        {
            mapModified.insert(viewByIndex(i), titleByIndex(i, true));
        }
    }

    return mapModified;
}

int CAscTabWidget::findModified(const QString& portalname)
{
    wstring portal = portalname.toStdWString();
    const CAscTabData * doc;
    for (int i(m_pBar->count()); i-- > 0; ) {
        doc = panel(i)->data();

        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                (portal.empty() || doc->url().find(portal) != wstring::npos) )
        {
            if ( doc->hasChanges() || panel(i)->hasUncommittedChanges() ) {
                return i;
            }
        }
    }

    return -1;
}

int CAscTabWidget::findFragmented(const QString& portalname)
{
    wstring portal = portalname.toStdWString();
    const CAscTabData * doc;
    const CTabPanel * cefpanel;
    for (int i(m_pBar->count()); i-- > 0; ) {
        cefpanel = panel(i);
        doc = cefpanel->data();
        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                (portal.empty() || doc->url().find(portal) != wstring::npos) )
        {
            if ( ((CCefViewEditor*)cefpanel->cef())->CheckCloudCryptoNeedBuild() ) {
                return i;
            }
        }
    }
    return -1;
}

bool CAscTabWidget::isFragmented(int index)
{
    if (indexIsValid(index)) {
        const CTabPanel * cefpanel = panel(index);
        const CAscTabData * doc = cefpanel->data();
        return /*!doc->closed() &&*/ doc->isViewType(cvwtEditor) && ((CCefViewEditor *)cefpanel->cef())->CheckCloudCryptoNeedBuild();
    }
    return false;
}

int CAscTabWidget::findProcessed() const
{
    const CAscTabData * doc;
    const CTabPanel * cefpanel;
    for (int i(count()); i-- > 0; ) {
        cefpanel = panel(i);
        doc = cefpanel->data();
        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                ((CCefViewEditor *)cefpanel->cef())->IsBuilding() )
        {
            return i;
        }
    }

    return -1;
}

bool CAscTabWidget::isProcessed(int index) const
{
    if (indexIsValid(index)) {
        const CTabPanel * cefpanel = panel(index);
        const CAscTabData * doc = cefpanel->data();

        return /*!doc->closed() &&*/ doc->isViewType(cvwtEditor) && ((CCefViewEditor *)cefpanel->cef())->IsBuilding();
    }

    return false;
}

void CAscTabWidget::setFullScreen(bool apply, int id)
{
    CTabPanel * fsWidget;
    static QMetaObject::Connection cefConnection;
    if (!apply) {
        if (m_dataFullScreen) {
            disconnect(cefConnection);

            AscAppManager::mainWindow()->show(false);
            int index = m_dataFullScreen->tabindex();
            fsWidget = qobject_cast<CTabPanel *>(m_dataFullScreen->widget());
            widget(index)->layout()->addWidget(fsWidget);

			// TODO: remove after switching to libVLC libraries on Linux
#ifdef _LINUX
			QCefView* cef_media_view = this->findChild<QCefView*>();
			if (cef_media_view)
			{
				cef_media_view->OnMediaEnd();
			}
#endif

            RELEASEOBJECT(m_dataFullScreen->parent)
            RELEASEOBJECT(m_dataFullScreen)
//            updateGeometry();
        }
    } else {
        int tabIndex = tabIndexByView(id);
        if ( tabIndex < 0 )
            tabIndex = currentIndex();
        else
        if ( tabIndex != currentIndex() ) {
            setCurrentIndex(tabIndex);
        }

        fsWidget = panel(tabIndex);

        if ( fsWidget ) {
            m_dataFullScreen = new CFullScreenData(tabIndex, fsWidget);

            m_dataFullScreen->parent = WindowHelper::constructFullscreenWidget(fsWidget);
            fsWidget->view()->setFocusToCef();
            AscAppManager::mainWindow()->hide();

            auto _break_demonstration = [=]() {
                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_Command(L"editor:stopDemonstration");
                NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                pEvent->m_pData = pCommand;
                fsWidget->cef()->Apply(pEvent);
            };

            cefConnection = connect(fsWidget, &CTabPanel::closePanel, [=](QCloseEvent * e){
                _break_demonstration();
                e->ignore();
                // TODO: associate panel with reporter window and close both simultaneously
                QTimer::singleShot(10, [=] {emit m_pBar->tabCloseRequested(m_dataFullScreen->tabindex());});
            });

            connect((CFullScrWidget*)m_dataFullScreen->parent, &CFullScrWidget::closeRequest, this, [=]() {
                disconnect((CFullScrWidget*)m_dataFullScreen->parent);
                _break_demonstration();
            });
        }
    }
}

QWidget * CAscTabWidget::fullScreenWidget()
{
    return m_dataFullScreen ? m_dataFullScreen->widget() : nullptr;
}

void CAscTabWidget::setStyleSheet(const QString& stylesheet)
{
    QStackedWidget::setStyleSheet(stylesheet);

//    auto _string_to_color = [](const QString& str) -> QColor {
//        int r = -1, g = -1, b = -1;
//        QRegExp re("^#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})$", Qt::CaseInsensitive);
//        if ( re.indexIn(str) < 0 ) {
//            re.setPattern("^#([a-f0-9])([a-f0-9])([a-f0-9])$");

//            if ( !(re.indexIn(str) < 0) ) {
//                r = (re.cap(1)+re.cap(1)).toInt(nullptr, 16),
//                g = (re.cap(2)+re.cap(2)).toInt(nullptr, 16),
//                b = (re.cap(3)+re.cap(3)).toInt(nullptr, 16);
//            }
//        } else {
//            r = re.cap(1).toInt(nullptr, 16),
//            g = re.cap(2).toInt(nullptr, 16),
//            b = re.cap(3).toInt(nullptr, 16);
//        }

//        if ( r < 0 || g < 0 || b < 0 )
//            return QColor();
//        else return {r,g,b};
//    };

//    QRegExp r("QTabBar::tab-label\\s?\\{\\s?active:\\s?([^;]{4,7});normal:\\s?([^;]{4,7})");
//    if (!(r.indexIn(stylesheet) < 0)) {
//        ((CTabBar *)tabBar())->setTabTextColor(QPalette::Active, _string_to_color(r.cap(1)) );
//        ((CTabBar *)tabBar())->setTabTextColor(QPalette::Inactive, _string_to_color(r.cap(2)) );
//    }

//    r.setPattern("QTabBar::tab-icon\\s*\\{([^\\}]+)");
//    if ( !(r.indexIn(stylesheet) < 0) ) {
//        QRegExp ri("width:\\s*(\\d+);\\s*height:\\s*(\\d+)");

//        if ( !(ri.indexIn(r.cap(1)) < 0) ) {
//            m_tabIconSize.setWidth(ri.cap(1).toInt());
//            m_tabIconSize.setHeight(ri.cap(2).toInt());
//        }
//    }
}

void CAscTabWidget::setCurrentIndex(int index)
{
    QStackedWidget::setCurrentIndex(index);
    m_pBar->setCurrentIndex(index);
}

void CAscTabWidget::applyUITheme(const std::wstring& theme)
{
    reloadTabIcons();
//    m_pBar->setIgnoreActiveTabColor(GetCurrentTheme().isDark());
//    m_pBar->polish();

    const CTheme & ui_theme = AscAppManager::themes().current();
    std::vector<QString> tab_color{QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabWordActive)),
                                      QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabCellActive)),
                                      QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSlideActive))};
    QColor back_color = ui_theme.color(CTheme::ColorRole::ecrWindowBackground);
    const auto tab_theme = ui_theme.value(CTheme::ColorRole::ecrTabThemeType, L"dark") == L"dark" ? CTabBar::DarkTab : CTabBar::LightTab;
    for (int i(count()); i-- > 0; ) {
        panel(i)->setBackground(back_color);

        const auto tab_type = panel(i)->data()->contentType();
        switch ( tab_type ) {
        case AscEditorType::etPresentation:
            m_pBar->setActiveTabColor(i, tab_color.at(2));
            break;
        case AscEditorType::etSpreadsheet:
            m_pBar->setActiveTabColor(i, tab_color.at(1));
            break;
        case AscEditorType::etDocument:
            m_pBar->setActiveTabColor(i, tab_color.at(0));
            break;
        case AscEditorType::etDocumentMasterForm:
        case AscEditorType::etPdf:
            m_pBar->setActiveTabColor(i, QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabViewerActive)));
            break;
        case AscEditorType::etDraw:
            m_pBar->setActiveTabColor(i, QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabDrawActive)));
            break;
        case etPortal:
            m_pBar->setTabThemeType(i, ui_theme.isDark() ? CTabBar::DarkTab : CTabBar::LightTab);
            m_pBar->setActiveTabColor(i, QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSimpleActiveBackground)));
            break;
        default:
            m_pBar->setTabThemeType(i, ui_theme.isDark() ? CTabBar::DarkTab : CTabBar::LightTab);
            m_pBar->setActiveTabColor(i, QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabDefaultActiveBackground)));
            break;
        }

        switch ( tab_type ) {
        case AscEditorType::etPresentation:
        case AscEditorType::etSpreadsheet:
        case AscEditorType::etDocumentMasterForm:
        case AscEditorType::etPdf:
        case AscEditorType::etDocument:
        case AscEditorType::etDraw:
            m_pBar->setTabThemeType(i, tab_theme);
            break;
        default: break;
        }
    }

    m_pBar->refreshTheme();
    style()->polish(this);
}
