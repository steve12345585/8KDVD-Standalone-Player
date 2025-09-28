/*****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef QVLC_INPUT_MANAGER_P_H_
#define QVLC_INPUT_MANAGER_P_H_

#include "player_controller.hpp"
#include "util/variables.hpp"
#include "input_models.hpp"
#include "util/varchoicemodel.hpp"
#include "util/shared_input_item.hpp"
#include "util/renderer_manager.hpp"

#include <QTimer>
#include <QUrl>

#ifndef QT_HAS_LIBATOMIC
#warning "libatomic is not available. Read write lock is going to be used instead."
#include <QReadWriteLock>
#endif

typedef struct vlc_preparser_t vlc_preparser_t;

class PlayerControllerPrivate {
    Q_DISABLE_COPY(PlayerControllerPrivate)
public:
    Q_DECLARE_PUBLIC(PlayerController)
    PlayerController * const q_ptr;

public:
    PlayerControllerPrivate(PlayerController* playercontroller, qt_intf_t* p_intf);
    PlayerControllerPrivate() = delete;
    ~PlayerControllerPrivate();

    void UpdateName( input_item_t *p_item );
    void UpdateArt( input_item_t *p_item );
    void UpdateMeta( input_item_t *p_item );
    void UpdateInfo( input_item_t *p_item );
    void UpdateStats( const input_stats_t& stats );
    void UpdateProgram(vlc_player_list_action action, const vlc_player_program *prgm);
    void UpdateVouts(vout_thread_t **vouts, size_t i_vouts, vlc_player_vout_action action);
    void UpdateTrackSelection(vlc_es_id_t *trackid, bool selected);
    void UpdateSpuOrder(vlc_es_id_t *es_id, enum vlc_vout_order spu_order);
    int interpolateTime(vlc_tick_t system_now);
    bool isCurrentItemSynced();
    void onArtFetchEnded(input_item_t *, bool fetched);

    // SMPTE Timer
    void addSMPTETimer();
    void removeSMPTETimer();

    ///call function @a fun on object thread
    template <typename Fun>
    void callAsync(Fun&& fun)
    {
        Q_Q(PlayerController);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        // NOTE: Starting with Qt 6.7.0, lambda expression here without a return value
        //       causes compilation issues with some compilers.
        // TODO: Find out if a more recent Qt version does not behave that way.
        QMetaObject::invokeMethod(q, [fun = std::forward<Fun>(fun)]() -> std::monostate { fun(); return std::monostate{}; }, Qt::QueuedConnection);
#else
        QMetaObject::invokeMethod(q, std::forward<Fun>(fun), Qt::QueuedConnection, nullptr);
#endif
    }

public:
    qt_intf_t           *p_intf;
    vlc_player_t            *m_player;
    vlc_preparser_t         *m_preparser = nullptr;

    //callbacks
    vlc_player_listener_id* m_player_listener = nullptr;
    vlc_player_aout_listener_id* m_player_aout_listener = nullptr;
    vlc_player_vout_listener_id* m_player_vout_listener = nullptr;

    //playback
    PlayerController::PlayingState m_playing_status = PlayerController::PLAYING_STATE_STOPPED;
    QString         m_name;
    float           m_buffering = 0.f;
    float           m_rate = 1.f;

    VLCTime      m_time;
    VLCDuration  m_remainingTime;
    double       m_position = 0.f;
    VLCDuration  m_length;

#ifdef QT_HAS_LIBATOMIC
    std::atomic<vlc_player_timer_smpte_timecode> m_highResolutionTime {{}};
#else
    vlc_player_timer_smpte_timecode m_highResolutionTime {};
    mutable QReadWriteLock m_highResolutionTimeLock;
#endif
    mutable QPair<vlc_player_timer_smpte_timecode, QString> m_highResolutionTimeSample;

    unsigned m_smpteTimerRequestCount = 0;

    SharedInputItem    m_currentItem;
    bool            m_canRestorePlayback = false;

    int             m_capabilities = 0;

    //tracks
    TrackListModel m_videoTracks;
    TrackListModel m_audioTracks;
    TrackListModel m_subtitleTracks;

    vlc_shared_data_ptr_type(vlc_es_id_t, vlc_es_id_Hold, vlc_es_id_Release) m_secondarySpuEsId;

    VLCDuration  m_audioDelay;
    VLCDuration  m_subtitleDelay;
    VLCDuration  m_secondarySubtitleDelay;
    float        m_subtitleFPS = 1.0;

    //timer
    vlc_player_timer_id* m_player_timer = nullptr;
    vlc_player_timer_id* m_player_timer_smpte = nullptr;
    struct vlc_player_timer_point m_player_time;
    bool seeking = false;
    QTimer m_position_timer;
    QTimer m_time_timer;

    //title/chapters/menu
    TitleListModel m_titleList;
    ChapterListModel m_chapterList;
    bool m_hasTitles = false;
    bool m_hasChapters = false;
    bool m_hasMenu = false;
    bool m_isMenu = false;
    bool m_isInteractive = false;

    //programs
    ProgramListModel m_programList;
    bool m_hasPrograms = false;
    bool m_encrypted = false;

    //teletext
    bool m_teletextEnabled = false;
    bool m_teletextAvailable = false;
    int m_teletextPage = false;
    bool m_teletextTransparent = false;

    //vout properties
    VLCVarChoiceModel m_videoStereoMode;
    VLCVarChoiceModel m_zoom;
    VLCVarChoiceModel m_aspectRatio;
    VLCVarChoiceModel m_crop;
    VLCVarChoiceModel m_fit;
    VLCVarChoiceModel m_deinterlace;
    VLCVarChoiceModel m_deinterlaceMode;
    QVLCBool m_autoscale;
    bool            m_hasVideo = false;
    bool            m_fullscreen = false;
    bool            m_wallpaperMode = false;
    int m_vout_ref = 0;

    //aout properties
    VLCVarChoiceModel m_audioStereoMode;
    VLCVarChoiceModel m_audioMixMode;
    float           m_volume = 0.f;
    bool            m_muted = false;
    AudioDeviceModel m_audioDeviceList;
    VLCVarChoiceModel m_audioVisualization;

    //misc
    bool            m_recording = false;
    PlayerController::ABLoopState m_ABLoopState = PlayerController::ABLOOP_STATE_NONE;
    VLCTime m_ABLoopA;
    VLCTime m_ABLoopB;

    //others
    QString         m_artUrl;
    struct input_stats_t m_stats;

    RendererManager m_rendererManager;

    // meta
    QString m_title;
    QString m_artist;
    QString m_album;
    QUrl m_artwork;
    QUrl m_url;
};

#endif /* QVLC_INPUT_MANAGER_P_H_ */
