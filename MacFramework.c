/*	File:		MacFramework.c	Contains:	Basic Macintosh Functions for window, menu handling and similar things for the 				SG/vdig environment.	Written by: 		Copyright:	Copyright � 1994-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/28/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/// INCLUDES#include <DiskInit.h>#include <SegLoad.h>#include <ToolUtils.h>#include <Devices.h>#include <Fonts.h>#include "DTSQTUtilities.h"#include "AppConfiguration.h"#include "MacFramework.h"// GLOBALSBoolean gQuitFlag = false;										// Flag that keeps track of termination state.unsigned long gWNEsleep = kWNEDefaultSleep;		// WaitNextEvent sleep time.// PURE MAC TOOLBOX FUNCTIONS// ______________________________________________________________________void InitMacEnvironment(long nMasters){	long i;	MaxApplZone();		for(i = 0; i <nMasters; i++)		MoreMasters();		InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	FlushEvents(everyEvent, 0);	TEInit();	InitCursor();	InitDialogs(NULL);}// ______________________________________________________________________void InitStack(long extraStackSpace){	Ptr size = GetApplLimit();	SetApplLimit(size - extraStackSpace);	// make room on the stack}// ______________________________________________________________________Boolean InitMenubar(void){	Handle aMenuHandle = NULL;		aMenuHandle = GetNewMBar(mMenubar); DebugAssert(aMenuHandle != NULL);	if(aMenuHandle == NULL)	{		ShowWarning("\pCould not find the Menubar resource!", 0);		return false;	}		SetMenuBar(aMenuHandle);	DisposeHandle(aMenuHandle);  DebugAssert(MemError() == noErr);		AppendResMenu(GetMenuHandle(mApple), 'DRVR');	DrawMenuBar();	return true;}// ______________________________________________________________________void HandleMenuCommand(long theMenuResult){	short		 	aMenuID, aMenuItem;	Str255			daName;	WindowRef	whichWindow;		aMenuID = HiWord(theMenuResult);	aMenuItem = LoWord(theMenuResult);		switch(aMenuID)	{		// APPLE MENU		case mApple:			switch(aMenuItem)			{				case iAbout:	// about box					ShowAboutDialogBox();	 					break;								default:	 // Apple menu handling					GetMenuItemText(GetMenuHandle(mApple), aMenuItem, daName);					(void)OpenDeskAcc(daName);					break;			} // end switch(aMenuItem)			break;		// FILE MENU					case mFile:			switch(aMenuItem)			{				case iNew:					{						CreateSGEnviroment();					}					break;								case iClose:					{						if( (whichWindow = FrontWindow() ) != NULL)						{								if(IsAppWindow(whichWindow))								DoDestroyMovieWindow(whichWindow);						}					}					break;				case iQuit:					{						gQuitFlag = true;						break;					}			} // end switch(aMenuItem), mFile			break;		default:		HandleApplicationMenu(aMenuID, aMenuItem);		break;	} // end switch(aMenuID)		HiliteMenu(0);}// ______________________________________________________________________void AdjustMenus(void){	WindowRef 			aWindow;		aWindow = FrontWindow();	if(aWindow != NULL)	{		// Enable the close entry of we have windows = movies.		EnableItem( GetMenuHandle(mFile), iClose);	} // end if(aWindow != NULL)	else 	{		DisableItem(GetMenuHandle(mFile), iClose);			}		AdjustApplicationMenus();					// fix any specific app menus as well.}// ______________________________________________________________________void MainEventLoop(void){	EventRecord 						anEvent;	WindowRef						whichWindow;	Boolean								aMovieEvent;	short								aWindowPart;	Rect									aRefreshArea;	Point									aPoint  = {100, 100};		while(!gQuitFlag)	{		WaitNextEvent(everyEvent, &anEvent, gWNEsleep, NULL);		#ifdef USESIOUX		SIOUXHandleOneEvent(&anEvent);#endif USESIOUX		AdjustMenus();		aMovieEvent = false;				if( (whichWindow = FrontWindow() ) != NULL)			DoIdle(whichWindow);		switch(anEvent.what)		{			case mouseDown:				aWindowPart = FindWindow(anEvent.where, &whichWindow);				// Window related events:							switch(aWindowPart)				{					case inMenuBar:						HandleMenuCommand(MenuSelect(anEvent.where));						break;						case inContent:						SelectWindow(whichWindow);						HandleContentClick(whichWindow, &anEvent);						break;										case inDrag:						DoDragWindow(whichWindow, &anEvent);						break;										case inGoAway:						// if the window is closed, dispose the movie, the controller and the window						if( TrackGoAway(whichWindow, anEvent.where) )							DoDestroyMovieWindow(whichWindow);						break;				} // end switch(aWindowPart):				break;				// System level events:				case updateEvt:					whichWindow = (WindowRef)anEvent.message;					aRefreshArea = ((**(whichWindow->visRgn)).rgnBBox);					DoUpdateWindow(whichWindow, &aRefreshArea);					break;									case keyDown:				case autoKey:					HandleKeyPress(&anEvent);					break;								case diskEvt:					if(HiWord(anEvent.message) != noErr)						(void)DIBadMount(aPoint, anEvent.message);					break;								case activateEvt:					whichWindow = (WindowRef)anEvent.message;										 if ( IsAppWindow(whichWindow) )					{						DoActivateWindow(whichWindow, ((anEvent.modifiers & activeFlag) != 0 ));					}					break;									case osEvt:					switch(( anEvent.message > 24) & 0x00FF )		// get high byte of word					{						case suspendResumeMessage:							if( FrontWindow() )							{								DoActivateWindow(FrontWindow(), !((anEvent.message & resumeFlag) == 0));							}							break;												case mouseMovedMessage:							break;					} // end switch(anEvent.message > 24) & 0x00FF)						break;								case nullEvent:					if(( whichWindow = FrontWindow() ) != NULL)						DoIdle(whichWindow);					break;		} // end switch(anEvent.what)	} // end while(!gQuitFlag)}// ______________________________________________________________________Boolean IsAppWindow(WindowRef theWindow){	short aWindowKind;		if (theWindow == NULL)		return false;	else	{		aWindowKind = ((WindowPeek)theWindow)->windowKind;		return ( (aWindowKind >= userKind) || (aWindowKind == dialogKind) );	}}// ______________________________________________________________________void HandleKeyPress(EventRecord *theEvent){	char aKey;		aKey = theEvent->message & charCodeMask;		if(theEvent->modifiers & cmdKey)		// command key down?		HandleMenuCommand(MenuKey(aKey));}// ______________________________________________________________________void ShowAboutDialogBox(void){	DialogPtr aDialog;	short 		itemHit;	FontInfo	aFontInfo;	GrafPtr		aSavedPort;		GetPort(&aSavedPort);	aDialog = GetNewDialog(kAboutBox, NULL, (WindowPtr) - 1L); DebugAssert(aDialog != NULL);	SetPort(aDialog);	// Change font to Geneva, 9pt, bold, just for the sake of it...	TextFont(applFont); TextSize(9); TextFace(bold);	GetFontInfo(&aFontInfo);		(*((DialogPeek)aDialog)->textH)->txFont = applFont;	(*((DialogPeek)aDialog)->textH)->txSize = 9;	(*((DialogPeek)aDialog)->textH)->lineHeight = aFontInfo.ascent + aFontInfo.descent + aFontInfo.leading;	(*((DialogPeek)aDialog)->textH)->fontAscent = aFontInfo.ascent;	SetDialogDefaultItem(aDialog, 1);			do	{		ModalDialog(NULL, &itemHit);	} while(itemHit != ok);		SetPort(aSavedPort);	DisposeDialog(aDialog);  DebugAssert(MemError() == noErr);}// ______________________________________________________________________void ShowWarning(Str255 theMessage, OSErr theErr){	Str255 errString;		NumToString(theErr, errString);	ParamText("\pWarning!", theMessage, theErr ? errString:  NULL, NULL);	Alert(kAlertError, NULL);}// ______________________________________________________________________void DoDestroyMovieWindow(WindowRef theWindow){	DoCloseWindow(theWindow);	DisposeWindow(theWindow); DebugAssert(MemError() == noErr);		CompactMem(0xFFFFFFFF);		//We might as well compact the mem here for getting better performance later.}// ______________________________________________________________________void DoActivateWindow(WindowRef theWindow, Boolean becomingActive){	#pragma unused(becomingActive)	WindowObject 		aWindowObject = NULL;	MovieController	mc = NULL;	GrafPtr					aSavedPort = NULL;		GetPort(&aSavedPort);	SetPort((GrafPtr)theWindow);		// @@@ Do something related to activation of movie here.		SetPort(aSavedPort);}