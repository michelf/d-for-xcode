
// D for Xcode Launcher
// Copyright (C) 2007-2008  Michel Fortin
//
// D for Xcode is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
//
// D for Xcode is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
// more details.
//
// You should have received a copy of the GNU General Public License
// along with D for Xcode; if not, write to the Free Software Foundation, 
// Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

// This launcher program respond to open and print documents AppleEvents 
// by passing them to Xcode.

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

/** Extract files from event and launch them using launchOptions. */
void openFromAppleEvent(const AppleEvent *event, LSLaunchFlags launchOption) {
    AEDescList documentList;
    OSErr err = AEGetParamDesc(event, keyDirectObject, typeAEList, &documentList);
    if (err != noErr) return;
	
	long documentCount;
	err = AECountItems(&documentList, &documentCount);
	if (err == noErr) {
		unsigned i;
		FSRef *documents = malloc(sizeof(FSRef)*documentCount);
		if (!documents) return;
		
		for (i = 0; i < documentCount; i++) {
			AEKeyword keyword;
			DescType returnedType;
			Size actualSize;
			err = AEGetNthPtr(&documentList, i+1, typeFSRef, &keyword,
				&returnedType, (Ptr)&documents[i], sizeof(FSRef), &actualSize);
			if (err != noErr) break;
		}
		if (err == noErr) {
			LSApplicationParameters params;
			FSRef app;
			Boolean isDir;

			err = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("com.apple.xcode"), NULL, &app, NULL);
			
			if (err == noErr) {
				params.version = 0;
				params.flags = launchOption;
				params.application = &app;
				params.asyncLaunchRefCon = NULL;
				params.environment = NULL;
				params.argv = NULL;
				params.initialEvent = NULL;
			
				LSOpenItemsWithRole(documents, documentCount,
					0, NULL, &params, NULL, 0);
			}
		}
			
		if (documents) free(documents);
	}
	AEDisposeDesc(&documentList);
}

// Event handler functions.

OSErr handleOpenDocument(const AppleEvent *event, AppleEvent *reply, long handlerRefcon) {
	openFromAppleEvent(event, kLSLaunchDefaults);
	return noErr;
}

OSErr handlePrintDocument(const AppleEvent *event, AppleEvent *reply, long handlerRefcon) {
	openFromAppleEvent(event, kLSLaunchAndPrint);
	return noErr;
}

// Expiration timer callback

pascal void doQuit(EventLoopTimerRef theTimer, void *userData) {
	QuitApplicationEventLoop();
}

int main(unsigned int argc, char **argv) {
	// Install Apple Event handler
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, 
		&handleOpenDocument, 0, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, 
		&handlePrintDocument, 0, FALSE);
	
	{
		// Installing quit timer after some fraction of a second
		EventLoopTimerUPP timerUPP = NewEventLoopTimerUPP(&doQuit);
		EventLoopTimerRef theTimer;
	
		InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0.1, 
			timerUPP, NULL, &theTimer);
	}
	
	RunApplicationEventLoop();
	
	return 0;
}