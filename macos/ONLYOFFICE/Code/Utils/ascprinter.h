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

#ifndef NSASCPRINTER_H
#define NSASCPRINTER_H

#include "applicationmanager.h"
#import <Cocoa/Cocoa.h>

class ASCPrinterInfo
{
public:
	double  m_dDpiX;
	double  m_dDpiY;

	int     m_nMarginLeft;
	int     m_nMarginRight;
	int     m_nMarginTop;
	int     m_nMarginBottom;

	int     m_nPaperWidth;
	int     m_nPaperHeight;

	int     m_nPagesCount;

	int     m_nPaperWidthOrigin;
	int     m_nPaperHeightOrigin;

	bool	m_bIsPrintingOnScreen;		// true if printing to preview panel on screen

	NSEditorApi::CAscPrinterContextBase* m_pContext;

public:
	ASCPrinterInfo()
	{
		m_dDpiX = 72.0;
		m_dDpiY = 72.0;

		m_nPaperWidth   = 1000;
		m_nPaperHeight  = 1000;

		m_nMarginLeft   = 0;
		m_nMarginRight  = 0;
		m_nMarginTop    = 0;
		m_nMarginBottom = 0;

		m_nPagesCount   = 0;
		m_pContext      = NULL;

		m_nPaperWidthOrigin = 0;
		m_nPaperHeightOrigin = 0;

		m_bIsPrintingOnScreen = false;
	}
};

@interface ASCPrintView : NSView
{
	CAscApplicationManager* m_pManager;
	CCefView*               m_pView;
	ASCPrinterInfo *        m_pPrinterInfo;

	std::vector<bool>       m_arOrientation; // true - Horizontal
	bool					m_isFirstPrint;	 // true if it is first printing process for current print operation
}

- (id) initWithParams:(CGRect)frame manager:(CAscApplicationManager*)_manager viewId:(int)_viewId info:(ASCPrinterInfo*)_info;
- (void) fillInfo;
- (void) recalcPageSizes;
@end

@implementation ASCPrintView

- (id) initWithParams:(CGRect)frame manager:(CAscApplicationManager*)_manager viewId:(int)_viewId info:(ASCPrinterInfo*)_info;
{
	self = [super initWithFrame:frame];
	if (self)
	{
		m_pManager      = _manager;
		m_pView         = m_pManager->GetViewById(_viewId);
		m_pPrinterInfo  = _info;
		m_isFirstPrint	= true;
	}
	return self;
}

- (void) drawRect:(NSRect)dirtyRect
{
	m_pPrinterInfo->m_pContext->AddRef();

	NSEditorApi::CAscPrintPage* pData = new NSEditorApi::CAscPrintPage();
	pData->put_Context(m_pPrinterInfo->m_pContext);

	int nPage = (int)[[NSPrintOperation currentOperation] currentPage] - 1;
	pData->put_Page(nPage);

	NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
	pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE;
	pEvent->m_pData = pData;

	m_pView->Apply(pEvent);
}

- (BOOL)knowsPageRange:(NSRangePointer)range
{
	NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];

	[pInfo setHorizontallyCentered:NO];
	[pInfo setVerticallyCentered:NO];
	[pInfo setHorizontalPagination:NSAutoPagination];
	[pInfo setVerticalPagination:NSAutoPagination];
	[pInfo setLeftMargin:0];
	[pInfo setRightMargin:0];
	[pInfo setTopMargin:0];
	[pInfo setBottomMargin:0];

	NSPaperOrientation prevOrientation = pInfo.orientation;
	bool areAllHor = true;	// true if all pages are in landscape (horizontal) orientation
	// calculate page width and height in PORTRAIT orientation
	[pInfo setOrientation:NSPaperOrientationPortrait];
	[self fillInfo];
	m_arOrientation.resize(m_pPrinterInfo->m_nPagesCount);
	for (int nPage = 0; nPage < m_pPrinterInfo->m_nPagesCount; nPage++)
	{
		NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE_CHECK);
		NSEditorApi::CAscPrintPage* pData = new NSEditorApi::CAscPrintPage();
		pData->put_Context(m_pPrinterInfo->m_pContext);
		m_pPrinterInfo->m_pContext->AddRef();

		pData->put_Page(nPage);

		pEvent->m_pData = pData;
		m_pView->Apply(pEvent);

		m_arOrientation[nPage] = pData->get_IsRotate();
		if (areAllHor && !pData->get_IsRotate())
			areAllHor = false;

		pEvent->Release();
	}

	range->location = 1;
	range->length   = m_pPrinterInfo->m_nPagesCount;

	if (m_isFirstPrint)
	{
		// apply landscape orientation right away if all pages are in landscape orientation
		if (areAllHor && m_pPrinterInfo->m_nPagesCount > 0)
			[pInfo setOrientation: NSPaperOrientationLandscape];
		m_isFirstPrint = false;
	}
	else
	{
		// restore previously set page orientation
		[pInfo setOrientation:prevOrientation];
	}

	return YES;
}

