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
//  NSCefView.m
//  CefViewTest
//
//  Created by Oleg Korshul on 14.09.15.
//  Copyright (c) 2015 Ascensio System. All rights reserved.
//

#import "NSCefView.h"
#import "NSString+Extensions.h"
#import "mac_application.h"
#import "mac_cefviewmedia.h"

@interface NSCefView () {
    CCefViewWrapper* m_pCefView;
}
@end


@implementation NSCefView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        CALayer *viewLayer = [CALayer layer];
        
        CGColorRef backColor = CGColorCreateGenericRGB(1.0, 1.0, 1.0, 1.0); //RGB plus Alpha Channel
        [viewLayer setBackgroundColor:backColor];
        CGColorRelease(backColor);
        
        [self setWantsLayer:YES]; // view's backing store is using a Core Animation Layer
        [self setLayer:viewLayer];

        m_pCefView = new CCefViewMedia(self);
    }
    return self;
}

- (void)dealloc {
    [self internalClean];
}

- (CCefView *)cef {
    return m_pCefView ? m_pCefView->GetCefView() : nullptr;
}

- (void)internalClean {
    if (NULL != m_pCefView) {
        if (m_pCefView->GetCefView()) {
            m_pCefView->GetCefView()->OnDestroyWidgetImpl();
        }
        
        delete m_pCefView;
        m_pCefView = NULL;
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)setFrameOrigin:(NSPoint)newOrigin {
    [super setFrameOrigin:newOrigin];
    
    if (m_pCefView) {
        m_pCefView->moveEvent();
    }
}

- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    if (m_pCefView) {
        m_pCefView->resizeEvent();
    }
}

- (void)setExternalCloud:(NSString *)provider {
    if (m_pCefView) {
        m_pCefView->GetCefView()->SetExternalCloud([provider stdwstring]);
    }
}

- (void)setBackgroundColor:(NSColor *)color {
    CGFloat red, green, blue, alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    
    CGColorRef backColor = CGColorCreateGenericRGB(red, green, blue, 1.0); //RGB plus Alpha Channel
    [self.layer setBackgroundColor:backColor];
    CGColorRelease(backColor);
    
    if (m_pCefView) {
        return m_pCefView->SetBackgroundCefColor(255 * red, 255 * green, 255 * blue);
    }
}

- (NSInteger)uuid {
    if (m_pCefView) {
        return m_pCefView->GetCefView()->GetId();
    }
    
    return NSNotFound;
}

- (void)create:(CAscApplicationManager *)manager withType:(CefViewWrapperType)type {
    switch (type) {
        case cvwtSimple: {
            m_pCefView->m_pCefView = manager->CreateCefView(m_pCefView);
            break;
        }
        case cvwtEditor: {
            m_pCefView->m_pCefView = manager->CreateCefEditor(m_pCefView);
            break;
        }
        default:
            break;
    }
}

- (void)createReporter:(CAscApplicationManager *)manager data:(void *)pData {    
    if (m_pCefView && manager) {
        m_pCefView->m_pCefView = manager->CreateCefPresentationReporter(m_pCefView, (CAscReporterData *)pData);
    }
}

- (void)apply:(NSEditorApi::CAscMenuEvent *)event {
    if (m_pCefView) {
        m_pCefView->GetCefView()->Apply(event);
    }
}

- (void)loadWithUrl:(NSString *)url {
    if (m_pCefView) {
        m_pCefView->GetCefView()->load([url stdwstring]);
    }
}

- (void)createFileWithName:(NSString *)name type:(AscEditorType)type {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());
        
        if (editorView) {
            editorView->CreateLocalFile((AscEditorType)type, [name stdwstring]);
        }
    }
}

- (void)createFileWithNameFromTemplate:(NSString *)name tplpath:(NSString *)path {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());

        if (editorView) {
            editorView->CreateLocalFile(AscEditorType::etUndefined, [name stdwstring], [path stdwstring]);
        }
    }
}

- (void)openFileWithName:(NSString *)name type:(NSInteger)type {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());
        
        if (editorView) {
            editorView->OpenLocalFile([name stdwstring], (int)type);
        }
    }
}

- (void)openRecentFileWithId:(NSInteger)index {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());
        
        if (editorView) {
            editorView->OpenRecentFile((int)index);
        }
    }
}

- (void)openRecoverFileWithId:(NSInteger)index {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());
        
        if (editorView) {
            editorView->OpenRecoverFile((int)index);
        }
    }
}

- (void)focus {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());
        
        if (editorView) {
            editorView->focus();
        }
    }
}

- (void)reload {
    if (m_pCefView) {
        CCefViewEditor * editorView = dynamic_cast<CCefViewEditor *>(m_pCefView->GetCefView());
        
        if (editorView) {
            editorView->reload();
        }
    }
}

- (BOOL)checkCloudCryptoNeedBuild {
    if (m_pCefView && m_pCefView->GetCefView()) {
        if (m_pCefView->GetCefView()->GetType() == cvwtEditor) {
            return ((CCefViewEditor *)m_pCefView->GetCefView())->CheckCloudCryptoNeedBuild();
        }
    }
    return NO;
}

- (BOOL)checkBuilding {
    if (m_pCefView && m_pCefView->GetCefView()) {
        if (m_pCefView->GetCefView()->GetType() == cvwtEditor) {
            return ((CCefViewEditor *)m_pCefView->GetCefView())->IsBuilding();
        }
    }
    return NO;
}

- (BOOL)isSaveLocked {
    if (m_pCefView && m_pCefView->GetCefView()) {
        if (m_pCefView->GetCefView()->GetType() == cvwtEditor) {
            return ((CCefViewEditor *)m_pCefView->GetCefView())->IsSaveLocked();
        }
    }
    return NO;
}

- (NSString *)originalUrl {
    if (m_pCefView && m_pCefView->GetCefView()) {
        return [NSString stringWithstdwstring:m_pCefView->GetCefView()->GetOriginalUrl()];
    }
    return nil;
}

- (void)setParentWidgetInfoWithJson:(NSString *)jsonString {
    CCefView * cef = [self cef];
    if (cef) {
        cef->SetParentWidgetInfo([jsonString stdwstring]);
    }
}

- (void)sendCommand:(NSString *)command withParam:(NSString *)param {
    CCefView * cef = [self cef];
    if (cef) {
        NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
        pCommand->put_Command([command stdwstring]);
        pCommand->put_Param([param stdwstring]);
        
        NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pCommand;
        if (cef->GetType() == cvwtEditor ) {
            pCommand->put_FrameName(L"frameEditor");
        }
        cef->Apply(pEvent);
    }
}

@end
