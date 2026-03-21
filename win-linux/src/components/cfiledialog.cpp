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

#include "components/cfiledialog.h"
#include <QFileDialog>
#include <QRegularExpression>
#include "defines.h"
#include "utils.h"
#include "components/cmessage.h"
#include "../Common/OfficeFileFormats.h"
#include <QList>
#include <QMimeDatabase>

#ifdef Q_OS_WIN
# include <shobjidl.h>
# include <platform_win/filechooser.h>
#else
# include "platform_linux/xdgdesktopportal.h"
# include "platform_linux/gtkfilechooser.h"
#endif

static const char *IMAGE_TYPE = "image",
                  *VIDEO_TYPE = "video",
                  *AUDIO_TYPE = "audio";

namespace CFileDialogHelper {
    auto useModalDialog() -> bool {
#if defined(__linux__) && defined(FILEDIALOG_DONT_USE_MODAL)
        return false;
#else
        return true;
#endif
    }
};


CFileDialogWrapper::CFileDialogWrapper(QWidget * parent) : QObject(parent)
{
	m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN]              = tr("All files (*.*)");

	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX]        = tr("DOCX Document") + " (*.docx)";
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX]        = tr("Document template") + " (*.dotx)";
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC]         = tr("DOC Document (*.doc)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT]         = tr("ODT Document (*.odt)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT]         = tr("OpenDocument Document template") + " (*.ott)";
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF]         = tr("RTF File (*.rtf)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT]         = tr("TXT File (*.txt)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML]        = tr("HTML File (*.html)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_MHT]         = tr("MHT File (*.mht)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB]        = tr("EPUB File (*.epub)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_FB2]         = tr("FB2 File (*.fb2)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_MOBI]        = tr("MOBI File (*.mobi)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF]   = tr("ONLYOFFICE Form Document (*.pdf)");
	m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF]       = tr("DOCXF Document (*.docxf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_MD]          = tr("Markdown File (*.md)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX]   = tr("PPTX File (*.pptx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPT]    = tr("PPT File (*.ppt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX]   = tr("Presentation template") + " (*.potx)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP]    = tr("ODP File (*.odp)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP]    = tr("OpenDocument Presentation Template") + " (*.otp)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX]   = tr("PPSX File (*.ppsx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTM]   = tr("Macro-enabled Presentation File (*.pptm)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX]    = tr("XLSX File (*.xlsx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX]    = tr("Spreadsheet template") + " (*.xltx)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTM]    = tr("Macro-enabled spreadsheet template") + " (*.xltm)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSB]    = tr("XLSB File (*.xlsb)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS]     = tr("XLS File (*.xls)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS]     = tr("ODS File (*.ods)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS]     = tr("OpenDocument Spreadsheet Template") + " (*.ots)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV]     = tr("CSV File (*.csv)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF]   = tr("PDF File (*.pdf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA]  = tr("PDFA File (*.pdf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU]  = tr("DJVU File (*.djvu)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS]   = tr("XPS File (*.xps)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_SVG]   = tr("SVG Image (*.svg)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_DRAW_VSDX]           = tr("VSDX File") + " (*.vsdx)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DRAW_VSDM]           = tr("VSDM File") + " (*.vsdm)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DRAW_VSSX]           = tr("VSSX File") + " (*.vssx)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DRAW_VSSM]           = tr("VSSM File") + " (*.vssm)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DRAW_VSTX]           = tr("VSTX File") + " (*.vstx)";
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DRAW_VSTM]           = tr("VSTM File") + " (*.vstm)";

    m_mapFilters[AVS_OFFICESTUDIO_FILE_IMAGE_JPG]           = tr("JPG Image (*.jpg *.jpeg)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_IMAGE_PNG]           = tr("PNG Image (*.png)");
}

CFileDialogWrapper::~CFileDialogWrapper()
{

}

bool CFileDialogWrapper::modalSaveAs(QString& fileName, int selected)
{
//    QString filter = tr("All files (*.*)"), ext_in;
    QString _filters, _sel_filter, _ext;

    if ( !(selected < 0) && m_mapFilters.contains(selected) )
        _sel_filter = m_mapFilters[selected];

    QFileInfo info(fileName);
    _ext = info.suffix();

    QRegularExpression reFilter("([^;]+\\(\\*\\."+_ext+"\\))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    if ( !m_filters.isEmpty() ) {
        _filters = m_filters;

        match = reFilter.match(m_filters);
        if ( match.hasMatch() ) {
            if ( _sel_filter.isEmpty() )
                _sel_filter = match.captured(1);
        } else {
            fileName = info.absoluteFilePath();
        }
    } else {
        _filters = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];

        if ( _sel_filter.isEmpty() )
            _sel_filter = getFilter(_ext);
        _filters.append(";;" + _sel_filter);
    }