- (void) fillInfo
{
	NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];

	PMPrintSession _session     = (PMPrintSession)[pInfo PMPrintSession];
	PMPrintSettings _settings   = (PMPrintSettings)[pInfo PMPrintSettings];

	PMPrinter _printer;
	OSStatus err = PMSessionGetCurrentPrinter(_session, &_printer);

	PMResolution _resolution;
	err = PMPrinterGetOutputResolution(_printer, _settings, &_resolution);
	if (err == 0)
	{
		m_pPrinterInfo->m_dDpiX = _resolution.hRes;
		m_pPrinterInfo->m_dDpiY = _resolution.vRes;
	}
	else
	{
		UInt32 _res_count = 0;
		PMPrinterGetPrinterResolutionCount(_printer, &_res_count);

		PMResolution* pResolutions = (PMResolution*)malloc(_res_count * sizeof(PMResolution));

		for (UInt32 i = 1; i <= _res_count; ++i)
		{
			PMPrinterGetIndexedPrinterResolution(_printer, i, pResolutions + i - 1);
		}

		m_pPrinterInfo->m_dDpiX = pResolutions[0].hRes;
		m_pPrinterInfo->m_dDpiY = pResolutions[0].vRes;

		free(pResolutions);
	}

	[self recalcPageSizes];
}

// Recalculate actual page sizes and margins taking into account orientation.
- (void) recalcPageSizes
{
	NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];

	double dKoefX = m_pPrinterInfo->m_dDpiX / 72.0;
	double dKoefY = m_pPrinterInfo->m_dDpiY / 72.0;

	m_pPrinterInfo->m_nPaperWidthOrigin = (int)pInfo.paperSize.width;
	m_pPrinterInfo->m_nPaperHeightOrigin = (int)pInfo.paperSize.height;

	m_pPrinterInfo->m_nPaperWidth = (int)(dKoefX * pInfo.paperSize.width);
	m_pPrinterInfo->m_nPaperHeight = (int)(dKoefY * pInfo.paperSize.height);

	NSRect _rectMargins = pInfo.imageablePageBounds;

	m_pPrinterInfo->m_nMarginLeft = (int)(dKoefX * _rectMargins.origin.x);
	m_pPrinterInfo->m_nMarginRight = (int)(dKoefX * (m_pPrinterInfo->m_nPaperWidthOrigin - (_rectMargins.origin.x + _rectMargins.size.width)));
	m_pPrinterInfo->m_nMarginTop = (int)(dKoefY * (m_pPrinterInfo->m_nPaperHeightOrigin - (_rectMargins.origin.y + _rectMargins.size.height)));
	m_pPrinterInfo->m_nMarginBottom = (int)(dKoefY * _rectMargins.origin.y);
}

- (NSRect)rectForPage:(NSInteger)page
{
	int nPage = (int)(page - 1);
	if (nPage < m_pPrinterInfo->m_nPagesCount)
	{
		NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];
		// check if printing on screen
		if (nPage == 0)
		{
			PMPrintSession printSession = (PMPrintSession)[pInfo PMPrintSession];
			PMPrintSettings printSettings = (PMPrintSettings)[pInfo PMPrintSettings];

			PMDestinationType destType;
			PMSessionGetDestinationType(printSession, printSettings, &destType);

			CFURLRef destLocation;
			PMSessionCopyDestinationLocation(printSession, printSettings, &destLocation);

			m_pPrinterInfo->m_bIsPrintingOnScreen = (destType == kPMDestinationPrinter ||
													 (destType == kPMDestinationFile && destLocation == NULL));
		}
		// apply page orientation only when not printing to screen
		if (!m_pPrinterInfo->m_bIsPrintingOnScreen)
		{
			[pInfo setOrientation: (m_arOrientation[nPage]) ? NSPaperOrientationLandscape : NSPaperOrientationPortrait];
		}
		// update page sizes and margins
		[self recalcPageSizes];
	}

	NSRect R = NSMakeRect( 0, 0, m_pPrinterInfo->m_nPaperWidth, m_pPrinterInfo->m_nPaperHeight );
	[self setFrame: R];
	return R;
}

- (NSPoint)locationOfPrintRect:(NSRect)rect
{
	// This method makes sure that we center the view on the page to remove unprintable margins
	return NSMakePoint((m_pPrinterInfo->m_nPaperWidth - rect.size.width) / 2.0, (m_pPrinterInfo->m_nPaperHeight - rect.size.height) / 2.0);
}

@end

