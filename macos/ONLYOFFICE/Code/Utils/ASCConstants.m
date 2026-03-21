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

//
//  ASCConstants.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/10/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASCConstants.h"
#import "OfficeFileFormats.h"
#import "ASCExternalController.h"

@implementation ASCConstants

+ (id)shared {
    static ASCConstants * sharedManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedManager = [[self alloc] init];
    });
    return sharedManager;
}

+ (NSArray *)images {
    return @[@"jpg", @"jpeg", @"png", @"gif", @"bmp", @"tif", @"tiff", @"heif", @"heic", @"ico", @"svg"];
}

+ (NSArray *)videos {
    return @[@"webm", @"mkv", @"flv", @"ogg", @"avi", @"mov", @"wmv", @"mp4", @"m4v", @"mpg", @"mp2", @"mpeg",
             @"mpe", @"mpv", @"m2v", @"m4v", @"3gp", @"3g2", @"f4v", @"m2ts", @"mts", @"ts"];
}

+ (NSArray *)audios {
    return @[@"flac", @"mp3", @"ogg", @"wav", @"wma", @"ape", @"aac", @"m4a", @"alac"];
}

+ (NSArray *)documents {
    return @[@"docx", @"doc", @"odt", @"ott", @"rtf", @"docm", @"dot", @"dotx", @"dotm", @"docxf", @"fodt", @"wps", @"wpt",
             @"xml", @"pdf", @"epub", @"djv", @"djvu", @"txt", @"html", @"htm", @"mht", @"mhtml", @"xps", @"doctx",
             @"fb2", @"oform", @"sxw", @"stw", @"md", @"pages", @"hwp", @"hwpx", @"md", @"hml"];
}

+ (NSArray *)spreadsheets {
    return @[@"xls", @"xlsx", @"xlsm", @"xlsb", @"csv", @"tsv", @"ods", @"xltx", @"ots", @"xltm", @"xml", @"fods", @"et", @"ett", @"sxc",@"numbers"];
}

+ (NSArray *)presentations {
    return @[@"ppt", @"pptx", @"pptm", @"ppsx", @"odg", @"odp", @"potx", @"otp", @"pps", @"ppsm", @"potm", @"fodp", @"dps", @"dpt", @"sxi",@"key"];
}

+ (NSArray *)draws {
    return @[@"vsdx",@"vssx",@"vstx",@"vsdm",@"vssm",@"vstm"];
}

+ (NSArray *)cancryptformats {
    return @[@"docm", @"docx", @"docxf", @"dotm", @"dotx", @"oform", @"potm", @"potx", @"ppsm", @"pptm", @"ppsx", @"pptx", @"xlsm", @"xlsx", @"xltm", @"xltx"];
}

+ (NSArray *)plugins {
    return @[@"plugin", @"zip"];
}

+ (NSArray *)csvtxt {
    return @[@"csv", @"txt"];
}

+ (NSArray *)xmldata {
    return @[@"xml"];
}

+ (NSString *)appInfo:(NSString *)key {
    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppInfo:)]) {
        return [externalDelegate onAppInfo:key];
    }

    NSDictionary * appInfo = @{
                               kRegHelpUrl: @"https://onlyoffice.com/desktopeditors.aspx",
                               kHelpUrl: @"http://helpcenter.onlyoffice.com/%@ONLYOFFICE-Editors/index.aspx",
                               kRegistrationPortalUrl: @"https://onlyoffice.com/registration.aspx?desktop=true"
                               };

    return appInfo[key];
}

