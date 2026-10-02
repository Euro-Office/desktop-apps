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
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#ifndef IPLATFORMBACKEND_H
#define IPLATFORMBACKEND_H

#include <QWidget>
#include <QMouseEvent>
#include <vector>

class IPlatformBackend {
public:
    virtual ~IPlatformBackend() = default;

    // Window management
    virtual void moveWindow(WId window, int x, int y) = 0;
    virtual bool isNativeFocus(WId window) = 0;
    virtual void setNativeFocusTo(WId window) = 0;
    virtual void setInputEnabled(WId window, bool enabled) = 0;
    virtual void getWindowStack(std::vector<WId> &winStack) = 0;
    virtual void findWindowAsync(const char *name, void *data,
                                 uint timeout_ms,
                                 void(*cb)(WId, void*)) = 0;

    // Decoration / WM interaction
    virtual void raiseWindow(QWidget *window) = 0;
    virtual void minimizeWindow(QWidget *window) = 0;
    virtual void setCursorPos(int x, int y) = 0;
    virtual void sendButtonRelease(QWidget *window) = 0;
    virtual void startInteractiveMove(QWidget *window, const QPoint &globalPos) = 0;
    virtual void startInteractiveResize(QWidget *window, Qt::Edges edges, const QPoint &globalPos) = 0;

    // Cursor management
    virtual void setCursor(WId window, int cursorShape) = 0;
    virtual void resetCursor(WId window) = 0;

    // Queries
    virtual bool isCompositingAvailable() = 0;
    virtual bool checkButtonState(Qt::MouseButton b) = 0;

    // XDG Portal parent window handle
    virtual QString portalParentHandle(QWidget *parent) = 0;

    // Factory
    static IPlatformBackend* create();
    static IPlatformBackend* instance();
};

#endif // IPLATFORMBACKEND_H
