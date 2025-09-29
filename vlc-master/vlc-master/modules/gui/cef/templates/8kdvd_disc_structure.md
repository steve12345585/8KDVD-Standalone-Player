# 8KDVD Disc Structure and Menu Population

## Disc Structure
```
8KDVD_DISC/
├── 8KDVD_TS/
│   ├── index.html          # Main menu template
│   ├── settings.html       # Settings menu template  
│   ├── about.html          # About menu template
│   ├── style.css           # Menu styling
│   ├── script.js           # Menu JavaScript
│   └── assets/             # Menu assets (images, fonts, etc.)
├── PAYLOAD_*.evo8          # 8K video files
├── AUDIO_*.evo8            # Audio tracks
├── SUBTITLE_*.evo8         # Subtitle tracks
└── METADATA/
    ├── disc_info.xml       # Disc metadata
    ├── titles.xml          # Title information
    ├── chapters.xml        # Chapter information
    └── settings.xml        # Default settings
```

## XML Data Sources

### 1. disc_info.xml
```xml
<?xml version="1.0" encoding="UTF-8"?>
<disc_info>
    <title>8KDVD Sample Disc</title>
    <version>1.0</version>
    <region>ALL</region>
    <language>en</language>
    <resolution>7680x4320</resolution>
    <frame_rate>60</frame_rate>
    <hdr_enabled>true</hdr_enabled>
    <dolby_vision_enabled>true</dolby_vision_enabled>
    <audio_tracks>
        <track id="1" language="en" codec="opus" channels="8"/>
        <track id="2" language="es" codec="opus" channels="8"/>
    </audio_tracks>
    <subtitle_tracks>
        <track id="1" language="en"/>
        <track id="2" language="es"/>
    </subtitle_tracks>
</disc_info>
```

### 2. titles.xml
```xml
<?xml version="1.0" encoding="UTF-8"?>
<titles>
    <title id="1">
        <name>Main Feature</name>
        <description>8K Ultra High Definition Feature Film</description>
        <duration>7200</duration>
        <thumbnail>thumbnails/title_1.jpg</thumbnail>
        <chapters>
            <chapter id="1" name="Chapter 1" start_time="0"/>
            <chapter id="2" name="Chapter 2" start_time="1800"/>
        </chapters>
    </title>
    <title id="2">
        <name>Behind the Scenes</name>
        <description>Making of documentary</description>
        <duration>1800</duration>
        <thumbnail>thumbnails/title_2.jpg</thumbnail>
    </title>
</titles>
```

### 3. chapters.xml
```xml
<?xml version="1.0" encoding="UTF-8"?>
<chapters>
    <chapter id="1" title_id="1">
        <name>Opening Credits</name>
        <start_time>0</start_time>
        <end_time>300</end_time>
        <thumbnail>thumbnails/chapter_1.jpg</thumbnail>
    </chapter>
    <chapter id="2" title_id="1">
        <name>Main Story</name>
        <start_time>300</start_time>
        <end_time>1800</end_time>
        <thumbnail>thumbnails/chapter_2.jpg</thumbnail>
    </chapter>
</chapters>
```

## Menu Population Process

### 1. Template Loading
The HTML templates are loaded as base templates with placeholder data.

### 2. XML Parsing
The C++ code parses the XML files and extracts:
- Disc information
- Title information  
- Chapter information
- Audio/subtitle tracks
- Settings

### 3. Data Injection
The parsed data is injected into the HTML templates via:
- JavaScript variables
- DOM manipulation
- Template replacement

### 4. Dynamic Content Generation
The JavaScript code dynamically generates menu items based on the XML data.
