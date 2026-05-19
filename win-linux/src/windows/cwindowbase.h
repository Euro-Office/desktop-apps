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

#ifndef CWINDOWBASE_H
#define CWINDOWBASE_H

#define WINDOW_MIN_WIDTH    520
#define WINDOW_MIN_HEIGHT   480

#define MAIN_WINDOW_MIN_WIDTH    960
#define MAIN_WINDOW_MIN_HEIGHT   661
#define MAIN_WINDOW_DEFAULT_SIZE QSize(1200,740)
#define EDITOR_WINDOW_MIN_WIDTH  920
#define SCREEN_THRESHOLD_SIZE    QSize(1366,768)

#define BUTTON_MAIN_WIDTH   112
#define MAIN_WINDOW_BORDER_WIDTH 3
#define WINDOW_TITLE_MIN_WIDTH 200
#define TOOLBTN_HEIGHT_WIN10 35
#define TOOLBTN_HEIGHT      28
#define TOOLBTN_WIDTH       40
#ifdef _WIN32
# define TITLEBTN_WIDTH     40
#else
# define TITLEBTN_WIDTH     (WindowHelper::getEnvInfo() == WindowHelper::KDE ? 24 : 28)
#endif
#define TITLE_HEIGHT        28

#include <QMainWindow>
#include <QPushButton>
#include <memory>
#include "components/celipsislabel.h"


class CPushButton;
class CWindowBase : public QMainWindow
{
public:
    explicit CWindowBase(const QRect&);
    virtual ~CWindowBase();   

    static QRect startRect(const QRect &rc, double &dpi);
    static QSize expectedContentSize(const QRect &rc, bool extended = false);
    QWidget * handle() const;
    QWidget * mainPanel() const;
    bool isCustomWindowStyle();
    void updateScaling(bool resize = true);
    virtual void adjustGeometry() = 0;
    virtual void setWindowColors(const QColor&, const QColor& border = QColor(), bool isActive = false) = 0;
    virtual void applyTheme(const std::wstring&);

protected:
    enum BtnType {
        Btn_Minimize, Btn_Maximize, Btn_Close
    };

    CPushButton* createToolButton(QWidget * parent, const QString& name);
    QWidget* createTopPanel(QWidget *parent);
    void saveWindowState(const QString &baseKey = "");
    void moveToPrimaryScreen();
    void setIsCustomWindowStyle(bool);
    virtual bool event(QEvent*);
    virtual void setScreenScalingFactor(double, bool resize = true);
    virtual void applyWindowState();
    virtual void setWindowTitle(const QString&);
    virtual void onMinimizeEvent();
    virtual void onMaximizeEvent();
    virtual void onCloseEvent();
    virtual void focus();

    QVector<CPushButton*> m_pTopButtons;
    CElipsisLabel *m_labelTitle = nullptr;
    QWidget       *m_pMainPanel = nullptr,
                  *m_boxTitleBtns = nullptr,
                  *m_pMainView = nullptr;
    double         m_dpiRatio;
    QColor         m_brdColor,
                   m_bkgColor;
    QRect          m_window_rect;
    int            m_toolbtn_height = TOOLBTN_HEIGHT;

    virtual void showEvent(QShowEvent *);

private:
    class CWindowBasePrivate;
    std::unique_ptr<CWindowBasePrivate> pimpl;
    bool  m_windowActivated;
};

#endif // CWINDOWBASE_H
