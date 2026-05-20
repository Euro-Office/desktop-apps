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


#include "cdialogcertificateinfo.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>


class CDialogCertificateInfo::Intf {
public:
    Intf(QWidget * p)
        : textInfo(new QTextEdit(p))
        , buttonOk(new QPushButton(p))
    {
        QPalette palette(textInfo->palette());
        palette.setColor(QPalette::Base, p->palette().color(QPalette::Window));
        textInfo->setPalette(palette);
        textInfo->setReadOnly(true);

        buttonOk->setText(tr("OK"));
        buttonOk->setFixedWidth(80);
        buttonOk->setAutoDefault(true);
    }

    QTextEdit * textInfo = nullptr;
    QPushButton * buttonOk = nullptr;
};

CDialogCertificateInfo::CDialogCertificateInfo(QWidget *parent)
    : QDialog(parent)
    , m_priv(new CDialogCertificateInfo::Intf(this))
{
    setMinimumWidth(500);
    setMinimumHeight(400);
    setWindowTitle(tr("Certificate Details"));

    QVBoxLayout * _layout = new QVBoxLayout;
    _layout->addWidget(m_priv->textInfo);

    QHBoxLayout * _hlayout = new QHBoxLayout;
    _hlayout->addWidget(m_priv->buttonOk);
    _layout->addLayout(_hlayout);

    connect(m_priv->buttonOk, &QPushButton::clicked, [=]{
        close();
    });

    setLayout(_layout);
}

CDialogCertificateInfo::CDialogCertificateInfo(QWidget * p, const std::wstring & info)
    : CDialogCertificateInfo(p)
{
    m_priv->textInfo->setText(QString::fromStdWString(info));
}

CDialogCertificateInfo::~CDialogCertificateInfo()
{
    delete m_priv, m_priv = nullptr;
}

