/*****************************************************************************
 * preparser.h
 *****************************************************************************
 * Copyright (C) 1999-2023 VLC authors and VideoLAN
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *          Clément Stenac <zorglub@videolan.org>
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

#ifndef VLC_PREPARSER_H
#define VLC_PREPARSER_H 1

#include <vlc_input_item.h>

/**
 * @defgroup vlc_preparser Preparser
 * @ingroup input
 * @{
 * @file
 * VLC Preparser API
 */

/**
 * Preparser opaque structure.
 *
 * The preparser object will retrieve the meta data of any given input item in
 * an asynchronous way.
 * It will also issue art fetching requests.
 */
typedef struct vlc_preparser_t vlc_preparser_t;
typedef size_t vlc_preparser_req_id;

#define VLC_PREPARSER_REQ_ID_INVALID 0

#define VLC_PREPARSER_TYPE_PARSE            0x01
#define VLC_PREPARSER_TYPE_FETCHMETA_LOCAL  0x02
#define VLC_PREPARSER_TYPE_FETCHMETA_NET    0x04
#define VLC_PREPARSER_TYPE_THUMBNAIL        0x08
#define VLC_PREPARSER_TYPE_THUMBNAIL_TO_FILES 0x10
#define VLC_PREPARSER_TYPE_FETCHMETA_ALL \
    (VLC_PREPARSER_TYPE_FETCHMETA_LOCAL|VLC_PREPARSER_TYPE_FETCHMETA_NET)

#define VLC_PREPARSER_OPTION_INTERACT 0x1000
#define VLC_PREPARSER_OPTION_SUBITEMS 0x2000

/**
 * Preparser thumbnailer callbacks
 *
 * Used by vlc_preparser_GenerateThumbnail()
 */
struct vlc_thumbnailer_cbs
{
    /**
     * Event received on thumbnailing completion or error
     *
     * This callback will always be called, provided
     * vlc_preparser_GenerateThumbnail() returned a valid request, and provided
     * the request is not cancelled before its completion.
     *
     * @note This callback is mandatory if calling
     * vlc_preparser_GenerateThumbnail()
     *
     * In case of failure, timeout or cancellation, p_thumbnail will be NULL.
     * The picture, if any, is owned by the thumbnailer, and must be acquired
     * by using \link picture_Hold \endlink to use it pass the callback's
     * scope.
     *
     * @param item item used for the thumbnailer
     * @param status VLC_SUCCESS in case of success, VLC_ETIMEOUT in case of
     * timeout, -EINTR if cancelled, an error otherwise
     * @param thumbnail The generated thumbnail, or NULL in case of failure or
     * timeout
     * @param data opaque pointer passed by
     * vlc_preparser_GenerateThumbnail()
     *
     */
    void (*on_ended)(input_item_t *item, int status, picture_t* thumbnail,
                     void *data);
};

/**
 * Preparser thumbnailer to file callbacks
 *
 * Used by vlc_preparser_GenerateThumbnailToFiles()
 */
struct vlc_thumbnailer_to_files_cbs
{
    /**
     * Event received on thumbnailing completion or error
     *
     * This callback will always be called, provided
     *
     * vlc_preparser_GenerateThumbnailToFiles() returned a valid request, and
     * provided the request is not cancelled before its completion.
     *
     * @note This callback is mandatory if calling
     * vlc_preparser_GenerateThumbnailToFiles()
     *
     * @param item item used for the thumbnailer
     * @param status VLC_SUCCESS in case of success, VLC_ETIMEOUT in case of
     * timeout, -EINTR if cancelled, an error otherwise. A success mean that an
     * image was generated but it is still possible that the export failed,
     * check result_array to assure export were successful.
     * @param array of results, if result_array[i] is true, the outputs[i] from
     * vlc_preparser_GenerateThumbnailToFiles() succeeded.
     * @param result_count size of the array, same than the output_count arg
     * from vlc_preparser_GenerateThumbnailToFiles()
     * @param data opaque pointer passed by
     * vlc_preparser_GenerateThumbnailToFiles()
     */
    void (*on_ended)(input_item_t *item, int status,
                     const bool *result_array, size_t result_count, void *data);
};

