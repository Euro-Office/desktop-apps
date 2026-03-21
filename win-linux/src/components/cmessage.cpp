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

#include "components/cmessage.h"
#include <QTextDocumentFragment>
#include <QDialog>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QTimer>
#include <functional>
#include <QEvent>
#include <QCheckBox>
#include <QApplication>
#include "utils.h"
#include "cascapplicationmanagerwrapper.h"
#include <initializer_list>
#include <memory.h>
#ifdef __linux__
# pragma push_macro("signals")
# undef signals
# include "platform_linux/gtkutils.h"
# pragma pop_macro("signals")
# include <gtk/gtkmessagedialog.h>
# include <gtk/gtkcheckbutton.h>
# include <gtk/gtktogglebutton.h>
# include <gdk/gdkx.h>
# include "res/gresource.c"
# define toCharPtr(qstr) qstr.toLocal8Bit().data()
# define AddButton(name, response) \
    gtk_dialog_add_button(GTK_DIALOG(dialog), name, response)
# define GrabFocus(response) \
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), response))
#else
# include "defines.h"
# include "platform_win/resource.h"
# include <string.h>
# include <Windows.h>
# ifndef __OS_WIN_XP
#  include <commctrl.h>
# endif
# define toCharPtr(qstr) _wcsdup(qstr.toStdWString().c_str())
#endif

#define BTN_TEXT_CANCEL QObject::tr("Cancel")
#define BTN_TEXT_YES    QObject::tr("Yes")
#define BTN_TEXT_NO     QObject::tr("No")
#define BTN_TEXT_OK     QObject::tr("OK")
#define BTN_TEXT_SKIP   QObject::tr("Skip")
#define BTN_TEXT_BUY    QObject::tr("Buy Now")
#define BTN_TEXT_ACTIVATE   QObject::tr("Activate")
#define BTN_TEXT_CONTINUE   QObject::tr("Continue")
#define BTN_TEXT_SKIPVER    QObject::tr("Skip this version")
#define BTN_TEXT_REMIND     QObject::tr("Remind me later")
#define BTN_TEXT_INSTALL    QObject::tr("Install update")
#define BTN_TEXT_INSLATER   QObject::tr("Later")
#define BTN_TEXT_RESTART    QObject::tr("Restart Now")
#define BTN_TEXT_SAVEANDINS QObject::tr("Save and Install Now")
#define BTN_TEXT_DOWNLOAD   QObject::tr("Download update")

#define TEXT_CANCEL toCharPtr(BTN_TEXT_CANCEL)
#define TEXT_YES    toCharPtr(BTN_TEXT_YES)
#define TEXT_NO     toCharPtr(BTN_TEXT_NO)
#define TEXT_OK     toCharPtr(BTN_TEXT_OK)
#define TEXT_SKIP   toCharPtr(BTN_TEXT_SKIP)
#define TEXT_BUY    toCharPtr(BTN_TEXT_BUY)
#define TEXT_ACTIVATE   toCharPtr(BTN_TEXT_ACTIVATE)
#define TEXT_CONTINUE   toCharPtr(BTN_TEXT_CONTINUE)
#define TEXT_SKIPVER    toCharPtr(BTN_TEXT_SKIPVER)
#define TEXT_REMIND     toCharPtr(BTN_TEXT_REMIND)
#define TEXT_INSTALL    toCharPtr(BTN_TEXT_INSTALL)
#define TEXT_INSLATER   toCharPtr(BTN_TEXT_INSLATER)
#define TEXT_RESTART    toCharPtr(BTN_TEXT_RESTART)
#define TEXT_SAVEANDINS toCharPtr(BTN_TEXT_SAVEANDINS)
#define TEXT_DOWNLOAD   toCharPtr(BTN_TEXT_DOWNLOAD)

#define MSG_ICON_WIDTH  35
#define MSG_ICON_HEIGHT 35

#define DLG_PADDING 7
#define BTN_SPACING 5
#define BTN_PADDING 13
#define DLG_PREF_WIDTH 240

#define DEFAULT_BUTTON(label) label + ":default"

//    class CMessageEventsFilter : public QObject {
//    public:
//        CMessageEventsFilter(CMessage * p, QObject * o)
//            : QObject(o), m_mess(p)
//        {}
//
//    protected:
//        bool eventFilter(QObject * obj, QEvent * event)
//        {
//            if (event->type()==QEvent::KeyPress) {
//                QKeyEvent * key = static_cast<QKeyEvent*>(event);
//                if ( key->key()==Qt::Key_Escape ) {
//                    m_mess->close();
//                    return true;
//                }
//            }
//
//            return QObject::eventFilter(obj, event);
//        }
//
//    private:
//        CMessage * m_mess;
//    };

class QtMsg : public QDialog
{
public:
    explicit QtMsg(QWidget *);
    ~QtMsg();

    static int showMessage(QWidget *parent,
                           const QString &msg,
                           MsgType msgType,
                           MsgBtns msgBtns,
                           const CMessageOpts &opts);
private:
    void setButtons(std::initializer_list<QString>);
    void setButtons(MsgBtns);
    void setIcon(MsgType);
    void setText(const QString&);
    void setContent(const QString&);
    void setCheckBox(const QString &chekBoxText, bool checkBoxState);
    bool getCheckStatus();

