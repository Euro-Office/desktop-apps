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

#ifndef CEDITORWINDOW_P_H
#define CEDITORWINDOW_P_H

#include "utils.h"
#include "common/Types.h"
#include "components/cmessage.h"
#include "ceditortools.h"
#include "components/cfullscrwidget.h"
#include "components/cmenu.h"
#include "defines.h"
#include "Network/FileTransporter/include/FileTransporter.h"
#include <QDir>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QGridLayout>
#include <QAction>

#include <qtcomp/qnativeevent.h>
#define DEFAULT_BTNS_COUNT 6
#define ICON_SPACER_WIDTH 9
#define ICON_SIZE QSize(20,20)
#define MARGINS 6

using namespace NSEditorApi;
using namespace NSNetwork::NSFileTransport;


auto prepare_editor_css(AscEditorType type, const CTheme& theme) -> QString {
    std::wstring c;
    switch (type) {
    default: c = theme.value(CTheme::ColorRole::ecrWindowBackground); break;
    case AscEditorType::etDocument: c = theme.value(CTheme::ColorRole::ecrTabWordActive); break;
    case AscEditorType::etPresentation: c = theme.value(CTheme::ColorRole::ecrTabSlideActive); break;
    case AscEditorType::etSpreadsheet: c = theme.value(CTheme::ColorRole::ecrTabCellActive); break;
    case AscEditorType::etDocumentMasterForm:
    case AscEditorType::etPdf: c = theme.value(CTheme::ColorRole::ecrTabViewerActive); break;
    case AscEditorType::etDraw: c = theme.value(CTheme::ColorRole::ecrTabDrawActive); break;
    }
    QString g_css(Utils::readStylesheets(":/styles/editor.qss"));
#ifdef __linux__
    g_css.append(Utils::readStylesheets(":styles/editor_unix.qss"));
#endif
    return g_css.arg(QString::fromStdWString(c), GetColorQValueByRole(ecrTextNormal), GetColorQValueByRole(ecrTextPretty));
}

// auto editor_color(AscEditorType type) -> QColor {
//     switch (type) {
//     case AscEditorType::etDocument: return GetColorByRole(ecrTabWordActive);
//     case AscEditorType::etPresentation: return GetColorByRole(ecrTabSlideActive);
//     case AscEditorType::etSpreadsheet: return GetColorByRole(ecrTabCellActive);
//     case AscEditorType::etPdf: return GetColorByRole(ecrTabViewerActive);
//     case AscEditorType::etDraw: return GetColorByRole(ecrTabDrawActive);
//     default: return GetColorByRole(ecrTabWordActive);
//     }
// }

