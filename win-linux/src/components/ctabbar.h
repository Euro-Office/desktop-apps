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

#ifndef CTABBAR_H
#define CTABBAR_H

#include <QFrame>


class CMenu;
class CTabBar : public QFrame
{
    Q_OBJECT
public:
    CTabBar(QWidget *parent = nullptr);
    ~CTabBar();

    enum TabTheme {
        LightTab,
        DarkTab
    };

    int addTab(const QString &text);
    int addTab(const QIcon &icon, const QString &text);
    int count() const;
    int currentIndex() const;
    Qt::TextElideMode elideMode() const;
    QSize iconSize() const;
    int insertTab(int index, const QString &text);
    int insertTab(int index, const QIcon &icon, const QString &text);
    void swapTabs(int from, int to);
    void moveTab(int from, int to);
    void removeTab(int index);
    void setElideMode(Qt::TextElideMode mode);
    void setIconSize(const QSize &size);
    void setTabIconLabel(int index, QWidget *widget);
    void setTabButton(int index, QWidget *widget);
    void setTabMenu(int index, CMenu *menu);
//    void setTabData(int index, const QVariant &data);
    void setTabIcon(int index, const QIcon &icon);
    void setTabText(int index, const QString &text);
    void setTabToolTip(int index, const QString &text);
    void setCurrentIndex(int index);
    void setActiveTabColor(int index, const QString&);
    void setUseTabCustomPalette(int, bool);
    void setTabLoading(int, bool start = true, const QString& theme = QString());
    void setTabThemeType(int, TabTheme);
    void setTabThemeIcons(int, const std::pair<QString, QString> &);
    void polish();
    void activate(bool);
    void refreshTheme();
    int tabIndexAt(const QPoint &pos) const;
    QWidget* tabAtIndex(int index) const;
    QWidget* tabIconLabel(int index) const;
    QWidget* tabButton(int index) const;
    CMenu* tabMenu(int index) const;
//    QVariant tabData(int index) const;
    QIcon tabIcon(int index) const;
    QRect tabRect(int index) const;
    QString tabText(int index) const;
    QVariant tabProperty(int index, const char *name);
    virtual void tabInserted(int index);

signals:
    void currentChanged(int index);
    void onCurrentChangedByWhell(int index);
    void tabBarClicked(int index);
//    void tabBarDoubleClicked(int index);
    void tabCloseRequested(int index);
    void tabMoved(int from, int to);
    void tabsSwapped(int from, int to);
    void tabUndock(int index, bool &accepted);
    void tabMenuRequested(int index, const QPoint &pos);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual bool eventFilter(QObject*, QEvent*) override;

private:
    class CTabBarPrivate;
    CTabBarPrivate* d = nullptr;
};

#endif // CTABBAR_H
