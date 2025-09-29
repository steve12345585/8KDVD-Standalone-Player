#include "cef_process_handler.h"
#include "include/cef_v8.h"
#include "include/wrapper/cef_helpers.h"
#include <vlc_common.h>

// VLCCefBrowserProcessHandler Implementation
VLCCefBrowserProcessHandler::VLCCefBrowserProcessHandler() {
}

void VLCCefBrowserProcessHandler::OnContextInitialized() {
    CEF_REQUIRE_UI_THREAD();
    // Browser context initialized - setup complete
}

void VLCCefBrowserProcessHandler::OnBeforeChildProcessLaunch(
    CefRefPtr<CefCommandLine> command_line) {
    // Configure child process command line
    command_line->AppendSwitch("--no-sandbox");
    command_line->AppendSwitch("--disable-web-security");
}

void VLCCefBrowserProcessHandler::OnScheduleMessagePumpWork(int64 delay_ms) {
    // Handle message pump work scheduling
}

// VLCCefRenderProcessHandler Implementation
VLCCefRenderProcessHandler::VLCCefRenderProcessHandler() {
}

void VLCCefRenderProcessHandler::OnContextCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
    
    CEF_REQUIRE_RENDERER_THREAD();
    
    // Create JavaScript API for 8KDVD menu interaction
    CefRefPtr<CefV8Value> global = context->GetGlobal();
    
    // Create VLC object in JavaScript
    CefRefPtr<CefV8Value> vlc_obj = CefV8Value::CreateObject(nullptr, nullptr);
    
    // Add 8KDVD specific functions
    CefRefPtr<CefV8Value> play_title = CefV8Value::CreateFunction("playTitle", nullptr);
    CefRefPtr<CefV8Value> show_menu = CefV8Value::CreateFunction("showMenu", nullptr);
    CefRefPtr<CefV8Value> navigate_menu = CefV8Value::CreateFunction("navigateMenu", nullptr);
    
    vlc_obj->SetValue("playTitle", play_title, V8_PROPERTY_ATTRIBUTE_NONE);
    vlc_obj->SetValue("showMenu", show_menu, V8_PROPERTY_ATTRIBUTE_NONE);
    vlc_obj->SetValue("navigateMenu", navigate_menu, V8_PROPERTY_ATTRIBUTE_NONE);
    
    global->SetValue("vlc", vlc_obj, V8_PROPERTY_ATTRIBUTE_NONE);
}

void VLCCefRenderProcessHandler::OnContextReleased(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
    
    CEF_REQUIRE_RENDERER_THREAD();
    // Context cleanup
}

bool VLCCefRenderProcessHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
    
    CEF_REQUIRE_RENDERER_THREAD();
    
    const CefString& message_name = message->GetName();
    
    if (message_name == "8kdvd_command") {
        // Handle 8KDVD specific commands
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        if (args->GetSize() > 0) {
            CefString command = args->GetString(0);
            
            if (command == "play_title") {
                // Handle title playback
                return true;
            } else if (command == "show_menu") {
                // Handle menu display
                return true;
            }
        }
    }
    
    return false;
}

// VLCCefApp Implementation
VLCCefApp::VLCCefApp() {
    browser_process_handler_ = new VLCCefBrowserProcessHandler();
    render_process_handler_ = new VLCCefRenderProcessHandler();
}

void VLCCefApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
    
    // Configure CEF for VLC integration
    command_line->AppendSwitch("--no-sandbox");
    command_line->AppendSwitch("--disable-web-security");
    command_line->AppendSwitch("--disable-features=TranslateUI");
    command_line->AppendSwitch("--disable-ipc-flooding-protection");
    
    // Enable hardware acceleration for 8K content
    command_line->AppendSwitch("--enable-gpu");
    command_line->AppendSwitch("--enable-gpu-rasterization");
    command_line->AppendSwitch("--enable-zero-copy");
}
