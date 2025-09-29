#ifndef VLC_CEF_PROCESS_HANDLER_H
#define VLC_CEF_PROCESS_HANDLER_H

#include "include/cef_app.h"
#include "include/cef_browser_process_handler.h"
#include "include/cef_render_process_handler.h"
#include <vlc_common.h>

// VLC CEF Browser Process Handler
class VLCCefBrowserProcessHandler : public CefBrowserProcessHandler {
public:
    VLCCefBrowserProcessHandler();
    
    // CefBrowserProcessHandler methods
    void OnContextInitialized() override;
    void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override;
    void OnScheduleMessagePumpWork(int64 delay_ms) override;
    
    IMPLEMENT_REFCOUNTING(VLCCefBrowserProcessHandler);
    
private:
    DISALLOW_COPY_AND_ASSIGN(VLCCefBrowserProcessHandler);
};

// VLC CEF Render Process Handler
class VLCCefRenderProcessHandler : public CefRenderProcessHandler {
public:
    VLCCefRenderProcessHandler();
    
    // CefRenderProcessHandler methods
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override;
    
    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;
    
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefProcessId source_process,
                                 CefRefPtr<CefProcessMessage> message) override;
    
    IMPLEMENT_REFCOUNTING(VLCCefRenderProcessHandler);
    
private:
    DISALLOW_COPY_AND_ASSIGN(VLCCefRenderProcessHandler);
};

// VLC CEF App Implementation with Process Handlers
class VLCCefApp : public CefApp {
public:
    VLCCefApp();
    
    // CefApp methods
    void OnBeforeCommandLineProcessing(const CefString& process_type,
                                     CefRefPtr<CefCommandLine> command_line) override;
    
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return browser_process_handler_;
    }
    
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return render_process_handler_;
    }
    
    IMPLEMENT_REFCOUNTING(VLCCefApp);
    
private:
    CefRefPtr<VLCCefBrowserProcessHandler> browser_process_handler_;
    CefRefPtr<VLCCefRenderProcessHandler> render_process_handler_;
    
    DISALLOW_COPY_AND_ASSIGN(VLCCefApp);
};

#endif // VLC_CEF_PROCESS_HANDLER_H