+ (NSDictionary *)ascFormatsInfo {
    return @{
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX): @{
                     @"description" : NSLocalizedString(@"Word 2007 Document", nil),
                     @"extension"   : @"docx"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC): @{
                     @"description" : NSLocalizedString(@"Word 97-2003 Document", nil),
                     @"extension"   : @"doc"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT): @{
                     @"description" : NSLocalizedString(@"OpenOffice Document", nil),
                     @"extension"   : @"odt"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF): @{
                     @"description" : NSLocalizedString(@"Rich Text Document", nil),
                     @"extension"   : @"rtf"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT): @{
                     @"description" : NSLocalizedString(@"Plain Text", nil),
                     @"extension"   : @"txt"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML): @{
                     @"description" : NSLocalizedString(@"Web Page", nil),
                     @"extension"   : @"html"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_MHT): @{
                     @"description" : NSLocalizedString(@"Web Page", nil),
                     @"extension"   : @"mht"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB): @{
                     @"description" : NSLocalizedString(@"Mobipocket e-book", nil),
                     @"extension"   : @"epub"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_MD): @{
                     @"description" : NSLocalizedString(@"Markdown File", nil),
                     @"extension"   : @"md"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX): @{
                     @"description" : NSLocalizedString(@"Document template", nil),
                     @"extension"   : @"dotx"
                     },

             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX): @{
                     @"description" : NSLocalizedString(@"PowerPoint 2007 Presentation", nil),
                     @"extension"   : @"pptx"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPT): @{
                     @"description" : NSLocalizedString(@"PowerPoint 97-2003 Presentation", nil),
                     @"extension"   : @"ppt"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTM): @{
                     @"description" : NSLocalizedString(@"Macro-enabled Presentation", nil),
                     @"extension"   : @"pptm"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP): @{
                     @"description" : NSLocalizedString(@"OpenOffice Presentation", nil),
                     @"extension"   : @"odp"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX): @{
                     @"description" : NSLocalizedString(@"PowerPoint Slide Show", nil),
                     @"extension"   : @"ppsx"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX): @{
                     @"description" : NSLocalizedString(@"Presentation template", nil),
                     @"extension"   : @"potx"
                     },

             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX): @{
                     @"description" : NSLocalizedString(@"Excel 2007 Spreadsheet", nil),
                     @"extension"   : @"xlsx"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSB): @{
                     @"description" : NSLocalizedString(@"Excel Binary Workbook", nil),
                     @"extension"   : @"xlsb"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS): @{
                     @"description" : NSLocalizedString(@"Excel 97-2003 Spreadsheet", nil),
                     @"extension"   : @"xls"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS): @{
                     @"description" : NSLocalizedString(@"OpenOffice Spreadsheet", nil),
                     @"extension"   : @"ods"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV): @{
                     @"description" : NSLocalizedString(@"Comma-Separated Values", nil),
                     @"extension"   : @"csv"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX): @{
                     @"description" : NSLocalizedString(@"Spreadsheet template", nil),
                     @"extension"   : @"xltx"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTM): @{
                     @"description" : NSLocalizedString(@"Macro-enabled spreadsheet template", nil),
                     @"extension"   : @"xltm"
                     },

             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF): @{
                     @"description" : NSLocalizedString(@"PDF File", nil),
                     @"extension"   : @"pdf"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA): @{
                     @"description" : NSLocalizedString(@"PDF/A File", nil),
                     @"extension"   : @"pdf"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU): @{
                     @"description" : NSLocalizedString(@"DjVu File", nil),
                     @"extension"   : @"djvu"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS): @{
                     @"description" : NSLocalizedString(@"XML Paper Specification", nil),
                     @"extension"   : @"xps"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT): @{
                     @"description" : NSLocalizedString(@"OpenDocument Document Template", nil),
                     @"extension"   : @"ott"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP): @{
                     @"description" : NSLocalizedString(@"OpenDocument Presentation Template", nil),
                     @"extension"   : @"otp"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS): @{
                     @"description" : NSLocalizedString(@"OpenDocument Spreadsheet Template", nil),
                     @"extension"   : @"ots"
             },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_FB2): @{
                     @"description" : NSLocalizedString(@"FictionBook File", nil),
                     @"extension"   : @"fb2"
             },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF): @{
                     @"description" : NSLocalizedString(@"ONLYOFFICE Document Form Template", nil),
                     @"extension"   : @"docxf"
             },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM): @{
                     @"description" : NSLocalizedString(@"ONLYOFFICE Fillable Online Form", nil),
                     @"extension"   : @"oform"
             },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF): @{
                     @"description" : NSLocalizedString(@"ONLYOFFICE Form Document", nil),
                     @"extension"   : @"pdf"
             },
             @(AVS_OFFICESTUDIO_FILE_IMAGE_PNG): @{
                     @"description" : NSLocalizedString(@"PNG Image", nil),
                     @"extension"   : @"png"
             },
             @(AVS_OFFICESTUDIO_FILE_IMAGE_JPG): @{
                     @"description" : NSLocalizedString(@"JPG Image", nil),
                     @"extension"   : @"jpg"
             },
             @(AVS_OFFICESTUDIO_FILE_DRAW_VSDX): @{
                     @"description" : NSLocalizedString(@"VSDX File", nil),
                     @"extension"   : @"vsdx"
             }
    };
}

@end
