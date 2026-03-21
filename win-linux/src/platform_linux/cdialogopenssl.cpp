#include "cdialogopenssl.h"
#include "components/cmessage.h"
#include "cscalingwrapper.h"
#include "cascapplicationmanagerwrapper.h"
#include "utils.h"
#include "components/cfiledialog.h"
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

#include "../../../../core/DesktopEditor/xmlsec/src/include/CertificateCommon.h"

class CSslDialog_Private
{
public:
    CSslDialog_Private(QWidget * parent = nullptr)
        : _txtCertPath(new QLineEdit(parent))
        , _txtCertPass(new QLineEdit(parent))
        , _txtKeyPath(new QLineEdit(parent))
        , _txtKeyPass(new QLineEdit(parent))
        , _btnCertFile(new QToolButton(parent))
        , _btnKeyFile(new QToolButton(parent))
        , _labelCertPass(new QLabel(parent))
        , _labelKeyPass(new QLabel(parent))
    {
        _txtCertPass->setEchoMode(QLineEdit::Password);
        _txtKeyPass->setEchoMode(QLineEdit::Password);
        _btnCertFile->setText("...");
        _btnKeyFile->setText("...");
    }

    void setKeyDisabled(bool v = true) {
        _txtKeyPath->setDisabled(v);
        _btnKeyFile->setDisabled(v);
        _txtKeyPass->setDisabled(v);
        _labelKeyPass->setDisabled(v);
    }

    void clearKey(bool disable = false) {
        _txtKeyPath->clear();
        _txtKeyPass->clear();

        if ( disable ) {
            setKeyDisabled(true);
        }
    }

    void setPassDisabled(QString type = QString(), bool value = true) {
        if ( type == "cert" || type.isEmpty() ) {
            _txtCertPass->setDisabled(value);
            _labelCertPass->setDisabled( value);
        }
        if ( type == "key" || type.isEmpty() ) {
            _txtKeyPass->setDisabled(value);
            _labelKeyPass->setDisabled( value);
        }
    }

    void clearCertPass(bool disable = false) {
        _txtCertPass->clear();
        if ( disable )
            _txtCertPass->setDisabled(true);
    }

    QString certificatePath() {
        return _txtCertPath->text().trimmed();
    }

    QString certificatePassword() {
        return _txtCertPass->text().trimmed();
    }

    QString keyPath() {
        if ( _txtKeyPath->isEnabled() )
            return _txtKeyPath->text().trimmed();
        else return "";
    }

    QString keyPassword() {
        if ( _txtKeyPass->isEnabled() )
            return _txtKeyPass->text().trimmed();
        else return "";
    }
public:
    QLineEdit * _txtCertPath,
                * _txtCertPass,
                * _txtKeyPath,
                * _txtKeyPass;
    QToolButton * _btnCertFile,
                * _btnKeyFile;
    QLabel * _labelCertPass,
                * _labelKeyPass;
};

CDialogOpenSsl::CDialogOpenSsl(QWidget *parent)
    : QDialog(parent)
    , m_private(new CSslDialog_Private)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    double dpiRatio = CScalingWrapper::parentScalingFactor(topLevelWidget());
    setMinimumWidth(300 * dpiRatio);
    setWindowTitle(tr("Select certificate"));
    setProperty("zoom", QString::number(dpiRatio) + "x");

    QGridLayout * _main_layout = new QGridLayout(this);

    m_private->_txtCertPath->setFixedHeight(24 * dpiRatio);
    m_private->_txtCertPath->setPlaceholderText(tr("select certificate file..."));
    m_private->_txtCertPass->setFixedHeight(24 * dpiRatio);
    m_private->_labelCertPass->setText(tr("Certificate password:"));
    m_private->_txtKeyPath->setFixedHeight(24 * dpiRatio);
    m_private->_txtKeyPath->setPlaceholderText(tr("select key file..."));
    m_private->_txtKeyPass->setFixedHeight(24 * dpiRatio);
    m_private->_labelKeyPass->setText(tr("Key password:"));

    connect(m_private->_btnCertFile, &QToolButton::clicked, this, &CDialogOpenSsl::onBtnCertificateClick);
    connect(m_private->_btnKeyFile, &QToolButton::clicked, this, &CDialogOpenSsl::onBtnKeyClick);

    QHBoxLayout * _pass_layout = new QHBoxLayout;
    _pass_layout->addWidget(m_private->_labelCertPass);
    _pass_layout->addWidget(m_private->_txtCertPass, 1);

    QHBoxLayout * _keypass_layout = new QHBoxLayout;
    _keypass_layout->addWidget(m_private->_labelKeyPass);
    _keypass_layout->addWidget(m_private->_txtKeyPass, 1);

    _main_layout->setContentsMargins(16 * dpiRatio, 24 * dpiRatio, 16 * dpiRatio, 16 * dpiRatio);
    _main_layout->setSpacing(6 * dpiRatio);
    _main_layout->setColumnStretch(0, 1 * dpiRatio);
    _main_layout->setRowStretch(3, 2 * dpiRatio);
    _main_layout->setColumnMinimumWidth(1, 20 * dpiRatio);

    _main_layout->addWidget(m_private->_txtCertPath, 0, 0);
    _main_layout->addWidget(m_private->_btnCertFile, 0, 1);
    _main_layout->addLayout(_pass_layout, 1, 0);
    _main_layout->addWidget(m_private->_txtKeyPath, 2, 0);
    _main_layout->addWidget(m_private->_btnKeyFile, 2, 1);
    _main_layout->addLayout(_keypass_layout, 3, 0);
    _main_layout->addItem(new QSpacerItem(6, 6, QSizePolicy::Fixed, QSizePolicy::Expanding), 4, 0);
    // _main_layout->setRowStretch(4, 1 * dpiRatio);

    m_private->setKeyDisabled(true);
    m_private->setPassDisabled();

    QHBoxLayout * _ok_layout = new QHBoxLayout;
    _main_layout->addLayout(_ok_layout, 5,0,1,2);

    QPushButton * _btn_ok = new QPushButton("OK");
    _btn_ok->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    _ok_layout->addWidget(_btn_ok);

    _btn_ok->setFixedSize(84 * dpiRatio, 24 * dpiRatio);
    m_private->_btnCertFile->setFixedSize(40 * dpiRatio, 24 * dpiRatio);
    m_private->_btnKeyFile->setFixedSize(40 * dpiRatio, 24 * dpiRatio);

    connect(_btn_ok, &QPushButton::clicked, [=]{
        if ( checkCertificate() ) {
            accept();
        }
    });

    QRect rect = this->geometry();

    int nX = rect.x();
    int nY = rect.y();
    int nW = 420 * dpiRatio;
    int nH = 210 * dpiRatio;

    if (parent)
    {
        QRect rectParent = parent->geometry();
        nX = rectParent.center().x() - (nW >> 1);
        nY = rectParent.center().y() - (nH >> 1);
    }

    this->setGeometry(nX, nY, nW, nH);

    QString css = Utils::readStylesheets(":/styles/openssl.qss");
    setStyleSheet(css.arg(GetColorQValueByRole(ecrDownloadWidgetBackground),
                          GetColorQValueByRole(ecrDownloadWidgetBorder),
                          GetColorQValueByRole(ecrTextNormal),
                          GetColorQValueByRole(ecrButtonHoverBackground),
                          GetColorQValueByRole(ecrButtonPressedBackground),
                          GetColorQValueByRole(ecrBorderControlFocus)));

    // Set native dialog from command line arguments
}

