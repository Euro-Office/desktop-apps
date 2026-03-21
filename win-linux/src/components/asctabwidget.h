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

#ifndef ASCTABWIDGET
#define ASCTABWIDGET

#include <QStackedWidget>
#include "ctabbar.h"
#include "cscalingwrapper.h"
#include "ctabpanel.h"

#define etLocalFile     AscEditorType(254)
#define etRecoveryFile  AscEditorType(253)
#define etRecentFile    AscEditorType(252)
#define etNewFile       AscEditorType(251)
#define etPortal        AscEditorType(250)
#define etNewPortal     AscEditorType(249)
#define etTemplateFile  AscEditorType(248)

#define DOCUMENT_CHANGED_LOADING_START          -255
#define DOCUMENT_CHANGED_LOADING_FINISH         -254
#define DOCUMENT_CHANGED_PAGE_LOAD_FINISH       -253

typedef CefViewWrapperType CefType;
typedef QMap<int, QString> MapEditors;


struct COpenOptions {
    COpenOptions();
    COpenOptions(std::wstring _url_);
    COpenOptions(std::wstring _url_, AscEditorType _type_);
    COpenOptions(std::wstring _url_, AscEditorType _type_, int _id_);
    COpenOptions(QString _name_, AscEditorType _type_);
    COpenOptions(QString _name_, AscEditorType _type_, QString _url_);
    COpenOptions(QString _name_, AscEditorType _type_, QString _url_, int _id_);
    COpenOptions(QString _name_, AscEditorType _type_, std::wstring _url_, int _id_);

    QString name;
    AscEditorType srctype;
    QString url;
    int id, format = 0;
    std::wstring wurl;
    int parent_id = -1;
    QSize panel_size;

    enum class eOpenMode {edit, view, review};
    eOpenMode mode = eOpenMode::edit;

    enum class eWidgetType {window, tab};
    eWidgetType parent_widget = eWidgetType::tab;
    QString cloud;
};

class CAscTabWidget : public QStackedWidget, public CScalingWrapper
{
    Q_OBJECT

    struct CFullScreenData {
    public:
        CFullScreenData(int i, QWidget * w) : _widget(w), _index(i) {}
        CFullScreenData() : CFullScreenData(-1, nullptr) {}

        QWidget * widget() { return _widget; }
        int tabindex() { return _index; }
        QWidget * parent = nullptr;
    private:
        QWidget * _widget;
        int _index;
    };

    struct size_params {
        struct tab_bounds {
            int min, max, cst;
        } tab;

        int main_button_width;
        int main_button_span;
        int tabs_span;
        int title_width;
        int tools_width;
        int custom_offset;

        void apply_scale(double dpi) {
            tab.max     *= dpi;
            tab.min     *= dpi;
            tabs_span   *= dpi;
            title_width *= dpi;
            tools_width *= dpi;
            main_button_width *= dpi;
            main_button_span  *= dpi;
        }
    };

    typedef std::map< AscEditorType, std::pair<const char*, const char*> > CTabIconSet;

private:
    std::map<int, QCefView*> m_mapDownloads;
    CFullScreenData * m_dataFullScreen;
    size_params m_widthParams,
                m_defWidthParams;
    bool m_isCustomStyle,
         m_isTabPinAllowed;
    CTabIconSet m_mapTabIcons;
//    QSize m_tabIconSize;
    CTabBar *m_pBar;

signals:
//    void sendAddEditor();
    void editorInserted(int, int);
    void editorRemoved(int, int);

public:
    CAscTabWidget(QWidget *parent = nullptr, CTabBar *_pBar = nullptr);

    CTabPanel * panel(int) const;
    CTabBar *tabBar() const;

//    int  addEditor(QString strName, AscEditorType etType = etDocument, std::wstring strUrl = L"");
    int  addEditor(const COpenOptions&);
    int  addPortal(const QString& url, const QString& name, const QString& provider, const QString& entrypage = QString());
    int  addOAuthPortal(const QString& portal, const QString& type, const QString& service, const QString& entrypage = QString());
    int  insertPanel(QWidget *, int);
    void closeEditorByIndex(int index, bool checkmodified = false);
    void closePortal(const std::wstring&, bool editors = false);
    void setStyleSheet(const QString&);
    void setCurrentIndex(int);
    void applyUITheme(const std::wstring&);

    using QStackedWidget::count;
    int  count(int type) const;
    int  count(const std::wstring& portal, bool exclude = false);
    bool hasForPortal(const QString&);

protected:
    int insertWidget(int index, QWidget* widget);
    void closeEditor(int, bool, bool);

public:
    int         tabIndexByView(int);
    int         viewByIndex(int);
    int         tabIndexByView(QString);
    int         tabIndexByTitle(QString t, CefType vt);
    int         tabIndexByTitle(QString t, AscEditorType vt);
    int         tabIndexByUrl(const QString&);
    int         tabIndexByUrl(const std::wstring&);
    int         tabIndexByEditorType(AscEditorType vt);
    QString     titleByIndex(int, bool original = true);
    QString     urlByView(int id);
    bool        modifiedByIndex(int);
    bool        isLocalByIndex(int);
    bool        slideshowHoldView(int id) const;
    bool        slideshowHoldViewByTitle(const QString&, AscEditorType) const;
    bool        slideshowHoldViewByUrl(const QString&) const;
    int         modifiedCount();
    bool        closedByIndex(int);
    void        editorCloseRequest(int);

    MapEditors  modified(const QString& portal);
    int         findModified(const QString& portal = QString());
    int         findFragmented(const QString& portal = QString());
    bool        isFragmented(int index);
    int         findProcessed() const;
    bool        isProcessed(int index) const;

    void activate(bool);
    bool isActiveWidget();
    bool isTabPinAllowed();
    void setTabPinAllowed();

    void setTabIcons(CTabIconSet&);
    void reloadTabIcons();
    void setTabActiveColor(int index, const std::wstring& color);
    void setTabTheme(int index, const QString& type, const QString& color);
    void setFocusedView(int index = -1);
    void setFullScreen(bool, int id = -1);
    QWidget * fullScreenWidget();

    int  openCloudDocument(COpenOptions&, bool, bool forcenew = false);
    int  openLocalDocument(const COpenOptions&, bool, bool forcenew = false);
//    void changeDocumentType(int, int);
    void applyDocumentChanging(int id, int type);
    void applyDocumentChanging(int id, const QString& name, const QString& descr);
    void applyDocumentChanging(int id, bool iscontentchanged);
    void applyPageLoadingStatus(int id, int state);
    void setCustomWindowParams(bool iscustom);
    void cancelDocumentSaving(int index);
    void setEditorOptions(int, const std::wstring&);

    int  openPortal(const QString& url, const QString& provider, const QString& entrypage);
    bool updatePortal(int index, const QString& url = QString());
    int  newPortal(const QString& url, const QString& name);

public slots:
//    void onDocumentNameChanged(int, QString);
//    void onDocumentChanged(int, bool);
};

#endif // ASCTABWIDGET
