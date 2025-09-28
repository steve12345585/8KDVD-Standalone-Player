
/*****************************************************************************
 * Copyright (C) 2020 VLC authors and VideoLAN
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

import VLC.Style
import VLC.Util

DoubleShadow {
    id: root

    opacity: 0.62

    primaryVerticalOffset: VLCStyle.dp(1, VLCStyle.scale)
    primaryBlurRadius: VLCStyle.dp(3, VLCStyle.scale)

    secondaryVerticalOffset: VLCStyle.dp(6, VLCStyle.scale)
    secondaryBlurRadius: VLCStyle.dp(14, VLCStyle.scale)

    z: -1
}
