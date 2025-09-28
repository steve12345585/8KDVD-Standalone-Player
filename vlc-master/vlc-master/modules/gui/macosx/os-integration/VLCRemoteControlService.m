/*****************************************************************************
 * VLCRemoteControlService.m: MacOS X interface module
 *****************************************************************************
 * Copyright (C) 2017-2021 VLC authors and VideoLAN
 *
 * Authors: Carola Nitz <nitz.carola # gmail.com>
 *          Felix Paul Kühne <fkuehne # videolan.org>
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

#import <MediaPlayer/MediaPlayer.h>

#import "VLCRemoteControlService.h"
#import "main/VLCMain.h"
#import "main/CompatibilityFixes.h"
#import "playqueue/VLCPlayQueueController.h"
#import "playqueue/VLCPlayerController.h"
#import "library/VLCInputItem.h"
#import "extensions/NSString+Helpers.h"

#define kVLCSettingPlaybackForwardSkipLength @(60)
#define kVLCSettingPlaybackBackwardSkipLength @(60)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpartial-availability"

@interface VLCRemoteControlService()
{
    VLCPlayQueueController *_playQueueController;
    VLCPlayerController *_playerController;
}
@end

@implementation VLCRemoteControlService

static inline NSArray * RemoteCommandCenterCommandsToHandle()
{
    MPRemoteCommandCenter *cc = [MPRemoteCommandCenter sharedCommandCenter];
    NSMutableArray *commands = [NSMutableArray arrayWithObjects:
                                cc.playCommand,
                                cc.pauseCommand,
                                cc.stopCommand,
                                cc.togglePlayPauseCommand,
                                cc.nextTrackCommand,
                                cc.previousTrackCommand,
                                cc.skipForwardCommand,
                                cc.skipBackwardCommand,
                                cc.changePlaybackPositionCommand,
                                nil];
    return [commands copy];
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _playQueueController = VLCMain.sharedInstance.playQueueController;
        _playerController = [_playQueueController playerController];

        NSNotificationCenter *notificationCenter = NSNotificationCenter.defaultCenter;
        [notificationCenter addObserver:self
                               selector:@selector(playbackPositionUpdated:)
                                   name:VLCPlayerTimeAndPositionChanged
                                 object:nil];
        [notificationCenter addObserver:self
                               selector:@selector(metaDataChangedForCurrentMedia:)
                                   name:VLCPlayerMetadataChangedForCurrentMedia
                                 object:nil];
        [notificationCenter addObserver:self
                               selector:@selector(playbackRateChanged:)
                                   name:VLCPlayerRateChanged
                                 object:nil];
        [notificationCenter addObserver:self
                               selector:@selector(metaDataChangedForCurrentMedia:)
                                   name:VLCPlayerCurrentMediaItemChanged
                                 object:nil];
        [notificationCenter addObserver:self
                               selector:@selector(playbackStateChanged:)
                                   name:VLCPlayerStateChanged
                                 object:nil];
    }
    return self;
}

- (void)dealloc
{
    [NSNotificationCenter.defaultCenter removeObserver:self];
}

- (void)playbackStateChanged:(NSNotification *)aNotification
{
    enum vlc_player_state playerState = _playerController.playerState;
    MPNowPlayingInfoCenter *nowPlayingInfoCenter = [MPNowPlayingInfoCenter defaultCenter];

    switch (playerState) {
        case VLC_PLAYER_STATE_PLAYING:
            nowPlayingInfoCenter.playbackState = MPNowPlayingPlaybackStatePlaying;
            break;

        case VLC_PLAYER_STATE_PAUSED:
            nowPlayingInfoCenter.playbackState = MPNowPlayingPlaybackStatePaused;
            break;

        case VLC_PLAYER_STATE_STOPPED:
        case VLC_PLAYER_STATE_STOPPING:
            nowPlayingInfoCenter.playbackState = MPNowPlayingPlaybackStateStopped;
            break;

        default:
            nowPlayingInfoCenter.playbackState = MPNowPlayingPlaybackStateUnknown;
            break;
    }
}

- (void)playbackPositionUpdated:(NSNotification *)aNotification
{
    MPNowPlayingInfoCenter *nowPlayingInfoCenter = [MPNowPlayingInfoCenter defaultCenter];

    NSMutableDictionary *currentlyPlayingTrackInfo = [nowPlayingInfoCenter.nowPlayingInfo mutableCopy];
    [self setTimeInformationForDictionary:currentlyPlayingTrackInfo];

    nowPlayingInfoCenter.nowPlayingInfo = currentlyPlayingTrackInfo;
}

- (void)playbackRateChanged:(NSNotification *)aNotification
{
    MPNowPlayingInfoCenter *nowPlayingInfoCenter = [MPNowPlayingInfoCenter defaultCenter];

    NSMutableDictionary *currentlyPlayingTrackInfo = [nowPlayingInfoCenter.nowPlayingInfo mutableCopy];
    [self setRateInformationForDictionary:currentlyPlayingTrackInfo];

    nowPlayingInfoCenter.nowPlayingInfo = currentlyPlayingTrackInfo;
}

- (void)metaDataChangedForCurrentMedia:(NSNotification *)aNotification
{
    VLCInputItem * const inputItem = _playerController.currentMedia;

    NSMutableDictionary * const currentlyPlayingTrackInfo = [NSMutableDictionary dictionary];
    [self setTimeInformationForDictionary:currentlyPlayingTrackInfo];
    [self setRateInformationForDictionary:currentlyPlayingTrackInfo];

    currentlyPlayingTrackInfo[MPMediaItemPropertyTitle] = inputItem.title;
    currentlyPlayingTrackInfo[MPMediaItemPropertyArtist] = inputItem.artist;
    currentlyPlayingTrackInfo[MPMediaItemPropertyAlbumTitle] = inputItem.album;
    currentlyPlayingTrackInfo[MPMediaItemPropertyAlbumTrackNumber] = @([inputItem.trackNumber intValue]);

    const vlc_tick_t duration = inputItem.duration;
    currentlyPlayingTrackInfo[MPMediaItemPropertyPlaybackDuration] = @(SEC_FROM_VLC_TICK(duration));
    currentlyPlayingTrackInfo[MPNowPlayingInfoPropertyIsLiveStream] = @(duration <= 0);

    NSURL * const artworkURL = inputItem.artworkURL;
    if (artworkURL) {
        NSImage * const coverArtImage = [[NSImage alloc] initWithContentsOfURL:artworkURL];
        if (coverArtImage) {
            MPMediaItemArtwork * const mpartwork = [[MPMediaItemArtwork alloc] initWithBoundsSize:coverArtImage.size
                                                                                   requestHandler:^NSImage* _Nonnull(CGSize size) {
                return coverArtImage;
            }];
            currentlyPlayingTrackInfo[MPMediaItemPropertyArtwork] = mpartwork;
        }
    }

    [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = currentlyPlayingTrackInfo;
}

- (void)setTimeInformationForDictionary:(NSMutableDictionary *)dictionary
{
    /* we don't set the duration here because this would add a dependency on the input item
     * additionally, when duration changes, the metadata callback is triggered anyway */
    dictionary[MPNowPlayingInfoPropertyElapsedPlaybackTime] = @(SEC_FROM_VLC_TICK([_playerController time]));
    dictionary[MPNowPlayingInfoPropertyPlaybackProgress] = @([_playerController position]);
}

