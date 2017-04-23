
; user

!define APP_BRAND_NAME "MultiDir"
!define VERSION "0.1.0"
!define COMPANY "Gres"
!define DESCRIPTION "Multiple directory access tool"
!define APP_WORK_NAME "multidir"
!define BINARY "multidir.exe"
!define CONTENT_DIR "."
!include /NONFATAL defines.nsi

; calculated and defaults

!define ICON "${CONTENT_DIR}\icon.ico"
!define UNINSTALLER "uninstall.exe"
!define BRANDING "${APP_BRAND_NAME} ver. ${VERSION}"
!define OUT_README "readme.txt"
!define VC_REDIST_VERSION "2015"
!define VC_REDIST_REG "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86"
!define VC_REDIST_LINK "https://download.microsoft.com/download/6/A/A/6AA4EDFF-645B-48C5-81CC-ED5963AEAD48/vc_redist.x86.exe"
!define VC_REDIST_TMP "$TEMP\vc_redist_2015_x86.exe"
!define REG_ROOT "HKCU"
!define REG_APP "Software\${COMPANY}\${APP_WORK_NAME}"
!define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_WORK_NAME}"
!define REG_AUTOSTART "Software\Microsoft\Windows\CurrentVersion\Run"
!define REG_SM_VALUE "StartMenuFolder"

; base

Unicode true
Name ${APP_BRAND_NAME}
OutFile "${APP_WORK_NAME}-${VERSION}.exe"
InstallDir "$PROGRAMFILES32\${APP_WORK_NAME}"
InstallDirRegKey ${REG_ROOT} ${REG_APP} ""
RequestExecutionLevel user
SetCompressor /SOLID lzma
BrandingText "${BRANDING}"

VIAddVersionKey "ProductName" "${APP_BRAND_NAME}"
VIAddVersionKey "CompanyName" "${COMPANY}"
VIAddVersionKey "LegalCopyright" "© ${COMPANY}"
VIAddVersionKey "FileDescription" "${DESCRIPTION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIProductVersion ${VERSION}.0
VIFileVersion ${VERSION}.0

; pages

!include "MUI2.nsh"
!define MUI_ICON ${ICON}
!define MUI_ABORTWARNING
!insertmacro MUI_RESERVEFILE_LANGDLL

!insertmacro MUI_PAGE_LICENSE $(LICENSE)

!define MUI_PAGE_CUSTOMFUNCTION_PRE onComponentsEnter
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_DIRECTORY

Var START_DIR
!define MUI_STARTMENUPAGE_REGISTRY_ROOT ${REG_ROOT}
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${REG_APP}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME ${REG_SM_VALUE}
!insertmacro MUI_PAGE_STARTMENU 0 $START_DIR

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${BINARY}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\${OUT_README}"
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!insertmacro MUI_PAGE_FINISH


!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

; sections

Section !$(SECT_APP) SECT_APP_ID
    SectionIn RO
    SetOutPath "$INSTDIR"
    File "${CONTENT_DIR}\app\${BINARY}"
    StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +2
        File /oname=${OUT_README} "${CONTENT_DIR}\Changelog_en.txt"
    StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +2
        File /oname=${OUT_README} "${CONTENT_DIR}\Changelog_ru.txt"
    File /oname=icon.ico ${ICON}
    File "${CONTENT_DIR}\app\*.dll"
    SetOutPath "$INSTDIR\platforms"
    File "${CONTENT_DIR}\app\platforms\*.dll"
SectionEnd


