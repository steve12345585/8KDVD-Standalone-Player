#include "cef_render_handler.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_messages.h>
#include <vlc_picture.h>
#include <vlc_fourcc.h>
#include <cstring>

VLCCefRenderHandler::VLCCefRenderHandler(vlc_object_t *obj) 
    : vlc_obj_(obj), vout_(nullptr), frame_buffer_(nullptr),
      frame_width_(1920), frame_height_(1080), frame_x_(0), frame_y_(0),
      enable_8kdvd_rendering_(false), kdvd_width_(7680), kdvd_height_(4320),
      vlc_picture_(nullptr), needs_vlc_update_(false) {
    
    AllocateFrameBuffer(frame_width_, frame_height_);
    msg_Info(vlc_obj_, "CEF render handler created: %dx%d", frame_width_, frame_height_);
}

VLCCefRenderHandler::~VLCCefRenderHandler() {
    FreeFrameBuffer();
    if (vlc_picture_) {
        picture_Release(vlc_picture_);
    }
    msg_Info(vlc_obj_, "CEF render handler destroyed");
}

void VLCCefRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    CEF_REQUIRE_UI_THREAD();
    
    if (enable_8kdvd_rendering_) {
        rect = CefRect(0, 0, kdvd_width_, kdvd_height_);
    } else {
        rect = CefRect(0, 0, frame_width_, frame_height_);
    }
}

void VLCCefRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                                  PaintElementType type,
                                  const RectList& dirtyRects,
                                  const void* buffer,
                                  int width,
                                  int height) {
    CEF_REQUIRE_UI_THREAD();
    
    if (type != PET_VIEW) return;
    
    // Update frame dimensions if changed
    if (width != frame_width_ || height != frame_height_) {
        frame_width_ = width;
        frame_height_ = height;
        AllocateFrameBuffer(width, height);
        needs_vlc_update_ = true;
    }
    
    // Copy frame buffer
    if (frame_buffer_ && buffer) {
        memcpy(frame_buffer_, buffer, width * height * 4); // RGBA format
        needs_vlc_update_ = true;
        
        // Debug output every 60 frames
        static int frame_count = 0;
        if (++frame_count % 60 == 0) {
            msg_Dbg(vlc_obj_, "CEF frame rendered: %dx%d (8KDVD: %s)", 
                   width, height, enable_8kdvd_rendering_ ? "enabled" : "disabled");
        }
    }
    
    // Update VLC video output if needed
    if (needs_vlc_update_) {
        UpdateVLCVideoOutput();
    }
}

void VLCCefRenderHandler::SetVout(vout_thread_t *vout) {
    vout_ = vout;
    msg_Info(vlc_obj_, "CEF render handler connected to VLC vout");
}

void VLCCefRenderHandler::SetRenderSize(int width, int height) {
    if (width != frame_width_ || height != frame_height_) {
        frame_width_ = width;
        frame_height_ = height;
        AllocateFrameBuffer(width, height);
        needs_vlc_update_ = true;
        msg_Info(vlc_obj_, "CEF render size changed: %dx%d", width, height);
    }
}

void VLCCefRenderHandler::SetRenderPosition(int x, int y) {
    frame_x_ = x;
    frame_y_ = y;
    msg_Dbg(vlc_obj_, "CEF render position: %d,%d", x, y);
}

void VLCCefRenderHandler::Enable8KDVDRendering(bool enable) {
    enable_8kdvd_rendering_ = enable;
    msg_Info(vlc_obj_, "8KDVD rendering %s", enable ? "enabled" : "disabled");
}

void VLCCefRenderHandler::Set8KDVDResolution(int width, int height) {
    kdvd_width_ = width;
    kdvd_height_ = height;
    msg_Info(vlc_obj_, "8KDVD resolution set: %dx%d", width, height);
}