/**
 * Thumbnailer argument
 *
 * Used by vlc_preparser_GenerateThumbnail() and
 * vlc_preparser_GenerateThumbnailToFiles()
 */
struct vlc_thumbnailer_arg
{
    /** Seek argument */
    struct seek
    {
        enum
        {
            /** Don't seek (default) */
            VLC_THUMBNAILER_SEEK_NONE,
            /** Seek by time */
            VLC_THUMBNAILER_SEEK_TIME,
            /** Seek by position */
            VLC_THUMBNAILER_SEEK_POS,
        } type;
        union
        {
            /** Seek time if type == VLC_THUMBNAILER_SEEK_TIME */
            vlc_tick_t time;
            /** Seek position if type == VLC_THUMBNAILER_SEEK_POS */
            double pos;
        };
        enum
        {
            /** Precise, but potentially slow */
            VLC_THUMBNAILER_SEEK_PRECISE,
            /** Fast, but potentially imprecise */
            VLC_THUMBNAILER_SEEK_FAST,
        } speed;
    } seek;

    /** True to enable hardware decoder (false by default) */
    bool hw_dec;
};

/**
 * Thumbnailer output format
 */
enum vlc_thumbnailer_format
{
    VLC_THUMBNAILER_FORMAT_PNG,
    VLC_THUMBNAILER_FORMAT_WEBP,
    VLC_THUMBNAILER_FORMAT_JPEG,
};

/**
 * Thumbnailer output argument
 *
 * Used by vlc_preparser_GenerateThumbnailToFiles()
 */
struct vlc_thumbnailer_output
{
    /**
     * Thumbnailer output format
     */
    enum vlc_thumbnailer_format format;

    /**
     * Requested width of the thumbnail
     *
     * cf. picture_Export() documentation.
     */
    int width;

    /**
     * Requested Height of the thumbnail
     *
     * cf. picture_Export() documentation.
     */
    int height;

    /**
     * True if the thumbnail should be cropped
     *
     * cf. picture_Export() documentation.
     */
    bool crop;

    /** File output path of the thumbnail */
    const char *file_path;
    /** File mode bits (cf. "mode_t mode" in `man 2 open`) */
    unsigned int creat_mode;
};

/**
 * Preparser creation configuration
 */
struct vlc_preparser_cfg
{
    /**
     * A combination of VLC_PREPARSER_TYPE_* flags, it is used to
     * setup the executors for each domain. Its possible to select more than
     * one type
     */
    int types;

    /**
     * The maximum number of threads used by the parser, 0 for default
     * (1 thread)
     */
    unsigned max_parser_threads;

    /**
     * The maximum number of threads used by the thumbnailer, 0 for default
     * (1 thread)
     */
    unsigned max_thumbnailer_threads;

    /**
     * Timeout of the preparser and/or thumbnailer, 0 for no limits.
     */
    vlc_tick_t timeout;
};

/**
 * This function creates the preparser object and thread.
 *
 * @param obj the parent object
 * @param cfg a pointer to a valid confiuration struct
 * @return a valid preparser object or NULL in case of error
 */
VLC_API vlc_preparser_t *vlc_preparser_New( vlc_object_t *obj,
                                            const struct vlc_preparser_cfg *cfg );

/**
 * This function enqueues the provided item to be preparsed or fetched.
 *
 * The input item is retained until the preparsing is done or until the
 * preparser object is deleted.
 *
 * @param preparser the preparser object
 * @param item a valid item to preparse
 * @param type_option a combination of VLC_PREPARSER_TYPE_* and
 * VLC_PREPARSER_OPTION_* flags. The type must be in the set specified in
 * vlc_preparser_New() (it is possible to select less types).
 * @param cbs callback to listen to events (can't be NULL)
 * @param cbs_userdata opaque pointer used by the callbacks
 * @param id unique id provided by the caller. This is can be used to cancel
 * the request with vlc_preparser_Cancel()
 * @return VLC_PREPARSER_REQ_ID_INVALID in case of error, or a valid id if the
 * item was scheduled for preparsing. If this returns an
 * error, the on_preparse_ended will *not* be invoked
 */
