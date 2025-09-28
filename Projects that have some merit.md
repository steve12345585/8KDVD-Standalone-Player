Cross-Platform & OS-Specific Candidates
Rank	Project (Git Repo)	Platforms	Closeness to Spec	Key Strengths	Notable Gaps
1	VLC Media Player<br/>https://code.videolan.org/videolan/vlc
	Windows / Linux / macOS	~85%	Full DVD menus (libdvdnav); Blu-ray playback with partial menu support (BD-J via libbluray-bdj + Java); plays VCD/SVCD; supports VP9 + Opus and up to 8K video; highly extensible.	No 8KDVD HTML/XML menu support; no HD-DVD menus (plays EVO only); no AACS2.0/BD+ (external keys/decryptors needed).
2	mpv / libmpv<br/>https://github.com/mpv-player/mpv
	Windows / Linux / macOS	~70%	Lightweight FFmpeg-based core; supports VCD, DVD, Blu-ray, AVCHD, HD-DVD EVO (decrypted); excellent 8K decode support; embeddable via libmpv; scriptable.	No menu rendering at all (DVD/BD/HD-DVD = title-only playback); no HTML/XML menus; no DRM.
3	MPC-BE (Black Edition)<br/>https://sourceforge.net/projects/mpcbe/
	Windows-only	~60%	Modern Windows UI; full DVD menus; wide codec coverage (HEVC, VP9, AV1, etc.); lightweight and fast.	No Blu-ray or HD-DVD menus (titles only); no HTML/XML menus; no DRM; Windows-only.
4	MPC-HC (clsid fork)<br/>https://github.com/clsid2/mpc-hc
	Windows-only	~55%	Classic Windows player; full DVD menus; active codec updates (AV1, H.266); low resource footprint.	Same as MPC-BE: no Blu-ray/HD-DVD menus, no HTML/XML menus, no DRM.
5	xine-lib + Kaffeine<br/>http://git.code.sf.net/p/xine/xine-lib
	Linux-only	~60%	Full DVD menus; VCD/SVCD; some unencrypted Blu-ray support; modular frontend/backend; maintained.	No BD-J/HD-DVD menus; no HTML/XML menus; no UHD DRM.
6	IINA (mpv-based)<br/>https://github.com/iina/iina
	macOS-only	~70%	Native macOS GUI; inherits mpv’s broad codec support; polished user experience; scripting/plugin system.	Same as mpv: no DVD/BD/HD-DVD menus, no HTML/XML menus, no DRM.
7	GNOME Videos (Totem)<br/>https://gitlab.gnome.org/GNOME/totem
	Linux-only	~50%	Built on GStreamer; supports DVD menus and VCD; simple desktop integration.	Limited format coverage; no Blu-ray/HD-DVD menus; no HTML/XML menus; no DRM.
Summary

VLC is the strongest foundation across all platforms — already covers legacy disc menus and modern codecs, with partial BD-J support.

mpv/libmpv is the next best engine candidate, but requires an external GUI layer for menus.

MPC-BE/H C are good for Windows-only but intentionally lack Blu-ray/HD-DVD interactivity.

xine-lib provides Linux DVD menus, while IINA is a modern macOS front-end to mpv.

Totem ranks lowest, suitable only for basic DVD/VCD use.

Cross-Platform candidates

VLC → Best balance, covers DVD (menus), partial Blu-ray menus, VP9/Opus/8K playback, but missing HTML/XML menus and AACS2.

mpv / libmpv → Strong engine, flexible embedding, but no menu UI at all (DVD, BD, HD-DVD all title-only playback). Needs a front-end menu layer built around it.

Windows-only

MPC-BE / MPC-HC → Full DVD menus, good codec support (via LAV), but no Blu-ray/HD-DVD menus, no HTML/XML engine.

Linux-only

xine-lib / Kaffeine → Full DVD menus, some Blu-ray navigation (no BD-J), no HD-DVD menus, no web menus.

Totem (GStreamer) → Basic DVD menus, nothing beyond.

macOS-only

IINA (mpv front-end) → Modern macOS GUI, inherits mpv’s limitations (no DVD/BD menus).

That leaves VLC as the strongest cross-platform base, because:

It already handles multiple disc formats and legacy menu systems (DVD, partial BD).

It can be extended with libbluray-bdj for BD-J menus.