auto rounded_pixmap(const QPixmap &px, int size) -> QPixmap {
    qreal diam = qMin(px.width(), px.height());
    QPixmap pxm(diam, diam);
    pxm.fill(Qt::transparent);
    QPainter p(&pxm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QBrush(px));
    p.drawEllipse(QRectF(0.5, 0.5, diam - 1.0, diam - 1.0));
    p.end();
    return pxm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

class CEditorWindowPrivate : public CCefEventsGate
{
    CEditorWindow * window = nullptr;
    QLabel * iconuser = nullptr;
    bool isPrinting = false,
        isFullScreen = false;
    int layoutType = LayoutNone;
    CFullScrWidget * fs_parent = nullptr;
    QLabel * iconcrypted = nullptr;
    QWidget * boxtitlelabel = nullptr,
            * leftboxbuttons = nullptr;
    QPixmap   avatar;

    QMap<QString, CSVGPushButton*> m_mapTitleButtons;
    CFileDownloader *fdl = nullptr;
    int leftBtnsCount = DEFAULT_BTNS_COUNT;

    enum LayoutType {
        LayoutNone = 0,
        LayoutViewer,
        LayoutEditor
    };

public:
    int titleLeftOffset = 0;

public:
    CEditorWindowPrivate(CEditorWindow * w) : window(w) {}
    ~CEditorWindowPrivate() override {
        if ( leftboxbuttons )
            leftboxbuttons->deleteLater();
        if (fdl)
            delete fdl, fdl = nullptr;
    }

    void createHomeButton() {
        leftboxbuttons = new QWidget;
        leftboxbuttons->setLayout(new QHBoxLayout);
        leftboxbuttons->layout()->setSpacing(0);
        QtComp::Widget::setLayoutMargin(leftboxbuttons->layout(), 0);

        CSVGPushButton * btnHome = new CSVGPushButton;
        btnHome->setProperty("class", "normal");
        btnHome->setProperty("act", "tool");
        btnHome->setFixedSize(QSize(TOOLBTN_WIDTH,TOOLBTN_HEIGHT) * window->m_dpiRatio);
        btnHome->setIconSize(QSize(20,20) * window->m_dpiRatio);
        btnHome->setMouseTracking(true);
        btnHome->setIcon(":/title/icons/buttons.svg", "svg-btn-home");
        //btnHome->setToolTip(CEditorWindow::tr("Open main window"));
        btnHome->setProperty("ToolTip", CEditorWindow::tr("Open main window"));
        btnHome->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
        m_mapTitleButtons["home"] = btnHome;
        connect(btnHome, &QPushButton::clicked, std::bind(&CEditorWindow::onClickButtonHome, window));
        leftboxbuttons->layout()->addWidget(btnHome);
    }

    void init(CTabPanel * const p) override {
        CCefEventsGate::init(p);
    }

    QPushButton * cloneEditorHeaderButton(const QJsonObject& jsonobj)  // For old editors only
    {
        QString action = jsonobj["action"].toString();
        CSVGPushButton * btn = new CSVGPushButton;
        btn->setProperty("class", "normal");
        btn->setProperty("act", "tool");
        btn->setFixedSize(QSize(TOOLBTN_WIDTH,TOOLBTN_HEIGHT) * window->m_dpiRatio);
        btn->setDisabled(jsonobj["disabled"].toBool());
        btn->setIconSize(QSize(20,20) * window->m_dpiRatio);
        btn->setMouseTracking(true);
        btn->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
        if ( jsonobj.contains("visible") && !jsonobj["visible"].toBool() ) {
            btn->hide();
        }

        m_mapTitleButtons[action] = btn;
        connect(btn, &QPushButton::clicked, [=]{
            if ( action == "home" ) {
            } else {
                QJsonObject _json_obj{{"action", action}};
                AscAppManager::sendCommandTo(panel()->cef(), L"button:click", Utils::stringifyJson(_json_obj).toStdWString());
                window->focus();
            }
        });

        if ( jsonobj.contains("icon") ) {
            btn->setIcon(":/title/icons/buttons.svg", "svg-btn-" + jsonobj["icon"].toString());
        } else btn->setIcon(":/title/icons/buttons.svg", "svg-btn-" + action);

        //btn->setToolTip(jsonobj["hint"].toString());
        btn->setProperty("ToolTip", jsonobj["hint"].toString());
        return btn;
    }

    auto extendableTitleToSimple(bool set_title = true) -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        QGridLayout * const _layout = static_cast<QGridLayout*>(window->m_pMainPanel->layout());
        if ( !_layout->itemAtPosition(0,0) && !_layout->findChild<QWidget*>(window->m_boxTitleBtns->objectName()) ) {
            _layout->addWidget(window->m_boxTitleBtns,0,0,Qt::AlignTop);
            if (iconuser)
                iconuser->hide();
            if (set_title)
                window->m_labelTitle->setText(APP_TITLE);
            changeTheme(GetCurrentTheme().id());
        }

        if (!leftboxbuttons)
            createHomeButton();
        else {
            for (auto it = m_mapTitleButtons.begin(); it != m_mapTitleButtons.end();) {
                if (it.key() != "home") {
                    delete it.value();
                    it = m_mapTitleButtons.erase(it);
                    continue;
                }
                ++it;
            }
        }
        QHBoxLayout *lut = qobject_cast<QHBoxLayout*>(window->m_boxTitleBtns->layout());
        if (lut->itemAt(0)->widget() != leftboxbuttons)
            lut->insertWidget(0, leftboxbuttons);
        leftboxbuttons->show();
    }

    auto resetSimpleTitleToDefault() -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        for (auto it = m_mapTitleButtons.begin(); it != m_mapTitleButtons.end();) {
            delete it.value();
            it = m_mapTitleButtons.erase(it);
            continue;
        }
        QHBoxLayout *lut = qobject_cast<QHBoxLayout*>(window->m_boxTitleBtns->layout());
        lut->removeWidget(leftboxbuttons);
        delete leftboxbuttons, leftboxbuttons = nullptr;

        QGridLayout * const _layout = static_cast<QGridLayout*>(window->m_pMainPanel->layout());
        _layout->removeWidget(window->m_boxTitleBtns);
        _layout->addWidget(window->m_boxTitleBtns, 1, 1, Qt::AlignTop);
        if (!m_panel->data()->title().isEmpty())
            window->m_labelTitle->setText(m_panel->data()->title());
    }

    auto getInitials(const QString &name) -> QString {
        auto fio = name.split(' ');
        QString initials = !fio[0].isEmpty() ? fio[0].mid(0, 1).toUpper() : "";
        for (int i = fio.size() - 1; i > 0; i--) {
            if (!fio[i].isEmpty() && fio[i].at(0) != '('
                    && fio[i].at(0) != ')') {
                initials += fio[i].mid(0, 1).toUpper();
                break;
            }
        }
        return initials;
    }

    auto centerTitle(double dpiRatio)->void
    {
        int left_btns = m_mapTitleButtons.size() != 0 ? m_mapTitleButtons.size() : leftBtnsCount;
        int right_btns = 3;
        int spacing = window->m_boxTitleBtns->layout()->spacing();
        int left_offset = left_btns*TOOLBTN_WIDTH + 3*spacing; // added extra spacing
        int right_offset = right_btns*(TITLEBTN_WIDTH + spacing);
        int diffW = (left_offset - right_offset)*dpiRatio;
        if (iconuser) {
            diffW -= ICON_SPACER_WIDTH + spacing*dpiRatio;
            if (!viewerMode() && !fillformMode() && !m_panel->data()->hasError()) {
                diffW -= iconuser->width() + spacing*dpiRatio;
            }
        }
        QMargins mrg(0, 0, 0, 2*dpiRatio);
        if (AscAppManager::isRtlEnabled())
            diffW > 0 ? mrg.setLeft(diffW) : mrg.setRight(-diffW);
        else
            diffW > 0 ? mrg.setRight(diffW) : mrg.setLeft(-diffW);
        boxtitlelabel->setContentsMargins(mrg);
    }

    auto onLayoutDirectionChanged()->void
    {
        if (boxtitlelabel) {
            QMargins mrg = boxtitlelabel->contentsMargins();
            if (AscAppManager::isRtlEnabled()) {
                mrg.setLeft(mrg.right());
                mrg.setRight(0);
            } else {
                mrg.setRight(mrg.left());
                mrg.setLeft(0);
            }
            boxtitlelabel->setContentsMargins(mrg);
        }
        if (iconcrypted) {
            QSize size = window->m_labelTitle->size();
            int offset = window->m_labelTitle->textWidth()/2 + MARGINS * window->m_dpiRatio;
            int x = size.width()/2;
            x += AscAppManager::isRtlEnabled() ? offset : -offset - ICON_SIZE.width() * window->m_dpiRatio;
            int y = (size.height() - ICON_SIZE.height() * window->m_dpiRatio)/2;
            iconcrypted->move(x, y);
        }
    }

    virtual void onImageLoadFinished(int err) override
    {
        if (err == 0) {
            QString path = QString::fromStdWString(fdl->GetFilePath());
            if (!(avatar = QPixmap(path)).isNull())
                iconuser->setPixmap(rounded_pixmap(avatar, iconuser->width()));
            QFile::remove(path);
        }
    }

    void onEditorConfig(int, std::wstring cfg) override
    {
//        if ( id == window->holdView(id) )
        if ( !window->isCustomWindowStyle() )
            return;

        if ( viewerMode() || panel()->data()->features().empty() ) {
            if (layoutType == LayoutViewer)
                return;
            layoutType = LayoutViewer;
            extendableTitleToSimple();
        } else
        if ( canExtendTitle() ) {
            if (layoutType == LayoutViewer) {
                resetSimpleTitleToDefault();
            } else
            if (layoutType == LayoutEditor)
                return;
            layoutType = LayoutEditor;
            QJsonParseError jerror;
            QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(cfg).toUtf8(), &jerror);
            if ( jerror.error == QJsonParseError::NoError ) {
                QJsonObject objRoot = jdoc.object();
                if ( objRoot.contains("user") ) {
                    QJsonObject objUser = objRoot["user"].toObject();
                    QString _user_name = objUser.value("name").toString();
                    //iconUser()->setToolTip(_user_name);
                    iconUser()->setProperty("ToolTip", _user_name);
                    adjustIconUser();
                    iconuser->setText(getInitials(_user_name));
                    if (objUser.contains("image")) {
                        QString img_url = objUser["image"].toString();
                        if (QUrl(img_url).scheme() == "data") {
                            auto list = img_url.split(";base64,");
                            if (list.size() == 2 && !list[1].isEmpty()) {
                                if (avatar.loadFromData(QByteArray::fromBase64(list[1].toLocal8Bit())))
                                    iconuser->setPixmap(rounded_pixmap(avatar, iconuser->width()));
                            }
                        } else {
                            if (!fdl) {
                                QString tmp_name = QString("/avatar_%1.png").arg(QUuid::createUuid().toString().remove('{').remove('}'));
                                fdl = new CFileDownloader(img_url.toStdWString(), false);
                                fdl->SetFilePath((QDir::tempPath() + tmp_name).toStdWString());
                                fdl->SetEvent_OnComplete([=](int err) {
                                    QMetaObject::invokeMethod(this, "onImageLoadFinished", Qt::QueuedConnection, Q_ARG(int, err));
                                });
                            } else {
                                fdl->Cancel();
                            }
                            fdl->Start(0);
                        }
                    }
                    iconuser->setVisible(true);
                }

                if ( objRoot.contains("title") /*&& m_mapTitleButtons.empty()*/ ) {
                    const QJsonArray _btns = objRoot["title"].toObject().value("buttons").toArray();
                    QHBoxLayout * _layout = qobject_cast<QHBoxLayout *>(window->m_boxTitleBtns->layout());

                    if (!m_panel->data()->hasFeature(L"btnhome")) {  // For old editors only
                        if (!leftboxbuttons)
                            createHomeButton();
                        for (const auto &jv: _btns) {
                            const QJsonObject obj = jv.toObject();
                            if ( !m_mapTitleButtons.contains(obj["action"].toString()) )
                                leftboxbuttons->layout()->addWidget(cloneEditorHeaderButton(obj));
                        }

                        if ( _layout->itemAt(0)->widget() != leftboxbuttons )
                            _layout->insertWidget(0, leftboxbuttons);
                    } else {
                        leftBtnsCount = 0;
                        bool usedQuickaccess = false;
                        for (const auto &jv: _btns) {
                            const QJsonObject obj = jv.toObject();
                            if (obj["action"].toString() == "quickaccess")
                                usedQuickaccess = true;
                            if (obj.contains("visible") && obj["visible"].toBool())
                                ++leftBtnsCount;
                        }
                        if (!usedQuickaccess)
                            leftBtnsCount = DEFAULT_BTNS_COUNT;
                        if (auto mainGridLayout = qobject_cast<QGridLayout*>(window->m_pMainPanel->layout())) {
                            window->m_pSpacer = new QSpacerItem(int(leftBtnsCount*TOOLBTN_WIDTH*window->m_dpiRatio), 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
                            mainGridLayout->addItem(window->m_pSpacer, 1, 0, Qt::AlignTop);
                        }
                    }
                }

                // update title caption for elipsis
                window->updateTitleCaption();
            }
        }
        if (window->m_labelTitle)
            window->m_labelTitle->setVisible(true);
        centerTitle(window->m_dpiRatio);
    }

    void onEditorActionRequest(int, const QString& json) override
    {
        if ( json.contains(QRegularExpression("action\\\":\\\"file:close")) ) {
            window->closeWindow();
        }
    }

