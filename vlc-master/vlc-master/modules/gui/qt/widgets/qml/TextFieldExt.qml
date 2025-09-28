/*****************************************************************************
 * Copyright (C) 2022 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

import QtQuick
import QtQuick.Controls


import VLC.Style

TextField {
    id: control

    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.TextField

        focused: control.activeFocus
        hovered: control.hovered
        enabled: control.enabled
    }

    selectedTextColor : theme.fg.highlight
    selectionColor : theme.bg.highlight
    color : theme.fg.primary
    placeholderTextColor: theme.fg.secondary

    font.pixelSize: VLCStyle.fontSize_normal

    verticalAlignment: Text.AlignVCenter

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        border.width: control.enabled ? VLCStyle.dp(2, VLCStyle.scale) : 0
        color: theme.bg.primary
        border.color: theme.border
    }

}