It has 8K codec readiness (VP9/Opus).

The missing pieces (HTML/XML menus, HD-DVD interactivity, AACS2.0) could be layered in.

Do you want me to produce a clean re-ranked table (without Kodi) showing only projects that can realistically serve as bases, with VLC at the top?

Open-Source Foundations for an 8KDVD-Capable Hybrid Disc Player
Overview and Selection Criteria

To build a cross-platform 8K DVD Player supporting VCD, SVCD, CD-i, DVD-Video, HD-DVD, Blu-ray, AVCHD, UHD Blu-ray, and the new 8KDVD format, we evaluated open-source media player projects. Each candidate was assessed for:

Supported Disc Formats & Menus: Which legacy and HD disc formats are playable, and whether interactive menu navigation (e.g. DVD menus, Blu-ray BD-J, etc.) is implemented (full, partial, or not at all).

Missing Features (Relative to 8KDVD Spec): Gaps such as unsupported codecs (e.g. VP9/Opus), lack of HTML/XML/JavaScript menu rendering, incomplete AACS 2.0 DRM handling, etc., required to meet the 8KDVD specification.

Advanced “Nice-to-Haves”: Extra capabilities not explicitly in the spec but beneficial, like streaming, plugin APIs, hardware acceleration, or library management.

Platform Support & Build Tools: Cross-platform availability (Windows, Linux, macOS) or OS-specific focus, and whether the project builds with free toolchains (GCC/Clang/VS Community). All listed projects are active (updated within ~36 months) and use permissive frameworks (e.g. Qt, GTK, Electron) as needed for GUI.

Closeness to Spec (%): An approximate ranking of how close each project’s current capabilities align with a “complete” 8KDVD implementation (100% would mean supporting all listed formats, codecs, menus, and DRM). This considers the breadth of format support and depth of menu/interactive features implemented.

Below we present viable open-source player projects for Windows, Linux, and macOS, with each project’s Git repository, supported formats/menus, notable missing features, and advanced extras. Projects are listed in order of descending spec compliance for each platform.

Windows Open-Source Player Projects (5 Selected)

Kodi (XBMC) – Closeness ~90%, Cross-Platform (Win/Linux/macOS)
Git: https://github.com/xbmc/xbmc

Supported Formats & Menus: Plays DVD-Video (with full menus), Audio CD, and Video CD (VCD/SVCD) out of the box
kodi.wiki
. Blu-ray playback (unencrypted) is supported via libbluray; Kodi v19+ even added Blu-ray Java (BD-J) menu support (requires an external JRE)
discourse.osmc.tv
. HD-DVD and AVCHD can be played if decrypted (reads EVO/M2TS containers). UHD Blu-ray content is playable if bypassing AACS2.0 encryption (Kodi can leverage external tools like MakeMKV for on-the-fly decryption
forum.kodi.tv
). 8K video playback (VP9/HEVC/AV1) is supported through FFmpeg decoders, though extremely high-bitrate 8K might tax hardware. Menu systems for Blu-ray are partial – HDMV menus work, and BD-J menus work with Java installed (still in beta, with some input quirks)
discourse.osmc.tv
discourse.osmc.tv
.

Missing/Incompatible: 8KDVD-specific menu system (HTML5/XML + JS) – no built-in web rendering engine for disc menus (Kodi’s GUI is skin-based, not a browser). AACS 2.0 DRM for UHD/8KDVD is unsupported (no open implementation for on-disc decryption), so encrypted UHD/8K discs require external decrypters. Philips CD-i interactive titles are not natively supported (Kodi cannot run CD-i software; only any Red Book audio or MPEG AV tracks would play). Multi-angle and advanced HD-DVD interactivity (HDi) are not implemented.

Advanced Features: Extensible Add-on System – Kodi’s Python plugins allow integrating new formats via add-ons. (The 8KDVD spec itself envisions a Kodi plugin for full support.) Kodi provides a rich 10-foot UI, media library, streaming services, PVR (TV tuner) support, and is highly customizable via skins
kodi.wiki
kodi.wiki
. It has hardware acceleration on Windows (DXVA2/DXVA-HD) and can output up to 8K with the right GPU. Kodi’s cross-platform core makes it a strong foundation to implement unified 8KDVD functionality across all OS.

