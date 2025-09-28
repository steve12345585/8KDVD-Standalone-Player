/*****************************************************************************
 * sub_renderer.c
 *****************************************************************************
 * Copyright (C) 2004-2020 VLC authors and VideoLAN
 * Copyright (C) 2009, 2011 Laurent Aimar
 *
 * Authors: Laurent Aimar <fenrir _AT_ videolan _DOT_ org>
 *          Ilkka Ollakka <ileoo@videolan.org>
 *          Rémi Denis-Courmont
 *          Adrien Maglo <magsoft at videolan dot org>
 *          Felix Paul Kühne <fkuehne at videolan dot org>
 *          Pierre d'Herbemont <pdherbemont at videolan dot org>
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

#include "sub_renderer.h"

#include <assert.h>
#include <stdbit.h>
#include <vlc_common.h>
#include <vlc_arrays.h>
#include <vlc_es.h>
#include <vlc_subpicture.h>
#include <vlc_opengl_interop.h>

#include "gl_util.h"

typedef struct {
    GLuint   texture;
    GLsizei  width;
    GLsizei  height;

    float    alpha;

    float    top;
    float    left;
    float    bottom;
    float    right;

    float    tex_width;
    float    tex_height;
} gl_region_t;

struct vlc_gl_sub_renderer
{
    vlc_gl_t *gl;
    const struct vlc_gl_api *api;
    const opengl_vtable_t *vt; /* for convenience, same as &api->vt */

    struct vlc_gl_interop *interop;

    gl_region_t *regions;
    unsigned region_count;
    unsigned output_width, output_height;

    GLuint program_id;
    struct {
        GLint vertex_pos;
        GLint tex_coords_in;
    } aloc;
    struct {
        GLint sampler;
        GLint alpha;
    } uloc;

    GLuint *buffer_objects;
    unsigned buffer_object_count;
};

static int
FetchLocations(struct vlc_gl_sub_renderer *sr)
{
    assert(sr->program_id);

    const opengl_vtable_t *vt = sr->vt;

#define GET_LOC(type, x, str) do { \
    x = vt->Get##type##Location(sr->program_id, str); \
    assert(x != -1); \
    if (x == -1) { \
        msg_Err(sr->gl, "Unable to Get"#type"Location(%s)", str); \
        return VLC_EGENERIC; \
    } \
} while (0)
#define GET_ULOC(x, str) GET_LOC(Uniform, x, str)
#define GET_ALOC(x, str) GET_LOC(Attrib, x, str)
    GET_ULOC(sr->uloc.sampler, "sampler");
    GET_ULOC(sr->uloc.alpha, "alpha");
    GET_ALOC(sr->aloc.vertex_pos, "vertex_pos");
    GET_ALOC(sr->aloc.tex_coords_in, "tex_coords_in");

#undef GET_LOC
#undef GET_ULOC
#undef GET_ALOC

    return VLC_SUCCESS;
}

struct vlc_gl_sub_renderer *
vlc_gl_sub_renderer_New(vlc_gl_t *gl, const struct vlc_gl_api *api,
                        struct vlc_gl_interop *interop)
{
    const opengl_vtable_t *vt = &api->vt;

    struct vlc_gl_sub_renderer *sr = malloc(sizeof(*sr));
    if (!sr)
        return NULL;

    /* Allocates our textures */
    assert(!interop->handle_texs_gen);

    sr->interop = interop;
    sr->gl = gl;
    sr->api = api;
    sr->vt = vt;
    sr->region_count = 0;
    sr->output_width = 0;
    sr->output_height = 0;
    sr->regions = NULL;

    static const char *const VERTEX_SHADER_SRC =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"

        "attribute vec2 vertex_pos;\n"
        "attribute vec2 tex_coords_in;\n"
        "varying vec2 tex_coords;\n"
        "void main() {\n"
        "  tex_coords = tex_coords_in;\n"
        "  gl_Position = vec4(vertex_pos, 0.0, 1.0);\n"
        "}\n";

    static const char *const FRAGMENT_SHADER_SRC =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"

        "uniform sampler2D sampler;\n"
        "uniform float alpha;\n"
        "varying vec2 tex_coords;\n"
        "void main() {\n"
        "  vec4 color = texture2D(sampler, tex_coords);\n"
        "  color.a *= alpha;\n"
        "  gl_FragColor = color;\n"
        "}\n";

    const char *glsl_version = gl->api_type == VLC_OPENGL ?
        "#version 120\n" : "#version 100\n";

    const char *vertex_shader[] = {
        glsl_version,
        VERTEX_SHADER_SRC,
    };

    const char *fragment_shader[] = {
        glsl_version,
        FRAGMENT_SHADER_SRC,
    };

    sr->program_id =
        vlc_gl_BuildProgram(VLC_OBJECT(sr->gl), vt,
                            ARRAY_SIZE(vertex_shader), vertex_shader,
                            ARRAY_SIZE(fragment_shader), fragment_shader);
    if (!sr->program_id)
        goto error_1;

    int ret = FetchLocations(sr);
    if (ret != VLC_SUCCESS)
        goto error_2;

