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

#ifndef X11BACKEND_H
#define X11BACKEND_H

#include "iplatformbackend.h"

class X11Backend : public IPlatformBackend {
public:
    X11Backend();
    ~X11Backend() override;

    // Window management
    void moveWindow(WId window, int x, int y) override;
    bool isNativeFocus(WId window) override;
    void setNativeFocusTo(WId window) override;
    void setInputEnabled(WId window, bool enabled) override;
    void getWindowStack(std::vector<WId> &winStack) override;
    void findWindowAsync(const char *name, void *data,
                         uint timeout_ms,
                         void(*cb)(WId, void*)) override;

    // Decoration / WM interaction
    void raiseWindow(QWidget *window) override;
    void minimizeWindow(QWidget *window) override;
    void setCursorPos(int x, int y) override;
    void sendButtonRelease(QWidget *window) override;
    void startInteractiveMove(QWidget *window, const QPoint &globalPos) override;
    void startInteractiveResize(QWidget *window, Qt::Edges edges, const QPoint &globalPos) override;

    // Cursor management
    void setCursor(WId window, int cursorShape) override;
    void resetCursor(WId window) override;

    // Queries
    bool isCompositingAvailable() override;
    bool checkButtonState(Qt::MouseButton b) override;

    // XDG Portal parent window handle
    QString portalParentHandle(QWidget *parent) override;
};

#endif // X11BACKEND_H