    QWidget *m_boxButtons = nullptr,
            *m_centralWidget = nullptr;
    QLabel  *m_message = nullptr,
            *m_content = nullptr,
            *m_typeIcon = nullptr;

    static int m_modalresult;

    class QtMsgPrivateIntf;
    std::unique_ptr<QtMsgPrivateIntf> m_priv;
};

class QtMsg::QtMsgPrivateIntf {
public:
    explicit QtMsgPrivateIntf(QtMsg * parent)
        : m_mess(parent)
        , dpiRatio(Utils::getScreenDpiRatioByWidget(parent))
    {}

    auto addButton(QPushButton * b) -> void {
        if ( !defaultButton )
            defaultButton = b;

        buttons.push_back(b);
    }

    auto firstButton() -> QPushButton * {
        return buttons.front();
    }

    auto lastButton() -> QPushButton * {
        return buttons.back();
    }

    auto clearButtons() -> void {
        defaultButton = nullptr;
        buttons.clear();
    }

    QtMsg * m_mess = nullptr;
    std::vector<QPushButton *> buttons;
    QPushButton * defaultButton = nullptr;
    QWidget * focusWidget = nullptr;
    double dpiRatio = 1;
    QMetaObject::Connection focusConnection;
    bool isWindowActive = false;
};

int QtMsg::m_modalresult(MODAL_RESULT_CANCEL);