#ifdef _WIN32
    // TODO: win 10 home doesn't crop file name by self. refactor for ver 7.5 with linux ver
//    QString _croped_name = fileName;
    QString _croped_name = WindowHelper::useNativeDialog() ? fileName.left(fileName.lastIndexOf('.')) : fileName;
#else
    QString _croped_name = fileName.left(fileName.lastIndexOf("."));
#endif
    reFilter.setPattern("\\(\\*(\\.\\w+)");

    auto _exec_dialog = [=] (QWidget * p, QString n, QString f, QString& sf) {
        QFileDialog::Options _opts{QFileDialog::DontConfirmOverwrite};
        const QString title = (m_title.isEmpty()) ? tr("Save As") : m_title;

        if (WindowHelper::useNativeDialog()) {
#ifdef __linux__
            QStringList result;
            if (WindowHelper::useGtkDialog()) {
                result = Gtk::openGtkFileChooser(p, Gtk::Mode::SAVE, title,
                                                 n, "", f, &sf);
            } else {
                result = Xdg::openXdgPortal(p, Xdg::Mode::SAVE, title,
                                            n, "", f, &sf);
            }
            return (result.size() > 0) ? result.at(0) : QString();
#else
# ifndef __OS_WIN_XP
            QStringList result;
            result = Win::openWinFileChooser(p, Win::Mode::SAVE, title,
                                             n, "", f, &sf);
            return (result.size() > 0) ? result.at(0) : QString();
# endif
#endif
        } else _opts |= QFileDialog::DontUseNativeDialog;
        return QFileDialog::getSaveFileName(p, title, n, f, &sf, _opts);
    };

    QWidget * _parent = CFileDialogHelper::useModalDialog() ?
                (QWidget *)parent() : nullptr;
#ifndef _WIN32
    WindowHelper::CParentDisable oDisabler(_parent);
#endif

    while (true) {
        fileName = _exec_dialog(_parent, _croped_name, _filters, _sel_filter);

        if ( !fileName.isEmpty() ) {
            match = reFilter.match(_sel_filter);

            if ( match.hasMatch() ) {
                _ext = match.captured(1);

                if (!fileName.endsWith(_ext))
                    fileName.append(_ext);
            }

            QFileInfo info(fileName);
            if ( info.exists() ) {
                QWidget * _mess_parent = (QWidget *)parent();
                int _answ = CMessage::showMessage(_mess_parent,
                                                  tr("%1 already exists.<br>Do you want to replace it?").arg(info.fileName().toHtmlEscaped()),
                                                  MsgType::MSG_WARN, MsgBtns::mbYesNo);
                if ( MODAL_RESULT_NO == _answ ) {
                    continue;
                } else
                if ( MODAL_RESULT_YES != _answ ) {
                    fileName.clear();
                }
            }
        }

        break;
    }

    m_format = 0;
    if (m_filters.length()) {
        m_format = getKey(_sel_filter);
    }

    return fileName.length() > 0;
}

QString CFileDialogWrapper::getFilter(const QString& extension) const
{
    QString out = extension.toLower();
    if (extension.contains(QRegularExpression("^docx?$"))) {
        return tr("Word Document") + " (*." + out +")";
    } else
    if (extension.contains(QRegularExpression("^xlsx?$"))) {
        return tr("Excel Workbook") + " (*." + out + ")";
    } else
    if (extension.contains(QRegularExpression("^pptx?$"))) {
        return tr("PowerPoint Presentation") + " (*." + out + ")";
    } else {
        out.replace(0, 1, extension.left(1).toUpper());
        return tr("%1 File (*.%2)").arg(out, out.toLower());
    }
}

