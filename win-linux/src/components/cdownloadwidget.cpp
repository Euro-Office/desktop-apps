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

#include "components/cdownloadwidget.h"
#include "components/celipsislabel.h"
#include "cascapplicationmanagerwrapper.h"
#include "clangater.h"
#include "utils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QFileInfo>
#include <QProcess>
#include <QDesktopServices>
#include "common/Types.h"
#ifdef __linux__
# include <QX11Info>
#endif

#define ICON_SIZE (QSizeF(20,20) * m_dpiRatio)
#define SMALL_ICON_SIZE (QSizeF(12,12) * m_dpiRatio)
#define WIDGET_MAX_WIDTH  300 * m_dpiRatio
#define WIDGET_MAX_HEIGHT 400 * m_dpiRatio
#define ITEM_MAX_HEIGHT   58 * m_dpiRatio
#define MARGINS 6 * m_dpiRatio
#define SPACING 2 * m_dpiRatio
#define SHADOW  6 * m_dpiRatio
#define RADIUS  3 * m_dpiRatio

using namespace NSEditorApi;


struct CDownloadWidget::CDownloadItem
{
    CDownloadItem(QWidget * w)
        : widget(w), is_temp(true), is_finished(false)
    {}
    QWidget *widget;
    bool is_temp, is_finished;
};

static bool isCompositingEnabled()
{
#ifdef __linux__
    return QX11Info::isCompositingManagerRunning();
#else
    return true;
#endif
}

static void polishItem(QWidget *item)
{
    item->style()->polish(item);
    if (QLayout *lut = item->layout()) {
        for (int i = 0; i < lut->count(); i++) {
            QLayoutItem *litem = lut->itemAt(i);
            if (litem && litem->widget())
                litem->widget()->style()->polish(litem->widget());
        }
    }
}

static void onHover(QObject *obj, bool hovered) {
    obj->setProperty("hovered", hovered);
    if (QWidget *wgt = qobject_cast<QWidget*>(obj)) {
        polishItem(wgt);
        if (QPushButton *open = wgt->findChild<QPushButton*>("buttonOpen")) {
            if (CElipsisLabel *info = wgt->findChild<CElipsisLabel*>("labelInfo")) {
                info->style()->polish(info);
                open->style()->polish(open);
                info->setVisible(!hovered);
                open->setVisible(hovered);
            }
        }
        if (QPushButton *open_folder = wgt->findChild<QPushButton*>("buttonOpenFolder")) {
            open_folder->style()->polish(open_folder);
            open_folder->setVisible(hovered);
        }
    }
}

static QString getFileSize(const QString &filePath)
{
    qint64 fsize = 0;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)){
        fsize = file.size();
        file.close();
    }
    QString str;
    if (fsize < 1024)
        str = QString::number(fsize) + " " + QObject::tr("B");
    else
    if (fsize < 1024*1024)
        str = QString::number((double)fsize/1024, 'f', 1) + " " + QObject::tr("kB");
    else
        str = QString::number((double)fsize/(1024*1024), 'f', 1) + " " + QObject::tr("MB");
    return str;
}


