/***************************************************************************

 openurl.library - universal URL display and browser launcher library
 Copyright (C) 1998-2005 by Troels Walsted Hansen, et al.
 Copyright (C) 2005-2013 by openurl.library Open Source Team

 This library is free software; it has been placed in the public domain
 and you can freely redistribute it and/or modify it. Please note, however,
 that some components may be under the LGPL or GPL license.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 openurl.library project: http://sourceforge.net/projects/openurllib/

 $Id: locale_h.sd 195 2013-02-20 20:48:34Z thboeckel $

***************************************************************************/

/*
** This file was created automatically by flexcat.
** It is written to be compatible with catcomp,
** so you choose, but... Do NOT edit by hand!
**/

#ifndef _LOC_H
#define _LOC_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

/****************************************************************************/

#ifdef CATCOMP_NUMBERS

#define MSG_ErrReqTitle 1
#define MSG_ErrReqGadget 2
#define MSG_Err_NoMem 3
#define MSG_Err_NoMUI 4
#define MSG_Err_NoIntuition 5
#define MSG_Err_NoUtility 6
#define MSG_Err_NoGfx 7
#define MSG_Err_NoIcon 8
#define MSG_Err_NoOpenURL 9
#define MSG_Err_NoAppClass 10
#define MSG_Err_NoWinClass 11
#define MSG_Err_NoAppListClass 13
#define MSG_Err_NoBrowserEditWinClass 14
#define MSG_Err_NoMailerEditWinClass 15
#define MSG_Err_NoFTPEditWinClass 16
#define MSG_Err_PopupPortClass 17
#define MSG_Err_PopphClass 18
#define MSG_Err_NoApp 19
#define MSG_Err_NoPrefs 20
#define MSG_Err_BadPrefs 21
#define MSG_Err_FailedSave 22
#define MSG_App_Description 100
#define MSG_App_Copyright 101
#define MSG_App_ScreenTitle 102
#define MSG_Menu_Project 200
#define MSG_Menu_About 201
#define MSG_Menu_AboutMUI 202
#define MSG_Menu_Hide 203
#define MSG_Menu_Quit 204
#define MSG_Menu_Prefs 205
#define MSG_Menu_Save 206
#define MSG_Menu_Use 207
#define MSG_Menu_LastSaved 208
#define MSG_Menu_Restore 209
#define MSG_Menu_Defaults 210
#define MSG_Menu_MUI 212
#define MSG_Win_WinTitle 300
#define MSG_Win_Labels_Browsers 301
#define MSG_Win_Labels_Mailers 302
#define MSG_Win_Labels_FTPs 303
#define MSG_Win_Labels_Misc 304
#define MSG_Win_Save 305
#define MSG_Win_Save_Help 306
#define MSG_Win_Use 307
#define MSG_Win_Use_Help 308
#define MSG_Win_Apply 309
#define MSG_Win_Apply_Help 310
#define MSG_Win_Cancel 311
#define MSG_Win_Cancel_Help 312
#define MSG_AppList_Add 400
#define MSG_AppList_Add_Help 401
#define MSG_AppList_Edit 402
#define MSG_AppList_Edit_Help 403
#define MSG_AppList_Clone 404
#define MSG_AppList_Clone_Help 405
#define MSG_AppList_Delete 406
#define MSG_AppList_Delete_Help 407
#define MSG_AppList_Disable 408
#define MSG_AppList_Disable_Help 409
#define MSG_AppList_MoveUp_Help 410
#define MSG_AppList_MoveDown_Help 411
#define MSG_Edit_Use 500
#define MSG_Edit_Use_Help 501
#define MSG_Edit_Cancel 502
#define MSG_Edit_Cancel_Help 503
#define MSG_Edit_ListName 504
#define MSG_Edit_ListPath 505
#define MSG_Edit_Definitions 506
#define MSG_Edit_ARexx 507
#define MSG_Edit_PopURL 508
#define MSG_Edit_PopScreen 509
#define MSG_Edit_Name 510
#define MSG_Edit_Name_Help 511
#define MSG_Edit_Path 512
#define MSG_Edit_Path_Help 513
#define MSG_Edit_Port 514
#define MSG_Edit_Port_Help 515
#define MSG_Edit_Show 516
#define MSG_Edit_Show_Help 517
#define MSG_Edit_Screen 518
#define MSG_Edit_Screen_Help 519
#define MSG_Edit_OpenURL 520
#define MSG_Edit_OpenURL_Help 521
#define MSG_Edit_NewWin 522
#define MSG_Edit_NewWin_Help 523
#define MSG_Browser_List_Help 600
#define MSG_Browser_WinTitle 601
#define MSG_Browser_NewBrowser 602
#define MSG_Mailer_List_Help 700
#define MSG_Mailer_WinTitle 701
#define MSG_Mailer_NewMailer 702
#define MSG_Mailer_Write 703
#define MSG_Mailer_Write_Help 704
#define MSG_Mailer_PopAddress 705
#define MSG_Mailer_Popsubject 706
#define MSG_Mailer_PopBodyText 707
#define MSG_Mailer_PopBodyFile 708
#define MSG_FTP_List_Help 800
#define MSG_FTP_WinTitle 801
#define MSG_FTP_NewFTP 802
#define MSG_FTP_RemoveURLQualifier 803
#define MSG_FTP_RemoveURLQualifier_Help 804
#define MSG_Misc_Options 900
#define MSG_Misc_Defaults 901
#define MSG_Misc_Prepend 902
#define MSG_Misc_Prepend_Help 903
#define MSG_Misc_UseMailer 904
#define MSG_Misc_UseMailer_Help 905
#define MSG_Misc_UseFTP 906
#define MSG_Misc_UseFTP_Help 907
#define MSG_Misc_Show 908
#define MSG_Misc_Show_Help 909
#define MSG_Misc_Bring 910
#define MSG_Misc_Bring_Help 911
#define MSG_Misc_Open 912
#define MSG_Misc_Open_Help 913
#define MSG_Misc_Launch 914
#define MSG_Misc_Launch_Help 915
#define MSG_About_WinTitle 1100
#define MSG_About_OK 1111
#define MSG_About_Descr 1112