Var APP_SIZE
Var IS_PORTABLE
Section -Registry
    IntCmp $IS_PORTABLE ${SF_SELECTED} done_registry
        SetOutPath "$INSTDIR"
        WriteUninstaller "$INSTDIR\${UNINSTALLER}"

        WriteRegStr   ${REG_ROOT} "${REG_APP}" "" $INSTDIR
        WriteRegStr   ${REG_ROOT} "${REG_UNINSTALL}" "DisplayName" "${APP_BRAND_NAME}"
        WriteRegStr   ${REG_ROOT} "${REG_UNINSTALL}" "DisplayVersion" "${VERSION}"
        WriteRegStr   ${REG_ROOT} "${REG_UNINSTALL}" "Publisher" "${COMPANY}"
        WriteRegDWORD ${REG_ROOT} "${REG_UNINSTALL}" "EstimatedSize" $APP_SIZE
        WriteRegStr   ${REG_ROOT} "${REG_UNINSTALL}" "UninstallString" '"$INSTDIR\${UNINSTALLER}"'
        WriteRegStr   ${REG_ROOT} "${REG_UNINSTALL}" "DisplayIcon" '"$INSTDIR\icon.ico"'
        WriteRegDWORD ${REG_ROOT} "${REG_UNINSTALL}" "NoModify" 1
        WriteRegDWORD ${REG_ROOT} "${REG_UNINSTALL}" "NoRepair" 1
    done_registry:
SectionEnd


Section -StartMenu
    !insertmacro MUI_STARTMENU_WRITE_BEGIN 0
        CreateDirectory "$SMPROGRAMS\$START_DIR"
        CreateShortcut "$SMPROGRAMS\$START_DIR\${APP_BRAND_NAME}.lnk" "$INSTDIR\${BINARY}"
        CreateShortcut "$SMPROGRAMS\$START_DIR\$(UNINSTALL_LINK).lnk" "$INSTDIR\${UNINSTALLER}"
    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd


Var GOT_VCREDIST 
Section "" SECT_VCREDIST_ID
    StrCmp $GOT_VCREDIST "-" done_vcredist
        SetOutPath "$TEMP"
	File ${CONTENT_DIR}\app\vcredist_x86.exe
        ExecWait '"$TEMP\vcredist_x86.exe"'
	Delete "$TEMP\vcredist_x86.exe"
        ;NSISdl::download ${VC_REDIST_LINK} ${VC_REDIST_TMP}
        ;Pop $R0
        ;StrCmp $R0 "success" success_vcredist
            ;MessageBox MB_OK|MB_ICONEXCLAMATION $(VC_DOWNLOAD_FAIL)
            ;Goto done_vcredist
        ;success_vcredist:
            ;ExecWait '${VC_REDIST_TMP}'
    done_vcredist:
SectionEnd


Section $(SECT_DESKTOP) SECT_DESKTOP_ID
    CreateShortcut "$DESKTOP\${APP_BRAND_NAME}.lnk" "$INSTDIR\${BINARY}" ""
SectionEnd


Section $(SECT_AUTOSTART) SECT_AUTOSTART_ID
    WriteRegStr ${REG_ROOT} "${REG_AUTOSTART}" "${APP_BRAND_NAME}" "$INSTDIR\${BINARY}"
SectionEnd

Section /o $(SECT_PORTABLE) SECT_PORTABLE_ID
SectionEnd


Section "Uninstall"
    ExecWait '"taskkill.exe" /IM ${BINARY} /T /F'
    RMDir /r "$INSTDIR"

    !insertmacro MUI_STARTMENU_GETFOLDER 0 $R0
    RMDir /r "$SMPROGRAMS\$R0"
    DeleteRegValue ${REG_ROOT} "${REG_APP}" "${REG_SM_VALUE}"

    Delete "$DESKTOP\${APP_BRAND_NAME}.lnk"

    DeleteRegKey ${REG_ROOT} "${REG_UNINSTALL}"
    DeleteRegValue ${REG_ROOT} "${REG_AUTOSTART}" "${APP_BRAND_NAME}"
    DeleteRegKey /ifempty ${REG_ROOT} ${REG_APP}
SectionEnd

; functions
 
Function .onSelChange
    SectionGetFlags ${SECT_PORTABLE_ID} $IS_PORTABLE
FunctionEnd

