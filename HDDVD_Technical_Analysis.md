# HD-DVD Technical Analysis - Complete Format Specifications

> **📋 Related Documentation**: For detailed PowerDVD HD-DVD implementation analysis and component specifications, see **[PowerDVD HD-DVD Implementation Analysis](PowerDVD_HDDVD_Implementation_Analysis.md)**. For complete 8KDVD specifications, see **[8KDVD Technical Implementation Guide](8KDVD_Technical_Implementation_Guide.md)**.

## Executive Summary

HD-DVD (High Definition Digital Versatile Disc) exists in two distinct variants with significantly different structures and capabilities:

1. **Standard HD-DVD** (15/30 GB): Full HD-DVD specification with blue laser technology
2. **3X DVD** (HDDVD-Revived): Modified HD-DVD format using red laser DVD technology

**Critical Implementation Discovery**: Analysis of PowerDVD 8 HD-DVD restoration files reveals that HD-DVD support requires a complete subsystem architecture with specialized navigation components, advanced codec support, interactive content engines, and hardware-level copy protection integration.

## HD-DVD Format Variants

### 1. Standard HD-DVD (Retail Format)

#### Technical Specifications
- **Disc Capacity**: 15 GB (single layer), 30 GB (dual layer)
- **Laser Technology**: Blue laser (405nm wavelength)
- **File System**: UDF 2.5
- **Video Resolution**: Up to 1920×1080 (1080p)
- **Frame Rates**: 24/25/30/50/60 fps
- **Video Codecs**: VC-1, H.264/AVC, MPEG-2
- **Audio Codecs**: Dolby Digital Plus, Dolby TrueHD, DTS-HD, PCM

#### Disc Structure Analysis (Based on Term 2 Sample)

```
HDDVD_ROOT/
├── ADV_OBJ/                    # Advanced Objects (Interactive Content)
│   ├── *.aca                   # Advanced Content Application files
│   ├── *.png                   # Graphics files
│   ├── VPLST*.XPL             # Video Playlist files
│   └── DISCID.DAT             # Disc identification
└── HVDVD_TS/                  # HD-DVD Transport Stream
    ├── *.EVO                  # Enhanced Video Object files
    ├── *.IFO                  # Information files
    ├── *.BUP                  # Backup files
    └── *.MAP                  # Map files
```

#### Key File Types Analysis

**ADV_OBJ Directory:**
- **Advanced Content Applications (.aca)**: Interactive menu and bonus content
- **Video Playlists (.xpl)**: Navigation and playlist definitions
- **Graphics (.png)**: Menu graphics and interface elements
- **Disc ID (DISCID.DAT)**: Disc identification and metadata

**HVDVD_TS Directory:**
- **Enhanced Video Objects (.evo)**: Main video content streams
- **Information Files (.ifo)**: Navigation and chapter information
- **Backup Files (.bup)**: Backup copies of IFO files
- **Map Files (.map)**: File mapping and indexing

### 2. 3X DVD (HDDVD-Revived Format)

#### Technical Specifications
- **Disc Capacity**: 4.7 GB (DVD-5), 8.5 GB (DVD-9)
- **Laser Technology**: Red laser (650nm wavelength)
- **File System**: UDF 2.5 (modified)
- **Video Resolution**: 720p, 1080i (not 1080p)
- **Frame Rates**: 24/25/30 fps
- **Video Codecs**: H.264/AVC (optimized for lower bitrates)
- **Audio Codecs**: Dolby Digital Plus, PCM

#### Disc Structure Analysis (Based on HDDVD_SAMPLE_DISC)

```
3XDVD_ROOT/
└── HVDVD_TS/                  # Simplified HD-DVD Transport Stream
    ├── HV000I01.IFO          # Main information file
    ├── HV000I01.BUP          # Backup of main IFO
    ├── HV000M02.EVO          # Main video stream
    ├── HV001I01.IFO          # Secondary information file
    ├── HV001I01.BUP          # Backup of secondary IFO
    ├── HV001M01.EVO          # Secondary video stream
    └── [Additional HV###M##.EVO files]
```

#### Key Differences from Standard HD-DVD

1. **No ADV_OBJ Directory**: Simplified structure without advanced interactive content
2. **Simplified File Naming**: Uses HV###M##.EVO pattern instead of complex naming
3. **Reduced Interactive Features**: Limited menu system capabilities
4. **Optimized for DVD Media**: Designed for standard DVD production infrastructure

