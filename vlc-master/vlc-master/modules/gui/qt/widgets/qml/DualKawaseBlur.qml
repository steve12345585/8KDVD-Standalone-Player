/*****************************************************************************
 * Copyright (C) 2025 VLC authors and VideoLAN
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
import QtQuick.Window

import VLC.Util

// This item provides the novel "Dual Kawase" effect [1], which offers a very ideal
// balance of quality and performance. It has been used by many applications since
// its introduction in 2015, including the KWin compositor. Qt 5's `FastBlur` from
// 2011, and its Qt 6 port `MultiEffect` already offers a performant blur effect,
// utilizing a similar down/up sampling trick, but it does not use the half-pixel
// trick, and does not work for textures in the atlas, or sub-textures (detaching
// or additional layer incurs an additional buffer).
// [1] SIGGRAPH 2015, "Bandwidth Efficient Rendering", Marius Bjorge (ARM).
Item {
    id: root

    implicitWidth: source ? Math.min(source.paintedWidth ?? Number.MAX_VALUE, source.width) : 0
    implicitHeight: source ? Math.min(source.paintedHeight ?? Number.MAX_VALUE, source.height) : 0

    enum Configuration {
        FourPass, // 2 downsample + 2 upsamples (3 layers/buffers)
        TwoPass // 1 downsample + 1 upsample (1 layer/buffer)
    }

    /// <postprocess>
    // The following property must be set in order to make other properties respected:
    property bool postprocess: false

    property alias tint: us2.tint
    property alias tintStrength: us2.tintStrength
    property alias noiseStrength: us2.noiseStrength
    property alias exclusionStrength: us2.exclusionStrength
    /// </postprocess>

    property int configuration: DualKawaseBlur.Configuration.FourPass

    // NOTE: This property is also an optimization hint. When it is false, the
    //       intermediate buffers for the blur passes may be released (only
    //       the two intermediate layers in four pass mode, we must have one
    //       layer regardless of the mode, so optimization-wise it has no
    //       benefit in two pass mode thus should be used solely as behavior
    //       instead):
    property bool live: true

    // Do not hesitate to use an odd number for the radius, there is virtually
    // no difference between odd or even numbers due to the halfpixel trick.
    // The effective radius is always going to be a half-integer.
    property int radius: 1

    // NOTE: It seems that if SG accumulated opacity is lower than 1.0, blending is
    //       used even if it is set false here. For that reason, it should not be
    //       necessary to check for opacity (well, accumulated opacity can not be
    //       checked directly in QML anyway).
    property bool blending: (!sourceTextureIsValid || sourceTextureProviderObserver.hasAlphaChannel ||
                             (postprocess && (tintStrength > 0.0 && tint.a < 1.0)))

    // source must be a texture provider item. Some items such as `Image` and
    // `ShaderEffectSource` are inherently texture provider. Other items needs
    // layering with either `layer.enabled: true` or `ShaderEffectSource`.
    // We purposefully are not going to create a layer on behalf of the source
    // here, unlike `MultiEffect` (see `hasProxySource`), because it is impossible
    // to determine whether the new layer is actually wanted (when the source is
    // already a texture provider), and it is very trivial to have a layer when
    // it is wanted or necessary anyway.
    property Item source

    // Arbitrary sub-texturing (no need to be set for atlas textures):
    // `QSGTextureView` can also be used instead of sub-texturing here.
    property rect sourceRect

    property alias sourceTextureProviderObserver: ds1SourceObserver // for accessory

    readonly property bool sourceTextureIsValid: sourceTextureProviderObserver.isValid

    onSourceTextureIsValidChanged: {
        if (root.sourceTextureIsValid) {
            if (root._queuedScheduledUpdate) {
                root._queuedScheduledUpdate = false

                // Normally it should be fine to call `scheduleUpdate()` directly for
                // the initial layer, even though the subsequent layers must be chained
                // for update scheduling regardless, but old Qt seems to want it:
                root.scheduleUpdate(true)
            }
        }
    }

    property var /*QtWindow*/ _window: null // captured window used for chaining through `afterAnimating()`

    property bool _queuedScheduledUpdate: false

    function scheduleUpdate(onNextAfterAnimating /* : bool */ = false) {
        if (live)
            return // no-op

        if (!root.sourceTextureIsValid) {
            root._queuedScheduledUpdate = true // if source texture is not valid, delay the update until valid
            return
        }

        if (root._window) {
            // One possible case for this is that the mipmaps for the source texture were generated too fast, and
            // the consumer wants to update the blur to make use of the mipmaps before the blur finished chained
            // updates for the previous source texture which is the non-mipmapped version of the same texture.
            console.debug(root, "scheduleUpdate(): There is an already ongoing chained update, re-scheduling...")
            root._queuedScheduledUpdate = true
            return
        }

        root._window = root.Window.window
        if (onNextAfterAnimating) {
            root._window.afterAnimating.connect(ds1layer, ds1layer.scheduleChainedUpdate)
        } else {
            ds1layer.scheduleChainedUpdate()
        }
    }

    onLiveChanged: {
        if (live) {
            ds1layer.parent = root
            ds2layer.inhibitParent = false
        } else {
            root.scheduleUpdate(false) // this triggers releasing intermediate layers (when applicable)
        }
    }
    
    // TODO: Get rid of this in favor of GLSL 1.30's `textureSize()`
    Connections {
        target: root.Window.window
        enabled: root.visible

        function onAfterAnimating() {
            // Sampling point for getting the native texture sizes:
            // This is emitted from the GUI thread.

            // Unlike high resolution timer widget, we should not
            // need to explicitly schedule update here, because if
            // an update is necessary, it should have been scheduled
            // implicitly (due to source texture provider's signal
            // `textureChanged()`).

            ds1.sourceTextureSize = ds1SourceObserver.nativeTextureSize
            ds2.sourceTextureSize = ds2SourceObserver.nativeTextureSize
            us1.sourceTextureSize = us1SourceObserver.nativeTextureSize
            us2.sourceTextureSize = us2SourceObserver.nativeTextureSize

            // It is not clear if `ShaderEffect` updates the uniform
            // buffer after `afterAnimating()` signal but before the
            // next frame. This is important because if `ShaderEffect`
            // updates the uniform buffer during item polish, we already
            // missed it here (`afterAnimating()` is signalled afterward).
            // However, we can call `ensurePolished()` slot to ask for
            // re-polish, which in case the `ShaderEffect` should now
            // consider the new values. If it does not exist (Qt 6.2),
            // we will rely on the next frame in worst case, which
            // should be fine as long as the size does not constantly
            // change in each frame.
            if (ds1.ensurePolished)
            {
                // No need to check for if such slot exists for each,
                // this is basically Qt version check in disguise.
                ds1.ensurePolished()
                ds2.ensurePolished()
                us1.ensurePolished()
                us2.ensurePolished()
            }
        }
    }

    ShaderEffect {
        id: ds1

        // When downsampled, we can decrease the size here so that the layer occupies less VRAM:
        width: parent.width / 2
        height: parent.height / 2

        readonly property Item source: root.source

        // TODO: Instead of normalizing here, we could use GLSL 1.30's `textureSize()`
        //       and normalize in the vertex shader, but we can not because we are
        //       targeting GLSL 1.20/ESSL 1.0, even though the shader is written in
        //       GLSL 4.40.
        readonly property rect normalRect: (root.sourceRect.width > 0.0 && root.sourceRect.height > 0.0) ? Qt.rect(root.sourceRect.x / sourceTextureSize.width,
                                                                                                                   root.sourceRect.y / sourceTextureSize.height,
                                                                                                                   root.sourceRect.width / sourceTextureSize.width,
                                                                                                                   root.sourceRect.height / sourceTextureSize.height)
                                                                                                         : Qt.rect(0.0, 0.0, 0.0, 0.0)

        readonly property int radius: root.radius

        // TODO: We could use `textureSize()` and get rid of this, but we
        //       can not because we are targeting GLSL 1.20/ESSL 1.0, even
        //       though the shader is written in GLSL 4.40.
        TextureProviderObserver {
            id: ds1SourceObserver
            source: ds1.source
        }

        property size sourceTextureSize

        // cullMode: ShaderEffect.BackFaceCulling // QTBUG-136611 (Layering breaks culling with OpenGL)

        fragmentShader: "qrc:///shaders/DualKawaseBlur_downsample.frag.qsb"
        // Maybe we should have vertex shader unconditionally, and calculate the half pixel there instead of fragment shader?
        vertexShader: (normalRect.width > 0.0 && normalRect.height > 0.0) ? "qrc:///shaders/SubTexture.vert.qsb"
                                                                          : ""

        visible: false

        supportsAtlasTextures: true

        blending: root.blending
    }

    ShaderEffectSource {
        id: ds1layer

        sourceItem: ds1
        visible: false
        smooth: true

        live: root.live

        function scheduleChainedUpdate() {
            if (!ds1layer) // context is lost, Qt bug (reproduced with 6.2)
                return

            // Common for both four and two pass mode:
            ds1layer.parent = root
            ds1layer.scheduleUpdate()

            if (root._window) {
                root._window.afterAnimating.disconnect(ds1layer, ds1layer.scheduleChainedUpdate)

                // In four pass mode, we can release the two intermediate layers:
                if (root.configuration === DualKawaseBlur.Configuration.FourPass) {
                    // Scheduling update must be done sequentially for each layer in
                    // a chain. It seems that each layer needs one frame for it to be
                    // used as a source in another layer, so we can not schedule
                    // update for each layer at the same time:
                    root._window.afterAnimating.connect(ds2layer, ds2layer.scheduleChainedUpdate)
                } else {
                    root._window = null
                }
            }
        }
    }

    ShaderEffect {
        id: ds2

        // When downsampled, we can decrease the size here so that the layer occupies less VRAM:
        width: ds1.width / 2
        height: ds1.height / 2

        // Qt uses reference counting, otherwise ds1layer may not be released, even if it has no parent (see `QQuickItemPrivate::derefWindow()`):
        readonly property Item source: ((root.configuration === DualKawaseBlur.Configuration.TwoPass) || !ds1layer.parent) ? null : ds1layer
        property rect normalRect // not necessary here, added because of the warning
        readonly property int radius: root.radius

        // TODO: We could use `textureSize()` and get rid of this, but we
        //       can not because we are targeting GLSL 1.20/ESSL 1.0, even
        //       though the shader is written in GLSL 4.40.
        TextureProviderObserver {
            id: ds2SourceObserver
            source: ds2.source
        }

        property size sourceTextureSize

        // cullMode: ShaderEffect.BackFaceCulling // QTBUG-136611 (Layering breaks culling with OpenGL)

        visible: false

        fragmentShader: source ? "qrc:///shaders/DualKawaseBlur_downsample.frag.qsb" : "" // to prevent warning if source becomes null

        supportsAtlasTextures: true

        blending: root.blending
    }

    ShaderEffectSource {
        id: ds2layer

        // So that if configuration is two pass (this is not used), the buffer is released:
        // This is mainly relevant for switching configuration case, as initially if this was
        // never visible and was never used as texture provider, it should have never allocated
        // resources to begin with.
        sourceItem: (root.configuration === DualKawaseBlur.Configuration.FourPass) ? ds2 : null
        parent: (!inhibitParent && sourceItem) ? root : null // this seems necessary to release resources even if sourceItem becomes null (non-live case)

        visible: false
        smooth: true

        live: root.live

        property bool inhibitParent: false

        function scheduleChainedUpdate() {
            if (!ds2layer) // context is lost, Qt bug (reproduced with 6.2)
                return

            ds2layer.inhibitParent = false
            ds2layer.scheduleUpdate()

            if (root._window) {
                root._window.afterAnimating.disconnect(ds2layer, ds2layer.scheduleChainedUpdate)
                root._window.afterAnimating.connect(us1layer, us1layer.scheduleChainedUpdate)
            }
        }
    }

    ShaderEffect {
        id: us1

        width: ds2.width * 2
        height: ds2.height * 2

        // Qt uses reference counting, otherwise ds2layer may not be released, even if it has no parent (see `QQuickItemPrivate::derefWindow()`):
        readonly property Item source: ((root.configuration === DualKawaseBlur.Configuration.TwoPass) || !ds2layer.parent) ? null : ds2layer
        property rect normalRect // not necessary here, added because of the warning
        readonly property int radius: root.radius

        // TODO: We could use `textureSize()` and get rid of this, but we
        //       can not because we are targeting GLSL 1.20/ESSL 1.0, even
        //       though the shader is written in GLSL 4.40.
        TextureProviderObserver {
            id: us1SourceObserver
            source: us1.source
        }

        property size sourceTextureSize

        // cullMode: ShaderEffect.BackFaceCulling // QTBUG-136611 (Layering breaks culling with OpenGL)

        visible: false

        fragmentShader: source ? "qrc:///shaders/DualKawaseBlur_upsample.frag.qsb" : "" // to prevent warning if source becomes null

        supportsAtlasTextures: true

        blending: root.blending
    }

    ShaderEffectSource {
        id: us1layer

        // So that if configuration is two pass (this is not used), the buffer is released:
        // This is mainly relevant for switching configuration case, as initially if this was
        // never visible and was never used as texture provider, it should have never allocated
        // resources to begin with.
        sourceItem: (root.configuration === DualKawaseBlur.Configuration.FourPass) ? us1 : null
        parent: sourceItem ? root : null // this seems necessary to release resources even if sourceItem becomes null (non-live case)

        visible: false
        smooth: true

        live: root.live

        function scheduleChainedUpdate() {
            if (!us1layer) // context is lost, Qt bug (reproduced with 6.2)
                return

            us1layer.scheduleUpdate()

            if (root._window) {
                root._window.afterAnimating.disconnect(us1layer, us1layer.scheduleChainedUpdate)
                root._window.afterAnimating.connect(us1layer, us1layer.releaseResourcesOfIntermediateLayers)
            }
        }

        function releaseResourcesOfIntermediateLayers() {
            if (!ds1layer || !ds2layer) // context is lost, Qt bug (reproduced with 6.2)
                return

            // Last layer is updated, now it is time to release the intermediate buffers:
            console.debug(root, ": releasing intermediate layers, expect the video memory consumption to drop.")

            // https://doc.qt.io/qt-6/qquickitem.html#graphics-resource-handling
            ds1layer.parent = null
            ds2layer.inhibitParent = true

            if (root._window) {
                root._window.afterAnimating.disconnect(us1layer, us1layer.releaseResourcesOfIntermediateLayers)
                root._window = null
            }

            if (root._queuedScheduledUpdate) {
                // Tried calling `scheduleUpdate()` before the ongoing chained updates completed.
                root._queuedScheduledUpdate = false
                root.scheduleUpdate(false)
            }
        }
    }

    ShaderEffect {
        id: us2

        anchors.fill: parent // {us1/ds1}.size * 2

        readonly property Item source: (root.configuration === DualKawaseBlur.Configuration.TwoPass) ? ds1layer : us1layer
        property rect normalRect // not necessary here, added because of the warning
        readonly property int radius: root.radius

        // TODO: We could use `textureSize()` and get rid of this, but we
        //       can not because we are targeting GLSL 1.20/ESSL 1.0, even
        //       though the shader is written in GLSL 4.40.
        TextureProviderObserver {
            id: us2SourceObserver
            source: us2.source
        }

        property size sourceTextureSize

        // cullMode: ShaderEffect.BackFaceCulling // QTBUG-136611 (Layering breaks culling with OpenGL)

        property color tint: "transparent"
        property real tintStrength: 0.0
        property real noiseStrength: 0.0
        property real exclusionStrength: 0.0

        fragmentShader: root.postprocess ? "qrc:///shaders/DualKawaseBlur_upsample_postprocess.frag.qsb"
                                         : "qrc:///shaders/DualKawaseBlur_upsample.frag.qsb"

        supportsAtlasTextures: true

        blending: root.blending
    }
}
