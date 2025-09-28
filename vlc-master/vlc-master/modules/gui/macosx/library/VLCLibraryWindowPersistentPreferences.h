/*****************************************************************************
 * VLCLibraryWindowPersistentPreferences.h: MacOS X interface module
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

#import "VLCLibraryWindow.h"

NS_ASSUME_NONNULL_BEGIN

@interface VLCLibraryWindowPersistentPreferences : NSObject

@property (class, readonly) VLCLibraryWindowPersistentPreferences *sharedInstance;

@property (readwrite, nonatomic) VLCLibraryViewModeSegment homeLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment favoritesLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment videoLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment showsLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment moviesLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment albumLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment genreLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment songsLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment artistLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment playlistLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment musicOnlyPlaylistLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment videoOnlyPlaylistLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment browseLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment streamLibraryViewMode;
@property (readwrite, nonatomic) VLCLibraryViewModeSegment groupsLibraryViewMode;

@end

NS_ASSUME_NONNULL_END
