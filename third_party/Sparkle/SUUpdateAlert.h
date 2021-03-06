//
//  SUUpdateAlert.h
//  Sparkle
//
//  Created by Andy Matuschak on 3/12/06.
//  Copyright 2006 Andy Matuschak. All rights reserved.
//

#ifndef SUUPDATEALERT_H
#define SUUPDATEALERT_H

#import "SUWindowController.h"

typedef enum
{
	SUInstallUpdateChoice,
	SURemindMeLaterChoice,
	SUSkipThisVersionChoice
} SUUpdateAlertChoice;

@class WebView, SUAppcastItem, SUHost;
@interface SUUpdateAlert : SUWindowController {
	SUAppcastItem *updateItem;
	SUHost *host;
	id delegate;

	IBOutlet WebView *releaseNotesView;
	IBOutlet NSTextField *description;
	NSProgressIndicator *releaseNotesSpinner;
	BOOL webViewFinishedLoading;
}

- (id)initWithAppcastItem:(SUAppcastItem *)item host:(SUHost *)host;
- (void)setDelegate:delegate;

- (IBAction)installUpdate:sender;
- (IBAction)skipThisVersion:sender;
- (IBAction)remindMeLater:sender;

@end

@interface NSObject (SUUpdateAlertDelegate)
- (void)updateAlert:(SUUpdateAlert *)updateAlert finishedWithChoice:(SUUpdateAlertChoice)updateChoice;
@end

#endif
