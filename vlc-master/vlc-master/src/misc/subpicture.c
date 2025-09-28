/*****************************************************************************
 * subpicture.c: Subpicture functions
 *****************************************************************************
 * Copyright (C) 2010 Laurent Aimar <fenrir _AT_ videolan _DOT_ org>
 *
 * Authors: Laurent Aimar <fenrir _AT_ videolan _DOT_ org>
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>

#include <vlc_common.h>
#include <vlc_image.h>
#include <vlc_subpicture.h>
#include "subpicture.h"

#include <limits.h>

struct subpicture_private_t
{
    video_format_t src;
    video_format_t dst;
};

subpicture_t *subpicture_New( const subpicture_updater_t *p_upd )
{
    subpicture_t *p_subpic = calloc( 1, sizeof(*p_subpic) );
    if( !p_subpic )
        return NULL;

    p_subpic->i_order    = 0;
    p_subpic->b_fade     = false;
    p_subpic->b_subtitle = false;
    p_subpic->i_alpha    = 0xFF;
    vlc_spu_regions_init(&p_subpic->regions);

    if( p_upd )
    {
        subpicture_private_t *p_private = malloc( sizeof(*p_private) );
        if( !p_private )
        {
            free( p_subpic );
            return NULL;
        }
        video_format_Init( &p_private->src, 0 );
        video_format_Init( &p_private->dst, 0 );

        p_subpic->updater   = *p_upd;
        p_subpic->p_private = p_private;
    }
    else
    {
        p_subpic->p_private = NULL;
        p_subpic->updater.ops = NULL;
        p_subpic->updater.sys = NULL;
    }
    return p_subpic;
}

void subpicture_Delete( subpicture_t *p_subpic )
{
    vlc_spu_regions_Clear( &p_subpic->regions );

    if (p_subpic->updater.ops != NULL &&
        p_subpic->updater.ops->destroy != NULL)
    {
        p_subpic->updater.ops->destroy(p_subpic);
    }

    if( p_subpic->p_private )
    {
        video_format_Clean( &p_subpic->p_private->src );
        video_format_Clean( &p_subpic->p_private->dst );
    }

    free( p_subpic->p_private );
    free( p_subpic );
}

vlc_render_subpicture *vlc_render_subpicture_New( void )
{
    vlc_render_subpicture *p_subpic = malloc( sizeof(*p_subpic) );
    if( unlikely(p_subpic == NULL ) )
        return NULL;
    vlc_vector_init(&p_subpic->regions);
    return p_subpic;
}

void vlc_render_subpicture_Delete( vlc_render_subpicture *p_subpic )
{
    struct subpicture_region_rendered *p_region;
    vlc_vector_foreach(p_region, &p_subpic->regions)
    {
        picture_Release( p_region->p_picture );
        free( p_region );
    }
    vlc_vector_clear( &p_subpic->regions );
    free( p_subpic );
}

subpicture_t *subpicture_NewFromPicture( vlc_object_t *p_obj,
                                         picture_t *p_picture, vlc_fourcc_t i_chroma )
{
    /* */
    video_format_t fmt_in = p_picture->format;

    /* */
    video_format_t fmt_out;
    fmt_out = fmt_in;
    fmt_out.i_chroma = i_chroma;

    /* */
    image_handler_t *p_image = image_HandlerCreate( p_obj );
    if( !p_image )
        return NULL;

    picture_t *p_pip = image_Convert( p_image, p_picture, &fmt_in, &fmt_out );

    image_HandlerDelete( p_image );

    if( !p_pip )
        return NULL;

    subpicture_t *p_subpic = subpicture_New( NULL );
    if( unlikely(!p_subpic) )
    {
         picture_Release( p_pip );
         return NULL;
    }

    p_subpic->i_original_picture_width  = fmt_out.i_visible_width;
    p_subpic->i_original_picture_height = fmt_out.i_visible_height;

    subpicture_region_t *p_region = subpicture_region_ForPicture( p_pip );
    picture_Release( p_pip );

    if (likely(p_region == NULL))
    {
        subpicture_Delete(p_subpic);
        return NULL;
    }

    p_region->b_absolute = true; p_region->b_in_window = false;
    p_region->fmt.i_sar_num =
    p_region->fmt.i_sar_den = 0;

    // margin to the center or to the top/left if the subpicture is absolute
    p_region->i_x = 0;
    p_region->i_y = 0;

    vlc_spu_regions_push( &p_subpic->regions, p_region );
    return p_subpic;
}

