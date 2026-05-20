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

#ifndef CMENU_H
#define CMENU_H

#include <QWidget>
#include <QIcon>


class CMenuWidget;
class CMenu : public QObject
{
    Q_OBJECT
public:
    explicit CMenu(QWidget * parent = nullptr);
    ~CMenu();

    enum Action {
        ActionClose = 0,
        ActionCloseSaved,
        ActionCloseAll,
        ActionShowInFolder,
        ActionMoveToStart,
        ActionMoveToEnd,
        ActionUnpinTab,
        ActionPinToTab,
        ActionCreateNew,
        ACTION_COUNT
    };

    QAction* addSection(Action action, const QIcon &icon = QIcon());
    QAction* addSection(const QString &text, const QIcon &icon = QIcon());
    QAction* addSeparator();
    void setSectionIcon(Action action, const QIcon &icon);
    void setSectionEnabled(Action action, bool enabled);
    void exec(const QPoint &pos);

signals:
    void wasHidden();

private:
    static const char* m_actionText[ACTION_COUNT];
    QVector<QAction*> m_actions;
    CMenuWidget *m_menu_widget;
    QWidget *m_parent;
};

#endif // CMENU_H