    /* Initial number of allocated buffer objects for subpictures, will grow dynamically. */
    static const unsigned INITIAL_BUFFER_OBJECT_COUNT = 8;
    sr->buffer_objects = vlc_alloc(INITIAL_BUFFER_OBJECT_COUNT, sizeof(GLuint));
    if (!sr->buffer_objects)
        goto error_2;

    sr->buffer_object_count = INITIAL_BUFFER_OBJECT_COUNT;

    vt->GenBuffers(sr->buffer_object_count, sr->buffer_objects);

    return sr;

error_2:
    vt->DeleteProgram(sr->program_id);
error_1:
    free(sr);

    return NULL;
}

void
vlc_gl_sub_renderer_Delete(struct vlc_gl_sub_renderer *sr)
{
    if (sr->buffer_object_count)
        sr->vt->DeleteBuffers(sr->buffer_object_count, sr->buffer_objects);
    free(sr->buffer_objects);

    for (unsigned i = 0; i < sr->region_count; ++i)
    {
        if (sr->regions[i].texture)
            sr->vt->DeleteTextures(1, &sr->regions[i].texture);
    }
    free(sr->regions);

    assert(sr->program_id);
    sr->vt->DeleteProgram(sr->program_id);

    free(sr);
}

void
vlc_gl_sub_renderer_SetOutputSize(struct vlc_gl_sub_renderer *sr,
                                  unsigned width, unsigned height)
{
    sr->output_width = width;
    sr->output_height = height;
}

int
vlc_gl_sub_renderer_Prepare(struct vlc_gl_sub_renderer *sr,
                            const vlc_render_subpicture *subpicture)
{
    GL_ASSERT_NOERROR(sr->vt);

    if (unlikely(sr->output_width == 0 || sr->output_height == 0))
        return VLC_EINVAL;

    const struct vlc_gl_interop *interop = sr->interop;

    int last_count = sr->region_count;
    gl_region_t *last = sr->regions;

    if (subpicture) {
        int count = subpicture->regions.size;
        const struct subpicture_region_rendered *r;

        gl_region_t *regions = calloc(count, sizeof(*regions));
        if (!regions)
            return VLC_ENOMEM;

        sr->region_count = count;
        sr->regions = regions;

        int i = 0;
        vlc_vector_foreach(r, &subpicture->regions) {
            gl_region_t *glr = &sr->regions[i];

            glr->width  = r->p_picture->format.i_visible_width;
            glr->height = r->p_picture->format.i_visible_height;
            if (!sr->api->supports_npot) {
                glr->width  = stdc_bit_ceil(r->p_picture->format.i_visible_width);
                glr->height = stdc_bit_ceil(r->p_picture->format.i_visible_height);
                glr->tex_width  = (float) r->p_picture->format.i_visible_width  / glr->width;
                glr->tex_height = (float) r->p_picture->format.i_visible_height / glr->height;
            } else {
                glr->tex_width  = 1.0;
                glr->tex_height = 1.0;
            }
            glr->alpha  = (float)r->i_alpha / 255;
            glr->left   =  2.0 * (r->place.x                  ) / sr->output_width  - 1.0;
            glr->top    = -2.0 * (r->place.y                  ) / sr->output_height + 1.0;
            glr->right  =  2.0 * (r->place.x + r->place.width ) / sr->output_width  - 1.0;
            glr->bottom = -2.0 * (r->place.y + r->place.height) / sr->output_height + 1.0;

            glr->texture = 0;
            /* Try to recycle the textures allocated by the previous
               call to this function. */
            for (int j = 0; j < last_count; j++) {
                if (last[j].texture &&
                    last[j].width  == glr->width &&
                    last[j].height == glr->height) {
                    glr->texture = last[j].texture;
                    memset(&last[j], 0, sizeof(last[j]));
                    break;
                }
            }

            const size_t pixels_offset =
                r->p_picture->format.i_y_offset * r->p_picture->p->i_pitch +
                r->p_picture->format.i_x_offset * r->p_picture->p->i_pixel_pitch;
            if (!glr->texture)
            {
                /* Could not recycle a previous texture, generate a new one. */
                int ret = vlc_gl_interop_GenerateTextures(interop, &glr->width,
                                                          &glr->height,
                                                          &glr->texture);
                if (ret != VLC_SUCCESS)
                    break;
            }
            /* Use the visible pitch of the region */
            r->p_picture->p[0].i_visible_pitch = r->p_picture->format.i_visible_width
                                               * r->p_picture->p[0].i_pixel_pitch;

            GLsizei width = r->p_picture->format.i_visible_width;
            GLsizei height = r->p_picture->format.i_visible_height;
            int ret = interop->ops->update_textures(interop, &glr->texture,
                                                    &width, &height,
                                                    r->p_picture, &pixels_offset);
            if (ret != VLC_SUCCESS)
                break;
            i++;
        }
    }
    else
    {
        sr->region_count = 0;
        sr->regions = NULL;
    }

    for (int i = 0; i < last_count; i++) {
        if (last[i].texture)
            vlc_gl_interop_DeleteTextures(interop, &last[i].texture);
    }
    free(last);

    GL_ASSERT_NOERROR(sr->vt);

    return VLC_SUCCESS;
}