//    void adjustToNewEditorVersion()  // For old editors only
//    {
//        if (window->isCustomWindowStyle()) {
//            leftboxbuttons->hide();
//            auto layout = qobject_cast<QHBoxLayout*>(window->m_boxTitleBtns->layout());
//            if (canExtendTitle()) {
//                auto icon = layout->takeAt(3);
//                if (icon && icon->widget())
//                    layout->insertWidget(2, icon->widget());
//            }
//            auto stretch = layout->takeAt(1);
//            if (stretch)
//                delete stretch;
//            auto mainGridLayout = dynamic_cast<QGridLayout*>(window->m_pMainPanel->layout());
//            if (mainGridLayout) {
//                auto mainView = mainGridLayout->itemAtPosition(1, 0);
//                if (mainView)
//                    mainGridLayout->removeItem(mainView);
//                QLayoutItem *boxTitleBtns = canExtendTitle() ? mainGridLayout->itemAtPosition(1, 0) : nullptr;
//                if (boxTitleBtns)
//                    mainGridLayout->removeItem(boxTitleBtns);

//                if (mainView && mainView->widget())
//                    mainGridLayout->addWidget(mainView->widget(), 1, 0, 1, 2);
//                if (boxTitleBtns && boxTitleBtns->widget()) {
//                    mainGridLayout->addWidget(boxTitleBtns->widget(), 1, 1, Qt::AlignTop);
//                    window->m_pSpacer = new QSpacerItem(int(TOP_PANEL_OFFSET*window->m_dpiRatio), 5,
//                                                QSizePolicy::Fixed, QSizePolicy::Fixed);
//                    mainGridLayout->addItem(window->m_pSpacer, 1, 0, Qt::AlignTop);
//                }
//            }
//        }
//    }

    void onDocumentReady(int uid) override
    {
//        if (window->holdView(uid))
            panel()->setReady();
            if (window->isActiveWindow())
                window->focus();
            AscAppManager::getInstance().onDocumentReady(uid);
    }

    void onDocumentName(void * data) override
    {
        CCefEventsGate::onDocumentName(data);

        if ( window->isCustomWindowStyle() ) {
            if ( !canExtendTitle() && !fillformMode() ) {
                window->m_labelTitle->setText(APP_TITLE);
            } else {
                window->setWindowTitle(m_panel->data()->title());
                window->m_boxTitleBtns->repaint();
            }
        }
    }

    void onDocumentType(int id, int type) override
    {
        CCefEventsGate::onDocumentType(id, type);

        if ( /*canExtendTitle() &&*/ window->isCustomWindowStyle() ) {
            window->m_pMainPanel->setProperty("window", "pretty");
            changeTheme(GetCurrentTheme().id());
        }
    }

    void setWindowColors()
    {
        std::wstring background, border;
        switch (panel()->data()->contentType()) {
        case AscEditorType::etDocument:
            background = GetColorValueByRole(ecrTabWordActive);
            // border = background;
            break;
        case AscEditorType::etPresentation:
            background = GetColorValueByRole(ecrTabSlideActive);
            // border = background;
            break;
        case AscEditorType::etSpreadsheet:
            background = GetColorValueByRole(ecrTabCellActive);
            // border = background;
            break;
        case AscEditorType::etDocumentMasterForm:
        case AscEditorType::etPdf:
            background = GetColorValueByRole(ecrTabViewerActive);
            // border = background;
            break;
        case AscEditorType::etDraw:
            background = GetColorValueByRole(ecrTabDrawActive);
            // border = background;
            break;
        default:
            background = GetColorValueByRole(ecrWindowBackground);
            // border = GetColorValueByRole(ecrWindowBorder);
        }
        border = GetColorValueByRole(ecrWindowBorder);

        window->setWindowColors(QColor(QString::fromStdWString(background)), QColor(QString::fromStdWString(border)), window->isActiveWindow());
    }

    void changeTheme(const std::wstring& theme)
    {
        if (window->isCustomWindowStyle()) {
            Q_ASSERT(window->m_pMainPanel);
            window->m_pMainPanel->setProperty("uitheme", QString::fromStdWString(GetActualTheme(theme)));
            window->m_pMainPanel->setProperty("uithemetype", GetCurrentTheme().stype());
            if (!viewerMode()) {
                foreach (auto btn, m_mapTitleButtons)
                    btn->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
            } else {
                window->m_pMainPanel->setProperty("window", "pretty");
                if ( m_mapTitleButtons.contains("home") )
                    m_mapTitleButtons["home"]->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
            }
            AscEditorType editor_type = panel()->data()->contentType();
            window->m_css = prepare_editor_css(editor_type, GetCurrentTheme());
            QString css(AscAppManager::getWindowStylesheets(window->m_dpiRatio));
            css.append(window->m_css);
#ifdef __linux__
            css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
            window->m_pMainPanel->setStyleSheet(css);
        }
        setWindowColors();
        AscAppManager::sendCommandTo(panel()->cef(), L"uitheme:changed", theme);
    }

    void onDocumentChanged(int id, bool state) override
    {
        CCefEventsGate::onDocumentChanged(id, state);
        if ( panel()->data()->hasChanges() != state ) {
            if ( (canExtendTitle() || fillformMode()) && window->isCustomWindowStyle() ) {
                window->setWindowTitle(m_panel->data()->title());
                window->m_boxTitleBtns->repaint();
            }
        }
    }

    void onDocumentSave(int id, bool cancel = false) override
    {
        CCefEventsGate::onDocumentSave(id, cancel);

        if ( m_panel->data()->closed() ) {
            if ( !((CCefViewEditor *)m_panel->cef())->CheckCloudCryptoNeedBuild() ) {
                AscAppManager::getInstance().DestroyCefView(m_panel->cef()->GetId());
                window->hide();
            }
        } else {
            if (!cancel && window->menu())
                window->menu()->setSectionEnabled(CMenu::ActionShowInFolder, true);
            AscAppManager::cancelClose();
        }
    }

    void onDocumentSaveInnerRequest(int) override
    {
        int reply = CMessage::showMessage(window->handle(),
                                          CEditorWindow::tr("Document must be saved to continue.<br>Save the document?"),
                                          MsgType::MSG_CONFIRM, MsgBtns::mbYesDefNo);
        CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
        pData->put_Value(reply == MODAL_RESULT_YES);

        CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
        pEvent->m_pData = pData;

        m_panel->cef()->Apply(pEvent);
    }

    void onDocumentFragmented(int id, bool needbuild) override
    {
        if ( needbuild ) {
//            static const bool _skip_user_warning = !Utils::appArgsContains("--warning-doc-fragmented");
//            if ( _skip_user_warning )
                m_panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
        } else
            onDocumentFragmentedBuild(id, 0);

    }

    void onDocumentFragmentedBuild(int id, int error) override
    {
        CCefEventsGate::onDocumentFragmentedBuild(id, error);

        if ( m_panel->data()->closed() ) {
            AscAppManager::getInstance().DestroyCefView(m_panel->cef()->GetId());
            window->hide();
        } else AscAppManager::cancelClose();
    }

    void onDocumentPrint(void * data)  override
    {
        onDocumentPrint(AscAppManager::printData().pageCurrent(), AscAppManager::printData().pagesCount());
    }

    void onDocumentPrint(int currentpage, uint pagescount) override
    {
        if ( isPrinting ) return;
        isPrinting = true;

        CCefView *pView = m_panel->cef();
        QString documentName = m_panel->data()->title(true);
        CEditorTools::onDocumentPrint(window->handle(), pView, documentName, currentpage, pagescount);

        isPrinting = false;
    }

    void onErrorPage(int id, const std::wstring& action) override
    {
        if (m_panel->data()->viewType() == cvwtEditor && action.compare(L"open") == 0) {
            m_panel->data()->setHasError();
            if (window->isCustomWindowStyle() && layoutType != LayoutViewer) {
                layoutType = LayoutViewer;
                extendableTitleToSimple(false);
                centerTitle(window->m_dpiRatio);
            }
        }
    }

    void onEditorAllowedClose(int) override
    {
        AscAppManager::closeEditorWindow(size_t(window));

        CInAppEventBase _event{CInAppEventBase::CEventType::etEditorClosed};
        AscAppManager::getInstance().commonEvents().signal(&_event);
    }

    void onScreenScalingFactor(double f)
    {
        if ( window->isCustomWindowStyle() ) {
            if ( iconuser ) {
                adjustIconUser();

                if (!avatar.isNull())
                    iconuser->setPixmap(rounded_pixmap(avatar, iconuser->width()));
            }

            if ( iconcrypted ) {
                iconcrypted->setPixmap(QIcon{":/title/icons/secure.svg"}.pixmap(QSize(20,20) * f));
                iconcrypted->setFixedSize(ICON_SIZE * f);
            }

            for (const auto& btn: m_mapTitleButtons) {
                btn->setFixedSize(QSize(int(TITLEBTN_WIDTH*f), int(TOOLBTN_HEIGHT*f)));
                btn->setIconSize(QSize(20,20) * f);
            }
            if (layoutType != LayoutNone)
                centerTitle(f);
        }
    }

    bool isSlideshowMode() const
    {
        return fs_parent != nullptr;
    }

    void onFullScreen(bool apply)
    {
        if ( apply == isFullScreen ) return;
        isFullScreen = apply;

        auto _break_demonstration = [&] {
            CAscExecCommandJS * pCommand = new CAscExecCommandJS;
            pCommand->put_Command(L"editor:stopDemonstration");

            CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
            pEvent->m_pData = pCommand;
            panel()->cef()->Apply(pEvent);
        };

        CTabPanel * _fs_widget = panel();
        static QMetaObject::Connection cefConnection;
        if (!apply) {
            _break_demonstration();

            window->show(false);

//            _fs_widget->view()->resize(_fs_widget->size().width(), _fs_widget->size().height()-1);
            if (m_mapTitleButtons.size() > 1) {  // For old editors only
                qobject_cast<QGridLayout *>(window->m_pMainPanel->layout())->addWidget(_fs_widget, 1, 0);
            } else {
                qobject_cast<QGridLayout *>(window->m_pMainPanel->layout())->addWidget(_fs_widget, 1, 0, 1, 2);
            }
            window->recalculatePlaces();
            _fs_widget->showNormal();
            _fs_widget->cef()->focus();

            if ( fs_parent )
                delete fs_parent, fs_parent = nullptr;

            disconnect(cefConnection);
        } else {
            fs_parent = WindowHelper::constructFullscreenWidget(_fs_widget);

            _fs_widget->view()->setFocusToCef();
            window->hide();
            connect(fs_parent, &CFullScrWidget::closeRequest, this, [=]() {
                disconnect(fs_parent);
                onFullScreen(false);
            });

            cefConnection = connect(_fs_widget, &CTabPanel::closePanel, [=](QCloseEvent * e){
                _break_demonstration();

                e->ignore();
                window->closeWindow();
            });
        }
    }

    void onFullScreen(int, bool apply) override
    {
        onFullScreen(apply);
    }

    void onPortalLogout(std::wstring wjson) override
    {
        if ( m_panel->data()->closed() ) return;

        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            std::vector<QString> _portals{objRoot["domain"].toString()};

            if ( objRoot.contains("extra") && objRoot["extra"].isArray() ) {
                QJsonArray a = objRoot["extra"].toArray();
                for (auto&& v: a) {
                    _portals.push_back(v.toString());
                }
            }

            for (auto& u: _portals) {
                if ( QString::fromStdWString(m_panel->data()->url()).startsWith(u) ) {
                    window->closeWindow();
                    break;
                }
            }
        }
    }

    void onFileLocation(int, QString param) override
    {
        if ( param == "offline" ) {
            const std::wstring& path = m_panel->data()->url();
            if (!path.empty()) {
                Utils::openFileLocation(QString::fromStdWString(path));
            } else
                CMessage::info(window->handle(), CEditorWindow::tr("Document must be saved firstly."));
        } else {
            /**
            * portals open in tabbar in main window only
            * processing is in app cntroller
            */
        }
    }

    QLabel * iconUser()
    {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        if ( !iconuser ) {
            iconuser = new QLabel(window->m_boxTitleBtns);
            iconuser->setObjectName("iconuser");
            iconuser->setVisible(false);
            iconuser->setContentsMargins(0,0,0,0);
            iconuser->setAlignment(Qt::AlignCenter);
        }

        return iconuser;
    }

    void adjustIconUser()
    {
        iconuser->setFixedHeight(20 * window->m_dpiRatio);
        iconuser->setFixedWidth(iconuser->height());
        iconuser->setStyleSheet(QString("#iconuser {border-radius: %1px;}")
                     .arg(QString::number(iconuser->height()/2)));
    }

    QLabel * iconCrypted()
    {
        Q_ASSERT(window->m_labelTitle != nullptr);
        if ( !iconcrypted ) {
            iconcrypted = new QLabel(window->m_labelTitle);
            iconcrypted->setObjectName("iconcrypted");

            iconcrypted->setPixmap(QIcon{":/title/icons/secure.svg"}.pixmap(QSize(20,20) * window->m_dpiRatio));
            iconcrypted->setFixedSize(ICON_SIZE * window->m_dpiRatio);
            iconcrypted->show();
            int y = (window->m_labelTitle->height() - ICON_SIZE.height() * window->m_dpiRatio)/2;
            iconcrypted->move(0, y);
            connect(window->m_labelTitle, &CElipsisLabel::onResize, this, [=](QSize size, int textWidth) {
                if (iconcrypted) {
                    int offset = textWidth/2 + MARGINS * window->m_dpiRatio;
                    int x = size.width()/2;
                    x += AscAppManager::isRtlEnabled() ? offset : -offset - ICON_SIZE.width() * window->m_dpiRatio;
                    int y = (size.height() - ICON_SIZE.height() * window->m_dpiRatio)/2;
                    iconcrypted->move(x, y);
                }
            });
        }

        return iconcrypted;
    }

    void onWebAppsFeatures(int, std::wstring f) override
    {
        bool is_read_only = panel()->data()->hasFeature(L"readonly\":true");
        if (f != L"{\"hasframe\":true}" || !panel()->data()->hasFeature(L"hasframe\":false"))
            panel()->data()->setFeatures(f);

        if ( layoutType == LayoutNone && fillformMode() ) {
            ffWindowCustomize();
        }

        if ( panel()->data()->hasFeature(L"crypted\":true") && boxtitlelabel && !iconcrypted ) {
             iconCrypted();
        }

        if ( is_read_only != panel()->data()->hasFeature(L"readonly\":true") && boxtitlelabel ) {
            window->setWindowTitle(m_panel->data()->title());
            window->m_boxTitleBtns->repaint();
        }
    }

    void onWebTitleChanged(int, std::wstring json) override
    {
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(json).toUtf8(), &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            if ( m_mapTitleButtons.size() > 0 ) {  // For old editors only
                if (objRoot.contains("disabled")) {
                    QJsonObject _disabled = objRoot["disabled"].toObject();

                    if ( _disabled.contains("all") ) {
                        bool _is_disabled = _disabled["all"].toBool();

                        for (auto& btn: m_mapTitleButtons) {
                            if ( m_mapTitleButtons["home"] != btn )
                                btn->setDisabled(_is_disabled);
                        }
                    } else {
                        for (const auto& k: _disabled.keys()) {
                            if ( m_mapTitleButtons.contains(k) )
                                m_mapTitleButtons[k]->setDisabled(_disabled.value(k).toBool());
                        }
                    }
                } else
                if (objRoot.contains("icon:changed")) {
                    QJsonObject _btns_changed = objRoot["icon:changed"].toObject();

                    for (const auto& b: _btns_changed.keys()) {
                        if ( m_mapTitleButtons.contains(b) )
                            m_mapTitleButtons[b]->setIcon(":/title/icons/buttons.svg", "svg-btn-" + _btns_changed.value(b).toString());
                    }
                } else
                if ( objRoot.contains("visible") ) {
                    QJsonObject _btns_changed = objRoot["visible"].toObject();
                    if ( _btns_changed.contains("quickprint") ) {
                        m_mapTitleButtons["quickprint"]->setVisible(_btns_changed["quickprint"].toBool());
                    }
                }
            } else
            if ( objRoot.contains("click") ) {
                QString _btns_changed = objRoot["click"].toString();

                if ( _btns_changed == "home" ) {
                    window->onClickButtonHome();
                }
            } else
            if ( objRoot.contains("quickaccesschanged") ) {
                leftBtnsCount = leftBtnsCount + (json.find(L"true") != std::wstring::npos ? 1 : -1);
                if (window->m_pSpacer)
                    window->m_pSpacer->changeSize(int(TOOLBTN_WIDTH*leftBtnsCount*window->m_dpiRatio), 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
                centerTitle(window->m_dpiRatio);
            }
        }
    }

    bool canExtendTitle() const
    {
        if ( m_panel->data()->features().empty() ) return true;
        else if ( fillformMode() ) return false;
        else return !viewerMode() && (m_panel->data()->isLocal() || m_panel->data()->hasFeature(L"titlebuttons\":"));
    }

    auto viewerMode() const -> bool {
        return m_panel->data()->hasFeature(L"viewmode\":true") || (!m_panel->data()->isLocal() && m_panel->data()->hasFrame());
    }

    auto fillformMode() const -> bool {
        return m_panel->data()->hasFeature(L"uitype\":\"fillform");
    }

