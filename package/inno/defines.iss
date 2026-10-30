; -- Euro-Office Desktop Editors Defines --

#define sCompanyName                    "Euro-Office"
#define sIntCompanyName                 sCompanyName
#define sProductName                    "Desktop Editors"
#define sIntProductName                 "DesktopEditors"
#define sAppName                        str(sCompanyName)
#define sPackageName                    str(sIntCompanyName + "-" + sIntProductName)
#define sAppPublisher                   "Euro-Office"
#define sAppPublisherURL                "https://github.com/Euro-Office/DesktopEditors"
#define sAppSupportURL                  "https://github.com/Euro-Office/DesktopEditors/issues"
#define sAppCopyright                   str("© Ascensio System SIA " + GetDateTimeString("yyyy",,) + ". All rights reserved.")
#define sAppIconName                    "Euro-Office"
#define sOldAppIconName                 "Euro-Office Editors"
#define sAppProtocol                    'oo-office'

#define APP_PATH                        str(sIntCompanyName + "\" + sIntProductName)
#define UPD_PATH                        str(sIntProductName + "Updates")
#define APP_REG_PATH                    str("Software\" + APP_PATH)
#define APP_REG_UNINST_KEY              str(sCompanyName + " " + sProductName)
#define APP_USER_MODEL_ID               "EuroOffice.Documents.5"
#define APP_MUTEX_NAME                  "EuroOffice-DesktopEditors"
#define APPWND_CLASS_NAME               "EuroOfficeEditorsWindowClass"

#define iconsExe                        "DesktopEditors.exe"
#define NAME_EXE_OUT                    "editors.exe"

#define ASSC_APP_NAME                   "Euro-Office"
#define ASCC_REG_PREFIX                 "EuroOffice"
#define ASCC_REG_REGISTERED_APP_NAME    "Euro-Office Editors"
#define ASSOC_PROG_ID                   "EuroOffice.Editors"
#define ASSOC_APP_FRIENDLY_NAME         "Euro-Office Editors"