- (void)setRateInformationForDictionary:(NSMutableDictionary *)dictionary
{
    dictionary[MPNowPlayingInfoPropertyPlaybackRate] = @([_playerController playbackRate]);
}

- (void)subscribeToRemoteCommands
{
    MPRemoteCommandCenter *commandCenter = [MPRemoteCommandCenter sharedCommandCenter];

    //Enable when you want to support these
    commandCenter.ratingCommand.enabled = NO;
    commandCenter.likeCommand.enabled = NO;
    commandCenter.dislikeCommand.enabled = NO;
    commandCenter.bookmarkCommand.enabled = NO;
    commandCenter.enableLanguageOptionCommand.enabled = NO;
    commandCenter.disableLanguageOptionCommand.enabled = NO;
    commandCenter.seekForwardCommand.enabled = NO;
    commandCenter.seekBackwardCommand.enabled = NO;

    commandCenter.skipForwardCommand.preferredIntervals = @[kVLCSettingPlaybackForwardSkipLength];
    commandCenter.skipBackwardCommand.preferredIntervals = @[kVLCSettingPlaybackBackwardSkipLength];

    for (MPRemoteCommand *command in RemoteCommandCenterCommandsToHandle()) {
        [command addTarget:self action:@selector(remoteCommandEvent:)];
    }

}

