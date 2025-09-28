/*****************************************************************************
 * va.c: hardware acceleration plugins for avcodec
 *****************************************************************************
 * Copyright (C) 2009 Laurent Aimar
 * Copyright (C) 2012-2013 Rémi Denis-Courmont
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
# include <config.h>
#endif

#include <vlc_common.h>
#include <vlc_modules.h>
#include <vlc_fourcc.h>
#include <libavutil/pixfmt.h>
#include <libavcodec/avcodec.h>
#include "va.h"

bool vlc_va_MightDecode(enum AVPixelFormat hwfmt)
{
    switch (hwfmt)
    {
        case AV_PIX_FMT_VAAPI:
        case AV_PIX_FMT_DXVA2_VLD:
        case AV_PIX_FMT_D3D11VA_VLD:
        case AV_PIX_FMT_VDPAU:
            return true;
        default:
            return false;
    }
}

vlc_va_t *vlc_va_New(vlc_object_t *obj, struct vlc_va_cfg *cfg)
{
    struct vlc_va_t *va = vlc_object_create(obj, sizeof (*va));
    if (unlikely(va == NULL))
        return NULL;

    module_t **mods;
    ssize_t total = vlc_module_match("hw decoder", NULL, false, &mods, NULL);

    for (ssize_t i = 0; i < total; i++) {
        vlc_va_open open = vlc_module_map(obj->logger, mods[i]);

        if (open != NULL && open(va, cfg) == VLC_SUCCESS)
        {
            free(mods);
            return va;
        }
    }

    free(mods);
    vlc_object_delete(va);
    return NULL;
}

void vlc_va_Delete(vlc_va_t *va, AVCodecContext *avctx)
{
    if (va->ops->close != NULL)
        va->ops->close(va, avctx);
    vlc_object_delete(va);
}
