
#include "ctabpanel.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "cefview.h"
#include <QHBoxLayout>


using namespace NSEditorApi;

CTabPanel * CTabPanel::createEditorPanel(QWidget *parent, const QSize& s)
{
    QColor c = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground);
    CTabPanel * panel = new CTabPanel(parent, s, c);
    panel->initAsEditor();
    return panel;
}

CTabPanel::CTabPanel(QWidget *parent, const QSize& s)
    : CTabPanel(parent, s, QColor(244,244,244))
{
}

CTabPanel::CTabPanel(QWidget *parent, const QSize& s, const QColor& c)
    : QWidget(parent)
    , m_pViewer(AscAppManager::createViewer(this, s))
{
    QHBoxLayout *_layout = new QHBoxLayout(this);
    _layout->setSpacing(0);
    _layout->setContentsMargins(0,0,0,0);
    _layout->addWidget(m_pViewer);
    setLayout(_layout);
    m_pViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pViewer->SetBackgroundCefColor(uchar(c.red()), uchar(c.green()), uchar(c.blue()));

    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
}

CTabPanel::~CTabPanel()
{
    if ( m_pViewer )
        delete m_pViewer, m_pViewer = nullptr;

    if ( m_pLoader )
        delete m_pLoader, m_pLoader = nullptr;

    if ( m_pData )
        delete m_pData, m_pData = nullptr;
}

void CTabPanel::setBackground(const QColor& c)
{
    m_pViewer->SetBackgroundCefColor(uchar(c.red()), uchar(c.green()), uchar(c.blue()));
}

QCefView * CTabPanel::view() const
{
    return m_pViewer;
}

CCefView * CTabPanel::cef() const
{
    return m_pViewer->GetCefView();
}

void CTabPanel::setView(QCefView * v)
{
    if ( m_pViewer )
        delete m_pViewer, m_pViewer = nullptr;

    m_pViewer = v;
}

CAscTabData * CTabPanel::data() const
{
    return m_pData;
}

void CTabPanel::setData(CAscTabData * d)
{
    if ( m_pData )
        delete m_pData, m_pData = nullptr;

    m_pData = d;
}

void CTabPanel::initAsEditor()
{
#ifdef USE_PARTICULAR_LOADER
    GET_REGISTRY_USER(_reg_user);

# if defined(QT_DEBUG)
    QString _loader_path = _reg_user.value("loaderpage").value<QString>();
# endif

    m_pLoader = new QCefView(this);
    //m_pLoader->setGeometry(0,0, width(), height());
    m_pLoader->Create(&AscAppManager::getInstance(), cvwtSimple);
    m_pLoader->GetCefView()->load(_loader_path.toStdWString());

    m_pViewer->hide();
#endif

    m_pViewer->Create(&AscAppManager::getInstance(), cvwtEditor);
}

void CTabPanel::initAsSimple()
{
    m_pViewer->Create(&AscAppManager::getInstance(), cvwtSimple);
}

void CTabPanel::openLocalFile(const std::wstring& path, int format, const std::wstring& params)
{
    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenLocalFile(path, format, params);
}

bool CTabPanel::openLocalFile(const std::wstring& path, const std::wstring& params)
{
    int _format = CCefViewEditor::GetFileFormat(path);
    if ( _format == 0 )
        return false;

    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenLocalFile(path, _format, params);
    return true;
}

void CTabPanel::createLocalFile(AscEditorType format, const std::wstring& name)
{
    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->CreateLocalFile(format, name);
}

void CTabPanel::createLocalFile(const std::wstring& templatepath, const std::wstring& name)
{
    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->CreateLocalFile(AscEditorType::etUndefined, name, templatepath);
}

void CTabPanel::createLocalFile(int templateid, const std::wstring& name)
{
    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->CreateLocalFile(AscEditorType::etUndefined, templateid, name);
}

bool CTabPanel::openRecoverFile(int id)
{
    return static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenRecoverFile(id);
}

bool CTabPanel::openRecentFile(int id)
{
    return static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenRecentFile(id);
}

bool CTabPanel::isReady()
{
    return m_isReady;
}

bool CTabPanel::hasUncommittedChanges()
{
    if (m_pViewer) {
        CCefViewEditor *pViewEditor = (CCefViewEditor*)m_pViewer->GetCefView();
        return pViewEditor && pViewEditor->IsSaveLocked();
    }
    return false;
}

/*void CTabPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_pViewer->resize(event->size());
}*/

/*void CTabPanel::showEvent(QShowEvent *)
{
//    cef()->resizeEvent();
}*/

/*void CTabPanel::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}*/

/*void CTabPanel::timerEvent(QTimerEvent *)
{
     if ( m_startSize == m_lastSize ) {
        cef()->resizeEvent();

        QObject::killTimer(m_idTimerResize);
        m_idTimerResize = 0;
    } else {
        m_startSize = m_lastSize;
    }
}*/

void CTabPanel::closeEvent(QCloseEvent *event)
{
    emit closePanel(event);
}

/*void CTabPanel::resize(int w, int h)
{
    if ( m_idTimerResize == 0 ) {
        m_startSize = QSize(w, h);
        m_idTimerResize = QObject::startTimer(200);
    }

    m_lastSize = QSize(w, h);
}*/

void CTabPanel::applyLoader(const QString& cmd, const QString& args)
{
    if ( m_pLoader ) {
        if ( cmd == "hide" ) {
            m_pViewer->show();
            m_pLoader->hide();
        } else {
            AscAppManager::sendCommandTo(m_pLoader, cmd, args);
        }
    }
}

void CTabPanel::setReady()
{
    m_isReady = true;
}

void CTabPanel::showFullScreen()
{
    QWidget::setWindowTitle(data()->title());
    QWidget::showFullScreen();
    //m_pViewer->setGeometry(QRect(0,0,width(),height()));
}

void CTabPanel::showNormal()
{
    QWidget::showNormal();
}

bool CTabPanel::reporterMode() const
{
    return m_isReporterOn;
}

void CTabPanel::setReporterMode(bool status)
{
    m_isReporterOn = status;
}