#endif /* CATCOMP_NUMBERS */

/****************************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_ErrReqTitle_STR "OpenURL Error"
#define MSG_ErrReqGadget_STR "_Abort"
#define MSG_Err_NoMem_STR "Too few memory"
#define MSG_Err_NoMUI_STR "Can't open muimaster.library %ld.%ld or higher"
#define MSG_Err_NoIntuition_STR "Can't open intuition.library %ld.%ld or higher"
#define MSG_Err_NoUtility_STR "Can't open utility.library %ld.%ld or higher"
#define MSG_Err_NoGfx_STR "Can't open graphics.library %ld.%ld or higher"
#define MSG_Err_NoIcon_STR "Can't open icon.library %ld.%ld or higher"
#define MSG_Err_NoOpenURL_STR "Can't open openurl.library %ld.%ld or higher"
#define MSG_Err_NoAppClass_STR "Can't create AppClass"
#define MSG_Err_NoWinClass_STR "Can't create WinClass"
#define MSG_Err_NoAppListClass_STR "Can't create AppListClass"
#define MSG_Err_NoBrowserEditWinClass_STR "Can't create BrowserEditWinClass"
#define MSG_Err_NoMailerEditWinClass_STR "Can't create MailerEditWinClass"
#define MSG_Err_NoFTPEditWinClass_STR "Can't create FTPEditWinClass"
#define MSG_Err_PopupPortClass_STR "Can't create PopupPortClass"
#define MSG_Err_PopphClass_STR "Can't create PopphClass"
#define MSG_Err_NoApp_STR "Can't create application object"
#define MSG_Err_NoPrefs_STR "Can't obtain prefs from openurl.library"
#define MSG_Err_BadPrefs_STR "This version of this program doesn't know how\nto handle openurl.library prefs version %ld"
#define MSG_Err_FailedSave_STR "Can't save prefs"
#define MSG_App_Description_STR "OpenURL preferences application."
#define MSG_App_Copyright_STR "Public Domain Software"
#define MSG_App_ScreenTitle_STR "OpenURL"
#define MSG_Menu_Project_STR "Project"
#define MSG_Menu_About_STR "?\000About..."
#define MSG_Menu_AboutMUI_STR "!\000About MUI..."
#define MSG_Menu_Hide_STR "H\000Hide"
#define MSG_Menu_Quit_STR "Q\000Quit"
#define MSG_Menu_Prefs_STR "Preferences"
#define MSG_Menu_Save_STR "S\000Save"
#define MSG_Menu_Use_STR "U\000Use"
#define MSG_Menu_LastSaved_STR "L\000Last saved"
#define MSG_Menu_Restore_STR "R\000Restore"
#define MSG_Menu_Defaults_STR "D\000Default settings"
#define MSG_Menu_MUI_STR "M\000MUI Setting..."
#define MSG_Win_WinTitle_STR "OpenURL"
#define MSG_Win_Labels_Browsers_STR "Browsers"
#define MSG_Win_Labels_Mailers_STR "Mailers"
#define MSG_Win_Labels_FTPs_STR "FTPs"
#define MSG_Win_Labels_Misc_STR "Misc"
#define MSG_Win_Save_STR "_Save"
#define MSG_Win_Save_Help_STR "Save preferences."
#define MSG_Win_Use_STR "_Use"
#define MSG_Win_Use_Help_STR "Use preferences."
#define MSG_Win_Apply_STR "_Apply"
#define MSG_Win_Apply_Help_STR "Use preferences without closing window."
#define MSG_Win_Cancel_STR "_Cancel"
#define MSG_Win_Cancel_Help_STR "Close window, without using preferences."
#define MSG_AppList_Add_STR "A_dd"
#define MSG_AppList_Add_Help_STR "Add a new entry."
#define MSG_AppList_Edit_STR "_Edit"
#define MSG_AppList_Edit_Help_STR "Edit active entry."
#define MSG_AppList_Clone_STR "C_lone"
#define MSG_AppList_Clone_Help_STR "Clone active entry."
#define MSG_AppList_Delete_STR "Dele_te"
#define MSG_AppList_Delete_Help_STR "Delete active entry."
#define MSG_AppList_Disable_STR "D_isable"
#define MSG_AppList_Disable_Help_STR "Disable active entry."
#define MSG_AppList_MoveUp_Help_STR "Move active entry up."
#define MSG_AppList_MoveDown_Help_STR "Move active entry down."
#define MSG_Edit_Use_STR "_Use"
#define MSG_Edit_Use_Help_STR "Apply changes."
#define MSG_Edit_Cancel_STR "_Cancel"
#define MSG_Edit_Cancel_Help_STR "Close window."
#define MSG_Edit_ListName_STR "Name"
#define MSG_Edit_ListPath_STR "Path"
#define MSG_Edit_Definitions_STR "Definitions"
#define MSG_Edit_ARexx_STR "ARexx commands"
#define MSG_Edit_PopURL_STR "URL"
#define MSG_Edit_PopScreen_STR "Public screen name"
#define MSG_Edit_Name_STR "_Name"
#define MSG_Edit_Name_Help_STR "Name of entry."
#define MSG_Edit_Path_STR "_Path"
#define MSG_Edit_Path_Help_STR "Complete path to the application."
#define MSG_Edit_Port_STR "_ARexx port"
#define MSG_Edit_Port_Help_STR "Name of the application ARexx port\nto send commands to."
#define MSG_Edit_Show_STR "_Show"
#define MSG_Edit_Show_Help_STR "ARexx command to uniconify the application."
#define MSG_Edit_Screen_STR "_To front"
#define MSG_Edit_Screen_Help_STR "ARexx command to move the application\nscreen to front."
#define MSG_Edit_OpenURL_STR "_Open URL"
#define MSG_Edit_OpenURL_Help_STR "ARexx command to reach an URL."
#define MSG_Edit_NewWin_STR "N_ew window"
#define MSG_Edit_NewWin_Help_STR "ARexx command to reach an URL\nin a new window (or tab)."
#define MSG_Browser_List_Help_STR "List of browsers."
#define MSG_Browser_WinTitle_STR "Edit Browser"
#define MSG_Browser_NewBrowser_STR "New browser"
#define MSG_Mailer_List_Help_STR "List of Mailers."
#define MSG_Mailer_WinTitle_STR "Edit Mailer"
#define MSG_Mailer_NewMailer_STR "New mailer"
#define MSG_Mailer_Write_STR "_Write mail"
#define MSG_Mailer_Write_Help_STR "ARexx command to open a\n\"write a new EMail window\"."
#define MSG_Mailer_PopAddress_STR "Address"
#define MSG_Mailer_Popsubject_STR "Subject"
#define MSG_Mailer_PopBodyText_STR "Body text"
#define MSG_Mailer_PopBodyFile_STR "Body file"
#define MSG_FTP_List_Help_STR "List of FTP clients."
#define MSG_FTP_WinTitle_STR "Edit FTP client"
#define MSG_FTP_NewFTP_STR "New FTP client"
#define MSG_FTP_RemoveURLQualifier_STR "Re_move 'ftp://'"
#define MSG_FTP_RemoveURLQualifier_Help_STR "If selected, \"ftp://\" is removed\nfrom the URL."
#define MSG_Misc_Options_STR "Options"
#define MSG_Misc_Defaults_STR "Defaults"
#define MSG_Misc_Prepend_STR "_Prepend \"http://\" to URLs without a scheme"
#define MSG_Misc_Prepend_Help_STR "If selected, \"http://\" is added to any URL\nwithout a scheme, so that it is opened in the\nbrowser."
#define MSG_Misc_UseMailer_STR "S_end mailto: URLs to email application"
#define MSG_Misc_UseMailer_Help_STR "If selected, \"mailto:\" URLs are sent to\nthe mailer, otherwise they are sent to the\nbrowser."
#define MSG_Misc_UseFTP_STR "Se_nd \"ftp://\" URLs to FTP application"
#define MSG_Misc_UseFTP_Help_STR "If selected, \"fpt://\" URLs are sent to\nthe FTP client, otherwise they are sent to\nthe browser."
#define MSG_Misc_Show_STR "Un_iconify application"
#define MSG_Misc_Show_Help_STR "If selected, the application is uniconified,\nbefore sending the URL to it."
#define MSG_Misc_Bring_STR "_Bring browser screen to front"
#define MSG_Misc_Bring_Help_STR "If selected, the application screen is moved\nto front, before sending the URL."
#define MSG_Misc_Open_STR "_Open URL in new client window (or tab)"
#define MSG_Misc_Open_Help_STR "If selected, a new application window (or tab)\nis opened, before sending the URL."
#define MSG_Misc_Launch_STR "_Launch browser if one isn't running"
#define MSG_Misc_Launch_Help_STR "If selected, a new application is run, if none is found."
#define MSG_About_WinTitle_STR "About OpenURL"
#define MSG_About_OK_STR "_OK"
#define MSG_About_Descr_STR "\033bOpenURL\033n is a shared library for easily sending\nURLs from applications to browsers, mailers\nand FTP clients.\n\n\033cOpenURL is a Public Domain Software."

#endif /* CATCOMP_STRINGS */