QtMsg::QtMsg(QWidget * p)
    : QDialog(p)
    , m_boxButtons(new QWidget)
    , m_message(new QLabel)
    , m_content(new QLabel)
    , m_typeIcon(new QLabel)
    , m_priv(new QtMsgPrivateIntf(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    //setWindowTitle(APP_TITLE);
    setLayout(new QVBoxLayout);
    layout()->setContentsMargins(0, 0, 0, 0);

    m_centralWidget = new QWidget(this);
    layout()->addWidget(m_centralWidget);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    m_centralWidget->setObjectName("messageBody");
    m_centralWidget->setProperty("uitheme", QString::fromStdWString(GetCurrentTheme().originalId()));

    QVBoxLayout * _c_layout  = new QVBoxLayout;
    QHBoxLayout * _h_layout2 = new QHBoxLayout;
    QHBoxLayout * _h_layout1 = new QHBoxLayout;
    _c_layout->addLayout(_h_layout2, 1);
    _c_layout->addLayout(_h_layout1, 0);

    const int _body_margin = int(12 * m_priv->dpiRatio);
    _c_layout->setContentsMargins(_body_margin,_body_margin,_body_margin,_body_margin);

    m_typeIcon->setProperty("class", "msg-icon");
    m_typeIcon->setFixedSize(int(round(MSG_ICON_WIDTH*m_priv->dpiRatio - 0.25)),
                             int(round(MSG_ICON_HEIGHT*m_priv->dpiRatio - 0.25)));
    _h_layout2->addWidget(m_typeIcon, 0, Qt::AlignTop);

//    m_message->setWordWrap(true);
    m_message->setProperty("class", "msg-report");
    m_message->setStyleSheet(QString("margin-bottom: %1px;").arg(int(8*m_priv->dpiRatio)));
    m_message->setTextFormat(Qt::PlainText);

    m_content->setProperty("class", "msg-report");
    m_content->setStyleSheet(QString("margin-bottom: %1px;").arg(int(8*m_priv->dpiRatio)));
    m_content->setTextFormat(Qt::RichText);
    m_content->setOpenExternalLinks(true);

    QFormLayout * _f_layout = new QFormLayout;
    _f_layout->addWidget(m_message);
    _f_layout->addWidget(m_content);
    _f_layout->setSpacing(0);
    _f_layout->setContentsMargins(int(10*m_priv->dpiRatio),0,int(5*m_priv->dpiRatio),0);
    _h_layout2->addLayout(_f_layout, 1);
    _h_layout2->setContentsMargins(0,0,0,0);

    QPushButton * btn_ok = new QPushButton("&" + QObject::tr("OK"));
    btn_ok->setAutoDefault(true);
    m_boxButtons->setLayout(new QHBoxLayout);
    m_boxButtons->layout()->addWidget(btn_ok);
    m_boxButtons->layout()->setContentsMargins(0,int(10*m_priv->dpiRatio),0,0);
    m_boxButtons->layout()->setSpacing(int(8*m_priv->dpiRatio));
    _h_layout1->addWidget(m_boxButtons, 0, Qt::AlignCenter);

    m_priv->addButton(btn_ok);

    QObject::connect(btn_ok, &QPushButton::clicked, this,
        [=] {
            m_modalresult = MODAL_RESULT_OK;
            close();
        }
    );

    m_centralWidget->setLayout(_c_layout);
    m_centralWidget->setMinimumWidth(int(350*m_priv->dpiRatio));
    m_centralWidget->move(0, 0);

    QString _styles(Utils::readStylesheets(":/styles/message.qss"));
    _styles.append(QString("QPushButton{min-width:%1px;}").arg(int(40*m_priv->dpiRatio)));
    m_centralWidget->setStyleSheet( _styles );

    QString zoom = QString::number(m_priv->dpiRatio) + "x";
    m_centralWidget->setProperty("scaling", zoom);

    m_priv->focusConnection = QObject::connect(qApp, &QApplication::focusChanged, this,
                                               [&] (QWidget * from, QWidget *to){
        Q_UNUSED(from)
        if ( m_priv->isWindowActive ) {
            if ( !to ) {
                m_priv->focusWidget ?
                    m_priv->focusWidget->setFocus(Qt::FocusReason::TabFocusReason) :
                    m_priv->firstButton()->setFocus(Qt::FocusReason::TabFocusReason);
            }
        } else {
        }
    });
}

QtMsg::~QtMsg()
{
    QObject::disconnect(m_priv->focusConnection);
}

void QtMsg::setButtons(std::initializer_list<QString> btns)
{
    QLayoutItem * item;
    QWidget * widget;
    while ( (item = m_boxButtons->layout()->takeAt(0)) ) {
        if ( (widget = item->widget()) ) {
            delete widget;
        }

        delete item;
    }
    m_priv->clearButtons();

    auto _fn_click = [=](MsgRes msgRes) {
        m_modalresult = msgRes;
        close();
    };

    QRegularExpression reFocus("([^:]+)\\:?(default)?$");
    QRegularExpressionMatch match;

    QPushButton * _btn;
    int _btn_num(0);
    for (auto &btn: btns) {
        match = reFocus.match(btn);

        _btn = new QPushButton(match.captured(1));
        if ( !match.captured(2).isEmpty() ) {
            _btn->setDefault(true);
        }

        QMap<MsgRes, QString> btnNames = {
            {MODAL_RESULT_CANCEL, BTN_TEXT_CANCEL},
            {MODAL_RESULT_YES,    BTN_TEXT_YES},
            {MODAL_RESULT_NO,     BTN_TEXT_NO},
            {MODAL_RESULT_OK,     BTN_TEXT_OK},
            {MODAL_RESULT_SKIP,   BTN_TEXT_SKIP},
            {MODAL_RESULT_BUY,    BTN_TEXT_BUY},
            {MODAL_RESULT_ACTIVATE,    BTN_TEXT_ACTIVATE},
            {MODAL_RESULT_CONTINUE,    BTN_TEXT_CONTINUE},
            {MODAL_RESULT_SKIPVER,   BTN_TEXT_SKIPVER},
            {MODAL_RESULT_REMIND,    BTN_TEXT_REMIND},
            {MODAL_RESULT_DOWNLOAD,  BTN_TEXT_DOWNLOAD},
            {MODAL_RESULT_INSTALL,   BTN_TEXT_INSTALL},
            {MODAL_RESULT_INSLATER,  BTN_TEXT_INSLATER},
            {MODAL_RESULT_RESTART,   BTN_TEXT_RESTART}
        };

        m_boxButtons->layout()->addWidget(_btn);
        MsgRes msgRes = btnNames.key(match.captured(1), MODAL_RESULT_CANCEL);
        QObject::connect(_btn, &QPushButton::clicked, std::bind(_fn_click, msgRes));

        m_priv->addButton(_btn);
        _btn_num++;
    }

    if (_btn_num > 2)
        m_centralWidget->setMinimumWidth(int(400*m_priv->dpiRatio));
}

void QtMsg::setButtons(MsgBtns btns)
{
    switch (btns) {
    case MsgBtns::mbYesNo:          setButtons({BTN_TEXT_YES, DEFAULT_BUTTON(BTN_TEXT_NO)}); break;
    case MsgBtns::mbYesDefNo:       setButtons({DEFAULT_BUTTON(BTN_TEXT_YES), BTN_TEXT_NO}); break;
    case MsgBtns::mbYesNoCancel:    setButtons({BTN_TEXT_YES, BTN_TEXT_NO, DEFAULT_BUTTON(BTN_TEXT_CANCEL)}); break;
    case MsgBtns::mbYesDefNoCancel: setButtons({DEFAULT_BUTTON(BTN_TEXT_YES), BTN_TEXT_NO, BTN_TEXT_CANCEL}); break;
    case MsgBtns::mbOkCancel:       setButtons({BTN_TEXT_OK, DEFAULT_BUTTON(BTN_TEXT_CANCEL)}); break;
    case MsgBtns::mbOkDefCancel:    setButtons({DEFAULT_BUTTON(BTN_TEXT_OK), BTN_TEXT_CANCEL}); break;
    case MsgBtns::mbYesDefSkipNo:   setButtons({DEFAULT_BUTTON(BTN_TEXT_YES), BTN_TEXT_SKIP, BTN_TEXT_NO}); break;
    case MsgBtns::mbBuy:            setButtons({DEFAULT_BUTTON(BTN_TEXT_BUY)}); break;
    case MsgBtns::mbActivateDefContinue:   setButtons({DEFAULT_BUTTON(BTN_TEXT_ACTIVATE), BTN_TEXT_CONTINUE}); break;
    case MsgBtns::mbContinue:       setButtons({DEFAULT_BUTTON(BTN_TEXT_CONTINUE)}); break;
    case MsgBtns::mbInslaterRestart:          setButtons({BTN_TEXT_INSLATER, DEFAULT_BUTTON(BTN_TEXT_RESTART)}); break;
    case MsgBtns::mbSkipRemindInstall:        setButtons({BTN_TEXT_SKIPVER, BTN_TEXT_REMIND, DEFAULT_BUTTON(BTN_TEXT_INSTALL)}); break;
    case MsgBtns::mbSkipRemindSaveandinstall: setButtons({BTN_TEXT_SKIPVER, BTN_TEXT_REMIND, DEFAULT_BUTTON(BTN_TEXT_INSTALL)}); break;
    case MsgBtns::mbSkipRemindDownload:       setButtons({BTN_TEXT_SKIPVER, BTN_TEXT_REMIND, DEFAULT_BUTTON(BTN_TEXT_DOWNLOAD)}); break;
    default: break;
    }
}

void QtMsg::setCheckBox(const QString &chekBoxText, bool checkBoxState)
{
    QBoxLayout * layout = qobject_cast<QBoxLayout *>(m_centralWidget->layout());
    QCheckBox * chbox = new QCheckBox(chekBoxText);
    chbox->setObjectName("check-apply-for-all");
    chbox->setStyleSheet(QString("margin-left: %1px").arg(15 + int(m_typeIcon->width() * m_priv->dpiRatio) + int(15 * m_priv->dpiRatio)));
    chbox->setChecked(checkBoxState);
    layout->insertWidget(1, chbox, 0);
}

bool QtMsg::getCheckStatus()
{
    QCheckBox * chbox = m_centralWidget->findChild<QCheckBox *>("check-apply-for-all");
    return chbox && chbox->checkState() == Qt::Checked;
}

int QtMsg::showMessage(QWidget *parent,
                          const QString &msg,
                          MsgType msgType,
                          MsgBtns msgBtns,
                          const CMessageOpts &opts)
{
#ifdef __linux__
    WindowHelper::CParentDisable oDisabler(parent);
#endif
    QtMsg dlg(parent);
    if (AscAppManager::isRtlEnabled()) {
#ifdef _WIN32
        LONG exstyle = GetWindowLong((HWND)dlg.winId(), GWL_EXSTYLE);
        SetWindowLong((HWND)dlg.winId(), GWL_EXSTYLE, exstyle | WS_EX_LAYOUTRTL);
#else
        dlg.setLayoutDirection(Qt::RightToLeft);
#endif
    }
    dlg.setText(QTextDocumentFragment::fromHtml(msg).toPlainText());
    QString content = opts.contentText;
    if (!content.isEmpty() && !opts.linkText.isEmpty()) {
        content.append("\n");
        content.replace("\n", "<br>");
    }
    content.append(opts.linkText);
    if (!content.isEmpty())
        dlg.setContent(content);
    dlg.setIcon(msgType);
    if (msgBtns != MsgBtns::mbOk)
        dlg.setButtons(msgBtns);
    if (opts.checkBoxState != nullptr)
        dlg.setCheckBox(opts.chekBoxText, *opts.checkBoxState);
    dlg.exec();
    if (opts.checkBoxState != nullptr)
        *opts.checkBoxState = dlg.getCheckStatus();
    return m_modalresult;
}

void QtMsg::setIcon(MsgType msgType)
{
    switch (msgType) {
    case MsgType::MSG_WARN:    m_typeIcon->setProperty("type", "msg-warn"); break;
    case MsgType::MSG_INFO:    m_typeIcon->setProperty("type", "msg-info"); break;
    case MsgType::MSG_CONFIRM: m_typeIcon->setProperty("type", "msg-conf"); break;
    case MsgType::MSG_ERROR:   m_typeIcon->setProperty("type", "msg-error"); break;
    case MsgType::MSG_BRAND:   m_typeIcon->setProperty("type", "msg-info"); break;
    default: break;
    }
}

void QtMsg::setText( const QString& t)
{
    m_message->setText(t);
}

void QtMsg::setContent( const QString& t)
{
    m_content->setText(t);
}

#ifdef _WIN32
# ifndef __OS_WIN_XP
static HRESULT CALLBACK Pftaskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    switch (msg) {
    case TDN_HYPERLINK_CLICKED:
        ShellExecute(NULL, L"open", (PCWSTR)lParam, NULL, NULL, SW_SHOWNORMAL);
        break;
    case TDN_DIALOG_CONSTRUCTED: {
        QTimer::singleShot(0, [=]() {
            if (hwnd)
                WindowHelper::bringToTop(hwnd);
        });
        break;
    }
    default:
        break;
    }
    return S_OK;
}

