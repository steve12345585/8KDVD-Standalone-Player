# VCD/SVCD/CDI Technical Analysis - Complete Format Specifications

> **📋 Related Documentation**: For complete format specifications, see **[8K DVD Player Specifications](8K_DVD_Player_Specifications.md)**. For detailed HD-DVD analysis, see **[HD-DVD Technical Analysis](HDDVD_Technical_Analysis.md)**. For 8KDVD technical guide, see **[8KDVD Technical Implementation Guide](8KDVD_Technical_Implementation_Guide.md)**.

## Executive Summary

This document provides comprehensive technical analysis of VCD (Video CD), SVCD (Super Video CD), and CD-I (Compact Disc Interactive) formats, detailing their file structures, menu systems, navigation mechanisms, and implementation requirements for the ultimate hybrid player.

## Format Overview

| Format | Menu Technology | File Location | Primary Language | Interactive Level |
|--------|----------------|---------------|------------------|-------------------|
| VCD | CD-I Application | CDI folder | Binary CD-I | Basic |
| SVCD | Limited Menu System | SVCD folder | Binary | Basic |
| CD-I | Native CD-I | Root/Application | CD-I Binary | Advanced |

---

## 1. VCD (Video CD) Format

### Technical Specifications
- **Video Codec:** MPEG-1 Video
- **Audio Codec:** MPEG-1 Audio Layer II
- **Resolution:** 352x240 (NTSC) / 352x288 (PAL)
- **Frame Rate:** 29.97 fps (NTSC) / 25 fps (PAL)
- **Bitrate:** Video: 1.15 Mbps, Audio: 224 kbps
- **File System:** ISO 9660
- **Menu System:** CD-I application with hotspot navigation
- **Disc Capacity:** 650-800 MB

### Disc Structure Analysis

```
VCD_ROOT/
├── CDI/                    # CD-I Application files
│   ├── *.APP               # CD-I application files
│   ├── *.IMG               # Menu image files
│   └── [CD-I data files]   # Interactive content
├── MPEGAV/                 # MPEG Audio/Video data
│   ├── AVSEQ01.DAT         # First video sequence
│   ├── AVSEQ02.DAT         # Second video sequence
│   └── AVSEQ##.DAT         # Additional sequences
├── EXT/                    # Extended information
│   ├── LOT_X.VCD           # List of tracks
│   └── PSD_X.VCD           # Play sequence descriptor
├── VCD/                    # VCD control data
│   ├── ENTRIES.VCD         # Chapter entry points
│   ├── INFO.VCD            # Disc information
│   └── LOT.VCD             # List of tracks
└── SEGMENT/                # Video segments
    └── [segment files]
```

### CD-I Menu System

#### CD-I Application Files (.APP)
- **Purpose**: Interactive menu applications for VCD playback
- **Format**: Binary CD-I application format
- **Functionality**: Menu navigation, hotspot handling, user interaction
- **Platform**: CD-I player environment
- **Loading Process**:
  1. **Boot Sector Confirmation**: Read Boot Sector (LBN 16 or 300) to confirm valid CD-I
  2. **Application Manifest**: Retrieve CDI.APL (CD-I Application Locator) file
  3. **Execution Parameters**: Define LBN location, memory allocation, entry point
  4. **Loading**: Load .APP file into system memory and transfer control
- **Entry Point**: Defined in CDI.APL manifest, often Motorola 68000 processor family
- **Memory Management**: Allocation requirements specified in manifest

#### Hotspot Coordinate System
- **Coordinate Type**: UCM (Unit Coordinate Model) coordinates
- **UCM Formula**: UCM value = 2 × pixel coordinate
- **Origin**: Upper-left corner (0,0)
- **Hotspot Format**: Rectangular coordinate pairs
  - Left-upper corner (x1, y1)
  - Right-bottom corner (x2, y2)