QStringList CFileDialogWrapper::modalOpen(const QString& path, const QString& filter, QString * selected, bool multi)
{
    QString _filter_ = filter;
    QString _sel_filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    if ( _filter_.isEmpty() ) {
//        _filter_ = joinFilters();
        _filter_ =  tr("Text documents") +
#ifndef __LOCK_OFORM_FORMATS
                        " (*.docx *.doc *.odt *.ott *.rtf *.docm *.dot *.dotx *.dotm *.fb2 *.fodt *.hml *.wps *.wpt *.xml *.pdf *.djv *.djvu *.md *.docxf *.oform *.sxw *.stw *.xps *.oxps *.pages *.hwp *.hwpx);;" +
#else
                        " (*.docx *.doc *.odt *.ott *.rtf *.docm *.dot *.dotx *.dotm *.fb2 *.fodt *.hml *.wps *.wpt *.xml *.pdf *.djv *.djvu *.md *.sxw *.stw *.xps *.oxps);;" +
#endif
                    tr("Spreadsheets") + " (*.xlsx *.xls *.xlsm *.xlsb *.ods *.ots *.xltx *.xltm *.xml *.fods *.et *.ett *.sxc *.numbers);;" +
                    tr("Presentations") + " (*.pptx *.ppt *.odp *.odg *.otp *.ppsm *.pptm *.ppsx *.pps *.potx *.pot *.potm *.fodp *.dps *.dpt *.sxi *.key);;" +
                    tr("Visio diagram") + " (*.vsdx *.vssx *.vstx *.vsdm *.vssm *.vstm);;" +
                    tr("Web Page") + " (*.html *.htm *.mht *.mhtml *.epub);;" +
                    tr("Text files") + " (*.txt *.csv *.tsv)";
//#ifdef __linux__
        _sel_filter = tr("All supported files") + " " + joinExtentions(_filter_);
        _filter_.prepend(_sel_filter + ";;");
//#endif
        _filter_.append(";;" + m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN]);
    } else
    if (!_filter_.contains(_sel_filter)) {
        int pos = _filter_.indexOf(";;");
        _sel_filter = (pos == -1) ? _filter_ : _filter_.mid(0, pos);
    }
    if (selected && _filter_.contains(*selected))
        _sel_filter = *selected;

    QWidget * _parent = CFileDialogHelper::useModalDialog() ?
                (QWidget *)parent() : nullptr;
    QFileDialog::Options _opts;       
#ifdef __linux__
    WindowHelper::CParentDisable oDisabler(_parent);
#endif
    const QString title = (m_title.isEmpty()) ? tr("Open Document") : m_title;

    if (WindowHelper::useNativeDialog()) {
#ifdef __linux__
        if (WindowHelper::useGtkDialog()) {
            return Gtk::openGtkFileChooser(_parent, Gtk::Mode::OPEN, title, "",
                                           path, _filter_, &_sel_filter, multi);
        } else {
            return Xdg::openXdgPortal(_parent, Xdg::Mode::OPEN, title, "",
                                      path, _filter_, &_sel_filter, multi);
        }    
#else
# ifndef __OS_WIN_XP
    return Win::openWinFileChooser(_parent, Win::Mode::OPEN, title, "",
                                   path, _filter_, &_sel_filter, multi);
# endif
#endif
    } else _opts |= QFileDialog::DontUseNativeDialog;
    return multi ? QFileDialog::getOpenFileNames(_parent, title, path, _filter_, &_sel_filter, _opts) :
                QStringList(QFileDialog::getOpenFileName(_parent, title, path, _filter_, &_sel_filter, _opts));
}

QString CFileDialogWrapper::modalOpenSingle(const QString& path, const QString& filter, QString * selected)
{
    QStringList _list = modalOpen(path, filter, selected, false);
    return _list.isEmpty() ? QString() : _list.at(0);
}

QStringList CFileDialogWrapper::modalOpenImage(const QString& path)
{
    QString selected = tr("All Images") + " (*.jpeg *.jpg *.png *.gif *.bmp *.tiff *.tif *.heif *.heic *.svg)";
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.append(";;" + selected + ";;" + tr("Jpeg (*.jpeg *.jpg);;Png (*.png);;Gif (*.gif);;Bmp (*.bmp);;Tiff (*.tiff *.tif);;Heif (*.heif *.heic)"));
    filter.append(";;" + m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_SVG]);

    auto result = modalOpen(path, filter, &selected, false);
    checkForMimeTypes(result, IMAGE_TYPE);
    return result;
}

QStringList CFileDialogWrapper::modalOpenImages(const QString& path)
{
    QString selected = tr("All Images") + " (*.jpeg *.jpg *.png *.gif *.bmp *.heif *.heic *.svg)";
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.append(";;" + selected + ";;" + tr("Jpeg (*.jpeg *.jpg);;Png (*.png);;Gif (*.gif);;Bmp (*.bmp);;Heif (*.heif *.heic)"));
    filter.append(";;" + m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_SVG]);

    auto result = modalOpen(path, filter, &selected, true);
    checkForMimeTypes(result, IMAGE_TYPE);
    return result;
}