bool VLCCefRenderHandler::RenderToVLC() {
    if (!vout_ || !frame_buffer_) return false;
    
    ConvertFrameToVLC();
    return true;
}

void VLCCefRenderHandler::UpdateVLCVideoOutput() {
    if (!vout_ || !frame_buffer_) return;
    
    // Convert CEF frame to VLC picture format
    ConvertFrameToVLC();
    
    // Update VLC video output
    if (vlc_picture_) {
        vout_PutPicture(vout_, vlc_picture_);
        vlc_picture_ = nullptr; // VLC takes ownership
    }
    
    needs_vlc_update_ = false;
}

void VLCCefRenderHandler::AllocateFrameBuffer(int width, int height) {
    FreeFrameBuffer();
    
    size_t buffer_size = width * height * 4; // RGBA
    frame_buffer_ = malloc(buffer_size);
    
    if (frame_buffer_) {
        msg_Dbg(vlc_obj_, "CEF frame buffer allocated: %dx%d (%zu bytes)", 
               width, height, buffer_size);
    } else {
        msg_Err(vlc_obj_, "Failed to allocate CEF frame buffer");
    }
}

void VLCCefRenderHandler::FreeFrameBuffer() {
    if (frame_buffer_) {
        free(frame_buffer_);
        frame_buffer_ = nullptr;
    }
}

void VLCCefRenderHandler::ConvertFrameToVLC() {
    if (!vout_ || !frame_buffer_) return;
    
    // Create VLC picture
    vlc_picture_ = picture_NewFromFormat(VLC_CODEC_RGBA, frame_width_, frame_height_);
    if (!vlc_picture_) {
        msg_Err(vlc_obj_, "Failed to create VLC picture");
        return;
    }
    
    // Copy frame data to VLC picture
    uint8_t* vlc_data = vlc_picture_->p[0].p_pixels;
    memcpy(vlc_data, frame_buffer_, frame_width_ * frame_height_ * 4);
    
    msg_Dbg(vlc_obj_, "CEF frame converted to VLC format: %dx%d", 
           frame_width_, frame_height_);
}

void VLCCefRenderHandler::UpdateVLCVideoFormat() {
    if (!vout_) return;
    
    // Update VLC video format for 8KDVD
    if (enable_8kdvd_rendering_) {
        // Set 8K video format
        msg_Info(vlc_obj_, "Updating VLC video format for 8KDVD: %dx%d", 
                kdvd_width_, kdvd_height_);
    } else {
        // Set standard video format
        msg_Info(vlc_obj_, "Updating VLC video format: %dx%d", 
                frame_width_, frame_height_);
    }
}

// Texture sharing with VLC
bool VLCCefRenderHandler::ShareTextureWithVLC() {
    if (!vout_ || !frame_buffer_) return false;
    
    // Share CEF frame buffer with VLC's video output
    // This would integrate with VLC's texture sharing system
    msg_Dbg(vlc_obj_, "Sharing CEF texture with VLC: %dx%d", frame_width_, frame_height_);
    
    // TODO: Implement actual texture sharing
    // This would use VLC's video output API to share the frame buffer
    
    return true;
}

// Render buffer management
void VLCCefRenderHandler::ManageRenderBuffers() {
    // Manage CEF render buffers for optimal performance
    if (enable_8kdvd_rendering_) {
        // Allocate larger buffers for 8K rendering
        if (frame_width_ < kdvd_width_ || frame_height_ < kdvd_height_) {
            AllocateFrameBuffer(kdvd_width_, kdvd_height_);
            msg_Info(vlc_obj_, "8KDVD render buffers allocated: %dx%d", kdvd_width_, kdvd_height_);
        }
    } else {
        // Use standard buffer sizes
        if (frame_width_ > 1920 || frame_height_ > 1080) {
            AllocateFrameBuffer(1920, 1080);
            msg_Info(vlc_obj_, "Standard render buffers allocated: 1920x1080");
        }
    }
}
