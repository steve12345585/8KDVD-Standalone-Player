/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
 *
 * Authors: Benjamin Arnaud <bunjee@omega.gg>
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


import VLC.Player
import VLC.Widgets as Widgets
import VLC.Style

Widgets.IconToolButton {
    id: root

    signal requestLockUnlockAutoHide(bool lock)

    text: VLCIcons.tv

    description: qsTr("Programs")

    // NOTE: We want to pop the menu above the button.
    onClicked: menu.popup(this.mapToGlobal(0, 0), true)

    enabled: (paintOnly === false && Player.hasPrograms)

    QmlProgramMenu {
        id: menu

        player: Player
        ctx: MainCtx

        onAboutToShow: root.requestLockUnlockAutoHide(true)
        onAboutToHide: root.requestLockUnlockAutoHide(false)
    }

    function forceUnlock() {
        if(menu)
            menu.close()
    }
}
