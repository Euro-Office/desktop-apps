#ifndef CTABPANEL_H
#define CTABPANEL_H

#include <QWidget>
#include <QTimer>

#include "qcefview.h"
#include "casctabdata.h"

class CTabPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CTabPanel(QWidget *parent, const QSize& s);
    explicit CTabPanel(QWidget *parent, const QSize& s, const QColor& c);
    ~CTabPanel();

    static CTabPanel * createEditorPanel(QWidget *parent, const QSize& size);

    QCefView * view() const;
    CCefView * cef() const;
    void setView(QCefView *);

    CAscTabData * data() const;
    void setData(CAscTabData *);

    void initAsEditor();
    void initAsSimple();

    void openLocalFile(const std::wstring& sFilePath, int nFileFormat, const std::wstring& params);
    bool openLocalFile(const std::wstring& sFilePath, const std::wstring& params);
    void createLocalFile(AscEditorType nFileFormat, const std::wstring& sName = L"");
    void createLocalFile(const std::wstring& templatepath, const std::wstring& name);
    void createLocalFile(int templateid, const std::wstring& name);
    bool openRecoverFile(int id);
    bool openRecentFile(int id);
    bool isReady();
    bool hasUncommittedChanges();
    void applyLoader(const QString& cmd, const QString& args = QString());
    void setReady();
    //void resize(int w, int h);
    //void resizeEvent(QResizeEvent *event);
    //void showEvent(QShowEvent *event);

    bool prettyTitle() { return m_prettyTitle; }
    void setPrettyTitle(bool v) { m_prettyTitle = v; }
    void setBackground(const QColor&);

    bool reporterMode() const;
    void setReporterMode(bool);

protected:
    //void timerEvent(QTimerEvent *event);
    //void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QCefView *      m_pViewer;
    QCefView *      m_pLoader = nullptr;
    CAscTabData *   m_pData = nullptr;

    QSize m_startSize, m_lastSize;
    int m_idTimerResize = 0;
    bool m_prettyTitle = false;
    bool m_isReady = false;
    bool m_isReporterOn = false;

signals:
    void closePanel(QCloseEvent *event);

public slots:
    void showFullScreen();
    void showNormal();
};

#endif // CTABPANEL_H
