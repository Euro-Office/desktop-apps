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
#include "cx11decoration.h"


class GtkMainWindow;
class CWindowPlatform : public CWindowBase, public CX11Decoration
{
public:
    explicit CWindowPlatform(const QRect&);
    virtual ~CWindowPlatform();

    void bringToTop();
    void show(bool);
    virtual void setWindowColors(const QColor&, const QColor& border = QColor(), bool isActive = false) final;
    virtual void adjustGeometry() final;

#ifndef DONT_USE_GTK_MAINWINDOW
    void move(const QPoint &pos);
    void setGeometry(const QRect &rc);
    void setWindowIcon(const QIcon &icon);
    void setWindowTitle(const QString &title) override;
    void setFocus();
    void setWindowState(Qt::WindowStates ws);
    void show();
    void showMinimized();
    void showMaximized();
    void showNormal();
    void activateWindow();
    void setMinimumSize(int w, int h);
    void hide() const;
    bool isMaximized();
    bool isMinimized();
    bool isActiveWindow();
    bool isVisible() const;
    bool isHidden() const;
    QString windowTitle() const;
    QPoint mapToGlobal(const QPoint &pt) const;
    QPoint mapFromGlobal(const QPoint &pt) const;
    QSize size() const;
    QRect geometry() const;
    QRect normalGeometry() const;
    Qt::WindowStates windowState() const;
#endif

protected:
    void onWindowActivate(bool is_active);
    virtual bool event(QEvent *event) override;
    virtual void setScreenScalingFactor(double, bool resize = true) override;
    virtual void onMaximizeEvent() override;
    virtual void onMinimizeEvent() override;
    virtual bool nativeEvent(const QByteArray&, void*, long*) final;
#ifdef DONT_USE_GTK_MAINWINDOW
    virtual void paintEvent(QPaintEvent *event) override;
#else
    virtual void applyWindowState() override;
    virtual void saveWindowState(const QString &baseKey = "") override;
#endif
    virtual void onLayoutDirectionChanged() = 0;

private:
#ifndef DONT_USE_GTK_MAINWINDOW
    GtkMainWindow *m_gtk_wnd = nullptr;
#endif
    QTimer *m_propertyTimer;
};

#endif // CWINDOWPLATFORM_H
