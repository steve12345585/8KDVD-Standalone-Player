/**
 * VLC JavaScript API for 8KDVD Integration
 * Provides JavaScript interface to VLC functionality
 */

class VLCApi {
    constructor() {
        this.initialized = false;
        this.callbacks = new Map();
        this.messageQueue = [];
        this.init();
    }
    
    init() {
        console.log('VLC API initializing...');
        
        // Wait for CEF to be ready
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', () => this.setupAPI());
        } else {
            this.setupAPI();
        }
    }
    
    setupAPI() {
        // Set up message handling
        this.setupMessageHandling();
        
        // Expose API to global scope
        window.vlc = this;
        
        // Mark as initialized
        this.initialized = true;
        
        console.log('VLC API ready');
        this.processMessageQueue();
    }
    
    setupMessageHandling() {
        // Listen for messages from C++ side
        if (window.cef) {
            window.cef.onMessage = (message) => {
                this.handleMessage(message);
            };
        }
        
        // Fallback for direct function calls
        this.setupDirectCalls();
    }
    
    setupDirectCalls() {
        // These will be called directly from C++ side
        window.vlc_play_title = (titleId) => this.playTitle(titleId);
        window.vlc_show_menu = (menuId) => this.showMenu(menuId);
        window.vlc_go_back = () => this.goBack();
        window.vlc_update_setting = (setting, value) => this.updateSetting(setting, value);
    }
    
    handleMessage(message) {
        try {
            const data = JSON.parse(message);
            this.processMessage(data);
        } catch (e) {
            console.error('Failed to parse VLC message:', e);
        }
    }
    
    processMessage(data) {
        const { type, payload } = data;
        
        switch (type) {
            case 'play_title':
                this.playTitle(payload.titleId);
                break;
            case 'show_menu':
                this.showMenu(payload.menuId);
                break;
            case 'update_setting':
                this.updateSetting(payload.setting, payload.value);
                break;
            case 'navigate':
                this.navigate(payload.direction);
                break;
            case 'select':
                this.select();
                break;
            case 'go_back':
                this.goBack();
                break;
            default:
                console.warn('Unknown VLC message type:', type);
        }
    }
    
    processMessageQueue() {
        // Process any queued messages
        while (this.messageQueue.length > 0) {
            const message = this.messageQueue.shift();
            this.processMessage(message);
        }
    }
    
    // 8KDVD Playback Functions
    playTitle(titleId) {
        console.log('VLC API: playTitle(' + titleId + ')');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_play_title', { titleId: titleId });
        
        // Trigger callback
        this.triggerCallback('playTitle', titleId);
    }
    
    showMenu(menuId) {
        console.log('VLC API: showMenu(' + menuId + ')');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_show_menu', { menuId: menuId });
        
        // Trigger callback
        this.triggerCallback('showMenu', menuId);
    }
    
    goBack() {
        console.log('VLC API: goBack()');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_navigate_menu', { direction: 'back' });
        
        // Trigger callback
        this.triggerCallback('goBack');
    }
    
    // Navigation Functions
    navigate(direction) {
        console.log('VLC API: navigate(' + direction + ')');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_navigate_menu', { direction: direction });
        
        // Trigger callback
        this.triggerCallback('navigate', direction);
    }
    
    select() {
        console.log('VLC API: select()');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_select', {});
        
        // Trigger callback
        this.triggerCallback('select');
    }
    
    // Settings Functions
    updateSetting(setting, value) {
        console.log('VLC API: updateSetting(' + setting + ', ' + value + ')');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_setting_update', { setting: setting, value: value });
        
        // Trigger callback
        this.triggerCallback('updateSetting', { setting, value });
    }
    
    // Volume Control
    setVolume(volume) {
        console.log('VLC API: setVolume(' + volume + ')');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_volume_change', { volume: volume });
        
        // Trigger callback
        this.triggerCallback('setVolume', volume);
    }
    
    // Seek Control
    seekTo(position) {
        console.log('VLC API: seekTo(' + position + ')');
        
        // Send message to C++ side
        this.sendMessage('8kdvd_seek_to', { position: position });
        
        // Trigger callback
        this.triggerCallback('seekTo', position);
    }
    
    // Callback System
    on(event, callback) {
        if (!this.callbacks.has(event)) {
            this.callbacks.set(event, []);
        }
        this.callbacks.get(event).push(callback);
    }
    
    off(event, callback) {
        if (this.callbacks.has(event)) {
            const callbacks = this.callbacks.get(event);
            const index = callbacks.indexOf(callback);
            if (index > -1) {
                callbacks.splice(index, 1);
            }
        }
    }
    
    triggerCallback(event, data) {
        if (this.callbacks.has(event)) {
            this.callbacks.get(event).forEach(callback => {
                try {
                    callback(data);
                } catch (e) {
                    console.error('Callback error:', e);
                }
            });
        }
    }
    
    // Message Sending
    sendMessage(type, payload) {
        const message = {
            type: type,
            payload: payload,
            timestamp: Date.now()
        };
        
        // Try to send via CEF message system
        if (window.cef && window.cef.sendMessage) {
            window.cef.sendMessage(JSON.stringify(message));
        } else {
            // Queue message for later
            this.messageQueue.push(message);
        }
    }
    
    // Utility Functions
    isReady() {
        return this.initialized;
    }
    
    getVersion() {
        return '1.0.0';
    }
    
    getCapabilities() {
        return {
            playTitle: true,
            showMenu: true,
            navigate: true,
            select: true,
            goBack: true,
            updateSetting: true,
            setVolume: true,
            seekTo: true
        };
    }
}

// Auto-initialize
const vlcApi = new VLCApi();

// Export for module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = VLCApi;
}