- **Example**: Pixel coordinate (100, 50) = UCM coordinate (200, 100)
- **Precision**: Exact scaling factor S=2 (not rounding/offset)
- **Formula**: Px = ⌊Ux/2⌋ and Py = ⌊Uy/2⌋ (UCM to physical pixel)
- **Sub-pixel Accuracy**: Odd UCM values = center between adjacent pixels
- **Resolution Modes**:
  - Normal: 768×560 UCM → 384×280 pixels
  - Double H: 768×560 UCM → 768×280 pixels  
  - Double H&V: 768×560 UCM → 768×560 pixels (interlaced)

#### Menu Background Images
- **Format Support**: JPEG, BMP, and other standard image formats
- **Resolution**: No fixed size requirement in CD-i spec
- **Display**: Matched to CD-i player display resolution
- **VCD Constraint**: MPEG video limited to 352×240/288, but menu backgrounds can be higher resolution

### Navigation System

#### VCD/SVCD Indexing and Timecode Structure (ENTRIES.VCD)
**Format Origin and Context (White Book, CD-i Bridge)**:
- **VCD Release**: 1993, SVCD Release: 1998
- **CD-i Bridge Conformance**: CD-ROM XA discs incorporating specific Green Book CD-i application data
- **Mode 2, Form 2/XA Sectors**: Data tracks containing multiplexed MPEG video and audio streams
- **Storage Capacity**: Up to 800MB on 80-minute CD (vs. 700MB Mode 1 limit)

**Binary Format Specification of ENTRIES.VCD**:
- **Location**: `VCD/ENTRIES.VCD` file within VCD structure
- **Format**: Tightly structured array of fixed-size records (distinct from ASCII Value Change Dump format)
- **Record Content**: Each record corresponds to designated program segment, chapter point, or accessible playback entry point
- **Key Fields**:
  - **Entry Type/Sequence ID**: Code identifying nature of entry (video track, chapter)
  - **Logical Block Number (LBN)**: 24-bit field specifying absolute sector address where MPEG Program Stream data begins
  - **M:S:F Timecode**: Exact Minutes:Seconds:Frames timestamp corresponding to LBN
  - **Metadata Pointers**: Additional data linking entry point to menu graphics or text overlays
- **Navigation Strategy**: Centralized LBN pointers bypass slower ISO 9660 file system traversal

**Logical Block Number (LBN) Calculation and Timecode Mapping**:
- **CD Time Standard**: Minutes (M), Seconds (S), and Frames (F) at 75 Frames (sectors) per second
- **LBN Definition**: Zero-indexed absolute sector position within program area
- **Lead-In Area**: Required 2-second (150-frame) area preceding first program track
- **Precise Formula**: LBN = ((M×60) + S) × 75 + F - 150
- **Mathematical Precision**: M:S:F value of 00:02:00 (program area start) corresponds precisely to LBN 0

**Precision Seeking Algorithm Implementation**:
1. **Index Lookup**: Application references ENTRIES.VCD file for corresponding record
2. **LBN Extraction**: 24-bit Logical Block Number retrieved directly from index record
3. **Physical Seek Command**: LBN value sent as direct positioning command to CD controller hardware
- **Performance**: Achieves immediate seek times crucial for format acceptance

**PSD Branching Logic**: Conditional navigation based on state machine
  - User Interaction Registers: Remote control input (selection 1, 2, 3)
  - System Parameters: Language, regional code, parental restrictions
  - Playback State Flags: Completion status, intro skip flags
  - Command Format: If (condition) JumpTo (PlayItem X) Else JumpTo (PlayItem Y)
  - Execution: Hardware/firmware references Player Register Memory (PRM)
- **Time Encoding**: 
  - Logical Block Number (LBN) of Play Item start
  - Minutes:Seconds:Frames (M:S:F) format
- **Precision**: Sub-second accurate start/end points
- **Alignment**: Matches CD-ROM XA Mode 2 Form 2 sector structure

#### File Naming Conventions
- **Video Files**: `AVSEQ##.DAT` (e.g., AVSEQ01.DAT, AVSEQ02.DAT)
- **Sequence Mapping**: Corresponds to playlist sequence items
- **Order**: Sequential numbering starting from 01

### Implementation Requirements

#### Core Components
1. **CD-I Application Runtime**
   - CD-I application file (.APP) execution
   - Hotspot coordinate processing (UCM to pixel conversion)
   - Interactive menu rendering

