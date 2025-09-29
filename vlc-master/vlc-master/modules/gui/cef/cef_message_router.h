#ifndef VLC_CEF_MESSAGE_ROUTER_H
#define VLC_CEF_MESSAGE_ROUTER_H

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_process_message.h"
#include "include/cef_v8.h"
#include <vlc_common.h>
#include <string>
#include <map>
#include <functional>

// CEF Message Router for VLC Communication
class VLCCefMessageRouter {
public:
    typedef std::function<void(const std::string&, const std::string&)> MessageHandler;
    
    VLCCefMessageRouter(vlc_object_t *obj);
    ~VLCCefMessageRouter();
    
    // Message routing
    void RegisterHandler(const std::string& message_name, MessageHandler handler);
    void UnregisterHandler(const std::string& message_name);
    
    // Send messages to CEF
    bool SendMessageToCEF(CefRefPtr<CefBrowser> browser, 
                          const std::string& message_name,
                          const std::string& data);
    
    // Handle messages from CEF
    bool HandleCEFMessage(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefProcessMessage> message);
    
    // 8KDVD specific message handlers
    void Register8KDVDHandlers();
    
    // VLC integration messages
    void SendPlayTitle(int title_id);
    void SendShowMenu(const std::string& menu_type);
    void SendNavigateMenu(const std::string& direction);
    void SendVolumeChange(int volume);
    void SendSeekTo(int position);
    
private:
    vlc_object_t *vlc_obj_;
    std::map<std::string, MessageHandler> handlers_;
    
    // 8KDVD message handlers
    void HandlePlayTitle(const std::string& message_name, const std::string& data);
    void HandleShowMenu(const std::string& message_name, const std::string& data);
    void HandleNavigateMenu(const std::string& message_name, const std::string& data);
    void HandleVolumeChange(const std::string& message_name, const std::string& data);
    void HandleSeekTo(const std::string& message_name, const std::string& data);
};

#endif // VLC_CEF_MESSAGE_ROUTER_H