CDownloadWidget::CDownloadWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , m_pToolButton(new CPushButton)
{
    int shd = 0, rad = 0;
    if (isCompositingEnabled()) {
        setAttribute(Qt::WA_TranslucentBackground);
        shd = qRound(SHADOW);
        rad = qRound(RADIUS);
    }
    installEventFilter(this);
    setLayout(new QVBoxLayout);
    layout()->setContentsMargins(shd, shd, shd, shd);
    layout()->setSpacing(0);

    m_mainFrame = new QFrame(this);
    m_mainFrame->setObjectName("mainFrame");
    m_mainFrame->setLayout(new QVBoxLayout);
    m_mainFrame->layout()->setContentsMargins(0, rad, 0, rad);
    m_mainFrame->layout()->setSpacing(0);
    layout()->addWidget(m_mainFrame);
    if (isCompositingEnabled()) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(m_mainFrame);
        shadow->setBlurRadius(16.0);
        shadow->setColor(QColor(0, 0, 0, 80));
        shadow->setOffset(1.0);
        m_mainFrame->setGraphicsEffect(shadow);
    }

    int mrg_half = qRound(MARGINS/2);
    int mrg_dbl = qRound(2 * MARGINS);
    m_titleFrame = new QFrame(m_mainFrame);
    m_titleFrame->setObjectName("titleFrame");
    m_titleFrame->setLayout(new QHBoxLayout);
    m_titleFrame->layout()->setContentsMargins(mrg_dbl, mrg_half, mrg_dbl, mrg_half);
    m_titleFrame->layout()->setSpacing(0);

    QLabel *labelTitle = new QLabel(m_titleFrame);
    labelTitle->setObjectName("labelTitle");
    labelTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    labelTitle->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft)  | Qt::AlignAbsolute);
    labelTitle->setText(tr("Downloads"));
    m_titleFrame->layout()->addWidget(labelTitle);

    QPushButton *clearButton = new QPushButton(tr("Clear"));
    clearButton->setObjectName("buttonClear");
    m_titleFrame->layout()->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, [=](){
        clearlAll();
    });
    m_mainFrame->layout()->addWidget(m_titleFrame);

    m_pArea = new QScrollArea(m_mainFrame);
    m_pArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pArea->setWidgetResizable(true);
    m_pArea->setLayout(new QVBoxLayout);
    m_pArea->layout()->setContentsMargins(0,0,0,0);
    m_pArea->layout()->setSpacing(0);

    m_pContentArea = new QWidget;
    m_pContentArea->setObjectName("contentArea");
    QVBoxLayout *lut = new QVBoxLayout(m_pContentArea);
    lut->setContentsMargins(0, 0, 0, 0);
    lut->setSpacing(0);
    m_pContentArea->setLayout(lut);
    m_pArea->setWidget(m_pContentArea);
    m_mainFrame->layout()->addWidget(m_pArea);

    m_pToolButton->setObjectName("toolButtonDownload");
    m_pToolButton->setProperty("act", "tool");
    m_pToolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pToolButton->setVisible(false);
    connect(m_pToolButton, &QPushButton::clicked, this, [=]() {
        if (isVisible()) {
            hide();
            return;
        }
        setProperty("rtl-font", CLangater::isRtlLanguage(CLangater::getCurrentLangCode()));
        polish();
        show();
        QPoint pos = AscAppManager::isRtlEnabled() ? parent->geometry().topLeft() : parent->geometry().topRight() - QPoint(WIDGET_MAX_WIDTH, 0);
        QPoint brd_offset((AscAppManager::isRtlEnabled() ? 1 : -1) * MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio, (TITLE_HEIGHT + MAIN_WINDOW_BORDER_WIDTH) * m_dpiRatio);
        QPoint shd_offset((AscAppManager::isRtlEnabled() ? -1 : 1) * qRound(SHADOW - MARGINS/3), qRound(-SHADOW + MARGINS/3));
        pos += brd_offset + shd_offset;
        move(pos);
        int prefHeight = m_mapDownloads.size() * ITEM_MAX_HEIGHT + 74 * m_dpiRatio;
        setGeometry(QRect(pos, QSize(WIDGET_MAX_WIDTH, (prefHeight > WIDGET_MAX_HEIGHT) ? WIDGET_MAX_HEIGHT : prefHeight)));
        activateWindow();
    });
    m_pToolButton->setAnimatedIcon(AscAppManager::themes().current().isDark() ? ":/loading_light.svg" : ":/loading.svg");
    lut->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding));
}

CDownloadWidget::~CDownloadWidget()
{

}

QPushButton * CDownloadWidget::toolButton()
{
    return m_pToolButton;
}