VLC media player – Closeness ~85%, Cross-Platform (Win/Linux/macOS)
Git: https://code.videolan.org/videolan/vlc (mirror: https://github.com/videolan/vlc)

Supported Formats & Menus: VLC plays virtually all optical media: VCD/SVCD, DVD-Video (with full DVD menu navigation), Audio CD, unencrypted Blu-ray, and AVCHD. It uses libdvdcss/libdvdnav for DVD (allowing region/CSS bypass) and libbluray for Blu-ray. By default, Blu-ray menus are not fully supported – VLC will skip BD-J interactive menus unless a Java runtime is configured
anymp4.com
. However, with libbluray-bdj and a JRE, VLC can render Blu-ray Java menus (the feature is present but considered experimental
reddit.com
). HD-DVD support is not official, but VLC can play decrypted .EVO files (using its MPEG-2/VC-1 decoders
forum.kodi.tv
forum.kodi.tv
). UHD Blu-ray playback is possible for ripped or key-provided discs (VLC supports HEVC and HDR video decoding, but does not circumvent AACS2.0 encryption). VLC’s video engine supports up to 8K resolution with hardware acceleration
images.videolan.org
, and it can decode VP9 and Opus (required for 8KDVD) using ffmpeg libraries. Basic menu rendering for 8KDVD (which uses HTML5/XML) is not built-in – VLC has no HTML browser engine.

Missing/Incompatible: AACS 2.0 and BD+ – no native support for UHD Blu-ray encryption (users must supply keys or use external decrypting libraries). HTML/JS Menus – no support for 8KDVD’s HTML5-based menu system; integrating a web engine would be a significant addition. Philips CD-i – not supported (VLC cannot execute CD-i interactive programs; it might only read MPEG AV tracks if present). HDi (HD-DVD menus) – unsupported. Also, interactive streaming features like HLS playlists are limited (VLC can parse .m3u8 for streaming, but adaptive multi-quality selection on disc is not a typical use-case in VLC currently).

Advanced Features: Highly Portable Core (libVLC) – one can embed VLC’s core in custom applications. It’s known for extensive codec support, including legacy ones and cutting-edge formats (AV1, etc.). VLC offers streaming/transcoding, a robust plugin architecture (for video filters, outputs, etc.), and even VR/360° video support. It has a simple UI but covers essentials like subtitles, audio track selection, and basic media library features. Its proven disc handling (especially DVD menus)
github.com
 and cross-platform consistency make it a solid base to extend (e.g. one could integrate an HTML menu component alongside VLC for 8KDVD).

MPV – Closeness ~70%, Cross-Platform (Win/Linux/macOS)
Git: https://github.com/mpv-player/mpv

Supported Formats: MPV is a modern player forked from MPlayer, using FFmpeg for broad codec support. It plays DVD-Video and Blu-ray content, but without full menu navigation. MPV can directly open a DVD’s VIDEO_TS or a Blu-ray disc/folder, and will play the main title. It links against libdvdread/libdvdnav and libbluray for basic navigation, but DVD menus are not yet implemented in the UI (a long-requested feature)
github.com
. Blu-ray Java menus are also not supported – MPV will simply choose a title to play. VCD/SVCD playback is supported (it can read MPEG DAT tracks via libcdio). HD-DVD: MPV can play decrypted EVO files (MPlayer/FFmpeg added support for HD-DVD containers
forum.kodi.tv
), though it offers no interactivity. UHD Blu-ray: if given an unencrypted 4K/UHD stream or provided with keys via libaacs, MPV can decode it (including HDR10, Dolby Vision to some extent) since it supports HEVC Main10 and VP9 profiles. MPV is capable of 8K video decoding (e.g. 8K VP9) with a powerful GPU/CPU, though rendering 8K may be near hardware limits.

Missing/Incompatible: Interactive Menus: No on-screen menu rendering for DVD/Blu-ray/8KDVD – navigation must be done by manually selecting titles/chapters. HTML/CSS/JS engine: none (MPV’s focus is playback, not web technology). DRM/Copy Protection: No built-in support for AACS or BD+ (users can install libaacs and provide keys for Blu-ray; AACS2 for UHD is unsupported). CD-i not supported. Essentially, MPV would require significant additions for the menu system and DRM aspects of the 8KDVD spec.