/****************************************************************************/

#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    ULONG  cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSG_ErrReqTitle,(STRPTR)MSG_ErrReqTitle_STR},
    {MSG_ErrReqGadget,(STRPTR)MSG_ErrReqGadget_STR},
    {MSG_Err_NoMem,(STRPTR)MSG_Err_NoMem_STR},
    {MSG_Err_NoMUI,(STRPTR)MSG_Err_NoMUI_STR},
    {MSG_Err_NoIntuition,(STRPTR)MSG_Err_NoIntuition_STR},
    {MSG_Err_NoUtility,(STRPTR)MSG_Err_NoUtility_STR},
    {MSG_Err_NoGfx,(STRPTR)MSG_Err_NoGfx_STR},
    {MSG_Err_NoIcon,(STRPTR)MSG_Err_NoIcon_STR},
    {MSG_Err_NoOpenURL,(STRPTR)MSG_Err_NoOpenURL_STR},
    {MSG_Err_NoAppClass,(STRPTR)MSG_Err_NoAppClass_STR},
    {MSG_Err_NoWinClass,(STRPTR)MSG_Err_NoWinClass_STR},
    {MSG_Err_NoAppListClass,(STRPTR)MSG_Err_NoAppListClass_STR},
    {MSG_Err_NoBrowserEditWinClass,(STRPTR)MSG_Err_NoBrowserEditWinClass_STR},
    {MSG_Err_NoMailerEditWinClass,(STRPTR)MSG_Err_NoMailerEditWinClass_STR},
    {MSG_Err_NoFTPEditWinClass,(STRPTR)MSG_Err_NoFTPEditWinClass_STR},
    {MSG_Err_PopupPortClass,(STRPTR)MSG_Err_PopupPortClass_STR},
    {MSG_Err_PopphClass,(STRPTR)MSG_Err_PopphClass_STR},
    {MSG_Err_NoApp,(STRPTR)MSG_Err_NoApp_STR},
    {MSG_Err_NoPrefs,(STRPTR)MSG_Err_NoPrefs_STR},
    {MSG_Err_BadPrefs,(STRPTR)MSG_Err_BadPrefs_STR},
    {MSG_Err_FailedSave,(STRPTR)MSG_Err_FailedSave_STR},
    {MSG_App_Description,(STRPTR)MSG_App_Description_STR},
    {MSG_App_Copyright,(STRPTR)MSG_App_Copyright_STR},
    {MSG_App_ScreenTitle,(STRPTR)MSG_App_ScreenTitle_STR},
    {MSG_Menu_Project,(STRPTR)MSG_Menu_Project_STR},
    {MSG_Menu_About,(STRPTR)MSG_Menu_About_STR},
    {MSG_Menu_AboutMUI,(STRPTR)MSG_Menu_AboutMUI_STR},
    {MSG_Menu_Hide,(STRPTR)MSG_Menu_Hide_STR},
    {MSG_Menu_Quit,(STRPTR)MSG_Menu_Quit_STR},
    {MSG_Menu_Prefs,(STRPTR)MSG_Menu_Prefs_STR},
    {MSG_Menu_Save,(STRPTR)MSG_Menu_Save_STR},
    {MSG_Menu_Use,(STRPTR)MSG_Menu_Use_STR},
    {MSG_Menu_LastSaved,(STRPTR)MSG_Menu_LastSaved_STR},
    {MSG_Menu_Restore,(STRPTR)MSG_Menu_Restore_STR},
    {MSG_Menu_Defaults,(STRPTR)MSG_Menu_Defaults_STR},
    {MSG_Menu_MUI,(STRPTR)MSG_Menu_MUI_STR},
    {MSG_Win_WinTitle,(STRPTR)MSG_Win_WinTitle_STR},
    {MSG_Win_Labels_Browsers,(STRPTR)MSG_Win_Labels_Browsers_STR},
    {MSG_Win_Labels_Mailers,(STRPTR)MSG_Win_Labels_Mailers_STR},
    {MSG_Win_Labels_FTPs,(STRPTR)MSG_Win_Labels_FTPs_STR},
    {MSG_Win_Labels_Misc,(STRPTR)MSG_Win_Labels_Misc_STR},
    {MSG_Win_Save,(STRPTR)MSG_Win_Save_STR},
    {MSG_Win_Save_Help,(STRPTR)MSG_Win_Save_Help_STR},
    {MSG_Win_Use,(STRPTR)MSG_Win_Use_STR},
    {MSG_Win_Use_Help,(STRPTR)MSG_Win_Use_Help_STR},
    {MSG_Win_Apply,(STRPTR)MSG_Win_Apply_STR},
    {MSG_Win_Apply_Help,(STRPTR)MSG_Win_Apply_Help_STR},
    {MSG_Win_Cancel,(STRPTR)MSG_Win_Cancel_STR},
    {MSG_Win_Cancel_Help,(STRPTR)MSG_Win_Cancel_Help_STR},
    {MSG_AppList_Add,(STRPTR)MSG_AppList_Add_STR},
    {MSG_AppList_Add_Help,(STRPTR)MSG_AppList_Add_Help_STR},
    {MSG_AppList_Edit,(STRPTR)MSG_AppList_Edit_STR},
    {MSG_AppList_Edit_Help,(STRPTR)MSG_AppList_Edit_Help_STR},
    {MSG_AppList_Clone,(STRPTR)MSG_AppList_Clone_STR},
    {MSG_AppList_Clone_Help,(STRPTR)MSG_AppList_Clone_Help_STR},
    {MSG_AppList_Delete,(STRPTR)MSG_AppList_Delete_STR},
    {MSG_AppList_Delete_Help,(STRPTR)MSG_AppList_Delete_Help_STR},
    {MSG_AppList_Disable,(STRPTR)MSG_AppList_Disable_STR},
    {MSG_AppList_Disable_Help,(STRPTR)MSG_AppList_Disable_Help_STR},
    {MSG_AppList_MoveUp_Help,(STRPTR)MSG_AppList_MoveUp_Help_STR},
    {MSG_AppList_MoveDown_Help,(STRPTR)MSG_AppList_MoveDown_Help_STR},
    {MSG_Edit_Use,(STRPTR)MSG_Edit_Use_STR},
    {MSG_Edit_Use_Help,(STRPTR)MSG_Edit_Use_Help_STR},
    {MSG_Edit_Cancel,(STRPTR)MSG_Edit_Cancel_STR},
    {MSG_Edit_Cancel_Help,(STRPTR)MSG_Edit_Cancel_Help_STR},
    {MSG_Edit_ListName,(STRPTR)MSG_Edit_ListName_STR},
    {MSG_Edit_ListPath,(STRPTR)MSG_Edit_ListPath_STR},
    {MSG_Edit_Definitions,(STRPTR)MSG_Edit_Definitions_STR},
    {MSG_Edit_ARexx,(STRPTR)MSG_Edit_ARexx_STR},
    {MSG_Edit_PopURL,(STRPTR)MSG_Edit_PopURL_STR},
    {MSG_Edit_PopScreen,(STRPTR)MSG_Edit_PopScreen_STR},
    {MSG_Edit_Name,(STRPTR)MSG_Edit_Name_STR},
    {MSG_Edit_Name_Help,(STRPTR)MSG_Edit_Name_Help_STR},
    {MSG_Edit_Path,(STRPTR)MSG_Edit_Path_STR},
    {MSG_Edit_Path_Help,(STRPTR)MSG_Edit_Path_Help_STR},
    {MSG_Edit_Port,(STRPTR)MSG_Edit_Port_STR},
    {MSG_Edit_Port_Help,(STRPTR)MSG_Edit_Port_Help_STR},
    {MSG_Edit_Show,(STRPTR)MSG_Edit_Show_STR},
    {MSG_Edit_Show_Help,(STRPTR)MSG_Edit_Show_Help_STR},
    {MSG_Edit_Screen,(STRPTR)MSG_Edit_Screen_STR},
    {MSG_Edit_Screen_Help,(STRPTR)MSG_Edit_Screen_Help_STR},
    {MSG_Edit_OpenURL,(STRPTR)MSG_Edit_OpenURL_STR},
    {MSG_Edit_OpenURL_Help,(STRPTR)MSG_Edit_OpenURL_Help_STR},
    {MSG_Edit_NewWin,(STRPTR)MSG_Edit_NewWin_STR},
    {MSG_Edit_NewWin_Help,(STRPTR)MSG_Edit_NewWin_Help_STR},
    {MSG_Browser_List_Help,(STRPTR)MSG_Browser_List_Help_STR},
    {MSG_Browser_WinTitle,(STRPTR)MSG_Browser_WinTitle_STR},
    {MSG_Browser_NewBrowser,(STRPTR)MSG_Browser_NewBrowser_STR},
    {MSG_Mailer_List_Help,(STRPTR)MSG_Mailer_List_Help_STR},
    {MSG_Mailer_WinTitle,(STRPTR)MSG_Mailer_WinTitle_STR},
    {MSG_Mailer_NewMailer,(STRPTR)MSG_Mailer_NewMailer_STR},
    {MSG_Mailer_Write,(STRPTR)MSG_Mailer_Write_STR},
    {MSG_Mailer_Write_Help,(STRPTR)MSG_Mailer_Write_Help_STR},
    {MSG_Mailer_PopAddress,(STRPTR)MSG_Mailer_PopAddress_STR},
    {MSG_Mailer_Popsubject,(STRPTR)MSG_Mailer_Popsubject_STR},
    {MSG_Mailer_PopBodyText,(STRPTR)MSG_Mailer_PopBodyText_STR},
    {MSG_Mailer_PopBodyFile,(STRPTR)MSG_Mailer_PopBodyFile_STR},
    {MSG_FTP_List_Help,(STRPTR)MSG_FTP_List_Help_STR},
    {MSG_FTP_WinTitle,(STRPTR)MSG_FTP_WinTitle_STR},
    {MSG_FTP_NewFTP,(STRPTR)MSG_FTP_NewFTP_STR},
    {MSG_FTP_RemoveURLQualifier,(STRPTR)MSG_FTP_RemoveURLQualifier_STR},
    {MSG_FTP_RemoveURLQualifier_Help,(STRPTR)MSG_FTP_RemoveURLQualifier_Help_STR},
    {MSG_Misc_Options,(STRPTR)MSG_Misc_Options_STR},
    {MSG_Misc_Defaults,(STRPTR)MSG_Misc_Defaults_STR},
    {MSG_Misc_Prepend,(STRPTR)MSG_Misc_Prepend_STR},
    {MSG_Misc_Prepend_Help,(STRPTR)MSG_Misc_Prepend_Help_STR},
    {MSG_Misc_UseMailer,(STRPTR)MSG_Misc_UseMailer_STR},
    {MSG_Misc_UseMailer_Help,(STRPTR)MSG_Misc_UseMailer_Help_STR},
    {MSG_Misc_UseFTP,(STRPTR)MSG_Misc_UseFTP_STR},
    {MSG_Misc_UseFTP_Help,(STRPTR)MSG_Misc_UseFTP_Help_STR},
    {MSG_Misc_Show,(STRPTR)MSG_Misc_Show_STR},
    {MSG_Misc_Show_Help,(STRPTR)MSG_Misc_Show_Help_STR},
    {MSG_Misc_Bring,(STRPTR)MSG_Misc_Bring_STR},
    {MSG_Misc_Bring_Help,(STRPTR)MSG_Misc_Bring_Help_STR},
    {MSG_Misc_Open,(STRPTR)MSG_Misc_Open_STR},
    {MSG_Misc_Open_Help,(STRPTR)MSG_Misc_Open_Help_STR},
    {MSG_Misc_Launch,(STRPTR)MSG_Misc_Launch_STR},
    {MSG_Misc_Launch_Help,(STRPTR)MSG_Misc_Launch_Help_STR},
    {MSG_About_WinTitle,(STRPTR)MSG_About_WinTitle_STR},
    {MSG_About_OK,(STRPTR)MSG_About_OK_STR},
    {MSG_About_Descr,(STRPTR)MSG_About_Descr_STR},
};

#endif /* CATCOMP_ARRAY */

/****************************************************************************/

#endif /* _LOC_H */
