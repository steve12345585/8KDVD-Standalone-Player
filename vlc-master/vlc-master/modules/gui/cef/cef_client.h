#ifndef VLC_CEF_CLIENT_H
#define VLC_CEF_CLIENT_H

#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_render_handler.h"
#include <vlc_common.h>

class VLCCefClient : public CefClient,
                     public CefDisplayHandler,
                     public CefLifeSpanHandler,
                     public CefLoadHandler,
                     public CefRenderHandler {
public:
    explicit VLCCefClient(vlc_object_t *obj);
    
    // CefClient methods
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
    
    // CefDisplayHandler methods
    void OnTitleChange(CefRefPtr<CefBrowser> browser,
                      const CefString& title) override;
    
    // CefLifeSpanHandler methods
    bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      const CefString& target_url,
                      const CefString& target_frame_name,
                      CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                      bool user_gesture,
                      const CefPopupFeatures& popupFeatures,
                      CefWindowInfo& windowInfo,
                      CefRefPtr<CefClient>& client,
                      CefBrowserSettings& settings,
                      CefRefPtr<CefDictionaryValue>& extra_info,
                      bool* no_javascript_access) override;
    
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    
    // CefLoadHandler methods
    void OnLoadStart(CefRefPtr<CefBrowser> browser,
                    CefRefPtr<CefFrame> frame,
                    TransitionType transition_type) override;
    
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                  CefRefPtr<CefFrame> frame,
                  int httpStatusCode) override;
    
    void OnLoadError(CefRefPtr<CefBrowser> browser,
                    CefRefPtr<CefFrame> frame,
                    ErrorCode errorCode,
                    const CefString& errorText,
                    const CefString& failedUrl) override;
    
    // CefRenderHandler methods (for offscreen rendering)
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    
    void OnPaint(CefRefPtr<CefBrowser> browser,
                PaintElementType type,
                const RectList& dirtyRects,
                const void* buffer,
                int width,
                int height) override;
    
    // Browser management
    CefRefPtr<CefBrowser> GetBrowser() const { return browser_; }
    void SetRenderSize(int width, int height);
    
    IMPLEMENT_REFCOUNTING(VLCCefClient);
    
private:
    vlc_object_t *vlc_obj_;
    CefRefPtr<CefBrowser> browser_;
    int render_width_;
    int render_height_;
    
    DISALLOW_COPY_AND_ASSIGN(VLCCefClient);
};

#endif // VLC_CEF_CLIENT_H