Function onComponentsEnter
    SectionGetSize ${SECT_APP_ID} $APP_SIZE
    
    ReadRegStr $1 HKLM ${VC_REDIST_REG} "Installed"
    StrCmp $1 "1" installed
        SectionSetText ${SECT_VCREDIST_ID} $(SECT_VCREDIST)
        StrCpy $GOT_VCREDIST "-"
    installed:
FunctionEnd


Function .onInit
    !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

; localization

LicenseLangString LICENSE ${LANG_ENGLISH} "${CONTENT_DIR}\LICENSE_en.md"
LicenseLangString LICENSE ${LANG_RUSSIAN} "${CONTENT_DIR}\LICENSE_ru.md"
LicenseData $(LICENSE)

LangString SECT_APP ${LANG_ENGLISH} "${APP_BRAND_NAME}"
LangString SECT_APP ${LANG_RUSSIAN} "${APP_BRAND_NAME}"
LangString SECT_APP_ID_I18N ${LANG_ENGLISH} "Application"
LangString SECT_APP_ID_I18N ${LANG_RUSSIAN} "Приложение"

LangString SECT_DESKTOP ${LANG_ENGLISH} "Desktop"
LangString SECT_DESKTOP ${LANG_RUSSIAN} "Рабочий стол"
LangString SECT_DESKTOP_ID_I18N ${LANG_ENGLISH} "Create desktop shortcut"
LangString SECT_DESKTOP_ID_I18N ${LANG_RUSSIAN} "Создать ярлык на рабочем столе"

LangString SECT_AUTOSTART ${LANG_ENGLISH} "Autostart"
LangString SECT_AUTOSTART ${LANG_RUSSIAN} "Автозапуск"
LangString SECT_AUTOSTART_ID_I18N ${LANG_ENGLISH} "Start with Windows"
LangString SECT_AUTOSTART_ID_I18N ${LANG_RUSSIAN} "Запускать при старте Windows"

LangString SECT_PORTABLE ${LANG_ENGLISH} "Portable"
LangString SECT_PORTABLE ${LANG_RUSSIAN} "Portable"
LangString SECT_PORTABLE_ID_I18N ${LANG_ENGLISH} "Do not make uninstaller and register in system."
LangString SECT_PORTABLE_ID_I18N ${LANG_RUSSIAN} "Не создавать деинсталлятор и не регистрироваться в системе."

LangString SECT_VCREDIST ${LANG_ENGLISH} "Libraries"
LangString SECT_VCREDIST ${LANG_RUSSIAN} "Библиотеки"
LangString SECT_VCREDIST_ID_I18N ${LANG_ENGLISH} "Download and install vc redist ${VC_REDIST_VERSION}."
LangString SECT_VCREDIST_ID_I18N ${LANG_RUSSIAN} "Загрузить и установить vc redist ${VC_REDIST_VERSION}."

LangString VC_DOWNLOAD_FAIL ${LANG_ENGLISH} "Unable to download vc redist ${VC_REDIST_VERSION} x86. Please, install it manually."
LangString VC_DOWNLOAD_FAIL ${LANG_RUSSIAN} "Не удалось загрузить пакет vc redist ${VC_REDIST_VERSION} x86. Пожалуйста, установите его самостоятельно."

LangString UNINSTALL_LINK ${LANG_ENGLISH} "Uninstall"
LangString UNINSTALL_LINK ${LANG_RUSSIAN} "Удалить"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SECT_APP_ID} $(SECT_APP_ID_I18N)
!insertmacro MUI_DESCRIPTION_TEXT ${SECT_DESKTOP_ID} $(SECT_DESKTOP_ID_I18N)
!insertmacro MUI_DESCRIPTION_TEXT ${SECT_AUTOSTART_ID} $(SECT_AUTOSTART_ID_I18N)
!insertmacro MUI_DESCRIPTION_TEXT ${SECT_PORTABLE_ID} $(SECT_PORTABLE_ID_I18N)
!insertmacro MUI_DESCRIPTION_TEXT ${SECT_VCREDIST_ID} $(SECT_VCREDIST_ID_I18N)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
