/*****************************************************************************
 * player_input.c: Player input implementation
 *****************************************************************************
 * Copyright © 2018-2019 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_memstream.h>
#include "player.h"

struct vlc_player_track_priv *
vlc_player_input_FindTrackById(struct vlc_player_input *input, vlc_es_id_t *id,
                               size_t *idx)
{
    vlc_player_track_vector *vec =
        vlc_player_input_GetTrackVector(input, vlc_es_id_GetCat(id));
    return vec ? vlc_player_track_vector_FindById(vec, id, idx) : NULL;
}

static void
vlc_player_input_HandleAtoBLoop(struct vlc_player_input *input, vlc_tick_t time,
                                double pos)
{
    vlc_player_t *player = input->player;

    if (player->input != input)
        return;

    assert(input->abloop_state[0].set && input->abloop_state[1].set);

    if (time != VLC_TICK_INVALID
     && input->abloop_state[0].time != VLC_TICK_INVALID
     && input->abloop_state[1].time != VLC_TICK_INVALID)
    {
        if (time >= input->abloop_state[1].time)
            vlc_player_SetTime(player, input->abloop_state[0].time);
    }
    else if (pos >= input->abloop_state[1].pos)
        vlc_player_SetPosition(player, input->abloop_state[0].pos);
}

vlc_tick_t
vlc_player_input_GetTime(struct vlc_player_input *input, bool seeking,
                         vlc_tick_t system_now)
{
    vlc_player_t *player = input->player;
    vlc_tick_t ts;

    if (input == player->input
     && vlc_player_GetTimerPoint(player, seeking, system_now, &ts, NULL) == 0)
        return ts;
    return input->time;
}

double
vlc_player_input_GetPos(struct vlc_player_input *input, bool seeking,
                        vlc_tick_t system_now)
{
    vlc_player_t *player = input->player;
    double pos;

    if (input == player->input
     && vlc_player_GetTimerPoint(player, seeking, system_now, NULL, &pos) == 0)
        return pos;
    return input->position;
}

static void
vlc_player_input_UpdateTime(struct vlc_player_input *input)
{
    if (input->abloop_state[0].set && input->abloop_state[1].set)
    {
        vlc_tick_t now = vlc_tick_now();
        vlc_player_input_HandleAtoBLoop(input,
                                        vlc_player_input_GetTime(input, false, now),
                                        vlc_player_input_GetPos(input, false, now));
    }
}

int
vlc_player_input_Start(struct vlc_player_input *input)
{
    int ret = input_Start(input->thread);
    if (ret != VLC_SUCCESS)
        return ret;
    input->started = true;
    return ret;
}

static inline void
vlc_player_assert_seek_params(enum vlc_player_seek_speed speed,
                              enum vlc_player_whence whence)
{
    assert(speed == VLC_PLAYER_SEEK_PRECISE
        || speed == VLC_PLAYER_SEEK_FAST);
    assert(whence == VLC_PLAYER_WHENCE_ABSOLUTE
        || whence == VLC_PLAYER_WHENCE_RELATIVE);
    (void) speed; (void) whence;
}

void
vlc_player_input_SeekByPos(struct vlc_player_input *input, double position,
                           enum vlc_player_seek_speed speed,
                           enum vlc_player_whence whence)
{
    vlc_player_t *player = input->player;
    vlc_player_assert_seek_params(speed, whence);

    if (whence != VLC_PLAYER_WHENCE_ABSOLUTE)
        position += vlc_player_input_GetPos(input, true, vlc_tick_now());

    if (position < 0)
        position = 0;
    else if (position > 1)
        position = 1;

    vlc_player_UpdateTimerSeekState(player, VLC_TICK_INVALID, position);

    int ret = input_ControlPush(input->thread, INPUT_CONTROL_SET_POSITION,
        &(input_control_param_t) {
            .pos.f_val = position,
            .pos.b_fast_seek = speed == VLC_PLAYER_SEEK_FAST,
    });

    if (ret == VLC_SUCCESS)
        vlc_player_osd_Position(player, input, VLC_TICK_INVALID, position);
}

void
vlc_player_input_SeekByTime(struct vlc_player_input *input, vlc_tick_t time,
                            enum vlc_player_seek_speed speed,
                            enum vlc_player_whence whence)
{
    vlc_player_t *player = input->player;
    vlc_player_assert_seek_params(speed, whence);

    if (whence != VLC_PLAYER_WHENCE_ABSOLUTE)
        time += vlc_player_input_GetTime(input, true, vlc_tick_now());

    if (time < VLC_TICK_0)
        time = VLC_TICK_0;

    vlc_player_UpdateTimerSeekState(player, time, -1);

    int ret = input_ControlPush(input->thread, INPUT_CONTROL_SET_TIME,
        &(input_control_param_t) {
            .time.i_val = time,
            .time.b_fast_seek = speed == VLC_PLAYER_SEEK_FAST,
    });

    if (ret == VLC_SUCCESS)
        vlc_player_osd_Position(player, input, time, -1);
}

void
vlc_player_input_UpdateViewpoint(struct vlc_player_input *input,
                                 const vlc_viewpoint_t *viewpoint,
                                 enum vlc_player_whence whence)
{
    input_control_param_t param = { .viewpoint = *viewpoint };
    if (whence == VLC_PLAYER_WHENCE_ABSOLUTE)
        input_ControlPush(input->thread, INPUT_CONTROL_SET_VIEWPOINT,
                          &param);
    else
        input_ControlPush(input->thread, INPUT_CONTROL_UPDATE_VIEWPOINT,
                          &param);
}

static bool
vlc_player_WaitRetryDelay(vlc_player_t *player)
{
#define RETRY_TIMEOUT_BASE VLC_TICK_FROM_MS(100)
#define RETRY_TIMEOUT_MAX VLC_TICK_FROM_MS(3200)
#define MAX_EOS_BURST 4
    /* Temporize only after a few successive EOS */
    if (player->eos_burst_count > MAX_EOS_BURST)
    {
        unsigned pow = player->eos_burst_count - MAX_EOS_BURST;
        /* Delay the next opening to avoid busy loops */
        vlc_tick_t delay = RETRY_TIMEOUT_BASE;
        for (unsigned i = 1; i < pow && delay < RETRY_TIMEOUT_MAX; ++i)
            delay *= 2; /* Wait 100, 200, 400, 800, 1600 and finally 3200ms */
        delay += vlc_tick_now();

        while (player->eos_burst_count != 0
            && vlc_cond_timedwait(&player->start_delay_cond, &player->lock,
                                  delay) == 0);
        if (player->eos_burst_count == 0)
            return false; /* canceled */
    }
    return true;
}