//    auto calcTitleLabelWidth(int basewidth) const -> int {
//        if ( iconuser )
//            basewidth -= iconuser->width();

//        basewidth -= boxtitlelabel->contentsMargins().left() + boxtitlelabel->contentsMargins().right();
//        if ( iconcrypted )
//            basewidth -= iconcrypted->width();

//        basewidth -= m_mapTitleButtons.count() * (TITLEBTN_WIDTH + 1) * window->m_dpiRatio;

//        return basewidth;
//    }

    auto leftButtonsCount() -> int {
        return leftBtnsCount;
    }

    auto customizeTitleLabel() -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        QHBoxLayout * _layout = qobject_cast<QHBoxLayout *>(window->m_boxTitleBtns->layout());
        _layout->removeWidget(window->m_labelTitle);

        if (QLayoutItem *stretch = _layout->takeAt(0))
            delete stretch;
        boxtitlelabel = new QWidget(window->m_boxTitleBtns);
        boxtitlelabel->setObjectName("boxtitlelabel");
        boxtitlelabel->setLayout(new QHBoxLayout(boxtitlelabel));
        boxtitlelabel->layout()->setSpacing(0);
        QtComp::Widget::setLayoutMargin(boxtitlelabel->layout(), 0);
        boxtitlelabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        boxtitlelabel->layout()->addWidget(window->m_labelTitle);
        if ( m_panel->data()->hasFeature(L"crypted\":true") && !iconcrypted ) {
            iconCrypted();
        }
        _layout->insertWidget(0, boxtitlelabel);
    }

    auto ffWindowCustomize() -> void {
        if ( layoutType == LayoutViewer || !window->isCustomWindowStyle() )
            return;
        layoutType = LayoutViewer;
        if (window->m_labelTitle)
            window->m_labelTitle->setVisible(true);
        extendableTitleToSimple(false);
        centerTitle(window->m_dpiRatio);
    }
};

#endif // CEDITORWINDOW_P_H