2. **MPEG-1 Decoder**
   - Video: MPEG-1 Video at 352×240/288
   - Audio: MPEG-1 Layer II at 224 kbps
   - Stream handling for multiple sequences

3. **Navigation Engine**
   - ENTRIES.VCD parsing for chapter points
   - PSD file processing for sequence control
   - Timestamp-based seeking

4. **File System Support**
   - ISO 9660 file system mounting
   - Multi-folder structure navigation

---

## 2. SVCD (Super Video CD) Format

### Technical Specifications
- **Video Codec:** MPEG-2 Video
- **Audio Codec:** MPEG-1 Audio Layer II
- **Resolution:** 480×480 (NTSC) / 480×576 (PAL)
- **Frame Rate:** 29.97 fps (NTSC) / 25 fps (PAL)
- **Bitrate:** Video: up to 2.5 Mbps, Audio: up to 384 kbps
- **File System:** ISO 9660 (CD-ROM XA Mode 2)
- **Menu System:** Limited menu system (not CD-I based)
- **Disc Capacity:** 650-800 MB

### Disc Structure Analysis

```
SVCD_ROOT/
├── MPEG2/                  # MPEG-2 video data
│   ├── AVSEQ01.MPG         # MPEG-2 video sequence
│   └── AVSEQ##.MPG         # Additional sequences
├── SVCD/                   # SVCD control data
│   ├── ENTRIES.SVD         # Entry points (similar to VCD)
│   ├── INFO.SVD            # Disc information
│   ├── LOT.SVD             # List of tracks
│   ├── PSD.SVD             # Play sequence descriptor
│   └── SEARCH.DAT          # Search index
└── [Additional SVCD files]
```

### Menu System Differences from VCD

#### Limited Menu System
- **Technology**: Not based on CD-i format
- **Standard**: White Book extension under CD-ROM XA Mode 2
- **Video Tracks**: Mode 2, Form 2
- **Metadata**: Mode 2, Form 1
- **Menu Features**: No extended menu or hotspot features beyond VCD
- **Authoring Tools**: Simplistic menus (static images with basic hotspots)

#### Audio Track Handling
- **Multiple Tracks**: Supports multiple stereo audio tracks (dual audio)
- **Audio Formats**: Multiple mono streams supported
- **Codec**: MPEG-1 Audio Layer II
- **Sample Rate**: 44.1 kHz
- **Switching**: Handled by authoring metadata layer in disc control data
- **Audio Track Selection Priority**:
  1. **Disc Mandatory Setting**: Primary sequence defined by disc authoring
  2. **Player Language Preference**: User's preferred language from firmware settings
  3. **Numerical Fallback**: Lowest numbered audio stream (Stream 0) if no match found
- **Configuration**: PCI data block defines selection hierarchy
- **Metadata**: Language codes embedded in MPEG stream audio tracks

### Encoding Requirements

#### MPEG-2 Video Parameters
- **Resolution**: 480×480 (NTSC) / 480×576 (PAL)
- **Maximum Combined Bitrate**: ≤ 2.7 Mbps (video + audio)
- **Video Bitrate**: Up to 2.5 Mbps
- **Audio Bitrate**: Up to 384 kbps

#### Audio Specifications
- **Codec**: MPEG-1 Layer II
- **Sample Rate**: 44.1 kHz
- **Channels**: Stereo or multiple mono streams

### Implementation Requirements

#### Core Components
1. **MPEG-2 Decoder**
   - Enhanced video decoding compared to VCD
   - Higher resolution support (480×480/576)
   - Improved bitrate handling

2. **Multi-Audio Track Support**
   - Multiple stereo track handling
   - Audio track switching functionality
   - Enhanced audio codec support

3. **SVCD File Parser**
   - .SVD file format parsing
   - Entry point and track list processing
   - Search index handling

---

## 3. CD-I (Compact Disc Interactive) Format - The Green Book Architecture