## File Format Specifications

### EVO (Enhanced Video Object) Files

**Purpose**: Container format for video, audio, and subtitle streams
**Structure**: Based on MPEG-2 Transport Stream with HD-DVD extensions
**Content**: Can contain multiple video streams, audio tracks, and subtitle streams

#### EVO Binary Structure and Index Units (IUs)
**Index Units (IUs)**: Fundamental proprietary metadata records for efficient random access
- **Presentation Time Stamp (PTS)**: High-resolution timestamp for stream timeline reference
- **Logical Block Address (LBA)**: Physical disc address or byte offset within EVO file
- **Stream Identification (PID)**: Packet Identifier for specific Elementary Stream
- **Flags**: Bit flags for GOP headers, I-frames, interactive data loading markers
- **Challenge**: Exact binary layout remains proprietary, hindering reverse engineering

#### Private Stream Packet IDs for Interactive Data
**Private Stream 1 (0xBD)**: 
- Historical DVD sub-picture usage
- HD-DVD allocates specific Sub-stream IDs for XPL interface graphics
- Content: PNG/JPEG images for menu buttons and backgrounds

**Private Stream 2 (0xBF)**: 
- Designated for navigation data
- Sub-stream IDs transport ACA bytecode, XPL XML metadata, N-Packs
- N-Packs contain synchronization commands and jump targets
- Links video timeline with ACA actions for interactive synchronization

#### Multi-Angle and Stream Synchronization
**Multi-Angle Implementation**: Multiple distinct video Elementary Streams multiplexed in same file
- Each ES assigned unique Program ID (PID) within MPEG-TS structure
- User angle selection triggers PID mapping switch in Program Map Table (PMT)
- Seamless switching via demultiplexer output redirection

**Stream Synchronization**: Time-based synchronization using shared Program Clock Reference (PCR)
- Media streams and interactive data streams include high-resolution PTS markers
- HDi runtime monitors Media Engine PTS value via ACA API
- Interactive overlays triggered when monitored PTS matches predefined time markers in N-Packs

