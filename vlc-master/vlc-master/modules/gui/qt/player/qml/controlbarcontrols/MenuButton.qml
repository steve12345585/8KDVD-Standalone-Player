/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
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


import VLC.MainInterface
import VLC.Widgets as Widgets
import VLC.Style
import VLC.Menus

Widgets.IconToolButton {
    id: menuBtn

    signal requestLockUnlockAutoHide(bool lock)

    text: VLCIcons.ellipsis
    description: qsTr("Menu")
    checked: contextMenu.shown

    onClicked: contextMenu.popup(this.mapToGlobal(0, 0))

    QmlGlobalMenu {
        id: contextMenu

        ctx: MainCtx
        playerViewVisible: History.match(History.viewPath, ["player"])

        onAboutToShow: menuBtn.requestLockUnlockAutoHide(true)
        onAboutToHide: menuBtn.requestLockUnlockAutoHide(false)
    }

    function forceUnlock() {
        if(contextMenu)
            contextMenu.close()
    }
}