int
vlc_gl_sub_renderer_Draw(struct vlc_gl_sub_renderer *sr)
{
    const struct vlc_gl_interop *interop = sr->interop;
    const opengl_vtable_t *vt = sr->vt;

    GL_ASSERT_NOERROR(vt);

    assert(sr->program_id);
    vt->UseProgram(sr->program_id);

    vt->Enable(GL_BLEND);
    vt->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vt->Viewport(0, 0, sr->output_width, sr->output_height);

    /* We need two buffer objects for each region: for vertex and texture coordinates. */
    if (2 * sr->region_count > sr->buffer_object_count) {
        if (sr->buffer_object_count > 0)
            vt->DeleteBuffers(sr->buffer_object_count, sr->buffer_objects);
        sr->buffer_object_count = 0;

        int new_count = 2 * sr->region_count;
        sr->buffer_objects = realloc_or_free(sr->buffer_objects, new_count * sizeof(GLuint));
        if (!sr->buffer_objects)
            return VLC_ENOMEM;

        sr->buffer_object_count = new_count;
        vt->GenBuffers(sr->buffer_object_count, sr->buffer_objects);
    }

    vt->ActiveTexture(GL_TEXTURE0 + 0);
    for (unsigned i = 0; i < sr->region_count; i++) {
        gl_region_t *glr = &sr->regions[i];

        GLfloat textureCoordNormal[] = {
            0.0, 0.0,
            0.0, glr->tex_height,
            glr->tex_width, 0.0,
            glr->tex_width, glr->tex_height,
        };

        const GLfloat textureCoord90[] = {
            0.0, glr->tex_height,
            glr->tex_width, glr->tex_height,
            0.0, 0.0,
            glr->tex_width, 0.0,
        };

        GLfloat left, right, top, bottom;
        const GLfloat *textureCoord;
        switch (sr->gl->orientation)
        {
            case ORIENT_TOP_LEFT:
            case ORIENT_TOP_RIGHT:
                left = glr->left;
                right = glr->right;
                top = glr->top;
                bottom = glr->bottom;
                textureCoord = textureCoordNormal;
                break;

            case ORIENT_LEFT_TOP:
            case ORIENT_LEFT_BOTTOM:
                left = glr->bottom;
                right = glr->top;
                top = -glr->left;
                bottom = -glr->right;
                textureCoord = textureCoord90;
                break;

            case ORIENT_BOTTOM_LEFT:
            case ORIENT_BOTTOM_RIGHT:
                left = -glr->left;
                right = -glr->right;
                top = -glr->top;
                bottom = -glr->bottom;
                textureCoord = textureCoordNormal;
                break;

            case ORIENT_RIGHT_TOP:
            case ORIENT_RIGHT_BOTTOM:
                left = - glr->bottom;
                right = - glr->top;
                top = glr->left;
                bottom = glr->right;
                textureCoord = textureCoord90;
                break;
            default:
                vlc_assert_unreachable();
        }

        if (ORIENT_IS_MIRROR(sr->gl->orientation))
        {
            switch (sr->gl->orientation)
            {
                case ORIENT_TOP_RIGHT:
                case ORIENT_BOTTOM_LEFT:
                    left  = -left;
                    right = -right;
                    break;

                case ORIENT_LEFT_TOP:
                case ORIENT_RIGHT_BOTTOM:
                    top    = -top;
                    bottom = -bottom;
                    break;

                default:
                    vlc_assert_unreachable();
            }
        }


        const GLfloat vertexCoord[] = {
            left,  top,
            left,  bottom,
            right, top,
            right, bottom,
        };

                assert(glr->texture != 0);
        vt->BindTexture(interop->tex_target, glr->texture);

        vt->Uniform1f(sr->uloc.alpha, glr->alpha);

        vt->BindBuffer(GL_ARRAY_BUFFER, sr->buffer_objects[2 * i]);
        static_assert(sizeof(textureCoordNormal) == sizeof(textureCoord90),
                      "textureCoordNormal != textureCoord90");
        vt->BufferData(GL_ARRAY_BUFFER, sizeof(textureCoordNormal), textureCoord, GL_STATIC_DRAW);
        vt->EnableVertexAttribArray(sr->aloc.tex_coords_in);
        vt->VertexAttribPointer(sr->aloc.tex_coords_in, 2, GL_FLOAT, 0, 0, 0);

        vt->BindBuffer(GL_ARRAY_BUFFER, sr->buffer_objects[2 * i + 1]);
        vt->BufferData(GL_ARRAY_BUFFER, sizeof(vertexCoord), vertexCoord, GL_STATIC_DRAW);
        vt->EnableVertexAttribArray(sr->aloc.vertex_pos);
        vt->VertexAttribPointer(sr->aloc.vertex_pos, 2, GL_FLOAT, 0, 0, 0);

        vt->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    vt->Disable(GL_BLEND);

    GL_ASSERT_NOERROR(vt);

    return VLC_SUCCESS;
}