void
vlc_player_input_HandleState(struct vlc_player_input *input,
                             enum vlc_player_state state, vlc_tick_t state_date)
{
    vlc_player_t *player = input->player;

    /* The STOPPING state can be set earlier by the player. In that case,
     * ignore all future events except the STOPPED one */
    if (input->state == VLC_PLAYER_STATE_STOPPING
     && state != VLC_PLAYER_STATE_STOPPED)
        return;

    enum vlc_player_state last_state = input->state;
    input->state = state;

    /* Override the global state if the player is still playing and has a next
     * media to play */
    bool send_event = player->global_state != state;
    switch (input->state)
    {
        case VLC_PLAYER_STATE_STOPPED:
            assert(!input->started);
            assert(input != player->input);

            if (input->titles)
            {
                vlc_player_title_list_Release(input->titles);
                input->titles = NULL;
                vlc_player_SendEvent(player, on_titles_changed, NULL);
            }

            vlc_player_ResetTimer(player);

            if (player->last_eos != VLC_TICK_INVALID)
            {
                vlc_tick_t diff = vlc_tick_now() - player->last_eos;
                if (diff < VLC_PLAYER_EOS_BURST_THRESHOLD)
                    ++player->eos_burst_count;
                else
                    player->eos_burst_count = 0;
            }

            if (player->started)
                vlc_player_WaitRetryDelay(player);

            /* Assign the current date after the wait */
            player->last_eos = vlc_tick_now();

            if (!player->deleting)
            {
                vlc_player_OpenNextMedia(player);
                /* It is possible to open several medias in a stopped state */
                if (player->input && player->started)
                    vlc_player_input_Start(player->input);
            }
            if (!player->input)
                player->started = false;

            send_event = !player->started && last_state != VLC_PLAYER_STATE_STOPPED;
            break;
        case VLC_PLAYER_STATE_STOPPING:
            input->started = false;

            vlc_player_UpdateTimerEvent(player, NULL,
                                        VLC_PLAYER_TIMER_EVENT_DISCONTINUITY,
                                        VLC_TICK_INVALID);

            vlc_player_UpdateTimerEvent(player, NULL,
                                        VLC_PLAYER_TIMER_EVENT_STOPPING,
                                        VLC_TICK_INVALID);

            if (input == player->input)
                player->input = NULL;

            if (player->started && !player->next_media)
                player->started = false;
            send_event = !player->started;
            break;
        case VLC_PLAYER_STATE_PLAYING:
            input->pause_date = VLC_TICK_INVALID;
            vlc_player_UpdateTimerEvent(player, NULL,
                                        VLC_PLAYER_TIMER_EVENT_PLAYING,
                                        input->pause_date);
            /* fall through */
        case VLC_PLAYER_STATE_STARTED:
            if (player->started &&
                player->global_state == VLC_PLAYER_STATE_PLAYING)
                send_event = false;
            break;

        case VLC_PLAYER_STATE_PAUSED:
            assert(player->global_state == VLC_PLAYER_STATE_PLAYING);
            assert(state_date != VLC_TICK_INVALID);
            input->pause_date = state_date;

            vlc_player_UpdateTimerEvent(player, NULL,
                                        VLC_PLAYER_TIMER_EVENT_PAUSED,
                                        input->pause_date);
            break;
        default:
            vlc_assert_unreachable();
    }

    if (send_event)
    {
        assert(player->global_state != input->state);
        player->global_state = input->state;
        vlc_player_SendEvent(player, on_state_changed, player->global_state);
    }
}