### Technical Specifications
- **Interactive Content:** Native CD-I applications under CD-RTOS
- **Menu System:** Advanced CD-I interactive system with UCM coordinate mapping
- **Graphics:** Hardware-accelerated graphics via dual VSC chips
- **Audio:** CD-quality audio support with ADPCM decoding
- **File System:** CD-I native file system with Real-Time Files (RTFs)
- **Applications:** Full interactive applications and games

### CD-I Application Runtime and Execution Environment

#### CD-RTOS Operating System and Application Structure
**CD-RTOS Foundation**:
- **Base System**: Microware's OS-9 real-time variant (Compact Disc – Real Time Operating System)
- **Architecture**: Motorola 68000-based microprocessor (MC68000/MC68EC000)
- **Memory Management**: Limited to <1MB contiguous memory space for applications
- **Real-Time Files (RTFs)**: Dynamic loading of multimedia modules from optical disc
- **Application Structure**: Modular design optimized for sequential data streaming

**CD-I Application Binary Format**:
- **Executable Standard**: Tailored for Motorola 68000 architecture and CD-RTOS environment
- **Format Resemblance**: Constrained Common Object File Format (COFF) structure
- **Mandatory Header**: Identifies file as CD-RTOS executable with initial execution parameters
- **Relocation Information**: Comprehensive relocation data for dynamic memory allocation
- **Binary Segments**: Executable code, initialized static data, stack configuration initialization
- **Linkage Tables**: Precise linkage tables for dynamic RTF loading via CD-RTOS routines

#### Motorola 68000 Instruction Execution and System State Management
**MPU Architecture**:
- **Processor**: Motorola 68000-based microprocessor (MC68000/MC68EC000)
- **Data Bus**: 16-bit data bus
- **Address Bus**: 24-bit address bus (logically supports 16MB addressable memory)
- **Instruction Set**: M68000 family code-compatible (MC68008, MC68010/68020 upward compatibility)

**Execution Environment**:
- **Application Execution**: M68000 MPU User State
- **System Control**: CD-RTOS kernel operates exclusively in privileged Supervisor State
- **Resource Access**: Applications interact with critical resources via controlled system calls
- **Trap Instructions**: Privileged trap instructions transfer execution to kernel momentarily
- **System Isolation**: Prevents application errors from corrupting core operating system
- **Timing Integrity**: Maintains continuous 75 sector-per-second data flow from optical disc

#### CD-I Memory Management and VRAM Allocation
**Memory Architecture**:
- **MPU Memory**: Limited physical memory allocated to MPU (often <1MB for applications)
- **Video Memory**: Dedicated video memory (VRAM) physically distinct from MPU memory
- **VRAM Partitioning**: Two independent planes (Plane A and Plane B)
- **VSC Control**: Each plane managed by dedicated Philips SCC66470 Video and System Controller (VSC) chip
- **VRAM Capacity**: 512KB per plane (1MB total dedicated video memory)
- **Hardware Co-processing**: VSCs handle run-length decoding and mosaic decoding

#### Hardware-Accelerated Graphics Rendering Pipeline
**Graphics Rendering**:
- **Dual VSC Chips**: Philips SCC66470 VSC chips as intelligent video co-processors
- **Display Control Program (DCP)**: Table of instructions established and executed by applications
- **Real-Time Synchronization**: DCP instructions executed at critical intervals:
  - **Field Control Table (FCT)**: Executed once at beginning of every video field (half-frame)
  - **Line Control Table (LCT)**: Executed at start of every horizontal scan line
- **Line-by-Line Control**: VSC receives Line Start Address for specific VRAM location reading
- **Synchronization**: Precise real-time sync between CPU commands, CD-ROM data flow, and video output refresh rate
- **Advanced Effects**: Tear-free vertical scrolling, sophisticated palette manipulation, controlled blending of video planes A and B

#### Audio System Implementation and ADPCM Decoding
**Audio System**:
- **High-Fidelity Support**: Sound effects alongside streamed audio content
- **Data Intermixing**: Audio data intermixed with video and program data within Mode 2 CD sectors
- **Playback Support**: Standard CD-DA (Red Book audio) and various ADPCM compression levels
- **Coding Information Byte**: Located within Mode 2 sector subheader, defines audio parameters
- **Dynamic Mode Switching**: Hardware decoder switches modes based on Coding Information Byte
- **Audio Parameters**: Number of bits per sample (4-bit or 8-bit), sampling rate, channel configuration (mono/stereo)
- **Bandwidth Optimization**: Trade-off between audio quality and streaming bandwidth for interactive multimedia

