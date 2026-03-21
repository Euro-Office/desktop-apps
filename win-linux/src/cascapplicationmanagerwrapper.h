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

#ifndef CASCAPPLICATIONMANAGERWRAPPER
#define CASCAPPLICATIONMANAGERWRAPPER

#include "qascapplicationmanager.h"
#include <QObject>
#include <QMutex>
#include <QDesktopServices>
#include <vector>
#include <memory>
#include "ccefeventstransformer.h"
#include "ccefeventsgate.h"
#include "windows/ceditorwindow.h"
#include "cwindowsqueue.h"
#include "ceventdriver.h"
#include "cprintdata.h"

#include "cmainwindowimpl.h"
#include "windows/cpresenterwindow.h"

#ifdef _UPDMODULE
    #include "cupdatemanager.h"
#endif

#include "cthemes.h"

#define SEND_TO_ALL_START_PAGE nullptr

typedef QWidget* ParentHandle;

#define CLOSE_QUEUE_WIN_TYPE_MAIN   1
#define CLOSE_QUEUE_WIN_TYPE_EDITOR 2


struct sWinTag
{
    int     type;
    size_t  handle;

    bool operator==(const sWinTag& other) const
    {
        return other.handle == this->handle;
    }
};

Q_DECLARE_METATYPE(sWinTag)
#if defined(_WIN32) && !defined(QT_VERSION_6)
Q_DECLARE_METATYPE(std::vector<std::wstring>)
#endif

enum class CScalingFactor
{
    SCALING_FACTOR_1,
    SCALING_FACTOR_1_25,
    SCALING_FACTOR_1_5,
    SCALING_FACTOR_1_75,
    SCALING_FACTOR_2,
    SCALING_FACTOR_2_25,
    SCALING_FACTOR_2_5,
    SCALING_FACTOR_2_75,
    SCALING_FACTOR_3,
    SCALING_FACTOR_3_5,
    SCALING_FACTOR_4,
    SCALING_FACTOR_4_5,
    SCALING_FACTOR_5
};

class CAscApplicationManagerWrapper;
class CAscApplicationManagerWrapper_Private;
typedef CAscApplicationManagerWrapper AscAppManager;

class CAscApplicationManagerWrapper : public QObject, public QAscApplicationManager, CCefEventsTransformer
{
    Q_OBJECT
protected:
    void addStylesheets(CScalingFactor, const std::string&);

private:
    std::vector<size_t> m_vecEditors;

    std::map<CScalingFactor, std::vector<std::string>> m_mapStyles;

    std::map<int, CCefEventsGate *> m_receivers;
    std::map<int, CPresenterWindow *> m_winsReporter;

    uint m_closeCount = 0;
    uint m_countViews = 0;
    std::wstring m_closeTarget;

    CWindowsQueue<sWinTag> * m_queueToClose;
    CEventDriver m_eventDriver;
    CMainWindow * m_pMainWindow = nullptr;

    std::shared_ptr<CThemes> m_themes;
    static bool m_rtlEnabled;

public:
    CWindowsQueue<sWinTag>& closeQueue();
    CEventDriver& commonEvents();

private:
    CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&);
    void operator =(CAscApplicationManagerWrapper const&);

    CAscApplicationManagerWrapper();
    ~CAscApplicationManagerWrapper();

    void StartSaveDialog(const std::wstring& sName, unsigned int nId);
    bool processCommonEvent(NSEditorApi::CAscCefMenuEvent *);
    void broadcastEvent(NSEditorApi::CAscCefMenuEvent *);
    bool applySettings(const std::wstring& wstrjson);
    void sendSettings(const std::wstring& opts);
    void checkSettings(const std::wstring& opts);
    void applyTheme(const std::wstring&, bool force = false);
    void handleDeeplinkActions(const std::vector<std::wstring>& actions);
    void setHasFrameFeature(CCefView*, const std::wstring&, int);

    CMainWindow * prepareMainWindow(const QRect& r = QRect());
    CMainWindow * mainWindowFromViewId(int uid) const;
    CEditorWindow * editorWindowFromViewId(int uid) const;
    CEditorWindow * editorWindowFromUrl(const QString&) const;

public:
    static void bindReceiver(int view_id, CCefEventsGate * const receiver);
    static void unbindReceiver(int view_id);
    static void unbindReceiver(const CCefEventsGate * receiver);

signals:
    void coreEvent(void *);
    void aboutToQuit();

public slots:
    void onCoreEvent(void *);
    void onDownloadSaveDialog(const std::wstring& name, uint id);
    void onQueueCloseWindow(const sWinTag&);
    void onFileChecked(const QString&, int, bool);
    void onEditorWidgetClosed();

private slots:
    void onMainWindowClose();

public:
    static CAscApplicationManagerWrapper & getInstance();
    static CAscApplicationManager * createInstance();

    CPresenterWindow * createReporterWindow(void *, int);

    static void             startApp();
    static void             initializeApp();
    static void             gotoMainWindow(size_t pw = 0);
    static void             handleInputCmd(const std::vector<std::wstring>&);
    static void             closeEditorWindow(const size_t);
    static void             pinWindowToTab(CEditorWindow *editor, bool by_position = true);

    static void             editorWindowMoving(const size_t, const QPoint&);
    static CMainWindow *    mainWindow();
    static const CEditorWindow *  editorWindowFromHandle(size_t);
    static void             sendCommandTo(QCefView * target, const QString& cmd, const QString& args = "");
    static void             sendCommandTo(CCefView * target, const std::wstring& cmd, const std::wstring& args = L"");
    static void             sendCommandToAllEditors(const std::wstring& cmd, const std::wstring& args = L"");
    static std::wstring     userSettings(const std::wstring& name);
    static void             setUserSettings(const std::wstring& name, const std::wstring& value);

    static void             sendEvent(int type, void * data);
    static QString          getWindowStylesheets(double);
    static QString          getWindowStylesheets(CScalingFactor);
    static bool             canAppClose();
    static bool             hasUnsavedChanges();
    static QCefView *       createViewer(QWidget * parent, const QSize& size);
    static QString          newFileName(int format);
    static QString          newFileName(const std::wstring& format);
    static CThemes &        themes();
    static CPrintData&      printData();

    static ParentHandle     windowHandleFromId(int id);

    static void             destroyViewer(int id);
    static void             destroyViewer(QCefView * v);

    static void             closeAppWindows();      // TODO: combine with launchAppClose
    static void             cancelClose();
    static void             setRtlEnabled(bool);
    static bool             isRtlEnabled();
    static bool             notificationSupported();

    std::wstring GetExternalSchemeName();
    using CAscApplicationManager::GetExternalSchemeName;

    uint logoutCount(const std::wstring& portal) const;
    void Logout(const std::wstring& portal);
    void launchAppClose();
    void onDocumentReady(int uid);
    void OnEvent(NSEditorApi::CAscCefMenuEvent *);
    bool event(QEvent *event);
private:
    friend class Association;
    friend class CAscApplicationManagerWrapper_Private;
    std::unique_ptr<CAscApplicationManagerWrapper_Private> m_private;

    CAscApplicationManagerWrapper(CAscApplicationManagerWrapper_Private *);

#ifdef _UPDMODULE
    CUpdateManager *m_pUpdateManager = nullptr;
#endif
};

#endif // QASCAPPLICATIONMANAGER
