/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
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