static int calcApproxMinWidth(TASKDIALOG_BUTTON *pButtons, uint cButtons)
{
    int width = 0;
    HDC hdc = GetDC(NULL);
    long units = GetDialogBaseUnits();
    HGDIOBJ hFont = GetStockObject(DEFAULT_GUI_FONT);
    SelectObject(hdc, hFont);
    for (uint i = 0; i < cButtons; i++) {
        SIZE textSize = {0,0};
        const wchar_t *text = pButtons[i].pszButtonText;
        GetTextExtentPoint32(hdc, text, (int)std::wcslen(text), &textSize);
        width += MulDiv(textSize.cx, 4, LOWORD(units));
    }
    ReleaseDC(NULL, hdc);
    width +=  (2 * DLG_PADDING) + (2 * BTN_PADDING * cButtons) + (BTN_SPACING * (cButtons - 1));
    return width;
}
# endif

namespace WinMsg
{
int showMessage(QWidget *parent, const QString &msg, MsgType msgType, MsgBtns msgBtns, const CMessageOpts &opts)
{
    std::wstring lpCaption = QString("  %1").arg(WINDOW_TITLE).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    std::wstring lpContent = opts.contentText.toStdWString();
    if (!lpContent.empty() && !opts.linkText.isEmpty())
        lpContent.append(L"\n");
    lpContent.append(opts.linkText.toStdWString());
    std::wstring lpCheckBoxText = opts.chekBoxText.toStdWString();
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : nullptr;
    if (parent_hwnd && IsIconic(parent_hwnd))
        ShowWindow(parent_hwnd, SW_RESTORE);

    int msgboxID = 0;
# ifndef __OS_WIN_XP
    PCWSTR pIcon = NULL;
    switch (msgType) {
    case MsgType::MSG_INFO:    pIcon = TD_INFORMATION_ICON; break;
    case MsgType::MSG_WARN:    pIcon = TD_WARNING_ICON; break;
    case MsgType::MSG_CONFIRM: pIcon = TD_SHIELD_ICON; break;
    case MsgType::MSG_ERROR:   pIcon = TD_ERROR_ICON; break;
    case MsgType::MSG_BRAND:   pIcon = MAKEINTRESOURCE(IDI_MAINICON); break;
    default:                   pIcon = TD_INFORMATION_ICON; break;
    }

    TASKDIALOG_BUTTON *pButtons;
    uint cButtons = 0;
    switch (msgBtns) {
    case MsgBtns::mbYesNo:
    case MsgBtns::mbYesDefNo:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_YES};
        pButtons[1] = {IDNO,  TEXT_NO};
        break;
    case MsgBtns::mbYesNoCancel:
    case MsgBtns::mbYesDefNoCancel:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_YES};
        pButtons[1] = {IDNO,  TEXT_NO};
        pButtons[2] = {IDCANCEL, TEXT_CANCEL};
        break;
    case MsgBtns::mbOkCancel:
    case MsgBtns::mbOkDefCancel:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDOK, TEXT_OK};
        pButtons[1] = {IDCANCEL, TEXT_CANCEL};
        break;
    case MsgBtns::mbYesDefSkipNo:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_YES};
        pButtons[1] = {IDRETRY, TEXT_SKIP};
        pButtons[2] = {IDNO, TEXT_NO};
        break;
    case MsgBtns::mbBuy:
        cButtons = 1;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_BUY};
        break;
    case MsgBtns::mbActivateDefContinue:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_ACTIVATE};
        pButtons[1] = {IDNO, TEXT_CONTINUE};
        break;
    case MsgBtns::mbContinue:
        cButtons = 1;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDOK, TEXT_CONTINUE};
        break;
    case MsgBtns::mbInslaterRestart:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_INSLATER};
        pButtons[1] = {IDNO,  TEXT_RESTART};
        break;
    case MsgBtns::mbSkipRemindInstall:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIPVER};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_INSTALL};
        break;
    case MsgBtns::mbSkipRemindSaveandinstall:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIPVER};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_SAVEANDINS};
        break;
    case MsgBtns::mbSkipRemindDownload:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIPVER};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_DOWNLOAD};
        break;
    default:
        cButtons = 1;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDOK, TEXT_OK};
        break;
    }

    int nDefltBtn{0};
    switch (msgBtns) {
    case MsgBtns::mbYesNo:          nDefltBtn = IDNO; break;
    case MsgBtns::mbYesDefNo:       nDefltBtn = IDYES; break;
    case MsgBtns::mbYesNoCancel:    nDefltBtn = IDCANCEL; break;
    case MsgBtns::mbYesDefNoCancel: nDefltBtn = IDYES; break;
    case MsgBtns::mbOkCancel:       nDefltBtn = IDCANCEL; break;
    case MsgBtns::mbOkDefCancel:    nDefltBtn = IDOK; break;
    case MsgBtns::mbYesDefSkipNo:   nDefltBtn = IDYES; break;
    case MsgBtns::mbBuy:            nDefltBtn = IDYES; break;
    case MsgBtns::mbActivateDefContinue:   nDefltBtn = IDYES; break;
    case MsgBtns::mbContinue:       nDefltBtn = IDOK; break;
    case MsgBtns::mbInslaterRestart:          nDefltBtn = IDNO; break;
    case MsgBtns::mbSkipRemindInstall:        nDefltBtn = IDYES; break;
    case MsgBtns::mbSkipRemindSaveandinstall: nDefltBtn = IDYES; break;
    case MsgBtns::mbSkipRemindDownload:       nDefltBtn = IDYES; break;
    default:                        nDefltBtn = IDOK; break;
    }

    BOOL chkState = (opts.checkBoxState) ? (BOOL)*opts.checkBoxState : FALSE;

    TASKDIALOGCONFIG config = {0};
    ZeroMemory(&config, sizeof(config));
    config.cbSize             = sizeof(config);
    config.dwFlags            = TDF_POSITION_RELATIVE_TO_WINDOW |
                     TDF_ALLOW_DIALOG_CANCELLATION |
                     TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS;
    if (AscAppManager::isRtlEnabled())
        config.dwFlags |= TDF_RTL_LAYOUT;
    config.hwndParent         = parent_hwnd;
    config.hInstance          = GetModuleHandle(NULL);
    config.pfCallback         = (PFTASKDIALOGCALLBACK)Pftaskdialogcallback;
    config.pButtons           = pButtons;
    config.cButtons           = cButtons;
    config.nDefaultButton     = nDefltBtn;
    config.pszMainIcon        = pIcon;
    config.pszWindowTitle     = lpCaption.c_str();
    config.pszMainInstruction = lpText.c_str();
    config.pszContent         = !lpContent.empty() ? lpContent.c_str() : NULL;
    if (msgType == MsgType::MSG_BRAND)
        config.cxWidth        = calcApproxMinWidth(pButtons, cButtons) > DLG_PREF_WIDTH ? 0 : DLG_PREF_WIDTH;

    if (chkState == TRUE)
        config.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
    if (opts.checkBoxState)
        config.pszVerificationText = lpCheckBoxText.c_str();

    TaskDialogIndirect(&config, &msgboxID, NULL, (opts.checkBoxState != nullptr) ? &chkState : NULL);
    if (opts.checkBoxState != nullptr)
        *opts.checkBoxState = (chkState == TRUE);

    for (int i = 0; i < (int)cButtons; i++)
        free((void*)pButtons[i].pszButtonText);
    delete[] pButtons;
