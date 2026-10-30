/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

// Fake of desktop-sdk's qt_wrapper: QCefView is a plain QWidget without a
// browser, the printer context never prints.

#include <QCoreApplication>
#include "qascapplicationmanager.h"
#include "qascprinter.h"
#include "qcefview.h"
#include "qcefview_media.h"
#include "qdpichecker.h"
#include "fakesdk_p.h"

using FakeSdk::record;

QAscApplicationManager::QAscApplicationManager() {}
QAscApplicationManager::~QAscApplicationManager() {}
CAscDpiChecker* QAscApplicationManager::InitDpiChecker() { return new QDpiChecker(this); }
IExternalMessageLoop* QAscApplicationManager::GetExternalMessageLoop() { return nullptr; }
int QAscApplicationManager::GetPlatformKeyboardLayout() { return -1; }
void QAscApplicationManager::processEvents() { QCoreApplication::processEvents(); }

QDpiChecker::QDpiChecker(CAscApplicationManager* manager) : CAscDpiChecker(manager) {}

int QDpiChecker::GetWindowDpi(WindowHandleId, unsigned int* dx, unsigned int* dy)
{
    *dx = *dy = 96;
    return 0;
}

int QDpiChecker::GetMonitorDpi(int, unsigned int* dx, unsigned int* dy)
{
    *dx = *dy = 96;
    return 0;
}

int QDpiChecker::GetWidgetImplDpi(CCefViewWidgetImpl*, unsigned int* dx, unsigned int* dy)
{
    *dx = *dy = 96;
    return 0;
}

int QDpiChecker::GetWidgetDpi(QWidget*, unsigned int* dx, unsigned int* dy)
{
    *dx = *dy = 96;
    return 0;
}

QList<QCefView*> QCefView::s_waylandViews;

QCefView::QCefView(QWidget* parent, const QSize&)
    : QWidget(parent), m_pCefView(nullptr), m_pProperties(nullptr), m_isWayland(false)
{
}

QCefView::~QCefView()
{
    if (m_pCefView)
        m_pCefView->OnDestroyWidgetImpl();
}

void QCefView::focusInEvent(QFocusEvent* e) { QWidget::focusInEvent(e); }
void QCefView::focusOutEvent(QFocusEvent* e) { QWidget::focusOutEvent(e); }
bool QCefView::focusNextPrevChild(bool next) { return QWidget::focusNextPrevChild(next); }
void QCefView::resizeEvent(QResizeEvent* e) { QWidget::resizeEvent(e); }
void QCefView::moveEvent(QMoveEvent* e) { QWidget::moveEvent(e); }
void QCefView::showEvent(QShowEvent* e) { QWidget::showEvent(e); }
void QCefView::mousePressEvent(QMouseEvent* e) { QWidget::mousePressEvent(e); }
void QCefView::mouseReleaseEvent(QMouseEvent* e) { QWidget::mouseReleaseEvent(e); }
void QCefView::mouseDoubleClickEvent(QMouseEvent* e) { QWidget::mouseDoubleClickEvent(e); }
void QCefView::mouseMoveEvent(QMouseEvent* e) { QWidget::mouseMoveEvent(e); }
void QCefView::wheelEvent(QWheelEvent* e) { QWidget::wheelEvent(e); }
void QCefView::keyPressEvent(QKeyEvent* e) { QWidget::keyPressEvent(e); }
void QCefView::keyReleaseEvent(QKeyEvent* e) { QWidget::keyReleaseEvent(e); }
void QCefView::inputMethodEvent(QInputMethodEvent* e) { QWidget::inputMethodEvent(e); }
QVariant QCefView::inputMethodQuery(Qt::InputMethodQuery query) const { return QWidget::inputMethodQuery(query); }
#if defined (_LINUX) && !defined(_MAC)
void QCefView::dragEnterEvent(QDragEnterEvent* e) { QWidget::dragEnterEvent(e); }
void QCefView::dragLeaveEvent(QDragLeaveEvent* e) { QWidget::dragLeaveEvent(e); }
void QCefView::dropEvent(QDropEvent* e) { QWidget::dropEvent(e); }
NSEditorApi::CAscLocalDragDropData* QCefView::convertMimeData(const QMimeData*) { return nullptr; }
#endif
void QCefView::UpdateSize() {}
void QCefView::AfterCreate() {}
void QCefView::closeEvent(QCloseEvent* e) { QWidget::closeEvent(e); }
CCefView* QCefView::GetCefView() { return m_pCefView; }

void QCefView::Create(CAscApplicationManager* pManager, CefViewWrapperType eType)
{
    m_pCefView = eType == cvwtEditor ? pManager->CreateCefEditor(this) : pManager->CreateCefView(this);
    Init();
}

void QCefView::CreateReporter(CAscApplicationManager* pManager, CAscReporterData*)
{
    record("QCefView::CreateReporter");
    m_pCefView = pManager->CreateCefEditor(this);
    Init();
}

void QCefView::OnMediaStart(NSEditorApi::CAscExternalMedia*) {}
void QCefView::OnMediaEnd(bool) {}
void QCefView::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand*) {}
void QCefView::OnLoaded() {}
void QCefView::OnRelease() {}
QWidget* QCefView::GetViewWidget() { return this; }

void QCefView::SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b)
{
    backgroundR = r;
    backgroundG = g;
    backgroundB = b;
}