void subpicture_Update( subpicture_t *p_subpicture,
                        const video_format_t *p_fmt_src,
                        const video_format_t *p_fmt_dst,
                        unsigned display_width, unsigned display_height,
                        vlc_tick_t i_ts )
{
    subpicture_updater_t *p_upd = &p_subpicture->updater;
    subpicture_private_t *p_private = p_subpicture->p_private;

    if (p_upd->ops == NULL)
        return;

    struct vlc_spu_updater_configuration cfg = {
        .prev_src  = &p_private->src,
        .prev_dst  = &p_private->dst,
        .display_width  = display_width,
        .display_height = display_height,
        .video_src = p_fmt_src,
        .video_dst = p_fmt_dst,
        .pts       = i_ts,
    };
    p_upd->ops->update( p_subpicture, &cfg );

    video_format_Clean( &p_private->src );
    video_format_Clean( &p_private->dst );

    video_format_Copy( &p_private->src, p_fmt_src );
    video_format_Copy( &p_private->dst, p_fmt_dst );
}

typedef struct subpicture_region_private_t
{
    subpicture_region_t region;
    video_format_t fmt;
    picture_t *p_picture;
} subpicture_region_private_t;

const video_format_t * subpicture_region_cache_GetFormat( const subpicture_region_t *p_region )
{
    subpicture_region_private_t *p_priv = container_of(p_region, subpicture_region_private_t, region);
    return &p_priv->fmt;
}

picture_t * subpicture_region_cache_GetPicture( subpicture_region_t *p_region )
{
    subpicture_region_private_t *p_priv = container_of(p_region, subpicture_region_private_t, region);
    return p_priv->p_picture;
}

bool subpicture_region_cache_IsValid(const subpicture_region_t *p_region)
{
    subpicture_region_private_t *p_priv = container_of(p_region, subpicture_region_private_t, region);
    return p_priv->fmt.i_chroma;
}
void subpicture_region_cache_Invalidate( subpicture_region_t *p_region )
{
    subpicture_region_private_t *p_priv = container_of(p_region, subpicture_region_private_t, region);
    if( p_priv->p_picture )
    {
        picture_Release( p_priv->p_picture );
        p_priv->p_picture = NULL;
    }
    video_format_Clean( &p_priv->fmt );
    video_format_Init( &p_priv->fmt, 0 );
}

int subpicture_region_cache_Assign( subpicture_region_t *p_region, picture_t *p_picture )
{
    subpicture_region_private_t *p_priv = container_of(p_region, subpicture_region_private_t, region);
    if ( video_format_Copy( &p_priv->fmt, &p_picture->format ) != VLC_SUCCESS )
        return VLC_EGENERIC;
    p_priv->p_picture = p_picture;
    return VLC_SUCCESS;
}

static subpicture_region_t * subpicture_region_NewInternal( void )
{
    subpicture_region_private_t *p_region = calloc( 1, sizeof(subpicture_region_private_t) );
    if( unlikely(p_region == NULL) )
        return NULL;

    p_region->region.i_alpha = 0xff;
    p_region->region.i_x = INT_MAX;
    p_region->region.i_y = INT_MAX;
    p_region->region.b_absolute = false; p_region->region.b_in_window = false;

    return &p_region->region;
}