# else
    DWORD uType{0};
    switch (msgType) {
    case MsgType::MSG_INFO:    uType |= MB_ICONINFORMATION; break;
    case MsgType::MSG_WARN:    uType |= MB_ICONWARNING; break;
    case MsgType::MSG_CONFIRM: uType |= MB_ICONQUESTION; break;
    case MsgType::MSG_ERROR:   uType |= MB_ICONERROR; break;
    case MsgType::MSG_BRAND:   uType |= MB_ICONINFORMATION; break;
    default:                   uType |= MB_ICONINFORMATION; break;
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo:          uType |= MB_YESNO | MB_DEFBUTTON2; break;
    case MsgBtns::mbYesDefNo:       uType |= MB_YESNO | MB_DEFBUTTON1; break;
    case MsgBtns::mbYesNoCancel:    uType |= MB_YESNOCANCEL | MB_DEFBUTTON3; break;
    case MsgBtns::mbYesDefNoCancel: uType |= MB_YESNOCANCEL | MB_DEFBUTTON1; break;
    case MsgBtns::mbOkCancel:       uType |= MB_OKCANCEL | MB_DEFBUTTON2; break;
    case MsgBtns::mbOkDefCancel:    uType |= MB_OKCANCEL | MB_DEFBUTTON1; break;
    default:                        uType |= MB_OK | MB_DEFBUTTON1; break;
    }

    msgboxID = MessageBoxW(parent_hwnd, lpText.c_str(), lpCaption.c_str(), uType);