static void
vlc_player_input_HandleStateEvent(struct vlc_player_input *input,
                                  input_state_e state, vlc_tick_t state_date)
{
    switch (state)
    {
        case OPENING_S:
            vlc_player_input_HandleState(input, VLC_PLAYER_STATE_STARTED,
                                         VLC_TICK_INVALID);
            break;
        case PLAYING_S:
            input->playing = true;
            vlc_player_input_HandleState(input, VLC_PLAYER_STATE_PLAYING,
                                         state_date);
            break;
        case PAUSE_S:
            vlc_player_input_HandleState(input, VLC_PLAYER_STATE_PAUSED,
                                         state_date);
            break;
        case END_S:
            input->playing = false;
            vlc_player_input_HandleState(input, VLC_PLAYER_STATE_STOPPING,
                                         VLC_TICK_INVALID);
            vlc_player_destructor_AddStoppingInput(input->player, input);
            break;
        case ERROR_S:
            /* Don't send errors if the input is stopped by the user */
            if (input->started)
            {
                /* Contrary to the input_thead_t, an error is not a state */
                input->error = VLC_PLAYER_ERROR_GENERIC;
                vlc_player_SendEvent(input->player, on_error_changed, input->error);
            }
            /* input->playing monitor the OPENING_S -> PLAYING_S transition.
             * If input->playing is false, we have an error at the opening of
             * the input thread and we won't reach END_S. */
            if (!input->playing)
            {
                vlc_player_input_HandleState(input, VLC_PLAYER_STATE_STOPPING,
                                             VLC_TICK_INVALID);
                vlc_player_destructor_AddStoppingInput(input->player, input);
            }
            break;
        default:
            vlc_assert_unreachable();
    }
}

static void
vlc_player_input_HandleProgramEvent(struct vlc_player_input *input,
                                    const struct vlc_input_event_program *ev)
{
    vlc_player_t *player = input->player;
    struct vlc_player_program *prgm;
    vlc_player_program_vector *vec = &input->program_vector;

    switch (ev->action)
    {
        case VLC_INPUT_PROGRAM_ADDED:
            prgm = vlc_player_program_New(ev->id, ev->title);
            if (!prgm)
                break;

            if (!vlc_vector_push(vec, prgm))
            {
                vlc_player_program_Delete(prgm);
                break;
            }
            vlc_player_SendEvent(player, on_program_list_changed,
                                 VLC_PLAYER_LIST_ADDED, prgm);
            break;
        case VLC_INPUT_PROGRAM_DELETED:
        {
            size_t idx;
            prgm = vlc_player_program_vector_FindById(vec, ev->id, &idx);
            if (prgm)
            {
                vlc_player_SendEvent(player, on_program_list_changed,
                                     VLC_PLAYER_LIST_REMOVED, prgm);
                vlc_vector_remove(vec, idx);
                vlc_player_program_Delete(prgm);
            }
            break;
        }
        case VLC_INPUT_PROGRAM_UPDATED:
        case VLC_INPUT_PROGRAM_SCRAMBLED:
            prgm = vlc_player_program_vector_FindById(vec, ev->id, NULL);
            if (!prgm)
                break;
            if (ev->action == VLC_INPUT_PROGRAM_UPDATED)
            {
                if (vlc_player_program_Update(prgm, ev->id, ev->title) != 0)
                    break;
            }
            else
                prgm->scrambled = ev->scrambled;
            vlc_player_SendEvent(player, on_program_list_changed,
                                 VLC_PLAYER_LIST_UPDATED, prgm);
            break;
        case VLC_INPUT_PROGRAM_SELECTED:
        {
            int unselected_id = -1, selected_id = -1;
            vlc_vector_foreach(prgm, vec)
            {
                if (prgm->group_id == ev->id)
                {
                    if (!prgm->selected)
                    {
                        assert(selected_id == -1);
                        prgm->selected = true;
                        selected_id = prgm->group_id;
                    }
                }
                else
                {
                    if (prgm->selected)
                    {
                        assert(unselected_id == -1);
                        prgm->selected = false;
                        unselected_id = prgm->group_id;
                    }
                }
            }
            if (unselected_id != -1 || selected_id != -1)
                vlc_player_SendEvent(player, on_program_selection_changed,
                                     unselected_id, selected_id);
            break;
        }
        default:
            vlc_assert_unreachable();
    }
}

static const struct vlc_player_track_priv *
vlc_player_FindTeletextSource(const struct vlc_player_input *input,
                              const struct vlc_player_track_priv *exclude,
                              bool selected)
{
    const struct vlc_player_track_priv *t;
    vlc_vector_foreach(t, &input->spu_track_vector)
    {
        if (t->t.fmt.i_codec == VLC_CODEC_TELETEXT &&
           t != exclude &&
           t->t.selected == selected)
            return t;
    }
    return NULL;
}

static unsigned
vlc_player_input_TeletextUserPage(const struct vlc_player_track_priv *t)
{
    const uint8_t mag = t->t.fmt.subs.teletext.i_magazine;
    const uint8_t page = t->t.fmt.subs.teletext.i_page;
    return (mag % 10) * 100 +
           (page & 0x0F) + ((page >> 4) & 0x0F) * 10;
}