class ASCPrinterContext : public NSEditorApi::CAscPrinterContextBase
{
	CAscApplicationManager* m_pManager;
	ASCPrinterInfo m_oInfo;
	NSView* m_pView;
	CCefView* m_pParent;

public:
	static bool isCurrentlyPrinting;

public:
	ASCPrinterContext(CAscApplicationManager* pManager) : NSEditorApi::CAscPrinterContextBase()
	{
		m_pManager = pManager;
		m_pView = nil;
		m_pParent = NULL;
	}

	void BeginPaint(NSDictionary * info, id sender, SEL didRunSelector)
	{
		isCurrentlyPrinting = true;

		int viewId          = [info[@"viewId"] intValue];
		int pageCurrent     = [info[@"currentPage"] intValue];

		m_oInfo.m_nPagesCount = [info[@"countPages"] intValue];
		m_oInfo.m_pContext = this;
		m_pView = [[ASCPrintView alloc] initWithParams:NSMakeRect(0, 0, 100, 100) manager:m_pManager viewId:viewId info:&m_oInfo];
		m_pParent = m_pManager->GetViewById(viewId);

		NSLog(@"page from %d to %d", pageCurrent, m_oInfo.m_nPagesCount);

		NSView* pViewParent = (__bridge NSView*)(m_pParent->GetWidgetImpl()->cef_handle);

		// start print dialog
		NSPrintInfo* pPrintInfo = [NSPrintInfo sharedPrintInfo];
		//        NSString* sKeyPrint = @"prihord"; // Ключ в котором расположен объект NSPrintInfo

		// Пробуем извлечь
		//        NSUserDefaults* pUserDef = [[NSUserDefaults alloc] init];
		//        NSData* pDataInfo = [pUserDef dataForKey:sKeyPrint];
		//        if (pDataInfo)
		//        {
		// Если удачно извлекли
		//            pPrintInfo = [NSUnarchiver unarchiveObjectWithData:pDataInfo];
		//        }
		//        else
		{
#if 0
			// Иначе выводим панель настройки печати
			NSPageLayout* pPageLayout = [[NSPageLayout alloc] init];
			if ([pPageLayout runModal] == NSModalResponseOK)
			{
				pPrintInfo = pPageLayout.printInfo;

				// сохраним введенные настройки в User Defaults
				pDataInfo = [NSArchiver archivedDataWithRootObject:pPrintInfo];
				[pUserDef setObject:pDataInfo forKey:sKeyPrint];
			}
#endif
		}

		NSDictionary * pWOptions = [info[@"options"] dictionary];
		if ( pWOptions && [pWOptions objectForKey:@"nativeOptions"] ) {
			NSDictionary * pNOptions = pWOptions[@"nativeOptions"];

			NSDictionary * paperSize = pNOptions[@"paperSize"];
			if ( [paperSize objectForKey:@"preset"] != nil )
				[pPrintInfo setPaperName:paperSize[@"preset"]];
			else {
				//            [pPrintInfo setPaperSize:NSMakeSize(595, 842)];
			}

			PMPrintSettings printSettings = (PMPrintSettings)pPrintInfo.PMPrintSettings;
			PMSetPageRange(printSettings, 1, m_oInfo.m_nPagesCount);
			PMSetCopies(printSettings , [pNOptions[@"copies"] intValue], kPMUnlocked);

			//            PMPageFormat pageFormat =  (PMPageFormat)pPrintInfo.PMPageFormat;
			//            if ( [pNOptions[@"paperOrientation"] isEqualTo:@"landscape"] )
			//                PMSetOrientation(pageFormat, kPMLandscape, kPMUnlocked);
			//            else PMSetOrientation(pageFormat, kPMPortrait, kPMUnlocked);
			//            [pPrintInfo updateFromPMPageFormat];

			PMSetFirstPage(printSettings, pageCurrent, kPMUnlocked ) ;

			NSString * duplex = pNOptions[@"sides"];
			if ( [duplex isEqualToString:@"both-long"] )
				PMSetDuplex(printSettings, kPMDuplexNoTumble);
			else
				if ( [duplex isEqualToString:@"both-short"] )
					PMSetDuplex(printSettings, kPMDuplexTumble);
				else PMSetDuplex(printSettings, kPMDuplexNone);
			[pPrintInfo updateFromPMPrintSettings];
		}

		NSPrintPanelOptions options = NSPrintPanelShowsCopies;
		options |= NSPrintPanelShowsPageRange;
		options |= NSPrintPanelShowsPaperSize;
		// commented since we can't select which pages to print directly in app
//		options |= NSPrintPanelShowsPrintSelection;
		options |= NSPrintPanelShowsOrientation;
		options |= NSPrintPanelShowsPreview;

		NSPrintOperation* pro;
		if (pPrintInfo)
			pro = [NSPrintOperation printOperationWithView:m_pView printInfo:pPrintInfo];
		else
			pro = [NSPrintOperation printOperationWithView:m_pView];

		[pro setShowsPrintPanel:YES]; // Выводим на печать или предпросмотр
		[[pro printPanel] setOptions:options];
		//[pro runOperation];
		[pro runOperationModalForWindow:pViewParent.window delegate:sender didRunSelector:didRunSelector contextInfo:nil];
	}
	void EndPaint()
	{
		NSEditorApi::CAscMenuEvent* pEventEnd = new NSEditorApi::CAscMenuEvent();
		pEventEnd->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_END;

		m_pParent->Apply(pEventEnd);

		m_pView = nil;
		isCurrentlyPrinting = false;
	}