# endif

    int result = MODAL_RESULT_CANCEL;
    switch (msgboxID) {
    case IDYES: result = (msgBtns == MsgBtns::mbBuy) ? MODAL_RESULT_BUY :
                         (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_ACTIVATE :
                         (msgBtns == MsgBtns::mbSkipRemindInstall || msgBtns == MsgBtns::mbSkipRemindSaveandinstall) ? MODAL_RESULT_INSTALL :
                         (msgBtns == MsgBtns::mbSkipRemindDownload) ? MODAL_RESULT_DOWNLOAD :
                         (msgBtns == MsgBtns::mbInslaterRestart) ? MODAL_RESULT_INSLATER : MODAL_RESULT_YES;
        break;
    case IDNO:  result = (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_CONTINUE :
                         (msgBtns == MsgBtns::mbSkipRemindInstall || msgBtns == MsgBtns::mbSkipRemindSaveandinstall
                             || msgBtns == MsgBtns::mbSkipRemindDownload) ? MODAL_RESULT_REMIND :
                         (msgBtns == MsgBtns::mbInslaterRestart) ? MODAL_RESULT_RESTART : MODAL_RESULT_NO;
        break;
    case IDOK:  result = (msgBtns == MsgBtns::mbContinue) ? MODAL_RESULT_CONTINUE : MODAL_RESULT_OK;
        break;
    case IDRETRY:  result = (msgBtns == MsgBtns::mbSkipRemindInstall || msgBtns == MsgBtns::mbSkipRemindSaveandinstall
                                || msgBtns == MsgBtns::mbSkipRemindDownload) ? MODAL_RESULT_SKIPVER : MODAL_RESULT_SKIP;
        break;
    case IDCANCEL:
    default:
        break;
    }

    return result;
}
}
#else
static void on_link_clicked(GtkWidget*, gchar *uri, gpointer)
{
    gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, NULL);
}