static void
vlc_player_input_HandleTeletextMenu(struct vlc_player_input *input,
                                    const struct vlc_input_event_es *ev,
                                    const struct vlc_player_track_priv *trackpriv)
{
    vlc_player_t *player = input->player;
    if (ev->fmt->i_cat != SPU_ES ||
        ev->fmt->i_codec != VLC_CODEC_TELETEXT)
        return;
    switch (ev->action)
    {
        case VLC_INPUT_ES_ADDED:
        {
            if (!input->teletext_source)
            {
                input->teletext_source = trackpriv;
                vlc_player_SendEvent(player, on_teletext_menu_changed, true);
            }
            break;
        }
        case VLC_INPUT_ES_DELETED:
        {
            if (input->teletext_source == trackpriv)
            {
                input->teletext_source =
                        vlc_player_FindTeletextSource(input, trackpriv, true);
                if (!input->teletext_source)
                    input->teletext_source =
                            vlc_player_FindTeletextSource(input, trackpriv, false);
                if (!input->teletext_source) /* no more teletext ES */
                {
                    if (input->teletext_enabled)
                    {
                        input->teletext_enabled = false;
                        vlc_player_SendEvent(player, on_teletext_enabled_changed, false);
                    }
                    vlc_player_SendEvent(player, on_teletext_menu_changed, false);
                }
                else /* another teletext ES was reselected */
                {
                    if (input->teletext_source->t.selected != input->teletext_enabled)
                    {
                        input->teletext_enabled = input->teletext_source->t.selected;
                        vlc_player_SendEvent(player, on_teletext_enabled_changed,
                                             input->teletext_source->t.selected);
                    }
                    input->teletext_page =
                            vlc_player_input_TeletextUserPage(input->teletext_source);
                    vlc_player_SendEvent(player, on_teletext_page_changed,
                                         input->teletext_page);
                }
            }
            break;
        }
        case VLC_INPUT_ES_UPDATED:
            break;
        case VLC_INPUT_ES_SELECTED:
        {
            if (!input->teletext_enabled) /* we stick with the first selected */
            {
                input->teletext_source = trackpriv;
                input->teletext_enabled = true;
                input->teletext_page = vlc_player_input_TeletextUserPage(trackpriv);
                vlc_player_SendEvent(player, on_teletext_enabled_changed, true);
                vlc_player_SendEvent(player, on_teletext_page_changed,
                                     input->teletext_page);
            }
            break;
        }
        case VLC_INPUT_ES_UNSELECTED:
            if (input->teletext_source == trackpriv)
            {
                /* If there's another selected teletext, it needs to become source */
                const struct vlc_player_track_priv *other =
                        vlc_player_FindTeletextSource(input, trackpriv, true);
                if (other)
                {
                    input->teletext_source = other;
                    if (!input->teletext_enabled)
                    {
                        input->teletext_enabled = true;
                        vlc_player_SendEvent(player, on_teletext_enabled_changed, true);
                    }
                    input->teletext_page = vlc_player_input_TeletextUserPage(other);
                    vlc_player_SendEvent(player, on_teletext_page_changed,
                                         input->teletext_page);
                }
                else
                {
                    input->teletext_enabled = false;
                    vlc_player_SendEvent(player, on_teletext_enabled_changed, false);
                }
            }
            break;
        default:
            vlc_assert_unreachable();
    }
}

static void
vlc_player_input_HandleEsEvent(struct vlc_player_input *input,
                               const struct vlc_input_event_es *ev)
{
    assert(ev->id && ev->title && ev->fmt);

    vlc_player_track_vector *vec =
        vlc_player_input_GetTrackVector(input, ev->fmt->i_cat);
    if (!vec)
        return; /* UNKNOWN_ES or DATA_ES not handled */

    vlc_player_t *player = input->player;
    struct vlc_player_track_priv *trackpriv;
    switch (ev->action)
    {
        case VLC_INPUT_ES_ADDED:
            trackpriv = vlc_player_track_priv_New(ev->id, ev->title, ev->fmt);
            if (!trackpriv)
                break;

            if (ev->fmt->i_cat == VIDEO_ES)
                input->ml.has_video_tracks = true;
            else if (ev->fmt->i_cat == AUDIO_ES)
                input->ml.has_audio_tracks = true;
            if (!vlc_vector_push(vec, trackpriv))
            {
                vlc_player_track_priv_Delete(trackpriv);
                break;
            }
            vlc_player_SendEvent(player, on_track_list_changed,
                                 VLC_PLAYER_LIST_ADDED, &trackpriv->t);
            vlc_player_input_HandleTeletextMenu(input, ev, trackpriv);
            break;
        case VLC_INPUT_ES_DELETED:
        {
            size_t idx;
            trackpriv = vlc_player_track_vector_FindById(vec, ev->id, &idx);
            if (trackpriv)
            {
                vlc_player_input_HandleTeletextMenu(input, ev, trackpriv);
                vlc_player_SendEvent(player, on_track_list_changed,
                                     VLC_PLAYER_LIST_REMOVED, &trackpriv->t);
                vlc_vector_remove(vec, idx);
                vlc_player_track_priv_Delete(trackpriv);
            }
            break;
        }
        case VLC_INPUT_ES_UPDATED:
            trackpriv = vlc_player_track_vector_FindById(vec, ev->id, NULL);
            if (!trackpriv)
                break;
            if (vlc_player_track_priv_Update(trackpriv, ev->title, ev->fmt) != 0)
                break;
            vlc_player_SendEvent(player, on_track_list_changed,
                                 VLC_PLAYER_LIST_UPDATED, &trackpriv->t);
            vlc_player_input_HandleTeletextMenu(input, ev, trackpriv);
            break;
        case VLC_INPUT_ES_SELECTED:
            trackpriv = vlc_player_track_vector_FindById(vec, ev->id, NULL);
            if (trackpriv)
            {
                trackpriv->t.selected = true;
                trackpriv->selected_by_user = ev->forced;
                trackpriv->vout_order = ev->vout_order;
                vlc_player_SendEvent(player, on_track_selection_changed,
                                     NULL, trackpriv->t.es_id);
                vlc_player_input_HandleTeletextMenu(input, ev, trackpriv);
            }
            break;
        case VLC_INPUT_ES_UNSELECTED:
            trackpriv = vlc_player_track_vector_FindById(vec, ev->id, NULL);
            if (trackpriv)
            {
                vlc_player_RemoveTimerSource(player, ev->id);
                trackpriv->t.selected = false;
                trackpriv->selected_by_user = false;
                vlc_player_SendEvent(player, on_track_selection_changed,
                                     trackpriv->t.es_id, NULL);
                vlc_player_input_HandleTeletextMenu(input, ev, trackpriv);
            }
            break;
        default:
            vlc_assert_unreachable();
    }
}