QWidget * CDownloadWidget::addFile(const QString& fn, int id)
{
    QFrame * widget = new QFrame;
    widget->setObjectName("downloadItem");
    widget->setAttribute(Qt::WA_Hover);
    widget->installEventFilter(this);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QGridLayout * grid = new QGridLayout;
    int mrg = qRound(MARGINS);
    int mrg_dbl = qRound(2 * MARGINS);
    grid->setContentsMargins(mrg_dbl, mrg, mrg_dbl, mrg);
    grid->setSpacing(qRound(SPACING));
    widget->setLayout(grid);

    CElipsisLabel * name = new CElipsisLabel(fn);
    name->setObjectName("labelName");
    name->setEllipsisMode(Qt::ElideRight);
    name->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignAbsolute);
    name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    grid->addWidget(name, 0, 0, 1, 1);

    QPushButton * cancel = new QPushButton(tr("Cancel"));
    cancel->setObjectName("buttonCancel");
    cancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(cancel, &QPushButton::clicked, this, [=](){
        AscAppManager::getInstance().CancelDownload(id);
    });
    grid->addWidget(cancel, 0, 1, 1, 1);

    QProgressBar * progress = new QProgressBar;
    progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    progress->setTextVisible(false);
    grid->addWidget(progress, 1, 0, 1, 2);

    QHBoxLayout *adv_lut = new QHBoxLayout;
    adv_lut->setObjectName("bottomLayout");
    adv_lut->setContentsMargins(0,0,0,0);
    adv_lut->setSpacing(qRound(4 * SPACING));
    grid->addLayout(adv_lut, 2, 0, 1, 2);

    CElipsisLabel * info = new CElipsisLabel(QString("0 %1").arg(tr("kBps")));
    info->setObjectName("labelInfo");
    info->setEllipsisMode(Qt::ElideRight);
    info->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignAbsolute);
    info->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    grid->addWidget(info, 2, 0, 1, 2);

    if (QVBoxLayout *lut = dynamic_cast<QVBoxLayout*>(m_pContentArea->layout()))
        lut->insertWidget(lut->count() - 1, widget);
    return widget;
}

void CDownloadWidget::downloadProcess(void * info)
{
    if ( info == NULL )
        return;

    CAscDownloadFileInfo * pData = reinterpret_cast<CAscDownloadFileInfo *>(info);
    int id = pData->get_IdDownload();

    std::map<int, CDownloadItem *>::iterator iter = m_mapDownloads.find(id);
    if (pData->get_IsCanceled()) {
        onFinish(id);
        if (iter != m_mapDownloads.end()) {
            QGridLayout *lut = (QGridLayout*)iter->second->widget->layout();
            if (QProgressBar *progress = qobject_cast<QProgressBar*>(lut->itemAt(2)->widget()))
                progress->setValue(0);
            if (CElipsisLabel *label_info = qobject_cast<CElipsisLabel*>(lut->itemAt(4)->widget()))
                label_info->setText(tr("Canceled"));
            if (QPushButton *cancel = qobject_cast<QPushButton*>(lut->itemAt(1)->widget())) {
                cancel->disconnect();
                cancel->setText("");
                cancel->setIcon(QIcon(":/message_warn.svg"));
            }
        }

    } else
    if (pData->get_IsComplete()) {
        onFinish(id);
        if (iter != m_mapDownloads.end()) {
            QString path = QString::fromStdWString(pData->get_FilePath());
            QGridLayout *lut = (QGridLayout*)iter->second->widget->layout();
            if (CElipsisLabel *label_info = qobject_cast<CElipsisLabel*>(lut->itemAt(4)->widget())) {
                label_info->setText(QFileInfo(path).absolutePath());
            }
            if (QPushButton *cancel = qobject_cast<QPushButton*>(lut->itemAt(1)->widget())) {
                cancel->disconnect();
                cancel->setText("");
                cancel->setIcon(QIcon(":/message_confirm.svg"));
            }
            if (QProgressBar *progress = qobject_cast<QProgressBar*>(lut->itemAt(2)->widget())) {
                lut->removeWidget(progress);
                delete progress;
            }
            QLabel *size_label = new QLabel;
            size_label->setObjectName("labelSize");
            size_label->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignAbsolute);
            size_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            size_label->setText(getFileSize(QString::fromStdWString(pData->get_FilePath())));
            lut->addWidget(size_label, 1, 0, 1, 2);

            if (QHBoxLayout *adv_lut = qobject_cast<QHBoxLayout*>(lut->itemAt(2)->layout())) {
                QPushButton *open = new QPushButton;
                open->setObjectName("buttonOpen");
                open->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                open->setText(tr("Open"));
                open->hide();
                adv_lut->addWidget(open);
                connect(open, &QPushButton::clicked, this, [=]() {
                    hide();
                    if (CCefViewEditor::GetFileFormat(path.toStdWString()) == 0) {
                        Utils::openUrl(QUrl::fromLocalFile(path).toString());
                    } else {
                        AscAppManager::handleInputCmd({path.toStdWString()});
                    }
                });

                QPushButton *open_folder = new QPushButton;
                open_folder->setObjectName("buttonOpenFolder");
                open_folder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                open_folder->setText(tr("Show in folder"));
                open_folder->hide();
                adv_lut->addWidget(open_folder);
                connect(open_folder, &QPushButton::clicked, this, [=]() {
                    hide();
                    Utils::openFileLocation(path);
                });
                adv_lut->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Preferred));
            }
        }

    } else {
        if (iter == m_mapDownloads.end()) {
            QString path = QString::fromStdWString(pData->get_FilePath()),
                    file_name = path.isEmpty() ? "Unconfirmed" : getFileName(path);
            CDownloadItem * item = new CDownloadItem(addFile(file_name, id));
            iter = m_mapDownloads.insert( std::pair<int, CDownloadItem *>(id, item) ).first;
            if (!path.isEmpty()) {
                item->is_temp = false;
                onStart();
            }
        }

        if (iter->second) {
            QLayout *lut = iter->second->widget->layout();
            if (QProgressBar *progress = qobject_cast<QProgressBar*>(lut->itemAt(2)->widget()))
                progress->setValue(pData->get_Percent());
            if (CElipsisLabel *label_info = qobject_cast<CElipsisLabel*>(lut->itemAt(4)->widget()))
                label_info->setText(QString("%1 %2").arg(QString::number(pData->get_Speed(), 'f', 0), tr("kBps")));

            if (iter->second->is_temp) {
                QString path = QString::fromStdWString(pData->get_FilePath());
                if (!path.isEmpty()) {
                    if (CElipsisLabel *label_name = static_cast<CElipsisLabel*>(lut->itemAt(0)->widget()))
                        label_name->setText(getFileName(path));
                    iter->second->is_temp = false;
                    onStart();
                }
            }
        }
    }
}