Advanced Features: High Quality Video – MPV provides advanced video scaling, interpolation, HDR tone-mapping, and is scriptable (Lua, JavaScript) for custom behaviors. It has a minimal UI by design, making it suitable as a backend engine (its libmpv can be integrated into a new GUI that could handle disc menus separately). Several front-ends (mpv.net on Windows, IINA on Mac, etc.) use MPV under the hood, demonstrating its flexibility. MPV’s lightweight, hardware-accelerated core (D3D11, Vulkan, NVDEC, etc. on Windows) could handle the 8K decoding and rendering requirements of 8KDVD, if paired with a new interface for menus.

Media Player Classic – BE (Black Edition) – Closeness ~60%, Windows-Only
Git: https://github.com/clsid2/mpc-hc (base fork shared with BE) and https://sourceforge.net/projects/mpcbe/

Supported Formats & Menus: MPC-BE is a popular Windows-only player based on the classic MPC-HC. It leverages DirectShow filters (and bundles LAV Filters) to support virtually all video/audio codecs. DVD-Video playback with menus is fully supported via DirectShow’s DVD navigator (MPC’s menu controls allow DVD menu interaction similar to a hardware player). VCD/SVCD can be played by opening the DAT or MPEG tracks. Blu-ray: MPC-BE can open unencrypted Blu-ray streams (e.g. if using MakeMKV’s libmmbd to emulate libaacs
reddit.com
, it will play the main title). However, Blu-ray menus are not supported – the developers have intentionally avoided BD-J menu support
reddit.com
. The player offers a simple “Open Disc” for Blu-ray which jumps to the largest title. HD-DVD: if decrypted files are provided, MPC could play the video, but there is no support for HDi menus. UHD Blu-ray: similar story – with external decryption (keys or AnyDVD HD) it can play the video content, but no UHD menu or AACS2 support.

Missing/Incompatible: Blu-ray and Advanced Menus: No BD-J or UHD menu support (and it’s not planned by MPC developers due to complexity/legal reasons
reddit.com
). 8KDVD menu (HTML/XML) and streaming features are not present and would require building a new menu system on top. 8K codec support: Through LAV Filters, MPC-BE does support modern codecs like HEVC, VP9, even VVC/H.266 if external decoders are present
github.com
. However, handling 8K at full framerate might be limited by the DirectShow pipeline overhead. Copy protection: out-of-the-box, MPC-BE does not decrypt CSS/AACS (it relies on external filters or decrypted content). Region coding and AACS2 are not handled internally. CD-i is unsupported.

Advanced Features: Refined Windows UI: MPC-BE offers a rich Windows GUI (skinnable, with playlist, subtitle downloader, etc.) and custom video renderers for high quality (e.g. MPC Video Renderer with DirectX 11 for HDR output). It also has extensive settings for tweaking playback, shaders, and supports Pan&Scan, 3D anaglyph output, etc. As a fork of MPC, it is lightweight and can be compiled with Visual Studio Community. MPC-BE could serve as a good Windows-front end if supplemented with an external menu framework (for example, invoking a browser for 8KDVD HTML menus) and using LAV Filters/FFmpeg for the new codecs.

Media Player Classic – HC (Home Cinema, clsid2 fork) – Closeness ~55%, Windows-Only
Git: https://github.com/clsid2/mpc-hc (active fork of discontinued MPC-HC)

Supported Formats & Menus: MPC-HC’s updated fork continues to incorporate new codecs (recently adding HEVC, AV1, VP9 via LAV Filters updates
github.com
). DVD playback with full menus is supported (similar to MPC-BE). Blu-ray playback is limited to non-encrypted content; it can open a Blu-ray disc and play titles if libaacs and keys are installed
reddit.com
, but without interactive menus (no BD-J support, identical to MPC-BE’s stance). Other formats: VCD, SVCD are playable, and AVCHD (Blu-ray structures on DVD media) can be opened if decrypted. UHD Blu-ray and 8K content can be played if decoded by updated filters (MPC-HC can utilize 64-bit LAV Filters which support 4K/8K video decoding and even hardware acceleration on modern GPUs).

Missing/Incompatible: Blu-ray/8KDVD Menus: like MPC-BE, it lacks any Blu-ray Java or web-based menu engine. HD-DVD/HDi not supported. AACS2.0 and advanced DRM: not supported (users must rely on external decryptors for UHD). 8KDVD-specific features (HTML5/CSS menus, HLS adaptive streaming) are not present. CD-i not supported. Essentially, MPC-HC is focused on file playback and legacy DVD, so it would need major development to meet the interactive features of the 8KDVD spec.

