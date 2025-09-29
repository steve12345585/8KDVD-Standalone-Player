#ifndef VLC_CEF_RENDER_HANDLER_H
#define VLC_CEF_RENDER_HANDLER_H

#include "include/cef_render_handler.h"
#include "include/cef_browser.h"
#include <vlc_common.h>
#include <vlc_vout.h>
#include <vlc_picture.h>

// CEF Render Handler for VLC Integration
class VLCCefRenderHandler : public CefRenderHandler {
public:
    VLCCefRenderHandler(vlc_object_t *obj);
    ~VLCCefRenderHandler();
    
    // CefRenderHandler methods
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    
    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override;
    
    // VLC integration
    void SetVout(vout_thread_t *vout);
    void SetRenderSize(int width, int height);
    void SetRenderPosition(int x, int y);
    
    // 8KDVD specific rendering
    void Enable8KDVDRendering(bool enable);
    void Set8KDVDResolution(int width, int height);
    
    // Frame buffer management
    void* GetFrameBuffer() const { return frame_buffer_; }
    int GetFrameWidth() const { return frame_width_; }
    int GetFrameHeight() const { return frame_height_; }
    
    // VLC video output integration
    bool RenderToVLC();
    void UpdateVLCVideoOutput();
    
private:
    vlc_object_t *vlc_obj_;
    vout_thread_t *vout_;
    
    // Frame buffer
    void* frame_buffer_;
    int frame_width_;
    int frame_height_;
    int frame_x_;
    int frame_y_;
    
    // 8KDVD settings
    bool enable_8kdvd_rendering_;
    int kdvd_width_;
    int kdvd_height_;
    
    // VLC integration
    picture_t* vlc_picture_;
    bool needs_vlc_update_;
    
    // Helper methods
    void AllocateFrameBuffer(int width, int height);
    void FreeFrameBuffer();
    void ConvertFrameToVLC();
    void UpdateVLCVideoFormat();
    
    IMPLEMENT_REFCOUNTING(VLCCefRenderHandler);
    DISALLOW_COPY_AND_ASSIGN(VLCCefRenderHandler);
};

#endif // VLC_CEF_RENDER_HANDLER_H