namespace GtkMsg
{
int showMessage(QWidget *parent, const QString &msg, MsgType msgType, MsgBtns msgBtns, const CMessageOpts &opts)
{
    QString primaryText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    QString secondaryText = opts.contentText;
    if (secondaryText.isEmpty()) {
        const int delim = primaryText.indexOf('\n');
        if (delim != -1) {
            secondaryText = primaryText.mid(delim + 1);
            primaryText = primaryText.mid(0, delim);
        }
    }
    Window parent_xid = (parent) ? (Window)parent->winId() : 0L;

    const char* img_name = NULL;
    switch (msgType) {
    case MsgType::MSG_INFO:    img_name = "dialog-information"; break;
    case MsgType::MSG_WARN:    img_name = "dialog-warning"; break;
    case MsgType::MSG_CONFIRM: img_name = "dialog-question"; break;
    case MsgType::MSG_ERROR:   img_name = "dialog-error"; break;
    case MsgType::MSG_BRAND:   img_name = "/icons/app-icon_64.png"; break;
    default:                   img_name = "dialog-information"; break;
    }

    GtkWidget *image = NULL;
    if (msgType == MsgType::MSG_BRAND) {
        image = gtk_image_new_from_resource(img_name);
    } else {
        image = gtk_image_new();
        gtk_image_set_from_icon_name(GTK_IMAGE(image), img_name, GTK_ICON_SIZE_DIALOG);
    }

    GtkDialogFlags flags;
    flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);

    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new(NULL, flags,
                                    GTK_MESSAGE_OTHER, // Message type doesn't show icon
                                    GTK_BUTTONS_NONE, "%s",
                                    primaryText.toLocal8Bit().data());

    // gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);
    //gtk_window_set_title(GTK_WINDOW(dialog), APP_TITLE);
    if (!secondaryText.isEmpty())
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondaryText.toLocal8Bit().data());
    if (image) {
        gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), image);
        if (msgType == MsgType::MSG_BRAND)
            gtk_widget_set_margin_top(image, 6);
        gtk_widget_show_all(image);
    }
    if (!opts.linkText.isEmpty()) {
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        GtkWidget *label = gtk_label_new(opts.linkText.toLocal8Bit().data());
        gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 50);
        g_signal_connect(G_OBJECT(label), "activate-link", G_CALLBACK(on_link_clicked), NULL);
        gtk_container_add(GTK_CONTAINER(msg_area), label);
        gtk_widget_show_all(label);
    }
    GtkWidget *chkbox = NULL;
    if (opts.checkBoxState != nullptr) {
        //GtkWidget *cont_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        chkbox = gtk_check_button_new_with_label(opts.chekBoxText.toLocal8Bit().data());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbox), *opts.checkBoxState);
        gtk_container_add(GTK_CONTAINER(msg_area), chkbox);
        gtk_widget_show_all(chkbox);
    }
    if (msgType == MsgType::MSG_BRAND) { // Set text alignment
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        GList *children = gtk_container_get_children(GTK_CONTAINER(msg_area));
        for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
            GtkWidget *child = GTK_WIDGET(iter->data);
            if (GTK_IS_LABEL(child))
                gtk_widget_set_halign(child, GTK_ALIGN_START);
        }
        g_list_free(children);
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo:
    case MsgBtns::mbYesDefNo:
        AddButton(TEXT_YES, GTK_RESPONSE_YES);
        AddButton(TEXT_NO, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbYesNoCancel:
    case MsgBtns::mbYesDefNoCancel:
        AddButton(TEXT_YES, GTK_RESPONSE_YES);
        AddButton(TEXT_NO, GTK_RESPONSE_NO);
        AddButton(TEXT_CANCEL, GTK_RESPONSE_CANCEL);
        break;
    case MsgBtns::mbOkCancel:
    case MsgBtns::mbOkDefCancel:
        AddButton(TEXT_OK, GTK_RESPONSE_OK);
        AddButton(TEXT_CANCEL, GTK_RESPONSE_CANCEL);
        break;
    case MsgBtns::mbYesDefSkipNo:
        AddButton(TEXT_YES, GTK_RESPONSE_YES);
        AddButton(TEXT_SKIP, GTK_RESPONSE_REJECT);
        AddButton(TEXT_NO, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbBuy:
        AddButton(TEXT_BUY, GTK_RESPONSE_YES);
        break;
    case MsgBtns::mbActivateDefContinue:
        AddButton(TEXT_ACTIVATE, GTK_RESPONSE_YES);
        AddButton(TEXT_CONTINUE, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbContinue:
        AddButton(TEXT_CONTINUE, GTK_RESPONSE_OK);
        break;
    case MsgBtns::mbInslaterRestart:
        AddButton(TEXT_INSLATER, GTK_RESPONSE_YES);
        AddButton(TEXT_RESTART, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbSkipRemindInstall:
        AddButton(TEXT_SKIPVER, GTK_RESPONSE_REJECT);
        AddButton(TEXT_REMIND, GTK_RESPONSE_NO);
        AddButton(TEXT_INSTALL, GTK_RESPONSE_YES);
        break;
    case MsgBtns::mbSkipRemindSaveandinstall:
        AddButton(TEXT_SKIPVER, GTK_RESPONSE_REJECT);
        AddButton(TEXT_REMIND, GTK_RESPONSE_NO);
        AddButton(TEXT_SAVEANDINS, GTK_RESPONSE_YES);
        break;
    case MsgBtns::mbSkipRemindDownload:
        AddButton(TEXT_SKIPVER, GTK_RESPONSE_REJECT);
        AddButton(TEXT_REMIND, GTK_RESPONSE_NO);
        AddButton(TEXT_DOWNLOAD, GTK_RESPONSE_YES);
        break;
    default:
        AddButton(TEXT_OK, GTK_RESPONSE_OK);
        break;
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo: GrabFocus(GTK_RESPONSE_NO); break;
    case MsgBtns::mbYesDefNo: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbYesNoCancel: GrabFocus(GTK_RESPONSE_CANCEL); break;
    case MsgBtns::mbYesDefNoCancel: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbOkCancel: GrabFocus(GTK_RESPONSE_CANCEL); break;
    case MsgBtns::mbOkDefCancel: GrabFocus(GTK_RESPONSE_OK); break;
    case MsgBtns::mbYesDefSkipNo: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbBuy: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbActivateDefContinue: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbContinue: GrabFocus(GTK_RESPONSE_OK); break;
    case MsgBtns::mbInslaterRestart:   GrabFocus(GTK_RESPONSE_NO); break;
    case MsgBtns::mbSkipRemindInstall: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbSkipRemindSaveandinstall: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbSkipRemindDownload: GrabFocus(GTK_RESPONSE_YES); break;
    default: GrabFocus(GTK_RESPONSE_OK); break;
    }

    int msgboxID = gtk_dialog_run (GTK_DIALOG (dialog));
    int result = MODAL_RESULT_CANCEL;
    switch (msgboxID) {
    case GTK_RESPONSE_YES: result = (msgBtns == MsgBtns::mbBuy) ? MODAL_RESULT_BUY :
                                    (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_ACTIVATE :
                                    (msgBtns == MsgBtns::mbSkipRemindInstall || msgBtns == MsgBtns::mbSkipRemindSaveandinstall) ? MODAL_RESULT_INSTALL :
                                    (msgBtns == MsgBtns::mbSkipRemindDownload) ? MODAL_RESULT_DOWNLOAD :
                                    (msgBtns == MsgBtns::mbInslaterRestart) ? MODAL_RESULT_INSLATER : MODAL_RESULT_YES;
        break;
    case GTK_RESPONSE_NO:  result = (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_CONTINUE :
                                    (msgBtns == MsgBtns::mbSkipRemindInstall || msgBtns == MsgBtns::mbSkipRemindSaveandinstall
                                        || msgBtns == MsgBtns::mbSkipRemindDownload) ? MODAL_RESULT_REMIND :
                                    (msgBtns == MsgBtns::mbInslaterRestart) ? MODAL_RESULT_RESTART : MODAL_RESULT_NO;
        break;
    case GTK_RESPONSE_OK:  result = (msgBtns == MsgBtns::mbContinue) ? MODAL_RESULT_CONTINUE : MODAL_RESULT_OK;
        break;
    case GTK_RESPONSE_REJECT:  result = (msgBtns == MsgBtns::mbSkipRemindInstall || msgBtns == MsgBtns::mbSkipRemindSaveandinstall
                                            || msgBtns == MsgBtns::mbSkipRemindDownload) ? MODAL_RESULT_SKIPVER : MODAL_RESULT_SKIP;
        break;
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:
    default:
        break;
    }

    if (opts.checkBoxState != nullptr) {
        gboolean chkState = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbox));
        *opts.checkBoxState = (chkState == TRUE);
    }

    gtk_widget_destroy(dialog);

    return result;
}
}
#endif

// -------------------- CMessage ------------------

int CMessage::showMessage(QWidget *parent,
                          const QString &msg,
                          MsgType msgType,
                          MsgBtns msgBtns,
                          const CMessageOpts &opts)
{
    if (WindowHelper::useNativeDialog()) {
#ifdef _WIN32
# ifndef __OS_WIN_XP
        return WinMsg::showMessage(parent, msg, msgType, msgBtns, opts);
# endif
#else
        WindowHelper::CParentDisable oDisabler(parent);
        return GtkMsg::showMessage(parent, msg, msgType, msgBtns, opts);
#endif
    }
    return QtMsg::showMessage(parent, msg, msgType, msgBtns, opts);
}

void CMessage::confirm(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_CONFIRM);
}

void CMessage::info(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_INFO);
}

void CMessage::warning(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_WARN);
}

void CMessage::error(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_ERROR);
}