Advanced Features: Integration with DirectShow – Since MPC-HC is DirectShow-based, it can leverage any available DirectShow filters. This means any new codec or format support can be added if a filter exists (for example, one could integrate an HTML engine as a filter or overlay, though that would be an ambitious project). MPC-HC has a very low resource footprint and a classic interface. It includes niceties like subtitle search, bookmarking, and extensive command-line options. Like MPC-BE, it’s a solid Windows player core that could be extended, but it ranks slightly below BE in “niceness” (no dark theme by default, etc.), hence a slightly lower percentile.

(Note: Other Windows open-source projects considered include SMPlayer (a GUI front-end for MPlayer/MPV) and bomi (MPV-based GUI). SMPlayer is cross-platform and active, but since it relies on MPV/MPlayer backends, it doesn’t independently add format support beyond those cores. Bomi has DVD/Blu-ray library support but is unmaintained. The listed projects above are the most viable and up-to-date.)

Linux Open-Source Player Projects (4 Selected)

Kodi (XBMC) – Closeness ~90%, Cross-Platform (Linux, Windows, macOS)
Git: https://github.com/xbmc/xbmc

Linux Notes: Kodi’s feature set on Linux is nearly identical to its Windows version (see above). It supports Blu-ray (no DRM), DVD (menus), VCD, etc.
kodi.wiki
. On Linux, Kodi can call external libraries for decryption (e.g. it can utilize libaacs or MakeMKV for Blu-ray playback
forum.kodi.tv
). Kodi’s plugin approach is especially relevant: a Python add-on could implement 8KDVD menu parsing (XML/HTML) and rendering using Kodi’s GUI controls – effectively allowing custom interactive menus within Kodi’s interface. This cross-platform flexibility makes Kodi one of the top foundations on Linux for an 8KDVD player.

(Missing features and advanced features are as noted in the Windows section – Kodi’s strengths (add-ons, 4K/8K video, etc.) and gaps (no HTML engine, no AACS2) apply equally on Linux.)

VLC media player – Closeness ~85%, Cross-Platform (Linux, Windows, macOS)
Git: https://code.videolan.org/videolan/vlc

Linux Notes: VLC on Linux supports the same range of discs: DVDs with menus, VCD/SVCD, Audio CD, and Blu-ray (unencrypted). Blu-ray Java menus require installing the libbluray-bdj package and a Java runtime
forum.manjaro.org
. Many Linux users rely on VLC’s library set (libdvdcss, libaacs, libbdplus) to play commercial DVDs/Blu-rays by providing the needed keys. For 8K content, VLC’s Linux version also supports hardware decode (VAAPI, VDPAU) and can output up to 8K (assuming X11 or Wayland setups that handle it).

(Missing features: same as Windows – no HTML menu, no AACS2; Advanced: same core streaming and codec features. VLC’s modular architecture would allow adding new plugins for, say, an 8KDVD menu interpreter, but this would be a complex endeavor.)

MPV – Closeness ~70%, Cross-Platform (Linux, Windows, macOS)
Git: https://github.com/mpv-player/mpv

Linux Notes: On Linux, MPV uses FFmpeg and can utilize VAAPI/Vulkan for acceleration. It similarly lacks DVD/Blu-ray menu UI support
github.com
. However, Linux users often script MPV (using Lua or send commands) which means an external script could potentially provide rudimentary menu navigation (e.g. choosing titles via overlays), though not comparable to full DVD menu visuals. MPV’s lightweight nature on Linux (no heavy GUI toolkit needed) makes it suitable for integration into a custom application that could embed a web view for HTML menus alongside the MPV video output.

(Missing and advanced features are as noted prior. Notably, MPV on Linux can leverage libbluray – it will play Blu-ray titles if the disc is mounted and keys are available, but as mentioned, BD-J menus are not shown. This is an area to be developed for full spec compliance.)

Xine / Xine UI – Closeness ~60%, Linux-Focused
Git: http://git.code.sf.net/p/xine/xine-lib (library) / Frontend: https://github.com/gavran/xine-ui (or use Kaffeine)