static void
vlc_player_input_HandleTitleEvent(struct vlc_player_input *input,
                                  const struct vlc_input_event_title *ev)
{
    vlc_player_t *player = input->player;
    switch (ev->action)
    {
        case VLC_INPUT_TITLE_NEW_LIST:
        {
            input_thread_private_t *input_th = input_priv(input->thread);
            const int title_offset = input_th->i_title_offset;
            const int chapter_offset = input_th->i_seekpoint_offset;

            if (input->titles)
                vlc_player_title_list_Release(input->titles);
            input->title_selected = input->chapter_selected = 0;
            input->titles =
                vlc_player_title_list_Create(ev->list.array, ev->list.count,
                                             title_offset, chapter_offset);
            vlc_player_SendEvent(player, on_titles_changed, input->titles);
            if (input->titles)
            {
                vlc_player_SendEvent(player, on_title_selection_changed,
                                     &input->titles->array[0], 0);
                if (input->ml.restore == VLC_RESTOREPOINT_TITLE &&
                    (size_t)input->ml.states.current_title < ev->list.count)
                {
                    vlc_player_SelectTitleIdx(player, input->ml.states.current_title);
                }
                input->ml.restore = VLC_RESTOREPOINT_POSITION;
            }
            else input->ml.restore = VLC_RESTOREPOINT_NONE;
            break;
        }
        case VLC_INPUT_TITLE_SELECTED:
            if (!input->titles)
                return; /* a previous VLC_INPUT_TITLE_NEW_LIST failed */
            assert(ev->selected_idx < input->titles->count);
            input->title_selected = ev->selected_idx;
            vlc_player_SendEvent(player, on_title_selection_changed,
                                 &input->titles->array[input->title_selected],
                                 input->title_selected);
            if (input->ml.restore == VLC_RESTOREPOINT_POSITION &&
                input->ml.states.current_title >= 0 &&
                (size_t)input->ml.states.current_title == ev->selected_idx &&
                input->ml.pos > .0f)
            {
                input_SetPosition(input->thread, input->ml.pos, false);
            }
            /* Reset the wanted title to avoid forcing it or the position
             * again during the next title change
             */
            input->ml.restore = VLC_RESTOREPOINT_NONE;
            break;
        default:
            vlc_assert_unreachable();
    }
}

static void
vlc_player_input_HandleChapterEvent(struct vlc_player_input *input,
                                    const struct vlc_input_event_chapter *ev)
{
    vlc_player_t *player = input->player;
    if (!input->titles || ev->title < 0 || ev->seekpoint < 0)
        return; /* a previous VLC_INPUT_TITLE_NEW_LIST failed */

    assert((size_t)ev->title < input->titles->count);
    const struct vlc_player_title *title = &input->titles->array[ev->title];
    if (!title->chapter_count)
        return;

    assert(ev->seekpoint < (int)title->chapter_count);
    input->title_selected = ev->title;
    input->chapter_selected = ev->seekpoint;

    const struct vlc_player_chapter *chapter = &title->chapters[ev->seekpoint];
    vlc_player_SendEvent(player, on_chapter_selection_changed, title, ev->title,
                         chapter, ev->seekpoint);
}

static void
vlc_player_input_HandleVoutEvent(struct vlc_player_input *input,
                                 const struct vlc_input_event_vout *ev)
{
    assert(ev->vout);
    assert(ev->id);

    vlc_player_t *player = input->player;

    struct vlc_player_track_priv *trackpriv =
        vlc_player_input_FindTrackById(input, ev->id, NULL);
    if (!trackpriv)
        return;

    const bool is_video_es = trackpriv->t.fmt.i_cat == VIDEO_ES;

    switch (ev->action)
    {
        case VLC_INPUT_EVENT_VOUT_STARTED:
            trackpriv->vout = ev->vout;
            vlc_player_SendEvent(player, on_vout_changed,
                                 VLC_PLAYER_VOUT_STARTED, ev->vout,
                                 ev->order, ev->id);

            if (is_video_es)
            {
                /* Register vout callbacks after the vout list event */
                vlc_player_vout_AddCallbacks(player, ev->vout);
            }
            break;
        case VLC_INPUT_EVENT_VOUT_STOPPED:
            if (is_video_es)
            {
                /* Un-register vout callbacks before the vout list event */
                vlc_player_vout_DelCallbacks(player, ev->vout);
            }

            trackpriv->vout = NULL;
            vlc_player_SendEvent(player, on_vout_changed,
                                 VLC_PLAYER_VOUT_STOPPED, ev->vout,
                                 VLC_VOUT_ORDER_NONE, ev->id);
            break;
        default:
            vlc_assert_unreachable();
    }
}