subpicture_region_t *subpicture_region_New( const video_format_t *p_fmt )
{
    assert(p_fmt->i_chroma != VLC_CODEC_TEXT);
    subpicture_region_t *p_region = subpicture_region_NewInternal( );
    if( !p_region )
        return NULL;

    video_format_Copy( &p_region->fmt, p_fmt );
    if (p_region->fmt.transfer == TRANSFER_FUNC_UNDEF)
        p_region->fmt.transfer = TRANSFER_FUNC_SRGB;
    if (p_region->fmt.primaries == COLOR_PRIMARIES_UNDEF)
        p_region->fmt.primaries = COLOR_PRIMARIES_SRGB;
    if (p_region->fmt.space == COLOR_SPACE_UNDEF)
        p_region->fmt.space = COLOR_SPACE_SRGB;
    if (p_region->fmt.color_range == COLOR_RANGE_UNDEF)
        p_region->fmt.color_range = COLOR_RANGE_FULL;
    p_region->p_picture = picture_NewFromFormat( &p_region->fmt );
    if( !p_region->p_picture )
    {
        video_format_Clean( &p_region->fmt );
        free( p_region );
        return NULL;
    }

    if ( p_fmt->i_chroma == VLC_CODEC_YUVP || p_fmt->i_chroma == VLC_CODEC_RGBP )
    {
        /* YUVP/RGBP should have a palette */
        if( p_region->p_picture->format.p_palette == NULL )
        {
            p_region->p_picture->format.p_palette = calloc( 1, sizeof(*p_region->p_picture->format.p_palette) );
            if( p_region->p_picture->format.p_palette == NULL )
            {
                video_format_Clean( &p_region->fmt );
                picture_Release( p_region->p_picture );
                free( p_region );
                return NULL;
            }
        }
    }
    else
    {
        assert(p_fmt->p_palette == NULL);
    }

    return p_region;
}

subpicture_region_t *subpicture_region_NewText( void )
{
    subpicture_region_t *p_region = subpicture_region_NewInternal( );
    if( !p_region )
        return NULL;

    p_region->text_flags |= VLC_SUBPIC_TEXT_FLAG_IS_TEXT;

    video_format_Init( &p_region->fmt, 0 );
    p_region->fmt.transfer = TRANSFER_FUNC_SRGB;
    p_region->fmt.primaries = COLOR_PRIMARIES_SRGB;
    p_region->fmt.space = COLOR_SPACE_SRGB;
    p_region->fmt.color_range = COLOR_RANGE_FULL;

    return p_region;
}

subpicture_region_t *subpicture_region_ForPicture( picture_t *pic )
{
    subpicture_region_t *p_region = subpicture_region_NewInternal( );
    if( !p_region )
        return NULL;

    video_format_Copy( &p_region->fmt, &pic->format );
    if ( pic->format.i_chroma == VLC_CODEC_YUVP || pic->format.i_chroma == VLC_CODEC_RGBP )
    {
        /* YUVP/RGBP should have a palette */
        if( pic->format.p_palette == NULL )
        {
            pic->format.p_palette = calloc( 1, sizeof(*pic->format.p_palette) );
            if( pic->format.p_palette == NULL )
            {
                video_format_Clean( &pic->format );
                free( p_region );
                return NULL;
            }
        }
    }
    else
    {
        assert(pic->format.p_palette == NULL);
    }

    p_region->p_picture = picture_Hold(pic);

    return p_region;
}

void subpicture_region_Delete( subpicture_region_t *p_region )
{
    if( !p_region )
        return;

    subpicture_region_private_t *p_priv = container_of(p_region, subpicture_region_private_t, region);

    subpicture_region_cache_Invalidate( p_region );

    if( p_region->p_picture )
        picture_Release( p_region->p_picture );

    text_segment_ChainDelete( p_region->p_text );
    video_format_Clean( &p_region->fmt );
    free( p_priv );
}

void vlc_spu_regions_Clear( vlc_spu_regions *regions )
{
    subpicture_region_t *p_head;
    vlc_spu_regions_foreach(p_head, regions)
    {
        vlc_spu_regions_remove( regions, p_head );
        subpicture_region_Delete( p_head );
    }
}

#include <vlc_filter.h>

unsigned picture_BlendSubpicture(picture_t *dst,
                                 vlc_blender_t *blend, vlc_render_subpicture *src)
{
    unsigned done = 0;

    assert(src);

    struct subpicture_region_rendered *r;
    vlc_vector_foreach(r, &src->regions) {
        assert(r->p_picture);

        if (filter_ConfigureBlend(blend, dst->format.i_width,
                                  dst->format.i_height,  &r->p_picture->format)
         || filter_Blend(blend, dst, r->place.x, r->place.y, r->p_picture, r->i_alpha))
            msg_Err(blend, "blending %4.4s to %4.4s failed",
                    (char *)&blend->fmt_in.video.i_chroma,
                    (char *)&blend->fmt_out.video.i_chroma );
        else
            done++;
    }
    return done;
}
