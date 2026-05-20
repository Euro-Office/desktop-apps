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

#include "cproviders.h"
#include <QVector>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>


struct ProviderData {
    QString provider, editorPage;
    bool hasFrame = false,
         useRegex = false;
};

class CProviders::CProvidersPrivate
{
public:
    QVector<ProviderData> m_provid_vec;
};

CProviders::CProviders() :
    pimpl(new CProvidersPrivate)
{}

CProviders::~CProviders()
{
    delete pimpl, pimpl = nullptr;
}

CProviders& CProviders::instance()
{
    static CProviders inst;
    return inst;
}

void CProviders::init(const QString &prvds_json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(prvds_json.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        const QJsonArray arr = doc.array();
        for (const auto &val : arr) {
            QJsonObject obj = val.toObject();
            ProviderData pd;
            pd.provider = obj["provider"].toString().toLower();
            pd.hasFrame = obj["editorFrameSize"].toString() == "finite";
            pd.editorPage = obj["editorPage"].toString();
            QString reg("regex:");
            int ind = pd.editorPage.indexOf(reg);
            if (ind != -1) {
                pd.useRegex = true;
                pd.editorPage = pd.editorPage.mid(ind + reg.length());
            }
            pimpl->m_provid_vec.push_back(std::move(pd));
        }
    }
}

bool CProviders::editorsHasFrame(const QString &url, const QString &cloud)
{
    foreach (const auto &pd, pimpl->m_provid_vec) {
        if (!pd.provider.isEmpty() && pd.provider == cloud)
            return pd.hasFrame;
        if (!pd.editorPage.isEmpty()) {
            if (pd.useRegex) {
                QRegularExpression rgx(pd.editorPage, QRegularExpression::CaseInsensitiveOption);
                if (rgx.match(url).hasMatch())
                    return pd.hasFrame;
            } else
            if (url.indexOf(pd.editorPage) != -1)
                return pd.hasFrame;
        }
    }
    return false;
}
