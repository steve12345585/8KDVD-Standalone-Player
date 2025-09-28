/*****************************************************************************
 * Copyright (C) 2022 VLC authors and VideoLAN
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
import QtQuick.Layouts


import VLC.MainInterface
import VLC.Style
import VLC.Widgets as Widgets

RowLayout {
    id: root

    // Properties

    default property alias content: content.data

    property int preferredWidth: VLCStyle.dp(512, VLCStyle.scale)

    //FIXME make TrackMenuController a proper type (see TrackMenu)
    required property QtObject trackMenuController

    // Settings

    spacing: 0

    focus: true

    Navigation.leftItem: button

    // Children

    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.Window
    }

    Item {
        Layout.preferredWidth: VLCStyle.dp(72, VLCStyle.scale)
        Layout.fillHeight: true

        Layout.topMargin: VLCStyle.margin_large

        Layout.alignment: Qt.AlignLeft | Qt.AlignTop

        Widgets.IconTrackButton {
            id: button

            anchors.horizontalCenter: parent.horizontalCenter

            description: qsTr("Back")
            text: VLCIcons.back

            Navigation.parentItem: root
            Navigation.rightItem: content

            onClicked: trackMenuController.requestBack()
        }
    }

    Rectangle {
        Layout.preferredWidth: VLCStyle.margin_xxxsmall
        Layout.fillHeight: true

        color: theme.border
    }

    FocusScope {
        id: content

        Layout.fillWidth: true
        Layout.fillHeight: true

        Layout.margins: VLCStyle.margin_large
    }
}