#### User Input and Peripheral Interaction (I/O Handling)
**Input System**:
- **Standardized I/O**: CD-RTOS device drivers abstract user input subsystem
- **Touchpad Controller**: Standard pointing device port mandated on all Philips CD-i players
- **RS-232 Serial Port**: Mandatory on every Philips unit for external peripherals
- **External Peripherals**: Modems, serial printers, display terminals (DEC VT220 emulators)
- **Event Processing**: CD-RTOS kernel dispatches generalized events (cursor movement, button events) to active application
- **Modular Architecture**: Designed for commercial applications (kiosks, educational systems, training interfaces)

### Universal Coordinate Mapping (UCM) System

#### UCM Fundamentals and Canonical Viewport Definition
**UCM Architecture**:
- **Purpose**: Proprietary CD-I specification feature ensuring proper screen layout regardless of video standard/resolution
- **Virtual Coordinate Space**: Single, fixed, canonical virtual coordinate space defined within Green Book
- **Abstraction Layer**: Allows developers to define graphic positions once in standardized space
- **Format Independence**: Independent of NTSC/PAL or standard/double resolution modes
- **Dynamic Transformation**: Transformation matrix converts logical UCM point (xa,ya) to physical pixel address (xp,yp)

#### UCM Coordinate Transformation Algorithm
**Transformation Process**:
- **Core Algorithm**: Scaled linear algorithm TUCM determining physical pixel coordinates
- **Scaling Factors**: Ratio between canonical UCM space (WUCM and HUCM) and active physical display resolution (Wphys and Hphys)
- **Mathematical Implementation**:
  - xp = Tx(xa, WUCM, Wphys)
  - yp = Ty(ya, HUCM, Hphys)
- **Standard Resolution**: Canonical UCM resolution standardized to fixed ratio (e.g., 768×560 for full PAL/NTSC compatibility)
- **Aspect Ratio Correction**: Automatic handling of necessary aspect ratio corrections
- **Performance Optimization**: VSC receives pre-transformed physical coordinates, simplifying DCP drawing commands

#### Display Resolution Scaling and Handling
**Resolution Support**:
- **Native Resolutions**: Standard television (384×280 NTSC) to doubled-resolution modes (768×560)
- **Dynamic Scaling**: UCM accommodates varying physical modes by adjusting internal scaling coefficients (Sx, Sy)
- **Matrix Recalculation**: CD-RTOS graphics library recalculates UCM transformation matrix on video mode transitions
- **Coordinate Invariance**: Graphical object at UCM coordinate (500, 500) maps precisely to expected physical pixel location
- **Performance Implications**: High-resolution modes require VSC to process up to four times the pixel data
- **Hardware Requirements**: Dual VSCs and dedicated VRAM capacity mandated for higher-resolution UCM mappings

#### Sub-Pixel Positioning and Interpolation Techniques
**Sub-Pixel Precision**:
- **Internal Precision**: Coordinates computed with high internal precision using fixed-point or floating-point arithmetic
- **Hardware Delegation**: Final sub-pixel accuracy resolution into blended color values delegated to VSC hardware
- **VSC Interpolation**: VSC performs necessary interpolation (bilinear filtering) during line-by-line rendering process
- **Smooth Transitions**: Enables smooth visual transitions and high-fidelity edge localization
- **Performance Benefit**: Computational efficiency by capitalizing on VSC's pixel data manipulation capabilities

#### Coordinate Clipping, Bounds Checking, and Viewport Safety
**Safety Mechanisms**:
- **Coordinate Clipping**: Mandatory mechanisms for discarding drawing primitives extending outside UCM viewport boundaries
- **Bounds Checking**: CD-RTOS graphics primitives validate application drawing commands against canonical UCM limits
- **Geometry Truncation**: Any geometry falling outside predefined bounds is truncated or discarded by graphics library
- **Memory Integrity**: Prevents VSC from attempting to write pixel data beyond designated 512KB plane memory
- **System Stability**: Enforces memory integrity and display coherence at fundamental level within platform's driver stack
- **Audio**: High-quality audio integration
- **Navigation**: Complex interactive navigation systems

