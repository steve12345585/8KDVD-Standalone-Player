/*****************************************************************************
 * VLCLibraryController.m: MacOS X interface module
 *****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
 *
 * Authors: Felix Paul Kühne <fkuehne # videolan -dot- org>
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

#import "VLCLibraryController.h"

#import "extensions/NSString+Helpers.h"

#import "main/VLCMain.h"

#import "playqueue/VLCPlayQueueController.h"
#import "playqueue/VLCPlayerController.h"

#import "library/VLCInputItem.h"
#import "library/VLCLibraryModel.h"
#import "library/VLCLibraryDataTypes.h"
#import "library/VLCMediaLibraryFolderObserver.h"

#import "playqueue/VLCPlayQueueItem.h"
#import "playqueue/VLCPlayQueueModel.h"

#import <vlc_media_library.h>

typedef int (*folder_action_f)(vlc_medialibrary_t*, const char*);

@interface VLCLibraryController()
{
    vlc_medialibrary_t *_p_libraryInstance;
}
@end

@implementation VLCLibraryController

- (instancetype)init
{
    self = [super init];
    if (self) {
        _p_libraryInstance = vlc_ml_instance_get(getIntf());
        if (!_p_libraryInstance) {
            msg_Info(getIntf(), "VLC runs without media library support");
            return self;
        }
        _libraryModel = [[VLCLibraryModel alloc] initWithLibrary:_p_libraryInstance];
        _unsorted = YES;

        NSNotificationCenter *defaultNotificationCenter = NSNotificationCenter.defaultCenter;
        [defaultNotificationCenter addObserver:self
                                      selector:@selector(playbackStateChanged:)
                                          name:VLCPlayerStateChanged
                                        object:nil];
        [defaultNotificationCenter addObserver:self
                                      selector:@selector(monitoredFolderChanged:)
                                          name:VLCMediaLibraryFolderFSEvent
                                        object:nil];
    }
    return self;
}

- (void)dealloc
{
    [NSNotificationCenter.defaultCenter removeObserver:self];
    _p_libraryInstance = NULL;
}

- (void)playbackStateChanged:(NSNotification *)aNotification
{
    if (!_p_libraryInstance) {
        return;
    }
    VLCPlayerController *playerController = aNotification.object;
    if (playerController.playerState == VLC_PLAYER_STATE_PLAYING) {
        dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
            vlc_ml_pause_background(self->_p_libraryInstance);
        });
    } else {
        dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
            vlc_ml_resume_background(self->_p_libraryInstance);
        });
    }
}

- (void)monitoredFolderChanged:(NSNotification *)notification
{
    NSURL * const folderURL = (NSURL *)notification.object;
    [self reloadFolderWithFileURL:folderURL];
}

- (int)appendItemToPlayQueue:(VLCMediaLibraryMediaItem *)mediaItem playImmediately:(BOOL)playImmediately
{
    if (!_p_libraryInstance) {
        return VLC_EACCES;
    }
    input_item_t *p_inputItem = vlc_ml_get_input_item(_p_libraryInstance, mediaItem.libraryID);
    if (!p_inputItem) {
        msg_Err(getIntf(), "No input item found for media id %lli", mediaItem.libraryID);
        return VLC_ENOENT;
    }
    int ret = [VLCMain.sharedInstance.playQueueController addInputItem:p_inputItem atPosition:-1 startPlayback:playImmediately];
    input_item_Release(p_inputItem);
    return ret;
}

- (int)appendItemsToPlayQueue:(NSArray <VLCMediaLibraryMediaItem *> *)mediaItemArray playFirstItemImmediately:(BOOL)playFirstItemImmediately
{
    if (!_p_libraryInstance) {
        return VLC_EACCES;
    }

    NSUInteger itemCount = [mediaItemArray count];
    int ret = VLC_SUCCESS;
    for (NSUInteger x = 0; x < itemCount; x++) {
        if (unlikely(x == 0 && playFirstItemImmediately)) {
            ret = [self appendItemToPlayQueue:mediaItemArray[x] playImmediately:YES];
        } else {
            ret = [self appendItemToPlayQueue:mediaItemArray[x] playImmediately:NO];
        }
        if (unlikely(ret != VLC_SUCCESS)) {
            break;
        }
    }
    return ret;
}

#pragma mark - folder management

- (int)performFolderAction:(folder_action_f)action withFileUrl:(NSURL *)fileURL
{
    if (!_p_libraryInstance) {
        return VLC_EACCES;
    }
    if (!fileURL) {
        return VLC_EINVAL;
    }

    return action(_p_libraryInstance, fileURL.absoluteString.UTF8String);
}

- (int)addFolderWithFileURL:(NSURL *)fileURL
{
    return [self performFolderAction:vlc_ml_add_folder withFileUrl:fileURL];
}

- (int)banFolderWithFileURL:(NSURL *)fileURL
{
    return [self performFolderAction:vlc_ml_ban_folder withFileUrl:fileURL];
}

- (int)unbanFolderWithFileURL:(NSURL *)fileURL
{
    return [self performFolderAction:vlc_ml_unban_folder withFileUrl:fileURL];
}

- (int)removeFolderWithFileURL:(NSURL *)fileURL
{
    return [self performFolderAction:vlc_ml_remove_folder withFileUrl:fileURL];
}

- (int)reloadFolderWithFileURL:(NSURL *)fileURL
{
    if (!_p_libraryInstance) {
        return VLC_EACCES;
    }
    if (!fileURL) {
        return VLC_EINVAL;
    }
    return vlc_ml_reload_folder(_p_libraryInstance, fileURL.absoluteString.UTF8String);
}

- (void)reloadMediaLibraryFoldersForInputItems:(NSArray<VLCInputItem *> *)inputItems
{
    NSArray<VLCMediaLibraryEntryPoint *> * const entryPoints = self.libraryModel.listOfMonitoredFolders;
    NSMutableSet<NSString *> * const reloadMRLs = NSMutableSet.set;
    NSMutableSet<VLCInputItem *> * const checkedInputItems = NSMutableSet.set;

    for (VLCMediaLibraryEntryPoint * const entryPoint in entryPoints) {
        for (VLCInputItem * const inputItem in inputItems) {
            if ([checkedInputItems containsObject:inputItem]) {
                continue;
            }

            if ([inputItem.decodedMRL hasPrefix:entryPoint.decodedMRL]) {
                [reloadMRLs addObject:entryPoint.MRL];
                [checkedInputItems addObject:inputItem];
                break;
            }
        }
    }

    for (NSString * const entryPointMRL in reloadMRLs) {
        NSURL * const entryPointURL = [NSURL URLWithString:entryPointMRL];
        [self reloadFolderWithFileURL:entryPointURL];
    }
}

- (int)clearHistory
{
    if (!_p_libraryInstance) {
        return VLC_EACCES;
    }
    return vlc_ml_clear_history(_p_libraryInstance, VLC_ML_HISTORY_TYPE_GLOBAL);
}

- (void)showCreatePlaylistDialogForPlayQueue
{
    VLCPlayQueueController * const playQueueController = VLCMain.sharedInstance.playQueueController;
    NSArray<VLCPlayQueueItem *> * const items = playQueueController.playQueueModel.playQueueItems;
    [self showCreatePlaylistDialogForPlayQueueItems:items];
}

- (void)showCreatePlaylistDialogForPlayQueueItems:(NSArray<VLCPlayQueueItem *> *)items
{
    if (!items || items.count == 0) {
        return;
    }

    NSAlert * const alert = [[NSAlert alloc] init];
    alert.messageText = _NS("Create New Playlist");
    alert.informativeText = _NS("Enter a name for the new playlist:");
    [alert addButtonWithTitle:_NS("Create")];
    [alert addButtonWithTitle:_NS("Cancel")];

    NSTextField * const input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    input.placeholderString = _NS("Playlist Name");
    alert.accessoryView = input;

    alert.window.initialFirstResponder = input;

    const NSModalResponse response = [alert runModal];
    if (response != NSAlertFirstButtonReturn) {
        return;
    }

    NSString * const playlistName = input.stringValue;
    if (playlistName.length == 0) {
        return;
    }

    const BOOL success = [self createPlaylistWithName:playlistName fromItems:items];
    if (success) {
        return;
    }

    NSAlert * const errorAlert = [[NSAlert alloc] init];
    errorAlert.messageText = _NS("Failed to Create Playlist");
    errorAlert.informativeText = _NS("The playlist could not be created. Please try again.");
    errorAlert.alertStyle = NSAlertStyleWarning;
    [errorAlert addButtonWithTitle:_NS("OK")];
    [errorAlert runModal];
}

- (BOOL)createPlaylistWithName:(NSString *)playlistName fromItems:(NSArray<VLCPlayQueueItem *> *)items
{
    if (!_p_libraryInstance || !playlistName || playlistName.length == 0) {
        return NO;
    }
    
    vlc_ml_playlist_t * const playlist = vlc_ml_playlist_create(_p_libraryInstance, playlistName.UTF8String);
    if (!playlist) {
        msg_Err(getIntf(), "Failed to create playlist with name: %s", playlistName.UTF8String);
        return NO;
    }
    
    const int64_t playlistId = playlist->i_id;
    vlc_ml_playlist_release(playlist);
    
    for (VLCPlayQueueItem * const item in items) {
        VLCMediaLibraryMediaItem * const mediaLibraryItem = item.mediaLibraryItem;
        
        if (!mediaLibraryItem) {
            msg_Warn(getIntf(), "No media library item found for playqueue item with name '%s'", item.title.UTF8String);
            continue;
        }
        
        const int64_t mediaId = mediaLibraryItem.libraryID;
        const int result = vlc_ml_playlist_append(_p_libraryInstance, playlistId, &mediaId, 1);
        if (result != VLC_SUCCESS) {
            msg_Warn(getIntf(), "Failed to add media library item %lld to playlist", mediaLibraryItem.libraryID);
        }
    }
    
    return YES;
}

- (BOOL)createPlaylistWithName:(NSString *)playlistName fromMediaItems:(NSArray<VLCMediaLibraryMediaItem *> *)mediaItems
{
    if (!_p_libraryInstance || !playlistName || playlistName.length == 0) {
        return NO;
    }
    
    vlc_ml_playlist_t * const playlist = vlc_ml_playlist_create(_p_libraryInstance, playlistName.UTF8String);
    if (!playlist) {
        msg_Err(getIntf(), "Failed to create playlist with name: %s", playlistName.UTF8String);
        return NO;
    }
    
    const int64_t playlistId = playlist->i_id;
    vlc_ml_playlist_release(playlist);
    
    for (VLCMediaLibraryMediaItem * const mediaItem in mediaItems) {
        const int64_t mediaId = mediaItem.libraryID;
        const int result = vlc_ml_playlist_append(_p_libraryInstance, playlistId, &mediaId, 1);
        if (result != VLC_SUCCESS) {
            msg_Warn(getIntf(), "Failed to add media library item %lld to playlist", mediaItem.libraryID);
        }
    }
    
    return YES;
}

- (void)showCreatePlaylistDialogForMediaItems:(NSArray<VLCMediaLibraryMediaItem *> *)mediaItems
{
    if (!mediaItems || mediaItems.count == 0) {
        return;
    }
    
    NSAlert * const alert = [[NSAlert alloc] init];
    alert.messageText = _NS("Create New Playlist");
    alert.informativeText = _NS("Enter a name for the new playlist:");
    [alert addButtonWithTitle:_NS("Create")];
    [alert addButtonWithTitle:_NS("Cancel")];
    
    NSTextField * const input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    input.placeholderString = _NS("Playlist Name");
    alert.accessoryView = input;
    
    alert.window.initialFirstResponder = input;
    
    const NSModalResponse response = [alert runModal];
    if (response != NSAlertFirstButtonReturn) {
        return;
    }
    
    NSString * const playlistName = input.stringValue;
    if (playlistName.length == 0) {
        return;
    }
    
    const BOOL success = [self createPlaylistWithName:playlistName fromMediaItems:mediaItems];
    if (success) {
        return;
    }
    
    NSAlert * const errorAlert = [[NSAlert alloc] init];
    errorAlert.messageText = _NS("Failed to Create Playlist");
    errorAlert.informativeText = _NS("The playlist could not be created. Please try again.");
    errorAlert.alertStyle = NSAlertStyleWarning;
    [errorAlert addButtonWithTitle:_NS("OK")];
    [errorAlert runModal];
}

- (void)sortByCriteria:(enum vlc_ml_sorting_criteria_t)sortCriteria andDescending:(bool)descending
{
    _unsorted = NO;
    _lastSortingCriteria = sortCriteria;
    _descendingLibrarySorting = descending;
    [_libraryModel sortByCriteria:sortCriteria andDescending:descending];
}

- (void)filterByString:(NSString*)filterString
{
    self.libraryModel.filterString = filterString;
}

@end
