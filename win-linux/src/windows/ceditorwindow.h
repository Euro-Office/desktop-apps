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

#ifndef CEDITORWINDOW_H
#define CEDITORWINDOW_H

#ifdef __linux__
# include "windows/platform_linux/cwindowplatform.h"
#else
# include "windows/platform_win/cwindowplatform.h"
#endif

#include "components/ctabpanel.h"
#include "components/asctabwidget.h"
#include <memory>
#include <QCoreApplication>

class QSpacerItem;
class CEditorWindowPrivate;
class CEditorWindow : public CWindowPlatform, public CScalingWrapper
{
    Q_OBJECT

public:
    CEditorWindow(const QRect& rect, CTabPanel* view);
    ~CEditorWindow();

    static CEditorWindow* create(const QRect& rect, const COpenOptions& opts);
    const QObject * receiver();
    CTabPanel * releaseEditorView() const;
    CTabPanel * mainView() const;
    AscEditorType editorType() const;
    QString documentName() const;
    double scaling() const;
    int closeWindow();
    bool closed() const;
    bool modified() const;
    bool holdView(const std::wstring& portal) const;
    bool isSlideshowMode() const;
    void undock(bool maximized = false);
    virtual bool holdView(int id) const final;
    virtual void applyTheme(const std::wstring&) final;

protected:
    void closeEvent(QCloseEvent *) override;
    virtual void onLayoutDirectionChanged() final;

private:
    CEditorWindow(const QRect& rect, const COpenOptions& opts);

    QWidget * createMainPanel(QWidget *, const QString&);
    CMenu* menu();
    void init(CTabPanel *panel);
    void setMenu();
    void recalculatePlaces();
    void updateTitleCaption();
    void onSizeEvent(int);
    void onMoveEvent(const QRect&);
//    void onExitSizeMove();
    void captureMouse();
//    virtual int calcTitleCaptionWidth() final;
    virtual void focus() final;
    virtual void onCloseEvent() final;
    virtual void onMinimizeEvent() final;
    virtual void onMaximizeEvent() final;
    virtual bool event(QEvent *) final;
    virtual void setScreenScalingFactor(double, bool resize = true) final;

    QMetaObject::Connection m_modalSlotConnection;
    QString m_css;
    bool m_restoreMaximized = false;
    QSpacerItem *m_pSpacer = nullptr;
    CMenu *m_pMenu = nullptr;

    friend class CEditorWindowPrivate;
    std::unique_ptr<CEditorWindowPrivate> d_ptr;

private slots:
    void onClickButtonHome();
};

#endif // CEDITORWINDOW_H
