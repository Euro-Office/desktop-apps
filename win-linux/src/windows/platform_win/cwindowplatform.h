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

#ifndef CWINDOWPLATFORM_H
#define CWINDOWPLATFORM_H

#include "windows/cwindowbase.h"
#include <QtWidgets/QApplication>
#include <qtcomp/qnativeevent.h>

struct FRAME {
    FRAME() : left(0), top(0)
    {}
    FRAME(FRAME &frame) {
        left = frame.left;
        top = frame.top;
    }
    int left, top;
};

class CWindowPlatform : public CWindowBase
{
public:
    explicit CWindowPlatform(const QRect&);
    virtual ~CWindowPlatform();

//    void toggleBorderless(bool);
//    void toggleResizeable();
    void bringToTop();
    virtual void show(bool);
    virtual void setWindowColors(const QColor&, const QColor& border = QColor(), bool isActive = false) final;
    virtual void adjustGeometry() final;

protected:
    bool isSessionInProgress();
    void onWindowActivate(bool is_active);
    virtual bool event(QEvent *event) override;
    virtual void onLayoutDirectionChanged() = 0;
#ifdef __OS_WIN_XP
    virtual void resizeEvent(QResizeEvent *ev) override;
#endif

private:
    virtual void changeEvent(QEvent*) final;
    virtual bool nativeEvent(const QByteArray&, void*, long_ptr*) final;

    QTimer *m_propertyTimer;
    double m_dpi;
    HWND m_hWnd;
    int  m_resAreaWidth;
    FRAME m_frame;
    bool m_borderless,
         m_closed,
         m_isResizeable,
//         m_allowMaximize,
         m_isMaximized = false,
         m_isThemeActive = true,
         m_isTaskbarAutoHideOn = false,
         m_scaleChanged = false,
         m_isSessionInProgress = true;
};

#endif // CWINDOWPLATFORM_H