void CDownloadWidget::removeFile(MapItem iter)
{
    if (iter != m_mapDownloads.end()) {
        CDownloadItem * di = iter->second;
        layout()->removeWidget(di->widget);
        RELEASEOBJECT(di->widget)
        RELEASEOBJECT(di)
        m_mapDownloads.erase(iter);
        if (m_mapDownloads.empty()) {
            m_pToolButton->deleteLater();
            deleteLater();
        }
    }
}

QString CDownloadWidget::getFileName(const QString& path) const
{
    QFileInfo info(path);
    return info.fileName();
}

void CDownloadWidget::closeEvent(QCloseEvent *ev)
{
    ev->ignore();
    hide();
}

bool CDownloadWidget::eventFilter(QObject *obj, QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::WindowDeactivate:
        if (obj == this && !m_pToolButton->underMouse())
            hide();
        break;
    case QEvent::HoverEnter:
        if (obj->objectName() == "downloadItem")
            onHover(obj, true);
        break;
    case QEvent::HoverLeave:
        if (obj->objectName() == "downloadItem")
            onHover(obj, false);
        break;
    default:
        break;
    }
    return QWidget::eventFilter(obj, ev);
}

void CDownloadWidget::polish()
{
    style()->polish(this);
    m_mainFrame->style()->polish(m_mainFrame);
    m_titleFrame->style()->polish(m_titleFrame);
    if (QLabel *labelTitle = m_titleFrame->findChild<QLabel*>("labelTitle"))
        labelTitle->style()->polish(labelTitle);
    if (QPushButton *buttonClear = m_titleFrame->findChild<QPushButton*>("buttonClear"))
        buttonClear->style()->polish(buttonClear);
    m_pArea->style()->polish(m_pArea);
    m_pArea->verticalScrollBar()->style()->polish(m_pArea->verticalScrollBar());
    m_pContentArea->style()->polish(m_pContentArea);
    for (int i(0); i < m_pContentArea->layout()->count(); ++i) {
        auto item = m_pContentArea->layout()->itemAt(i);
        if (item && item->widget())
            polishItem(item->widget());
    }
}