**Naming Conventions:**
- **Standard HD-DVD**: Complex naming (a00.EVO, a02.EVO, etc.)
- **3X DVD**: Simplified naming (HV###M##.EVO pattern)

### Folder Name Variations (HVDVD_TS vs HDDVD_TS)
- **Canonical Name**: HVDVD_TS (High-Definition Video DVD Transport Stream)
- **Abbreviation**: HDDVD_TS (occasionally used)
- **Usage Context**: 
  - Early specification drafts
  - Internal testing discs
  - Regional authoring house variations
- **Player Compatibility**: Compliant players check both directory names
- **Backward Compatibility**: Provides resilience across manufacturing origins

### IFO (Information) Files

**Purpose**: Navigation information and chapter definitions
**Structure**: Binary format containing menu structures and playback information
**Backup**: Corresponding .BUP files provide redundancy

### ACA (Advanced Content Application) Files

**Purpose**: Interactive menu and bonus content applications
**Technology**: Based on web technologies (HTML, CSS, JavaScript)
**Features**: Dynamic menus, games, interactive features
**Availability**: Only in Standard HD-DVD format
**Architecture**: Controller layer in ACA/XPL separation

#### ACA Binary Structure and Execution Flow
- **Binary Format**: Proprietary structure with fixed-size header (magic number 0xACAC, version info)
- **Metadata Block**: Links application to associated XPL resources
- **Executable Payload**: Machine-agnostic bytecode or highly tokenized JavaScript
- **Optimization**: Fast loading and execution within limited player resources
- **JavaScript Engine**: js32.dll (32-bit JavaScript runtime, likely JScript or SpiderMonkey derivative)
- **Native Bridge**: Foreign Function Interface (FFI) for system resource access
- **Security**: Sandboxed execution environment with restricted system access

#### ACA API Functions (Inferred)
**Media Control**: 
- `ACA.play()`, `ACA.pause()`, `ACA.seekTime(pts)`
- `ACA.setAngle(n)` for multi-angle video
- Frame-accurate stream control with PTS synchronization

**Navigation**: 
- `ACA.goToTitle(id)`, `ACA.goToMenu(id)`
- Seamless disc structure navigation

**Graphics and Overlay**: 
- Dynamic layering and visibility control
- PNG/JPEG asset management for menus and pop-ups

**System and Live Features**: 
- `ACA.fetchURL(url)` for HD-DVD Live network connectivity
- `ACA.getDiscVolumeID()` for AACS volume information
- Persistent storage I/O operations

### XPL (Video Playlist) Files

**Purpose**: Define video playlists and navigation paths
**Structure**: XML-based playlist definitions
**Features**: Chapter navigation, multiple video streams, conditional playback
**Architecture**: View layer in ACA/XPL separation
- **XPL (View)**: Declarative XML documents
- **Function**: Defines visual appearance, graphics, layout, animation paths, UI structure
- **ACA Control**: Loaded and manipulated by controlling ACA script
- **Design Purpose**: Streamline development workflows, separate logic from presentation

## HDi Interactive System Analysis

### HDi Runtime Architecture
**Complete HDi Runtime Components**:
- **Media Engine**: High-speed MPEG-TS demultiplexing, VC-1/H.264 decoding, PCR time base synchronization
- **HDi Core/Application Manager (AppMan)**: Lifecycle and resource allocation for ACA applications
- **JavaScript Engine (js32.dll)**: Executes ACA procedural logic
- **Graphics Renderer**: Renders XPL and HTML/CSS elements, compositing interactive overlay plane
- **Native Bridge (ACA API)**: Secure interface between scripting environment and system hardware

### HTML/CSS Rendering Capabilities and Constraints
**Technology Profile**: Consumer Electronics (CE-HTML) profile
- **HTML Support**: HTML 4.01/XHTML subset
- **CSS Support**: CSS 2.0 subset
- **Constraints**: Limited complex layouts, restricted font libraries, specific positioning requirements
- **Performance**: Must operate without compromising 72 Mbit/s media playback
- **Customization**: Proprietary DOM and Media API extensions

### DOM API and Multimedia Synchronization
**HDi DOM Extensions**:
- **HTMLMediaElement Extensions**: Input management, video plane control, Picture-in-Picture windows
- **Time-based Events**: Integration with media stream PTS for synchronized interactive overlays
- **Event Registration**: ACA scripts register listeners for specific time markers in EVO files
- **Synchronization**: App Manager triggers JavaScript event handlers when Media Engine reaches predefined PTS

### JavaScript Security and Sandboxing
**Security Restrictions**:
- **Origin Policy**: Scripts restricted to specific disc directory or network domain
- **Partitioned Storage**: Persistent storage partitioned and limited per application
- **Network Restrictions**: Requests limited to predetermined domains
- **Sandbox Vulnerability**: Compromised on general-purpose computing platforms
- **Trusted Execution Environment (TEE)**: Required for effective sandboxing

### HD-DVD Live Network Connectivity
**Network Features**:
- **Ethernet LAN Port**: Mandatory for HD-DVD Live functionality
- **HTTP/TCP/IP Communication**: Standard network protocols via ACA.fetchURL
- **Content Downloads**: Supplementary disc content, software/firmware updates
- **Persistent Storage**: Dedicated local storage with App Manager-controlled access

### 3X DVD Menu System

**Technology Stack**:
- **Simplified Navigation**: Basic menu system without advanced interactivity
- **Limited Features**: Basic chapter navigation and quality selection
- **Reduced Complexity**: Optimized for lower-capacity media

**Key Limitations**:
- No advanced interactive content
- Limited bonus features
- Simplified menu graphics
- No network connectivity features

## Codec and Quality Specifications

### Video Codecs

| Format | Primary Codec | Resolution | Bitrate Range |
|--------|---------------|------------|---------------|
| Standard HD-DVD | VC-1, H.264 | 1920×1080p | 15-40 Mbps |
| 3X DVD | H.264 | 1280×720p, 1920×1080i | 8-15 Mbps |

### Audio Codecs

| Format | Audio Codecs | Channels | Bitrate Range |
|--------|--------------|----------|---------------|
| Standard HD-DVD | Dolby TrueHD, DTS-HD, DD+ | Up to 7.1 | Up to 18 Mbps |
| 3X DVD | Dolby Digital Plus, PCM | Up to 5.1 | Up to 6 Mbps |

## AACS Cryptography and Integration Specifics

### AACS Key Derivation Process for HD-DVD
**Hierarchical Key System**:
1. **Device Keys (Dk)**: Unique set possessed by each licensed player device
2. **Media Key Derivation (Km)**: Device processes MKB using Dk set + Volume ID + Format Code
   - **HD-DVD Format Code**: 00002 (Hex) - Ensures cryptographic separation from Blu-ray
   - **MKB Processing**: Subset Difference Tree (SDT) algorithm for broadcast encryption
3. **Volume Key Derivation (Kv)**: Km decrypts Volume Key
4. **Title Key Derivation (Kt)**: Kv + Title ID yields Title Key
5. **Content Decryption**: Kt decrypts AES-encrypted media streams

### AACS Implementation Differences (HD-DVD vs. Blu-ray)
**Format Code Separation**:
- **HD-DVD/DVD Media**: Format Code 00002 (Hex)
- **Blu-ray Disc (BD) Media**: Format Code 00012 (Hex)
- **Result**: Cryptographic isolation prevents cross-format key usage

**Mandatory Managed Copy (MMC)**: HD-DVD adopted earlier than Blu-ray
- Full-resolution copies for local storage (media servers)
- Scaled-down copies for portable devices
- Pro-consumer feature distinguishing HD-DVD from Blu-ray

### Hardware and Firmware Integration
**Protected Media Path (PMP)**:
- **Kernel Driver Requirements**: Specific drivers for secure processing pipeline
- **Trusted Execution Environment (TEE)**: Prevents unauthorized access to decrypted streams
- **Drive Firmware**: Contains non-revocable AACS Device Keys
- **Secure Key Exchange**: Drive firmware handles initial secure key exchange with host player

### AACS Authentication and Key Revocation
**Authentication Process**:
1. Host reads MKB and Volume ID
2. Cryptographic engine (protected by kernel drivers) processes derivation
3. Device with valid, un-revoked Dk set completes process and begins decryption

**Key Revocation Mechanism**:
- **Reactive Security**: MKB versioning and SDT structure updates
- **Compromised Key Response**: New MKB (MKBvN+1) issued when Device Keys compromised
- **Blacklisting**: Cryptographically constructed to exclude compromised keys
- **User Impact**: Legitimate users must update software/firmware for new discs

**Historical Vulnerability**: 2007 Processing Key (09 F9) extraction from PC software players
- Demonstrated DRM model limitations in untrusted environments
- Significantly impacted HD-DVD competitive viability against Blu-ray

## Player Requirements and Compatibility

### Hardware Requirements

**Standard HD-DVD**:
- HD-DVD compatible optical drive
- HD-DVD decoder hardware/software
- AACS-compliant player
- Network connectivity (for HD-DVD Live features)

**3X DVD**:
- Standard DVD drive (with 3X DVD support)
- HD-DVD compatible decoder
- AACS-compliant player (simplified)

### Software Support

**PowerDVD Compatibility**:
- **PowerDVD 6.5 HD**: Full HD-DVD support including 3X DVD
- **PowerDVD 6.6**: Enhanced HD-DVD support
- **PowerDVD 8**: HD-DVD support removed but can be restored with specific components
- **Later Versions**: HD-DVD support phased out

**PowerDVD 8 HD-DVD Restoration Requirements**:
- **Core Components**: HDDVDNav.ax (417KB), HDDVDAdvNav.dll (3.4MB), HDDVDNavi.dll (1.4MB)
- **Decoder Support**: CLHDSPDecoder.dll (75KB) for HD-DVD specific formats
- **Interactive Engine**: js32.dll for JavaScript execution in ACA applications
- **Registration**: HDDVDNav.ax must be registered via regsvr32 command
- **Registry Configuration**: 20+ HD-DVD specific settings for performance optimization
- **ADV_VM_MT_PREF**: Advanced Video Module Multi-Threading Preference control
  - **Purpose**: Allocate computational resources (multiple CPU cores) for decoding
  - **Function**: Fine-tune performance for high-bitrate/complex video streams
  - **Strategy**: Higher values dedicate more threads at higher priority
  - **Trade-off**: Smoother playback vs. other concurrent system processes

## Implementation Requirements for Windows Player

### Core Components Needed

**Based on PowerDVD Implementation Analysis:**

1. **Navigation System Architecture**
   - **Main Navigation Engine**: Equivalent to HDDVDNav.ax (417KB ActiveX control)
   - **Advanced Navigation**: HDDVDAdvNav.dll (3.4MB) for complex interactive content
   - **Navigation Interface**: HDDVDNavi.dll (1.4MB) providing APIs and interfaces
   - **Component Registration**: ActiveX control registration via regsvr32

2. **Disc Detection and Mounting**
   - UDF 2.5 file system support
   - Automatic format detection (Standard HD-DVD vs 3X DVD)
   - AACS copy protection handling with kernel driver integration

3. **Video/Audio Decoding**
   - VC-1, H.264/AVC, MPEG-2 video decoders
   - Dolby TrueHD, DTS-HD, DD+ audio decoders
   - **HD-DVD Specific Decoder**: CLHDSPDecoder.dll (75KB) for special formats
   - Hardware acceleration support with DirectShow integration

4. **Interactive Content Engine**
   - **JavaScript Engine**: js32.dll for ACA application execution
   - **HTML/CSS Rendering**: Full web technology support for interactive menus
   - **ACA File Processing**: Advanced Content Application parsing and execution
   - **XPL Playlist Parsing**: XML-based navigation and playlist handling

5. **Performance Optimization**
   - **Multi-threading Configuration**: 8 different threading preferences for video/audio processing
   - **Timing Optimization**: Demux leading time (2000ms), max demux time (16ms)
   - **Hardware Integration**: Specific kernel driver GUID for secure processing
   - **Color Space Processing**: Optimized color space handling (option 3)

### Technical Challenges

1. **AACS Compliance**: Requires licensed AACS implementation with specific kernel driver (`{95808DC4-FA4A-4c74-92FE-5B863F82066B}`)
2. **Advanced Content**: Complex ACA application support requiring full JavaScript engine
3. **Component Architecture**: Multi-layered navigation system with 4+ specialized DLLs
4. **Hardware Acceleration**: Performance optimization for HD video with extensive multi-threading
5. **Registry Configuration**: 20+ HD-DVD specific settings for optimal performance
6. **Legacy Support**: Maintaining compatibility with obsolete format and hardware

## Development Priority Recommendations

### Phase 1: Basic HD-DVD Support
1. **3X DVD format implementation** (simplified structure without ADV_OBJ)
2. **Basic EVO/IFO file parsing** with HV###M##.EVO naming convention
3. **Simple menu system** without advanced interactive content
4. **Core video/audio playback** with H.264/DD+ support

### Phase 2: Standard HD-DVD Support
1. **Full HD-DVD structure implementation** with ADV_OBJ directory support
2. **Navigation system architecture** (equivalent to HDDVDNav.ax functionality)
3. **ACA application support** with JavaScript engine integration
4. **Advanced menu systems** with HTML/CSS/JavaScript rendering

### Phase 3: Advanced Features
1. **Complete navigation subsystem** (HDDVDAdvNav.dll + HDDVDNavi.dll equivalent)
2. **HD-DVD Live network features** and advanced interactivity
3. **Multi-angle and PiP support** with complex navigation
4. **Performance optimization** with multi-threading and registry configuration
5. **Complete AACS compliance** with kernel driver integration

## Conclusion

HD-DVD presents a complex dual-format challenge requiring support for both the full-featured Standard HD-DVD format and the simplified 3X DVD variant. The PowerDVD implementation analysis reveals that HD-DVD support requires a complete subsystem architecture with specialized components:

**Key Implementation Insights:**
- **Multi-layered Architecture**: 4+ specialized DLLs working together (HDDVDNav.ax, HDDVDAdvNav.dll, HDDVDNavi.dll, CLHDSPDecoder.dll)
- **Interactive Content Engine**: Full JavaScript engine (js32.dll) for ACA application execution
- **Performance Optimization**: 20+ registry settings for multi-threading and timing optimization
- **Hardware Integration**: Specific kernel driver requirements for AACS compliance
- **Component Registration**: ActiveX control registration via regsvr32

The 3X DVD format offers a more manageable starting point for implementation, while Standard HD-DVD requires comprehensive support for advanced interactive features and complex menu systems. The format's obsolescence means implementation must focus on legacy compatibility and may require specialized hardware/software combinations for full functionality.

**Critical Discovery**: HD-DVD was not a simple format addition but a complete subsystem within PowerDVD, requiring dedicated navigation, decoding, interactive content, and copy protection components working in concert.