VLC_API vlc_preparser_req_id
vlc_preparser_Push( vlc_preparser_t *preparser, input_item_t *item, int type_option,
                    const input_item_parser_cbs_t *cbs, void *cbs_userdata );

/**
 * This function enqueues the provided item for generating a thumbnail
 *
 * @param preparser the preparser object
 * @param item a valid item to generate the thumbnail for
 * @param arg pointer to the arg struct, NULL for default options
 * @param cbs callback to listen to events (can't be NULL)
 * @param cbs_userdata opaque pointer used by the callbacks
 * @return VLC_PREPARSER_REQ_ID_INVALID in case of error, or a valid id if the
 * item was scheduled for thumbnailing. If this returns an
 * error, the thumbnailer.on_ended callback will *not* be invoked
 *
 * The provided input_item will be held by the thumbnailer and can safely be
 * released safely after calling this function.
 */
VLC_API vlc_preparser_req_id
vlc_preparser_GenerateThumbnail( vlc_preparser_t *preparser, input_item_t *item,
                                 const struct vlc_thumbnailer_arg *arg,
                                 const struct vlc_thumbnailer_cbs *cbs,
                                 void *cbs_userdata );

/**
 * Get the best possible format
 *
 * @param[out] format pointer to the best format
 * @param[out] out_ext pointer to the extension of the format
 * @return 0 if a format was found, VLC_ENOENT otherwise (in case there are no
 * "image encoder" modules)
 */
VLC_API int
vlc_preparser_GetBestThumbnailerFormat(enum vlc_thumbnailer_format *format,
                                       const char **out_ext);

/**
 * Check if the format is handled by VLC
 *
 * @param format format to check
 * @return 0 if the format was found, VLC_ENOENT otherwise (in case there are
 * no "image encoder" modules)
 */
VLC_API int
vlc_preparser_CheckThumbnailerFormat(enum vlc_thumbnailer_format format);

/**
 * This function generates a thumbnail to one or several files
 *
 * @param preparser the preparser object
 * @param item a valid item to generate the thumbnail for
 * @param arg pointer to the arg struct, NULL for default options
 * @param outputs array of outputs, one file will be generated per output for a
 * single thumbnail
 * @param output_count outputs array size, must be > 1
 * @param cbs callback to listen to events (can't be NULL)
 * @param cbs_userdata opaque pointer used by the callbacks
 * @return VLC_PREPARSER_REQ_ID_INVALID in case of error, or a valid id if the
 * item was scheduled for thumbnailing. If this returns an
 * error, the thumbnailer.on_ended callback will *not* be invoked
 *
 * The provided input_item will be held by the thumbnailer and can safely be
 * released safely after calling this function.
 */
VLC_API vlc_preparser_req_id
vlc_preparser_GenerateThumbnailToFiles( vlc_preparser_t *preparser, input_item_t *item,
                                        const struct vlc_thumbnailer_arg *arg,
                                        const struct vlc_thumbnailer_output *outputs,
                                        size_t output_count,
                                        const struct vlc_thumbnailer_to_files_cbs *cbs,
                                        void *cbs_userdata );

/**
 * This function cancel all preparsing requests for a given id
 *
 * @param preparser the preparser object
 * @param id unique id returned by vlc_preparser_Push(),
 * VLC_PREPARSER_REQ_ID_INVALID to cancels all tasks
 * @return number of tasks cancelled
 */
VLC_API size_t vlc_preparser_Cancel( vlc_preparser_t *preparser,
                                     vlc_preparser_req_id id );

/**
 * This function destroys the preparser object and thread.
 *
 * @param preparser the preparser object
 * All pending input items will be released.
 */
VLC_API void vlc_preparser_Delete( vlc_preparser_t *preparser );

/**
 * Do not use, libVLC only fonction, will be removed soon
 */
VLC_API void vlc_preparser_SetTimeout( vlc_preparser_t *preparser,
                                       vlc_tick_t timeout ) VLC_DEPRECATED;

/** @} vlc_preparser */

#endif