CDialogOpenSsl::~CDialogOpenSsl()
{
    delete m_private, m_private = nullptr;
}

void CDialogOpenSsl::onBtnCertificateClick()
{
    QString sDirectory = "~/";
    CFileDialogWrapper dlg(this);
    QStringList result = dlg.modalOpenAny(sDirectory);
    QString _file_name = (result.size() > 0) ? result.at(0) : QString();
    if ( !_file_name.isEmpty() ) {
        m_private->clearKey(true);
        m_private->setPassDisabled();

        m_private->_txtCertPath->setText(_file_name);
        checkCertificate();
    }
}

void CDialogOpenSsl::onBtnKeyClick()
{
    QString sDirectory = "~/";
    CFileDialogWrapper dlg(this);
    QStringList result = dlg.modalOpenAny(sDirectory);
    QString _file_name = (result.size() > 0) ? result.at(0) : QString();
    if ( !_file_name.isEmpty() ) {
        m_private->setPassDisabled("key");
        m_private->_txtKeyPath->setText(_file_name);

        checkCertificate();
    }
}

bool CDialogOpenSsl::checkCertificate()
{
    if ( m_private->certificatePath().isEmpty() ) {
        CMessage::info(this, tr("Enter certificate path"));
        return false;
    }

    int _result  = NSOpenSSL::LoadCert( m_private->_txtCertPath->text().toStdWString(),
                                        m_private->_txtCertPass->text().toStdString() );

    switch ( _result ) {
    case OPEN_SSL_WARNING_ERR:
        CMessage::error(this, tr("Certificate is not supported"));
        break;
    case OPEN_SSL_WARNING_PASS:
        m_private->setPassDisabled("cert", false);
        m_private->_txtCertPass->setFocus();
        if ( m_private->_txtCertPass->text().isEmpty() )
            CMessage::info(this, tr("Enter certificate password"));
        else CMessage::info(this, tr("Wrong certificate password.<br>Please enter again"));
        break;
    case OPEN_SSL_WARNING_OK:
    case OPEN_SSL_WARNING_ALL_OK:
    default: break;
    }

    if ( _result == OPEN_SSL_WARNING_OK ) {
        if ( m_private->_txtKeyPath->text().isEmpty() ) {
            CMessage::info(this, tr("Enter valid private key"));
            m_private->_txtKeyPath->setDisabled(false);
            m_private->_btnKeyFile->setDisabled(false);
            m_private->_txtKeyPath->setFocus();
        } else {
            _result = NSOpenSSL::LoadKey( m_private->_txtKeyPath->text().toStdWString(),
                                            m_private->_txtKeyPass->text().toStdString() );

            switch ( _result ) {
            case OPEN_SSL_WARNING_ERR:
                CMessage::info(this, tr("Key is not supported"));
                break;
            case OPEN_SSL_WARNING_PASS:
                m_private->setPassDisabled("key", false);
                m_private->_txtKeyPass->setFocus();
                CMessage::info(this, tr("Enter key password"));
                break;
            case OPEN_SSL_WARNING_OK:
                _result = OPEN_SSL_WARNING_ALL_OK;
                break;
            case OPEN_SSL_WARNING_ALL_OK:
            default:
                break;
            }
        }
    }

    return _result == OPEN_SSL_WARNING_ALL_OK;
}

void CDialogOpenSsl::getResult(NSEditorApi::CAscOpenSslData& data)
{
    data.put_CertPath(m_private->certificatePath().toStdWString());
    data.put_CertPassword(m_private->certificatePassword().toStdWString());
    data.put_KeyPath(m_private->keyPath().toStdWString());
    data.put_KeyPassword(m_private->keyPassword().toStdWString());
}