Supported Formats & Menus: Xine is an established multimedia engine on Linux. It can play DVDs with full menus, VCDs and SVCDs
man.archlinux.org
, and standard media files. Historically, Xine could play unencrypted Blu-ray and HD-DVD streams when paired with xine-lib 1.2+ and libbluray
phoronix.com
, although BD-J support is rudimentary or nonexistent. The xine-lib 1.2 series (recent as of late 2022) improved Blu-ray support, but encrypted Blu-ray/UHD still require external help (no built-in AACS). Xine’s UI (or front-ends like Kaffeine on KDE) presents DVD menus and basic interactive navigation. AVCHD discs can be handled similarly to Blu-rays. 8K video: xine-lib can utilize FFmpeg, so in theory it can decode 8K/VP9/HEVC if codecs are present, but the X11 output might struggle with such high resolution unless updated for hardware acceleration (VDPAU/VAAPI are supported for 4K, unsure for 8K).

Missing/Incompatible: Blu-ray Java menus and UHD Blu-ray DRM – not supported (xine will play main title of Blu-ray but no BD-J). 8KDVD menus (HTML/CSS) – no support; xine has no web engine. Adaptive streaming (HLS) – xine can’t natively handle HLS playlists or dynamic streams as required for 8KDVD’s multi-quality feature. CD-i – not supported. Xine is primarily a playback engine; adding the new interactive features would require significant extension or a custom front-end.

Advanced Features: Engine/Frontend separation – xine-lib is a backend that can be used by different frontends (Kaffeine, Totem had a xine mode in the past, etc.). This modularity could allow replacing or augmenting the UI portion to implement new menu systems. Xine has good DVB support (digital TV) in Kaffeine, showing its extensibility. It’s less active than VLC or Kodi, but still maintained and uses free toolchains. A developer looking to implement a custom player could use xine-lib for decoding and build a new GUI for 8KDVD menus on top of it.

GNOME Videos (Totem) – Closeness ~50%, Linux (GNOME Desktop)
Git: https://gitlab.gnome.org/GNOME/totem

Supported Formats: Totem is the default GNOME media player, built on GStreamer. It supports DVD playback with menus (via the GStreamer DVD plugin) and VCD playback
manpages.ubuntu.com
. Blu-ray support in Totem is limited – it can play unencrypted Blu-ray if GStreamer’s bad/ugly plugins are installed and configured (Blu-ray navigation is available through libbluray integration, but BD-J menus would not function by default). Totem focuses on file playback and simple streaming; it doesn’t officially list Blu-ray or HD-DVD as supported due to the encryption issues.

Missing/Incompatible: Blu-ray menus, UHD, 8KDVD features – not present. Totem/GStreamer currently have no support for BD-J or HTML interactive content. Codec support is broad with GStreamer plugins, but VP9/Opus for 8K might require installing the latest gstreamer-libav or bad set. AACS2 not supported. CD-i not supported. Essentially, Totem itself would need major enhancements for our purposes; however, it’s mentioned here as an example of a GStreamer-based approach on Linux.

Advanced Features: Integration with GNOME – Totem provides a polished, simple UI integrated with the desktop (MPRIS controls, etc.). GStreamer Framework – the real power is in GStreamer: it’s a pipeline that has elements for almost everything (decoding, rendering). A custom player could be built using GStreamer pipelines to handle 8K video and even could integrate web engines (there was an old “GstClutter” for combining video with UI). GStreamer ensures cross-platform codec handling, so one could reuse its decoding in a new 8KDVD player while writing a new menu system in a toolkit of choice. Totem itself, though, is only a basic example and ranks lower in spec completeness.

macOS Open-Source Player Projects (3 Selected)

VLC media player – Closeness ~85%, Cross-Platform (macOS, Win, Linux)
Git: https://code.videolan.org/videolan/vlc

macOS Notes: VLC is fully supported on macOS and plays DVDs (with menus), Video CD, etc., similar to other platforms. On macOS, Blu-ray support is also present (again requiring an external Java for BD-J menus and manual key installation for AACS). The macOS version has the same core capabilities (8K decode, wide format support) and uses native macOS frameworks for output. It does not support the Mac-specific Blu-ray Disc Player DRM (since that’s proprietary); all Blu-ray/UHD handling is via the same open libs. As a Cocoa app, VLC can be extended with modules, but adding an HTML/CSS engine for 8KDVD menus would likely mean embedding a WebView or using Chromium Embedded (which is outside the current scope of VLC).