static void
vlc_player_input_NavigationFallback(struct vlc_player_input *input, int nav_type)
{
    vlc_player_t *player = input->player;

    /* Handle Up/Down/Left/Right if the demux can't navigate */
    vlc_viewpoint_t vp = { 0 };
    bool viewpoint_updated = true;
    float yaw = 0.f, pitch = 0.f, roll = 0.f;

    int vol_direction = 0;
    int seek_direction = 0;
    switch (nav_type)
    {
        case INPUT_CONTROL_NAV_UP:
            vol_direction = 1;
            pitch = -1.f;
            break;
        case INPUT_CONTROL_NAV_DOWN:
            vol_direction = -1;
            pitch = 1.f;
            break;
        case INPUT_CONTROL_NAV_LEFT:
            seek_direction = -1;
            yaw = -1.f;
            break;
        case INPUT_CONTROL_NAV_RIGHT:
            seek_direction = 1;
            yaw = 1.f;
            break;
        case INPUT_CONTROL_NAV_ACTIVATE:
        case INPUT_CONTROL_NAV_POPUP:
        case INPUT_CONTROL_NAV_MENU:
            viewpoint_updated = false;
            return;
        default:
            vlc_assert_unreachable();
    }

    /* Try to change the viewpoint if possible */
    bool viewpoint_ch = false;
    size_t vout_count;
    vout_thread_t **vouts = vlc_player_vout_HoldAll(input->player, &vout_count);
    for (size_t i = 0; i < vout_count; ++i)
    {
        if (!viewpoint_ch && var_GetBool(vouts[i], "viewpoint-changeable"))
            viewpoint_ch = true;
        vout_Release(vouts[i]);
    }
    free(vouts);

    if (viewpoint_ch && viewpoint_updated)
    {
        vlc_viewpoint_from_euler(&vp, yaw, pitch, roll);
        vlc_player_input_UpdateViewpoint(input, &vp, VLC_PLAYER_WHENCE_RELATIVE);
    }
    else if (seek_direction != 0)
    {
        /* Seek or change volume if the input doesn't have navigation or viewpoint */
        vlc_tick_t it = vlc_tick_from_sec(seek_direction
                      * var_InheritInteger(player, "short-jump-size"));
        vlc_player_input_SeekByTime(input, it, VLC_PLAYER_SEEK_PRECISE,
                                    VLC_PLAYER_WHENCE_RELATIVE);
    }
    else
    {
        assert(vol_direction != 0);
        if (input == player->input)
            vlc_player_aout_IncrementVolume(player, vol_direction, NULL);
    }
}

static void
vlc_player_input_MouseFallback(struct vlc_player_input *input)
{
    vlc_player_t *player = input->player;
    vlc_player_TogglePause(player);
}

