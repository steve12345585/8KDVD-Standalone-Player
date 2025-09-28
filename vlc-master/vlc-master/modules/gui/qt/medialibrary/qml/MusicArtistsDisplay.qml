/*****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
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
import QtQuick.Controls
import QtQuick
import QtQml.Models
import QtQuick.Layouts

import VLC.MediaLibrary

import VLC.Util
import VLC.Widgets as Widgets
import VLC.MainInterface
import VLC.Style


Widgets.PageLoader {
    id: root

    pageModel: [{
        name: "all",
        default: true,
        component: allArtistsComponent
    }, {
        name: "albums",
        component: artistAlbumsComponent
    }]

    property int displayMarginBeginning: 0
    property int displayMarginEnd: 0

    property bool enableBeginningFade: true
    property bool enableEndFade: true

    Component {
        id: allArtistsComponent

        MusicAllArtists {
            id: artistsView

            header: Widgets.ViewHeader {
                view: artistsView

                visible: view.count > 0

                text: qsTr("Artists")
            }

            searchPattern: MainCtx.search.pattern
            sortOrder: MainCtx.sort.order
            sortCriteria: MainCtx.sort.criteria

            displayMarginBeginning: root.displayMarginBeginning
            displayMarginEnd: root.displayMarginEnd

            enableBeginningFade: root.enableBeginningFade
            enableEndFade: root.enableEndFade

            onCurrentIndexChanged: History.viewProp.initialIndex = currentIndex

            onArtistAlbumViewRequested: (id, reason) => {
                History.push([...root.pagePrefix, "albums"], { artistId: id  }, reason)
            }
        }
    }

    Component {
        id: artistAlbumsComponent

        MusicArtistsAlbums {
            searchPattern: MainCtx.search.pattern
            sortOrder: MainCtx.sort.order
            sortCriteria: MainCtx.sort.criteria

            displayMarginBeginning: root.displayMarginBeginning
            displayMarginEnd: root.displayMarginEnd

            enableBeginningFade: root.enableBeginningFade
            enableEndFade: root.enableEndFade

            onArtistIdChanged: History.viewProp.artistId = artistId
            onCurrentAlbumIndexChanged: History.viewProp.initialAlbumIndex = currentAlbumIndex
        }
    }
}