QStringList CFileDialogWrapper::modalOpenPlugin(const QString& path)
{
    QString _filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    QString _plugins_filter = tr("Plugin file (*.plugin)");
    _filter.append(";;" + _plugins_filter);

    return modalOpen(path, _filter, &_plugins_filter, false);
}

QStringList CFileDialogWrapper::modalOpenPlugins(const QString& path)
{
    QString _filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    QString _plugins_filter = tr("Plugin file (*.plugin)");
    _filter.append(";;" + _plugins_filter);

    return modalOpen(path, _filter, &_plugins_filter, true);
}

QStringList CFileDialogWrapper::modalOpenDocuments(const QString& path, bool multi)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Text documents") + " (*.docx *.doc *.odt *.ott *.rtf *.docm *.dotx *.dotm *.fb2 *.fodt *.hml *.wps *.wpt *.xml);;");

    return modalOpen(path, filter, &filter, multi);
}

QStringList CFileDialogWrapper::modalOpenSpreadsheets(const QString& path, bool multi)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Spreadsheets") + " (*.xlsx *.xls *.ods *.ots *.csv *.tsv *.xltx *.xltm *.fods *.et *.ett);;");

    return modalOpen(path, filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenPresentations(const QString& path, bool multi)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Presentations") + " (*.pptx *.ppt *.odp *.otp *.ppsm *.ppsx *.pps *.potx *.pot *.potm *.fodp *.dps *.dpt);;");

    return modalOpen(path, filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenForEncrypt(const QString& path, bool multi)
{
#ifndef __LOCK_OFORM_FORMATS
    QString _filter = tr("Text documents") + " (*.docx *.docxf *.docm *.dotm *.dotx *.oform);;" +
#else
    QString _filter = tr("Text documents") + " (*.docx *.docm *.dotm *.dotx);;" +
#endif
                        tr("Spreadsheets") + " (*.xlsx *.xlsm *.xltm *.xltx);;" +
                        tr("Presentations") + " (*.potm *.potx *.ppsm *.pptm *.ppsx *.pptx)";

    const QString _all_supported = tr("All supported files") + " " + joinExtentions(_filter);
    _filter.prepend(_all_supported + ";;");
    _filter.append(";;" + m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN]);

    return modalOpen(path, _filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenAny(const QString& path, bool multi)
{
    QString _filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    return modalOpen(path, _filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenMedia(const QString& type, const QString& path, bool multi)
{
    QString selected, extra;
    if ( type == VIDEO_TYPE ) {
        selected = tr("Video file") + " (*.mp4 *.mkv *.avi *.mpg *.mpeg *.mpe *.mpv *.mov *.wmv *.m2v *.m4v *.webm *.ogg *.f4v *.m2ts *.mts *.ts)";
        extra = "Avi (*.avi);;Mpeg (*.mpg *.mpeg *.mpe *.mpv *.m2v *.m4v *.mp4);;Mkv (*.mkv);;Mts (*.m2ts *.mts);;Webm (*.webm);;Mov (*.mov)"
                                      ";;Wmv (*.wmv);;F4v (*.f4v);;Ogg (*.ogg)";
    } else
    if ( type == AUDIO_TYPE ) {
        selected = tr("Audio file") + " (*.mp3 *.mp2 *.ogg *.wav *.wma *.flac *.ape *.aac *.m4a)";
        extra = "Mp3 (*.mp3);;Mp2 (*.mp2);;Wav (*.wav);;Flac (*.flac);;Wma (*.wma);;Ogg (*.ogg);;Ape (*.ape);;Aac (*.aac);;M4a (*.m4a)";
    }

    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN] + ";;" + selected + ";;" + extra;
    auto result = modalOpen(path, filter, &selected, multi);
    checkForMimeTypes(result, type);
    return result;
}

QString CFileDialogWrapper::selectFolder(const QString& folder)
{
    QWidget * _parent = CFileDialogHelper::useModalDialog() ?
                (QWidget *)parent() : nullptr;
    QFileDialog::Options _opts{QFileDialog::ShowDirsOnly};

#ifdef __linux__
    WindowHelper::CParentDisable oDisabler(_parent);
#endif
    const QString title = (m_title.isEmpty()) ? tr("Select Folder") : m_title;

    if (WindowHelper::useNativeDialog()) {
#ifdef __linux__
        QStringList result;
        if (WindowHelper::useGtkDialog()) {
            result = Gtk::openGtkFileChooser(_parent, Gtk::Mode::FOLDER, title,
                                             "", folder, "", nullptr);
        } else {
            result = Xdg::openXdgPortal(_parent, Xdg::Mode::FOLDER, title,
                                        "", folder, "", nullptr);
        }
        return (result.size() > 0) ? result.at(0) : QString();
#else
# ifndef __OS_WIN_XP
        QStringList result;
        result = Win::openWinFileChooser(_parent, Win::Mode::FOLDER, title,
                                         "", folder, "", nullptr);
        return (result.size() > 0) ? result.at(0) : QString();
# endif
#endif
    } else _opts |= QFileDialog::DontUseNativeDialog;
    return QFileDialog::getExistingDirectory(_parent, title, folder, _opts);
}

void CFileDialogWrapper::setTitle(const QString &title)
{
    m_title = title;
}

void CFileDialogWrapper::setFormats(std::vector<int>& vf)
{
    m_filters.clear();

    if ( vf.size() ) {
        std::vector<int>::iterator i = vf.begin();
        m_filters = m_mapFilters.value(*(i++));
        while (i != vf.end()) {
            m_filters += ";;" + m_mapFilters.value(*(i++));
        }
    }
}

int CFileDialogWrapper::getKey(const QString &value)
{
#ifdef Q_OS_LINUX
    QString _sv{value};
    if ( WindowHelper::getEnvInfo() == WindowHelper::GNOME ) {
        QRegularExpression _re_strbegin("^(.+)\\s\\（", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch _re_match = _re_strbegin.match(value);

        if ( _re_match.hasMatch() ) {
            _sv = _re_match.captured(1);
        }
    }

    foreach (QString v, m_mapFilters) {
        if (v.startsWith(_sv))
           return m_mapFilters.key(v);
    }
#else
    foreach (QString v, m_mapFilters) {
        if (v == value)
           return m_mapFilters.key(value);
    }
#endif
    return -1;
}

void CFileDialogWrapper::checkForMimeTypes(QStringList &files, const QString &type)
{
    QMimeDatabase mdb;
    foreach (const auto &filePath, files) {
        QMimeType mt = mdb.mimeTypeForFile(filePath);
        if (!mt.name().startsWith(type)) {
            files.clear();
            QWidget *_parent = CFileDialogHelper::useModalDialog() ? (QWidget*)parent() : nullptr;
            CMessage::warning(_parent, (type == IMAGE_TYPE) ? tr("Unknown image format.") :
                                       (type == VIDEO_TYPE) ? tr("Unknown video format.") :
                                       (type == AUDIO_TYPE) ? tr("Unknown audio format.") : tr("Unknown format."));
            break;
        }
    }
}

int CFileDialogWrapper::getFormat()
{
    return m_format;
}

//QString CFileDialogWrapper::joinFilters() const
//{
//    auto _get_all_exts = [] (const QList<QString>& l) {
//        QRegExp re("[\\w\\s]+\\((\\*\\.\\w+)\\)");
//        QString extns;
//        for ( auto f : l ) {
//            if ( !(re.indexIn(f) < 0) ) {
//                if ( !extns.isEmpty() )
//                    extns.append(" ");

//                extns.append( re.cap(1) );
//            }
//        }

//        return extns;
//    };

//    QString _out;
//    QList<QString> _vl(m_mapFilters.values());
////    _vl.insert(1, tr("All supported documents") + " (" + _get_all_exts(_vl) + ")");
//    for ( auto f : _vl ) {
//        if ( !_out.isEmpty() )
//            _out.append(";;");

//        _out.append(f);
//    }

//    return _out;
//}

QString CFileDialogWrapper::joinExtentions(const QString &filter) const
{
    QString _out;
    foreach (QString str, filter.split(";;")) {
        const int start = str.indexOf('(');
        const int end = str.indexOf(')');
        if (start != -1 && start < end)
            _out += str.mid(start + 1, end - start - 1) + " ";
    }
    const int pos = _out.lastIndexOf(' ');
    if (pos != -1)
        _out = _out.mid(0, pos);
    if (!_out.isEmpty()) {
        _out.prepend('(');
        _out.append(')');
    }
    return _out;
}