- (void)unsubscribeFromRemoteCommands
{
    [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;

    for (MPRemoteCommand *command in RemoteCommandCenterCommandsToHandle()) {
        [command removeTarget:self];
    }
}

- (MPRemoteCommandHandlerStatus )remoteCommandEvent:(MPRemoteCommandEvent *)event
{
    MPRemoteCommandCenter *cc = [MPRemoteCommandCenter sharedCommandCenter];

    if (event.command == cc.playCommand) {
        return [_playQueueController startPlayQueue] ? MPRemoteCommandHandlerStatusSuccess : MPRemoteCommandHandlerStatusNoActionableNowPlayingItem;
    }
    if (event.command == cc.pauseCommand) {
        [_playQueueController pausePlayback];
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.stopCommand) {
        [_playQueueController stopPlayback];
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.togglePlayPauseCommand) {
        [_playerController togglePlayPause];
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.nextTrackCommand) {
        return [_playQueueController playNextItem] ? MPRemoteCommandHandlerStatusSuccess : MPRemoteCommandHandlerStatusCommandFailed;
    }
    if (event.command == cc.previousTrackCommand) {
        return [_playQueueController playPreviousItem] ? MPRemoteCommandHandlerStatusSuccess : MPRemoteCommandHandlerStatusCommandFailed;
    }
    if (event.command == cc.skipForwardCommand) {
        [_playerController jumpForwardMedium];
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.skipBackwardCommand) {
        [_playerController jumpBackwardMedium];
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.changePlaybackPositionCommand) {
        MPChangePlaybackPositionCommandEvent *positionEvent = (MPChangePlaybackPositionCommandEvent *)event;
        [_playerController setTimeFast:vlc_tick_from_sec( positionEvent.positionTime )];
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.changeRepeatModeCommand) {
        MPChangeRepeatModeCommandEvent *repeatEvent = (MPChangeRepeatModeCommandEvent *)event;
        MPRepeatType repeatType = repeatEvent.repeatType;
        switch (repeatType) {
            case MPRepeatTypeAll:
                [_playQueueController setPlaybackRepeat:VLC_PLAYLIST_PLAYBACK_REPEAT_ALL];
                 break;

            case MPRepeatTypeOne:
                [_playQueueController setPlaybackRepeat:VLC_PLAYLIST_PLAYBACK_REPEAT_CURRENT];
                break;

            default:
                [_playQueueController setPlaybackRepeat:VLC_PLAYLIST_PLAYBACK_REPEAT_NONE];;
                break;
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }
    if (event.command == cc.changeShuffleModeCommand) {
        [_playQueueController setPlaybackOrder:[_playQueueController playbackOrder] == VLC_PLAYLIST_PLAYBACK_ORDER_NORMAL ? VLC_PLAYLIST_PLAYBACK_ORDER_RANDOM : VLC_PLAYLIST_PLAYBACK_ORDER_NORMAL];
        return MPRemoteCommandHandlerStatusSuccess;
    }

    msg_Dbg(getIntf(), "%s Wasn't able to handle remote control event: %s",__PRETTY_FUNCTION__,[event.description UTF8String]);
    return MPRemoteCommandHandlerStatusCommandFailed;
}

@end

#pragma clang diagnostic pop