Missing features: as per other platforms (no built-in BD-J without Java, no HTML menu, no AACS2). Advanced: cross-platform consistency, 8K hardware decode on Mac (using VideoToolbox) etc.

Kodi (XBMC) – Closeness ~90%, Cross-Platform (macOS, Win, Linux)
Git: https://github.com/xbmc/xbmc

macOS Notes: Kodi runs on macOS (using GPU decode via VideoToolbox/Metal for 4K/8K). It brings the same extensive disc support on Mac: DVD with menus, unencrypted Blu-ray, etc.
kodi.wiki
. On macOS, Java-based BD-J menus in Kodi v19+ should work if a JDK is installed (similar caveats as on Linux). Mac users can also leverage Kodi’s add-on system to integrate with external tools (e.g. calling MakeMKV for on-the-fly Blu-ray decryption). The limitations (no native web engine for HTML menus, no AACS2) remain.

Kodi’s UI is uniform across platforms, so an 8KDVD menu could be rendered via Kodi’s skin/interface on macOS just as on Windows/Linux. This one-codebase approach is a major advantage for a unified player.

IINA – Closeness ~70%, macOS-Specific (built on mpv)
Git: https://github.com/iina/iina

Supported Formats: IINA is a modern macOS media player that wraps the MPV player engine
iina.io
. Therefore, it inherits MPV’s playback capabilities: virtually any file format, DVD and Blu-ray content (no menus), up to 4K/8K video decoding (depending on hardware). IINA’s claim is that if MPV/FFmpeg can play it, IINA can play it – which means VCD, SVCD are playable (via FFmpeg’s MPEG-1 support), DVDs can be played (IINA will play the main title, but lacks DVD menu navigation UI), and Blu-ray streams can play if decrypted. There is no built-in support for Blu-ray disc menus or decryption – advanced disc use is not IINA’s focus. AVCHD discs would be treated like a folder of files.

Missing/Incompatible: Menus & Interactivity – IINA/MPV does not show DVD or Blu-ray menus
github.com
, and it has no facility for BD-J or HTML menus. DRM – no AACS or BD+ handling (same limitations as MPV). 8KDVD specifics – no HLS adaptive streaming or HTML parsing. Essentially, IINA would require embedding a browser for menus or coordinating with an external menu renderer.

Advanced Features: Native macOS UX: IINA is written in Swift and uses a design philosophy aligning with macOS (dark mode, gesture support, Touch Bar, picture-in-picture, etc.). It provides a plugin system that allows extending functionality with JavaScript scripts
iina.io
 – potentially one could script some menu logic, though rendering a full interactive menu would be challenging. IINA demonstrates how an open-source UI can sit on top of MPV – similarly, a macOS-specific 8KDVD player could use MPV (or FFmpeg/GStreamer) for decoding and a WebView for menu UI. IINA’s approach yields a very polished user experience on Mac, at the cost of relying on MPV’s limited disc navigation. It’s a good starting point for Mac because it already integrates mpv’s decoding prowess with a modern GUI, so adding specific 8KDVD features (if done on MPV or via external modules) could be integrated into IINA’s interface.

Comparative Ranking and Coverage

The following table summarizes the above projects and their relative completeness toward the full 8KDVD specification:

Rank	Project (GitHub Repo)	Platforms	Closeness to Spec	Key Strengths	Notable Gaps
1	Kodi (XBMC)<br/>
kodi.wiki
discourse.osmc.tv
	Win / Lin / macOS	~90% (Highest)	Broad disc format support (DVD, VCD, Blu-ray), menu handling for DVD and partial BD-J
discourse.osmc.tv
, 8K/VP9 codec ready, extensible add-ons, cross-platform single codebase.	No built-in HTML/JS engine for 8KDVD menus, no AACS2.0 (UHD) support, CD-i not supported.
2	VLC Media Player<br/>
anymp4.com
images.videolan.org
	Win / Lin / macOS	~85%	Extremely wide format/codec support (incl. 8K/HDR
images.videolan.org
), full DVD menus, partial Blu-ray menu via Java, libbluray integration
images.videolan.org
, light footprint, streaming capabilities.	No default Blu-ray Java menus (requires setup)
anymp4.com
, no HTML/CSS menu support, no AACS2 for UHD, limited HD-DVD support.
3	mpv (libmpv engine)<br/>
github.com
	Win / Lin / macOS	~70%	Powerful FFmpeg-based core (8K decode, new codecs), scriptable and embeddable, minimal UI good for integration, cross-platform.	No interactive menu UI (DVD/BD menus not navigable)