void QCefView::paintEvent(QPaintEvent* e) { QWidget::paintEvent(e); }
double QCefView::GetDeviceScaleFactor() { return devicePixelRatioF(); }
double QCefView::GetUIScalePercentage() { return 100; }
bool QCefView::IsWayland() { return m_isWayland; }
void QCefView::OnPaint(const void*, int, int) {}

void QCefView::GetWidgetScreenPosition(int& screenX, int& screenY)
{
    const QPoint pos = mapToGlobal(QPoint(0, 0));
    screenX = pos.x();
    screenY = pos.y();
}

void QCefView::SetClipboardData(const std::wstring&) {}
std::wstring QCefView::GetClipboardData() { return L""; }
void QCefView::SetCursorType(int) {}
void QCefView::SetCursorCustom(const void*, int, int, int, int) {}
void QCefView::FlushDirtyWaylandViews() {}
bool QCefView::IsSupportLayers() { return false; }
void QCefView::SetCaptionMaskSize(int) {}
bool QCefView::eventFilter(QObject* watched, QEvent* event) { return QWidget::eventFilter(watched, event); }

bool QCefView::setFocusToCef()
{
    record("QCefView::setFocusToCef");
    return true;
}

void QCefView::Init() {}
void QCefView::_loadedSlot() {}
void QCefView::_closedSlot() {}

QCefGLWidget::QCefGLWidget(QWidget* parent) : QOpenGLWidget(parent) {}
void QCefGLWidget::SetFrame(const QImage& image) { m_frame = image; }
void QCefGLWidget::paintGL() {}

QCefEmbedWindow::QCefEmbedWindow(QPointer<QCefView> _qcef_parent, QWindow* _parent)
    : QWindow(_parent), m_nCaptionSize(0), qcef_parent(_qcef_parent)
{
}

void QCefEmbedWindow::SetCaptionMaskSize(int size) { m_nCaptionSize = size; }
void QCefEmbedWindow::moveEvent(QMoveEvent* e) { QWindow::moveEvent(e); }
void QCefEmbedWindow::resizeEvent(QResizeEvent* e) { QWindow::resizeEvent(e); }
bool QCefEmbedWindow::eventFilter(QObject* watched, QEvent* event) { return QWindow::eventFilter(watched, event); }

QCefView_Media::QCefView_Media(QWidget* parent, const QSize& size) : QCefView(parent, size), m_pMediaView(nullptr) {}
QCefView_Media::~QCefView_Media() {}
void QCefView_Media::OnMediaStart(NSEditorApi::CAscExternalMedia*) {}
void QCefView_Media::OnMediaEnd(bool) {}
void QCefView_Media::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand*) {}
void QCefView_Media::showMediaControl(NSEditorApi::CAscExternalMediaPlayerCommand*) {}
void QCefView_Media::hideMediaControl() {}
void QCefView_Media::updateGeometry(NSEditorApi::CAscExternalMediaPlayerCommand*) {}
void QCefView_Media::onMediaKeyDown(int, Qt::KeyboardModifiers) {}

QAscPrinterContext::QAscPrinterContext(QPrinter::PrinterMode eMode)
    : m_oPrinter(eMode), m_bIsUsePainter(false), m_pDevice(nullptr), m_eDeviceType(pdtNone)
{
}

QAscPrinterContext::QAscPrinterContext(const QPrinterInfo& pi, QPrinter::PrinterMode eMode)
    : m_oPrinter(pi, eMode), m_bIsUsePainter(false), m_pDevice(nullptr), m_eDeviceType(pdtNone)
{
}

QAscPrinterContext::QAscPrinterContext(QPaintDevice* pDevice)
    : m_bIsUsePainter(false), m_pDevice(pDevice), m_eDeviceType(pdtSimple)
{
}

QAscPrinterContext::QAscPrinterContext(QPagedPaintDevice* pDevice)
    : m_bIsUsePainter(false), m_pDevice(pDevice), m_eDeviceType(pdtPaged)
{
}

QAscPrinterContext::~QAscPrinterContext() {}

bool QAscPrinterContext::BeginPaint()
{
    record("QAscPrinterContext::BeginPaint");
    return false;
}

void QAscPrinterContext::EndPaint() { record("QAscPrinterContext::EndPaint"); }
QPrinter* QAscPrinterContext::getPrinter() { return &m_oPrinter; }
QPainter* QAscPrinterContext::GetPainter() { return &m_oPainter; }

void QAscPrinterContext::GetLogicalDPI(int& nDpiX, int& nDpiY)
{
    nDpiX = nDpiY = 96;
}

void QAscPrinterContext::GetPhysicalRect(int& nX, int& nY, int& nW, int& nH)
{
    nX = nY = nW = nH = 0;
}

void QAscPrinterContext::GetPrintAreaSize(int& nW, int& nH)
{
    nW = nH = 0;
}

void QAscPrinterContext::BitBlt(unsigned char*, const int&, const int&, const int&, const int&,
                                const double&, const double&, const double&, const double&, const double&)
{
}

void* QAscPrinterContext::GetNativeRenderer() { return nullptr; }
void* QAscPrinterContext::GetNativeRendererUnsupportChecker() { return nullptr; }
void QAscPrinterContext::NewPage() {}
void QAscPrinterContext::InitRenderer(void*, void*) {}

void QAscPrinterContext::PrepareBitBlt(void*, const int&, const int&, const int&, const int&,
                                       const double&, const double&, const double&, const double&, const double&,
                                       const double&, const double&)
{
}

void QAscPrinterContext::SetPageOrientation(int) {}
void QAscPrinterContext::SaveState() {}
void QAscPrinterContext::RestoreState() {}