#### Interactive Features
- **Graphics Engine**: Advanced 2D graphics capabilities
- **Animation Support**: Frame-based animations
- **Audio Integration**: Synchronized audio and video
- **User Input**: Mouse, keyboard, and gamepad support
- **File Access**: Direct file system access

### Implementation Requirements

#### Core Components
1. **CD-I Runtime Environment**
   - Complete CD-I application execution
   - Graphics and audio subsystem
   - User input handling

2. **Interactive Graphics Engine**
   - Advanced 2D graphics rendering
   - Animation system
   - Image and video overlay

3. **Audio System**
   - CD-quality audio playback
   - Audio synchronization
   - Multiple audio stream support

---

## Menu System Implementation Matrix

### Technology Requirements by Format

| Format | CD-I Runtime | Graphics Engine | Audio System | File System | Interactive Level |
|--------|--------------|-----------------|--------------|-------------|-------------------|
| VCD | ✅ (Basic) | ✅ (Hotspots) | ✅ (MPEG-1) | ✅ (ISO 9660) | Basic |
| SVCD | ❌ | ✅ (Limited) | ✅ (Enhanced) | ✅ (ISO 9660) | Basic |
| CD-I | ✅ (Full) | ✅ (Advanced) | ✅ (CD-Quality) | ✅ (CD-I Native) | Advanced |

### Implementation Priority Recommendations

#### Phase 1: Basic VCD Support
1. **CD-I Application Runtime**: Basic .APP file execution
2. **Hotspot System**: UCM coordinate conversion and hotspot handling
3. **MPEG-1 Playback**: Video and audio decoding
4. **Navigation**: ENTRIES.VCD parsing and chapter navigation

#### Phase 2: Enhanced VCD/SVCD Support
1. **SVCD MPEG-2 Support**: Enhanced video decoding
2. **Multi-Audio Tracks**: Audio track selection and switching
3. **Advanced Navigation**: PSD file processing
4. **Menu Rendering**: Static image menu with hotspot support

#### Phase 3: Full CD-I Support
1. **Complete CD-I Runtime**: Full interactive application support
2. **Advanced Graphics**: 2D graphics and animation engine
3. **Audio Integration**: CD-quality audio with synchronization
4. **Interactive Features**: Complete user input and navigation

---

## Technical Implementation Requirements

### Core Menu Engine Components

1. **CD-I Application Engine**
   - .APP file execution and runtime
   - UCM coordinate system handling
   - Hotspot detection and processing

2. **Graphics Rendering System**
   - Image format support (JPEG, BMP)
   - Hotspot overlay rendering
   - Multi-resolution support

3. **Audio/Video Decoders**
   - MPEG-1 video/audio (VCD)
   - MPEG-2 video/audio (SVCD)
   - CD-quality audio (CD-I)

4. **File System Support**
   - ISO 9660 mounting and navigation
   - Multi-folder structure handling
   - Binary file parsing (.VCD, .SVD)

5. **Navigation Engine**
   - Chapter point management
   - Sequence control and branching
   - Timestamp-based seeking

---

## Conclusion

VCD, SVCD, and CD-I formats represent a progression from basic video playback to advanced interactive content:

**VCD**: Basic CD-I menu system with hotspot navigation and MPEG-1 playback
**SVCD**: Enhanced video quality with MPEG-2 and multi-audio track support
**CD-I**: Full interactive applications with advanced graphics and audio

The implementation complexity increases significantly with CD-I, requiring a complete interactive runtime environment. VCD and SVCD provide more manageable starting points for basic video disc support, while CD-I represents the most advanced interactive disc format requiring comprehensive application runtime support.

Key implementation priorities:
1. **UCM coordinate system** for hotspot handling
2. **MPEG-1/MPEG-2 decoders** for video playback
3. **ENTRIES file parsing** for chapter navigation
4. **CD-I application runtime** for interactive content