github.com
, no built-in DRM support, needs external UI for 8KDVD menus.
4	MPC-BE (Media Player Classic BE)<br/>
reddit.com
	Windows-only	~60%	Polished Windows GUI, full DVD menu support, utilizes modern codecs (via LAV Filters) including 4K, some UHD ability, open C++ code (VS Community).	No Blu-ray or HD-DVD menu support (intentionally)
reddit.com
, Windows-only, lacks 8KDVD’s web-based menu system, no built-in decryption for Blu-ray/UHD.
5	MPC-HC (clsid fork)<br/>
github.com
	Windows-only	~55%	Lightweight, open source classic player, good DVD support, updated with new codec support (AV1, H.266)
github.com
. Integrates with DirectShow filters for extendibility.	Same Blu-ray/menu limitations as MPC-BE, fewer modern UI features, no advanced interactive support, Windows-only.
6	Xine / Xine-lib<br/>
man.archlinux.org
phoronix.com
	Linux (Unix)	~60%	Proven DVD/VCD playback
man.archlinux.org
, some Blu-ray/HD-DVD ability (unencrypted)
phoronix.com
, modular backend/frontend design, active maintenance.	No BD-J or HTML menu support, no UHD DRM support, Linux-centric (no native Windows GUI).
7	IINA (mpv-based)<br/>
iina.io
	macOS-only	~70%	Modern macOS Cocoa interface, leverages mpv’s codec support (plays “almost every media file”
iina.io
), supports scripting and plugins, great user experience on Mac.	Relies on mpv limitations (no disc menus), no built-in disc DRM support, Mac-only (though core is open).
8	GNOME Videos (Totem)<br/>
manpages.ubuntu.com
	Linux-only	~50%	Simple user-friendly interface, uses GStreamer for playback (inheriting its broad codec/plugins), can play DVDs and basic streams, well integrated in GNOME.	Limited disc interactivity (basic DVD only), no Blu-ray menu or 8KDVD features, not geared for high-end format like 8K or UHD Blu-ray by default.

(Projects above the line are the primary recommendations; lower-ranked ones either lack cross-platform reach or have significant feature gaps but are included for completeness in their categories.)

Conclusion: Kodi and VLC emerge as the top foundation choices due to their comprehensive format support and cross-platform codebases. Kodi’s extensible architecture (with Python add-ons and skinable UI) makes it particularly suitable to implement the 8KDVD’s dual menu system (one could parse 8KDVD’s XML/HTML menus and render via Kodi’s interface, or even incorporate a browser component for the HTML fallback). VLC offers a robust C/C++ core with all the necessary decoding (it already supports VP9 video and Opus audio as required by 8KDVD) and could be extended with plugins for new menu systems, though integrating a web engine would be non-trivial.

Other projects like mpv (or its libmpv) and xine-lib can serve as powerful back-end engines, but would need a custom UI layer to handle the sophisticated menu/navigation aspects of 8KDVD. On Windows, the MPC family provides a solid starting UI and DirectShow integration, but adding 8KDVD support would involve bridging in web technology and Java (for BD-J) which those projects currently lack.

In summary, no single open-source player yet meets 100% of the 8KDVD spec out-of-the-box, mainly due to the new HTML5/XML menu requirement and updated DRM. However, Kodi and VLC come closest (≈85–90%), each already implementing ~80% of the needed functionality (legacy formats, codecs, partial interactivity). By leveraging one of these projects – extending them with an HTML/JS rendering engine and updated DRM handling – developers could achieve a cross-platform 8KDVD player more efficiently than building from scratch, inheriting decades of multimedia playback expertise from these open-source communities.

Sources: The information above was synthesized from project documentation and forums, including official wiki entries for format support
kodi.wiki
, developer discussions on Blu-ray menu support
discourse.osmc.tv
reddit.com
, and release notes highlighting new codec integrations
github.com
. Notably, libbluray’s documentation confirms open-source Blu-ray menu (BD-J) capabilities
images.videolan.org
 which projects like VLC and Kodi utilize. References are provided inline for verification.