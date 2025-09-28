/*****************************************************************************
 * VLCLibraryWindowSplitViewManager.h: MacOS X interface module
 *****************************************************************************
 * Copyright (C) 2023 VLC authors and VideoLAN
 *
 * Authors: Claudio Cambra <developer@claudiocambra.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class VLCLibraryWindow;
@class VLCLibraryWindowNavigationSidebarViewController;
@class VLCLibraryWindowSidebarRootViewController;

typedef NS_ENUM(NSUInteger, VLCLibraryWindowSplitViewDividerIndex) {
    VLCLibraryWindowNavigationSidebarSplitViewDividerIndex = 0,
    VLCLibraryWindowLibraryTargetViewSplitViewDividerIndex,
    VLCLibraryWindowPlayQueueSidebarSplitViewDividerIndex,
};

@interface VLCLibraryWindowSplitViewController : NSSplitViewController

@property (readwrite, weak) IBOutlet VLCLibraryWindow *libraryWindow;

@property (readonly) VLCLibraryWindowNavigationSidebarViewController *navSidebarViewController;
@property (readonly) NSViewController *libraryTargetViewController;
@property (readonly) VLCLibraryWindowSidebarRootViewController *multifunctionSidebarViewController;

@property (readonly) NSSplitViewItem *navSidebarItem;
@property (readonly) NSSplitViewItem *libraryTargetViewItem;
@property (readonly) NSSplitViewItem *multifunctionSidebarItem;

@property (readwrite, nonatomic) BOOL mainVideoModeEnabled;

- (IBAction)toggleNavigationSidebar:(id)sender;
- (IBAction)toggleMultifunctionSidebar:(id)sender;

@end

NS_ASSUME_NONNULL_END
