/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
 *
 * Authors: Prince Gupta <guptaprince8832@gmail.com>
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


import VLC.Widgets
import VLC.Style


ViewBlockingRectangle {
    id: root

    readonly property bool usingAcrylic: visible && enabled && AcrylicController.enabled

    property color tintColor: "gray"

    property color alternativeColor: tintColor

    readonly property color _actualTintColor: tintColor.alpha(0.7)

    color: root._actualTintColor.tint(Qt.alpha(root.alternativeColor,
                                               1 - (usingAcrylic ? AcrylicController.uiTransluency : 0)))
}
