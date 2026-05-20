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

#include "cscalingwrapper.h"
#include "utils.h"

CScalingWrapper::CScalingWrapper(QWidget * parent)
    : CScalingWrapper( CScalingWrapper::parentScalingFactor(parent) )
{
}

CScalingWrapper::CScalingWrapper(double f)
{
    m_scaleFactor = ( f > 1 ) ? f : 1.0;
}

CScalingWrapper::~CScalingWrapper()
{

}

void CScalingWrapper::updateScalingFactor(double f)
{
    m_scaleFactor = f;
}

double CScalingWrapper::scaling() const
{
    return m_scaleFactor;
}

void CScalingWrapper::updateChildScaling(const QObject * parent, double factor)
{
    QObjectList _l = parent->children();
    if ( _l.size() ) {
        foreach ( QObject * o, _l ) {
            CScalingWrapper * _s = dynamic_cast<CScalingWrapper *>(o);
            if ( _s ) _s->updateScalingFactor(factor);

            if ( o->children().size() )
                CScalingWrapper::updateChildScaling(o, factor);
        }
    }
}

double CScalingWrapper::parentScalingFactor(const QObject * parent)
{
    if ( parent ) {
        const CScalingWrapper * _s = dynamic_cast<const CScalingWrapper *>(parent);
        if ( _s ) return _s->scaling();
        else return CScalingWrapper::parentScalingFactor(parent->parent());
    }

    return -1;
}