void CDownloadWidget::updateScalingFactor(double factor)
{
    m_dpiRatio = factor;
    setProperty("zoom", QString::number(factor) + "x");
    int shd = 0, rad = 0;
    int mrg = qRound(MARGINS);
    int mrg_half = qRound(MARGINS/2);
    int mrg_dbl = qRound(2 * MARGINS);
    if (isCompositingEnabled()) {
        shd = qRound(SHADOW);
        rad = qRound(RADIUS);
    }
    layout()->setContentsMargins(shd, shd, shd, shd);
    m_mainFrame->layout()->setContentsMargins(0, rad, 0, rad);
    m_titleFrame->layout()->setContentsMargins(mrg_dbl, mrg_half, mrg_dbl, mrg_half);
    m_pArea->layout()->setContentsMargins(0, 0, 0, 0);
    m_pArea->verticalScrollBar()->setFixedWidth(qRound(6 * factor));
    m_pContentArea->layout()->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < m_pContentArea->layout()->count(); ++i) {
        auto item = m_pContentArea->layout()->itemAt(i);
        if (item && item->widget() && item->widget()->layout()) {
            item->widget()->layout()->setContentsMargins(mrg_dbl, mrg, mrg_dbl, mrg);
            item->widget()->layout()->setSpacing(qRound(SPACING));
            if (QHBoxLayout *adv_lut = item->widget()->layout()->findChild<QHBoxLayout*>("bottomLayout"))
                adv_lut->setSpacing(qRound(4 * SPACING));
        }
    }
    m_pToolButton->setIconSize(ICON_SIZE.toSize());
    polish();
}

void CDownloadWidget::applyTheme()
{
    QString css = Utils::readStylesheets(":/styles/download.qss");
    setStyleSheet(css.arg(GetColorQValueByRole(ecrDownloadWidgetBackground),
                          GetColorQValueByRole(ecrDownloadWidgetBorder),
                          GetColorQValueByRole(ecrDownloadItemHoverBackground),
                          GetColorQValueByRole(ecrDownloadGhostButtonText),
                          GetColorQValueByRole(ecrDownloadGhostButtonTextHover),
                          GetColorQValueByRole(ecrDownloadGhostButtonTextPressed),
                          GetColorQValueByRole(ecrDownloadGhostButtonTextPressedItemHover),
                          GetColorQValueByRole(ecrDownloadLabelText),
                          GetColorQValueByRole(ecrDownloadLabelTextInfo))
                     .arg(GetColorQValueByRole(ecrDownloadLabelTextInfoItemHover),
                          GetColorQValueByRole(ecrDownloadProgressBarChunk),
                          GetColorQValueByRole(ecrDownloadProgressBarBackground),
                          GetColorQValueByRole(ecrDownloadProgressBarBackgroundItemHover),
                          GetColorQValueByRole(ecrDownloadScrollBarHandle)));

    if (m_pToolButton->isStarted())
        m_pToolButton->setAnimatedIcon(AscAppManager::themes().current().isDark() ? ":/loading_light.svg" : ":/loading.svg");
    else
        m_pToolButton->setStaticIcon(AscAppManager::themes().current().isDark() ? ":/loading_finished_light.svg" : ":/loading_finished.svg");
}

void CDownloadWidget::onLayoutDirectionChanged()
{
    setLayoutDirection(AscAppManager::isRtlEnabled() ? Qt::RightToLeft : Qt::LeftToRight);
    if (QLabel *labelTitle = m_titleFrame->findChild<QLabel*>("labelTitle"))
        labelTitle->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignAbsolute);
    for (int i = 0; i < m_pContentArea->layout()->count(); ++i) {
        auto item = m_pContentArea->layout()->itemAt(i);
        if (item && item->widget()) {
            const auto lb_list = item->widget()->findChildren<QLabel*>();
            for (QLabel *lb : lb_list) {
                lb->setAlignment((AscAppManager::isRtlEnabled() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignAbsolute);
            }
        }
    }
}

void CDownloadWidget::onStart()
{
    if (!m_pToolButton->isVisible())
        m_pToolButton->setVisible(true);

    if (!m_pToolButton->isStarted())
        m_pToolButton->setAnimatedIcon(AscAppManager::themes().current().isDark() ? ":/loading_light.svg" : ":/loading.svg");
}

void CDownloadWidget::onFinish(int id)
{
    auto iter = m_mapDownloads.find(id);
    if (iter != m_mapDownloads.end())
        iter->second->is_finished = true;

    bool allIsCompleted = true;
    for (auto &pair : m_mapDownloads) {
        if (!pair.second->is_finished) {
            allIsCompleted = false;
            break;
        }
    }
    if (allIsCompleted)
        m_pToolButton->setStaticIcon(AscAppManager::themes().current().isDark() ? ":/loading_finished_light.svg" : ":/loading_finished.svg");
}

void CDownloadWidget::clearlAll()
{
    auto iter = m_mapDownloads.begin();
    while (iter != m_mapDownloads.end()) {
        if (iter->second->is_finished) {
            removeFile(iter);
            iter = m_mapDownloads.begin();
            continue;
        }
        iter++;
    }
}