static bool
input_thread_Events(input_thread_t *input_thread,
                    const struct vlc_input_event *event, void *user_data)
{
    struct vlc_player_input *input = user_data;
    vlc_player_t *player = input->player;
    input_thread_private_t *priv = input_priv(input_thread);

    assert(input_thread == input->thread);

    /* No player lock for this event */
    if (event->type == INPUT_EVENT_OUTPUT_CLOCK)
    {
        if (event->output_clock.system_ts != VLC_TICK_INVALID)
        {
            const struct vlc_player_timer_point point = {
                .position = 0,
                .rate = event->output_clock.rate,
                .ts = event->output_clock.ts,
                .length = VLC_TICK_INVALID,
                .system_date = event->output_clock.system_ts,
            };
            vlc_player_UpdateTimer(player, event->output_clock.id,
                                   event->output_clock.master, &point,
                                   VLC_TICK_INVALID,
                                   event->output_clock.frame_rate,
                                   event->output_clock.frame_rate_base, 0);
        }
        else
        {
            vlc_player_UpdateTimerEvent(player, event->output_clock.id,
                                        VLC_PLAYER_TIMER_EVENT_DISCONTINUITY,
                                        VLC_TICK_INVALID);
        }
        return true;
    }

    bool handled = true;

    vlc_mutex_lock(&player->lock);

    switch (event->type)
    {
        case INPUT_EVENT_STATE:
            vlc_player_input_HandleStateEvent(input, event->state.value,
                                              event->state.date);
            break;
        case INPUT_EVENT_RATE:
            input->rate = event->rate;
            vlc_player_SendEvent(player, on_rate_changed, input->rate);
            break;
        case INPUT_EVENT_CAPABILITIES:
        {
            int old_caps = input->capabilities;
            input->capabilities = event->capabilities;
            vlc_player_SendEvent(player, on_capabilities_changed,
                                 old_caps, input->capabilities);
            break;
        }
        case INPUT_EVENT_TIMES:
        {
            bool changed = false;
            vlc_tick_t system_date = VLC_TICK_INVALID;
            vlc_tick_t duration = input_GetItemDuration(input->thread, event->times.length);

            if (event->times.time != VLC_TICK_INVALID
             && (input->time != event->times.time
              || input->position != event->times.position))
            {
                input->time = event->times.time;
                input->position = event->times.position;
                system_date = vlc_tick_now();
                changed = true;
                vlc_player_SendEvent(player, on_position_changed,
                                     input->time, input->position);

                vlc_player_input_UpdateTime(input);
            }
            if (input->length != duration)
            {
                input->length = duration;
                input_item_SetDuration(input_GetItem(input->thread), duration);
                vlc_player_SendEvent(player, on_length_changed, input->length);
                changed = true;
            }

            if (input->live != event->times.live)
            {
                input->live = event->times.live;
                changed = true;
            }

            if (input->normal_time != event->times.normal_time)
            {
                input->normal_time = event->times.normal_time;
                changed = true;
            }

            if (changed)
            {
                const struct vlc_player_timer_point point = {
                    .position = input->position,
                    .rate = input->rate,
                    .ts = input->time,
                    .length = input->length,
                    .live = input->live,
                    .system_date = system_date,
                };
                vlc_player_UpdateTimer(player, NULL, false, &point,
                                       input->normal_time, 0, 0, priv->i_start);
            }
            break;
        }
        case INPUT_EVENT_PROGRAM:
            vlc_player_input_HandleProgramEvent(input, &event->program);
            break;
        case INPUT_EVENT_ES:
            vlc_player_input_HandleEsEvent(input, &event->es);
            break;
        case INPUT_EVENT_TITLE:
            vlc_player_input_HandleTitleEvent(input, &event->title);
            break;
        case INPUT_EVENT_CHAPTER:
            vlc_player_input_HandleChapterEvent(input, &event->chapter);
            break;
        case INPUT_EVENT_RECORD:
            input->recording = event->record;
            vlc_player_SendEvent(player, on_recording_changed, input->recording);
            break;
        case INPUT_EVENT_STATISTICS:
            input->stats = *event->stats;
            vlc_player_SendEvent(player, on_statistics_changed, &input->stats);
            break;
        case INPUT_EVENT_SIGNAL:
            input->signal_quality = event->signal.quality;
            input->signal_strength = event->signal.strength;
            vlc_player_SendEvent(player, on_signal_changed,
                                 input->signal_quality, input->signal_strength);
            break;
        case INPUT_EVENT_CACHE:
            if (event->cache == 0.0f)
                vlc_player_UpdateTimerEvent(player, NULL,
                                            VLC_PLAYER_TIMER_EVENT_DISCONTINUITY,
                                            VLC_TICK_INVALID);
            input->cache = event->cache;
            vlc_player_SendEvent(player, on_buffering_changed, event->cache);
            break;
        case INPUT_EVENT_VOUT:
            vlc_player_input_HandleVoutEvent(input, &event->vout);
            break;
        case INPUT_EVENT_ITEM_META:
        case INPUT_EVENT_ITEM_INFO:
            vlc_player_SendEvent(player, on_media_meta_changed,
                                 input_GetItem(input->thread));
            break;
        case INPUT_EVENT_ITEM_EPG:
            vlc_player_SendEvent(player, on_media_epg_changed,
                                 input_GetItem(input->thread));
            break;
        case INPUT_EVENT_SUBITEMS:
            vlc_player_SendEvent(player, on_media_subitems_changed,
                                 input_GetItem(input->thread), event->subitems);
            input_item_node_Delete(event->subitems);
            break;
        case INPUT_EVENT_DEAD:
            if (input->started) /* Can happen with early input_thread fails */
                vlc_player_input_HandleState(input, VLC_PLAYER_STATE_STOPPING,
                                             VLC_TICK_INVALID);
            vlc_player_destructor_AddJoinableInput(player, input);
            break;
        case INPUT_EVENT_VBI_PAGE:
            input->teletext_page = event->vbi_page < 999 ? event->vbi_page : 100;
            vlc_player_SendEvent(player, on_teletext_page_changed,
                                 input->teletext_page);
            break;
        case INPUT_EVENT_VBI_TRANSPARENCY:
            input->teletext_transparent = event->vbi_transparent;
            vlc_player_SendEvent(player, on_teletext_transparency_changed,
                                 input->teletext_transparent);
            break;
        case INPUT_EVENT_ATTACHMENTS:
            vlc_player_SendEvent(player, on_media_attachments_added,
                                 input_GetItem(input->thread),
                                 event->attachments.array,
                                 event->attachments.count);
            break;
        case INPUT_EVENT_NAV_FAILED:
            vlc_player_input_NavigationFallback(input, event->nav_type);
            break;
        case INPUT_EVENT_MOUSE_LEFT:
            vlc_player_input_MouseFallback(input);
            break;
        default:
            handled = false;
            break;
    }

    vlc_mutex_unlock(&player->lock);
    return handled;
}

void
vlc_player_input_SelectTracksByStringIds(struct vlc_player_input *input,
                                         enum es_format_category_e cat,
                                         const char *str_ids)
{
    input_SetEsCatIds(input->thread, cat, str_ids);
}

char *
vlc_player_input_GetSelectedTrackStringIds(struct vlc_player_input *input,
                                           enum es_format_category_e cat)
{
    vlc_player_track_vector *vec = vlc_player_input_GetTrackVector(input, cat);
    assert(vec);
    bool first_track = true;
    struct vlc_memstream ms;

