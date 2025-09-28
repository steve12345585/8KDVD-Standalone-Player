/*****************************************************************************
 * VLCLibraryWindowPersistentPreferences.m: MacOS X interface module
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

#import "VLCLibraryWindowPersistentPreferences.h"

NSString * const VLCLibraryWindowPreferencePrefix = @"VLCLibraryWindow";

NSString * const VLCLibraryHomeLibraryViewModePreferenceKey = @"HomeLibraryViewMode";
NSString * const VLCLibraryFavoritesLibraryViewModePreferenceKey = @"FavoritesLibraryViewMode";
NSString * const VLCLibraryVideoLibraryViewModePreferenceKey = @"VideoLibraryViewMode";
NSString * const VLCLibraryShowsLibraryViewModePreferenceKey = @"ShowsLibraryViewMode";
NSString * const VLCLibraryMoviesLibraryViewModePreferenceKey = @"MoviesLibraryViewMode";
NSString * const VLCLibraryAlbumLibraryViewModePreferenceKey = @"AlbumLibraryViewMode";
NSString * const VLCLibraryGenreLibraryViewModePreferenceKey = @"GenreLibraryViewMode";
NSString * const VLCLibrarySongsLibraryViewModePreferenceKey = @"SongsLibraryViewMode";
NSString * const VLCLibraryArtistLibraryViewModePreferenceKey = @"ArtistLibraryViewMode";
NSString * const VLCLibraryPlaylistLibraryViewModePreferenceKey = @"PlaylistLibraryViewMode";
NSString * const VLCLibraryMusicOnlyPlaylistLibraryViewModePreferenceKey = 
    @"MusicOnlyPlaylistLibraryViewMode";
NSString * const VLCLibraryVideoOnlyPlaylistLibraryViewModePreferenceKey = 
    @"VideoOnlyPlaylistLibraryViewMode";
NSString * const VLCLibraryBrowseLibraryViewModePreferenceKey = @"BrowseLibraryViewMode";
NSString * const VLCLibraryStreamLibraryViewModePreferenceKey = @"StreamLibraryViewMode";
NSString * const VLCLibraryGroupsLibraryViewModePreferenceKey = @"GroupsLibraryViewMode";

@implementation VLCLibraryWindowPersistentPreferences

static VLCLibraryWindowPersistentPreferences *sharedInstance = nil;

+ (VLCLibraryWindowPersistentPreferences *)sharedInstance
{
    static dispatch_once_t pred;
    dispatch_once(&pred, ^{
        sharedInstance = [[VLCLibraryWindowPersistentPreferences alloc] init];
    });

    return sharedInstance;
}

- (NSString * const)fullLibraryWindowKey:(NSString *)partialKey
{
    return [NSString stringWithFormat:@"%@.%@", VLCLibraryWindowPreferencePrefix, partialKey];
}

- (const NSInteger)libraryViewModePreferenceWithKey:(NSString *)key
{
    NSUserDefaults * const standardUserDefaults = NSUserDefaults.standardUserDefaults;
    NSString * const fullKey = [self fullLibraryWindowKey:key];
    NSInteger viewModeValue = [standardUserDefaults integerForKey:fullKey];

    if (viewModeValue <= VLCLibrarySmallestSentinelViewModeSegment ||
        viewModeValue >= VLCLibraryLargestSentinelViewModeSegment) {

        NSLog(@"WARNING: Retrieved invalid values for library view mode. Resorting to default.");
        viewModeValue = VLCLibraryGridViewModeSegment; // Set to workable default
    }

    return viewModeValue;
}

- (void)setLibraryWindowViewModePreferenceWithKey:(NSString *)key
                                            value:(VLCLibraryViewModeSegment)viewMode
{
    NSParameterAssert(viewMode > VLCLibrarySmallestSentinelViewModeSegment &&
                      viewMode < VLCLibraryLargestSentinelViewModeSegment);

    NSUserDefaults * const standardUserDefaults = NSUserDefaults.standardUserDefaults;
    NSString * const fullKey = [self fullLibraryWindowKey:key];
    [standardUserDefaults setInteger:viewMode forKey:fullKey];
}

- (VLCLibraryViewModeSegment)homeLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryHomeLibraryViewModePreferenceKey];
}

- (void)setHomeLibraryViewMode:(VLCLibraryViewModeSegment)homeLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryHomeLibraryViewModePreferenceKey
                                              value:homeLibraryViewMode];
}

- (VLCLibraryViewModeSegment)favoritesLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryFavoritesLibraryViewModePreferenceKey];
}

- (void)setFavoritesLibraryViewMode:(VLCLibraryViewModeSegment)favoritesLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryFavoritesLibraryViewModePreferenceKey
                                              value:favoritesLibraryViewMode];
}

- (VLCLibraryViewModeSegment)videoLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryVideoLibraryViewModePreferenceKey];
}

- (void)setVideoLibraryViewMode:(VLCLibraryViewModeSegment)videoLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryVideoLibraryViewModePreferenceKey
                                              value:videoLibraryViewMode];
}

- (VLCLibraryViewModeSegment)showsLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryShowsLibraryViewModePreferenceKey];
}

- (void)setShowsLibraryViewMode:(VLCLibraryViewModeSegment)showsLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryShowsLibraryViewModePreferenceKey
                                              value:showsLibraryViewMode];
}

- (VLCLibraryViewModeSegment)moviesLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryMoviesLibraryViewModePreferenceKey];
}

- (void)setMoviesLibraryViewMode:(VLCLibraryViewModeSegment)moviesLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryMoviesLibraryViewModePreferenceKey
                                              value:moviesLibraryViewMode];
}

- (VLCLibraryViewModeSegment)albumLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryAlbumLibraryViewModePreferenceKey];
}

- (void)setAlbumLibraryViewMode:(VLCLibraryViewModeSegment)albumLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryAlbumLibraryViewModePreferenceKey
                                              value:albumLibraryViewMode];

}

- (VLCLibraryViewModeSegment)genreLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryGenreLibraryViewModePreferenceKey];
}

- (void)setGenreLibraryViewMode:(VLCLibraryViewModeSegment)genreLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryGenreLibraryViewModePreferenceKey
                                              value:genreLibraryViewMode];
}

- (VLCLibraryViewModeSegment)songsLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibrarySongsLibraryViewModePreferenceKey];
}

- (void)setSongsLibraryViewMode:(VLCLibraryViewModeSegment)songsLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibrarySongsLibraryViewModePreferenceKey
                                              value:songsLibraryViewMode];
}

- (VLCLibraryViewModeSegment)artistLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryArtistLibraryViewModePreferenceKey];
}

- (void)setArtistLibraryViewMode:(VLCLibraryViewModeSegment)artistLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryArtistLibraryViewModePreferenceKey
                                              value:artistLibraryViewMode];
}

- (VLCLibraryViewModeSegment)playlistLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryPlaylistLibraryViewModePreferenceKey];
}

- (void)setPlaylistLibraryViewMode:(VLCLibraryViewModeSegment)playlistLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryPlaylistLibraryViewModePreferenceKey
                                              value:playlistLibraryViewMode];
}

- (VLCLibraryViewModeSegment)musicOnlyPlaylistLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryMusicOnlyPlaylistLibraryViewModePreferenceKey];
}

- (void)setMusicOnlyPlaylistLibraryViewMode:(VLCLibraryViewModeSegment)musicOnlyPlaylistLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryMusicOnlyPlaylistLibraryViewModePreferenceKey
                                              value:musicOnlyPlaylistLibraryViewMode];
}

- (VLCLibraryViewModeSegment)videoOnlyPlaylistLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryVideoOnlyPlaylistLibraryViewModePreferenceKey];
}

- (void)setVideoOnlyPlaylistLibraryViewMode:(VLCLibraryViewModeSegment)videoOnlyPlaylistLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryVideoOnlyPlaylistLibraryViewModePreferenceKey
                                              value:videoOnlyPlaylistLibraryViewMode];
}

- (VLCLibraryViewModeSegment)browseLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryBrowseLibraryViewModePreferenceKey];
}

- (void)setBrowseLibraryViewMode:(VLCLibraryViewModeSegment)browseLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryBrowseLibraryViewModePreferenceKey
                                              value:browseLibraryViewMode];
}

- (VLCLibraryViewModeSegment)streamLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryStreamLibraryViewModePreferenceKey];
}

- (void)setStreamLibraryViewMode:(VLCLibraryViewModeSegment)streamLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryStreamLibraryViewModePreferenceKey
                                              value:streamLibraryViewMode];
}

- (VLCLibraryViewModeSegment)groupsLibraryViewMode
{
    return [self libraryViewModePreferenceWithKey:VLCLibraryGroupsLibraryViewModePreferenceKey];
}

- (void)setGroupsLibraryViewMode:(VLCLibraryViewModeSegment)groupsLibraryViewMode
{
    [self setLibraryWindowViewModePreferenceWithKey:VLCLibraryGroupsLibraryViewModePreferenceKey
                                              value:groupsLibraryViewMode];
}

@end