	virtual void GetLogicalDPI(int& nDpiX, int& nDpiY)
	{
		nDpiX = (int)m_oInfo.m_dDpiX;
		nDpiY = (int)m_oInfo.m_dDpiY;
	}

	virtual void GetPhysicalRect(int& nX, int& nY, int& nW, int& nH)
	{
		nX = m_oInfo.m_nMarginLeft;
		nY = m_oInfo.m_nMarginTop;
		nW = m_oInfo.m_nPaperWidth;
		nH = m_oInfo.m_nPaperHeight;
	}

	virtual void GetPrintAreaSize(int& nW, int& nH)
	{
		nW = m_oInfo.m_nPaperWidth - m_oInfo.m_nMarginLeft - m_oInfo.m_nMarginRight;
		nH = m_oInfo.m_nPaperHeight - m_oInfo.m_nMarginTop - m_oInfo.m_nMarginBottom;
	}

	static void releasePixels(void *info, const void *data, size_t size)
	{
		delete ((unsigned char*)data);
	}

	virtual void BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
						const double& x, const double& y, const double& w, const double& h, const double& dAngle)
	{
		CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
		CGDataProviderRef pDataProvider = CGDataProviderCreateWithData(NULL, pBGRA, 4 * nRasterW * nRasterH, releasePixels);

		CGImageRef pCgImage = CGImageCreate(nRasterW, nRasterH, 8, 32, 4 * nRasterW, cs,
											kCGImageAlphaPremultipliedLast, pDataProvider,
											NULL, false, kCGRenderingIntentAbsoluteColorimetric);

		// calculate scale coeffs that properly scale page content on a choosen paper
		// this coeffs DO NOT scale the content itself
		const CGFloat scaleX = (CGFloat)m_oInfo.m_nPaperWidthOrigin / m_oInfo.m_nPaperWidth;
		const CGFloat scaleY = (CGFloat)m_oInfo.m_nPaperHeightOrigin / m_oInfo.m_nPaperHeight;

		// calculate scaled width and height of a page content
		CGFloat contentWidth = nRasterW * scaleX;
		CGFloat contentHeight = nRasterH * scaleY;

		// IGNORE margins, calculate offset for x and y manually
//		CGFloat fX = (x + m_oInfo.m_nMarginLeft) * scaleX;
//		CGFloat fY = (y + m_oInfo.m_nMarginTop) * scaleY;
//		fY = (m_oInfo.m_nPaperHeightOrigin - fY - fH);

		// calculate offset of content on a page to center the content
		auto calcOffset = [](CGFloat paperSize, CGFloat contentSize) {
			return (paperSize - contentSize) / 2;
		};

		CGFloat offsetX = calcOffset(m_oInfo.m_nPaperWidthOrigin, contentWidth);
		CGFloat offsetY = calcOffset(m_oInfo.m_nPaperHeightOrigin, contentHeight);

		CGContextRef _context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
		CGContextSaveGState(_context);

		// if printing to a preview panel (or an actual printer) and `dAngle` is PI/2
		if (m_oInfo.m_bIsPrintingOnScreen && fabs(dAngle - M_PI_2) <= 1e-5)
		{
			// rotate image by 90 degrees and move it up by `contentWidth` to fit original position
			CGAffineTransform transform = CGAffineTransformMake(0, -1, 1, 0, 0, contentWidth);
			CGContextConcatCTM(_context, transform);
			// recalculate offsets, since page content is now rotated
			offsetX = -calcOffset(m_oInfo.m_nPaperHeightOrigin, contentWidth);
			offsetY = calcOffset(m_oInfo.m_nPaperWidthOrigin, contentHeight);
		}

		CGRect _rect = NSMakeRect(offsetX, offsetY, contentWidth, contentHeight);
		CGContextDrawImage(_context, _rect, pCgImage);
		CGImageRelease(pCgImage);

		CGContextRestoreGState(_context);

		CGColorSpaceRelease(cs);
		CGDataProviderRelease(pDataProvider);
	}
};

// TODO: move outside of the header file
bool ASCPrinterContext::isCurrentlyPrinting = false;

#endif  // NSASCPRINTER_H