    struct vlc_player_track_priv* t;
    vlc_vector_foreach(t, vec)
    {
        if (t->selected_by_user && vlc_es_id_IsStrIdStable(t->t.es_id))
        {
            if (first_track)
            {
                int ret = vlc_memstream_open(&ms);
                if (ret != 0)
                    return NULL;
            }
            const char *str_id = vlc_es_id_GetStrId(t->t.es_id);
            assert(str_id);

            if (!first_track)
                vlc_memstream_putc(&ms, ',');
            vlc_memstream_puts(&ms, str_id);
            first_track = false;
        }
    }
    return !first_track && vlc_memstream_close(&ms) == 0 ? ms.ptr : NULL;
}

struct vlc_player_input *
vlc_player_input_New(vlc_player_t *player, input_item_t *item)
{
    struct vlc_player_input *input = malloc(sizeof(*input));
    if (!input)
        return NULL;

    input->player = player;
    input->started = false;
    input->playing = false;

    input->state = VLC_PLAYER_STATE_STOPPED;
    input->error = VLC_PLAYER_ERROR_NONE;
    input->rate = 1.f;
    input->capabilities = 0;
    input->length = input->time = VLC_TICK_INVALID;
    input->live = false;
    input->normal_time = VLC_TICK_0;
    input->pause_date = VLC_TICK_INVALID;
    input->position = 0.f;

    input->recording = false;

    input->cache = 0.f;
    input->signal_quality = input->signal_strength = -1.f;

    memset(&input->stats, 0, sizeof(input->stats));

    vlc_vector_init(&input->program_vector);
    vlc_vector_init(&input->video_track_vector);
    vlc_vector_init(&input->audio_track_vector);
    vlc_vector_init(&input->spu_track_vector);
    input->teletext_source = NULL;

    input->titles = NULL;
    input->title_selected = input->chapter_selected = 0;

    input->teletext_enabled = input->teletext_transparent = false;
    input->teletext_page = 0;

    input->abloop_state[0].set = input->abloop_state[1].set = false;

    memset(&input->ml.states, 0, sizeof(input->ml.states));
    input->ml.states.aspect_ratio = input->ml.states.crop =
        input->ml.states.deinterlace = input->ml.states.video_filter = NULL;
    input->ml.states.current_title = -1;
    input->ml.states.current_video_track =
        input->ml.states.current_audio_track =
        input->ml.states.current_subtitle_track = NULL;
    input->ml.restore = VLC_RESTOREPOINT_NONE;
    input->ml.restore_states = false;
    input->ml.delay_restore = false;
    input->ml.pos = -1.f;
    input->ml.has_audio_tracks = input->ml.has_video_tracks = false;

    static const struct vlc_input_thread_callbacks cbs = {
        .on_event = input_thread_Events,
    };

    const struct vlc_input_thread_cfg cfg = {
        .type = INPUT_TYPE_PLAYBACK,
        .hw_dec = INPUT_CFG_HW_DEC_DEFAULT,
        .resource = player->resource,
        .renderer = player->renderer,
        .cbs = &cbs,
        .cbs_data = input,
    };

    input->thread = input_Create(player, item, &cfg);
    if (!input->thread)
    {
        free(input);
        return NULL;
    }
    vlc_player_input_RestoreMlStates(input, false);

    if (player->video_string_ids)
        vlc_player_input_SelectTracksByStringIds(input, VIDEO_ES,
                                                 player->video_string_ids);

    if (player->audio_string_ids)
        vlc_player_input_SelectTracksByStringIds(input, AUDIO_ES,
                                                 player->audio_string_ids);

    if (player->sub_string_ids)
        vlc_player_input_SelectTracksByStringIds(input, SPU_ES,
                                                 player->sub_string_ids);

    /* Initial sub/audio delay */
    const vlc_tick_t cat_delays[DATA_ES] = {
        [AUDIO_ES] =
            VLC_TICK_FROM_MS(var_InheritInteger(player, "audio-desync")),
        [SPU_ES] =
            vlc_tick_from_samples(var_InheritInteger(player, "sub-delay"), 10),
    };

    for (enum es_format_category_e i = UNKNOWN_ES; i < DATA_ES; ++i)
    {
        input->cat_delays[i] = cat_delays[i];
        if (cat_delays[i] != 0)
        {
            int ret = input_SetEsCatDelay(input->thread, i, cat_delays[i]);
            if (ret == VLC_SUCCESS)
                vlc_player_SendEvent(player, on_category_delay_changed, i,
                                     cat_delays[i]);
        }
    }
    return input;
}

void
vlc_player_input_Delete(struct vlc_player_input *input)
{
    assert(input->titles == NULL);
    assert(input->program_vector.size == 0);
    assert(input->video_track_vector.size == 0);
    assert(input->audio_track_vector.size == 0);
    assert(input->spu_track_vector.size == 0);
    assert(input->teletext_source == NULL);

    vlc_vector_destroy(&input->program_vector);
    vlc_vector_destroy(&input->video_track_vector);
    vlc_vector_destroy(&input->audio_track_vector);
    vlc_vector_destroy(&input->spu_track_vector);

    input_Close(input->thread);
    free(input);
}
