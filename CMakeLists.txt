cmake_minimum_required(VERSION 3.13)
project(hwui)

set(CMAKE_CXX_STANDARD 14)

set(SKIA_PATH ~/work/gitlab/skia)
set(ANGLE_PATH ~/work/github/angle)

set(SKIA_INCLUDES
        ${SKIA_PATH}/include/atlastext
        ${SKIA_PATH}/include/c
        ${SKIA_PATH}/include/codec
        ${SKIA_PATH}/include/config
        ${SKIA_PATH}/include/core
        ${SKIA_PATH}/include/docs
        ${SKIA_PATH}/include/effects
        ${SKIA_PATH}/include/encode
        ${SKIA_PATH}/include/gpu
        ${SKIA_PATH}/src/gpu
        ${SKIA_PATH}/include/pathops
        ${SKIA_PATH}/include/ports
        ${SKIA_PATH}/include/private
        ${SKIA_PATH}/include/svg
        ${SKIA_PATH}/include/third_party
        ${SKIA_PATH}/include/utils
        ${SKIA_PATH}/src/core
        ${SKIA_PATH}/src/image
        ${SKIA_PATH}/src/images
        ${SKIA_PATH}/tools
        ${SKIA_PATH}/third_party/externals/harfbuzz/src
        ${SKIA_PATH}/third_party/harfbuzz
        ${SKIA_PATH}/third_party/externals/icu/source/common
        ${SKIA_PATH}/third_party/externals/icu/source/i18n
        )

set(ANGLE_INCLUDES
        ${ANGLE_PATH}/include
        ${ANGLE_PATH}/src)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ANGLE_INCLUDES})
include_directories(${SKIA_INCLUDES})

include_directories(src)
include_directories(src/debug)
include_directories(src/font)
include_directories(src/hwui)
include_directories(src/mediatek)
include_directories(src/protos)
include_directories(src/renderstate)
include_directories(src/renderthread)
include_directories(src/thread)
include_directories(src/utils)

# minikin
set(MINIKIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/minikin")
set(MINIKIN_SRC
        ${MINIKIN_DIR}/CmapCoverage.cpp
        ${MINIKIN_DIR}/Emoji.cpp
        ${MINIKIN_DIR}/FontCollection.cpp
        ${MINIKIN_DIR}/FontFamily.cpp
        ${MINIKIN_DIR}/FontLanguage.cpp
        ${MINIKIN_DIR}/FontLanguageListCache.cpp
        ${MINIKIN_DIR}/FontUtils.cpp
        ${MINIKIN_DIR}/GraphemeBreak.cpp
        ${MINIKIN_DIR}/HbFontCache.cpp
        ${MINIKIN_DIR}/Hyphenator.cpp
        ${MINIKIN_DIR}/Layout.cpp
        ${MINIKIN_DIR}/LayoutUtils.cpp
        ${MINIKIN_DIR}/LineBreaker.cpp
        ${MINIKIN_DIR}/Measurement.cpp
        ${MINIKIN_DIR}/MinikinFont.cpp
        ${MINIKIN_DIR}/MinikinInternal.cpp
        ${MINIKIN_DIR}/SparseBitSet.cpp
        ${MINIKIN_DIR}/WordBreaker.cpp
        )
include_directories(${MINIKIN_DIR})

add_executable(hwui
        log/log.c
        ui/Rect.cpp
        ui/Region.cpp
        ${MINIKIN_SRC}
        src/hwui/Bitmap.cpp
        src/font/CacheTexture.cpp
        src/font/Font.cpp
        src/hwui/Canvas.cpp
        src/hwui/MinikinSkia.cpp
        src/hwui/MinikinUtils.cpp
        src/hwui/PaintImpl.cpp
        src/hwui/Typeface.cpp
        src/renderstate/Blend.cpp
        src/renderstate/MeshState.cpp
        src/renderstate/OffscreenBufferPool.cpp
        src/renderstate/PixelBufferState.cpp
        src/renderstate/RenderState.cpp
        src/renderstate/Scissor.cpp
        src/renderstate/Stencil.cpp
        src/renderstate/TextureState.cpp
        src/renderthread/CanvasContext.cpp
        src/renderthread/OpenGLPipeline.cpp
        src/renderthread/DrawFrameTask.cpp
        src/renderthread/EglManager.cpp
        src/renderthread/RenderProxy.cpp
        src/renderthread/RenderTask.cpp
        src/renderthread/RenderThread.cpp
        src/renderthread/TimeLord.cpp
        src/renderthread/Frame.cpp
        src/thread/TaskManager.cpp
        src/utils/Blur.cpp
        src/utils/Color.cpp
        src/utils/GLUtils.cpp
        src/utils/LinearAllocator.cpp
        src/utils/StringUtils.cpp
        src/utils/VectorDrawableUtils.cpp
        src/AmbientShadow.cpp
        src/BakedOpDispatcher.cpp
        src/BakedOpRenderer.cpp
        src/BakedOpState.cpp
        src/Caches.cpp
        src/CanvasState.cpp
        src/ClipArea.cpp
        src/DamageAccumulator.cpp
        src/DeferredLayerUpdater.cpp
        src/DeviceInfo.cpp
        src/DisplayList.cpp
        src/Extensions.cpp
        src/FboCache.cpp
        src/FontRenderer.cpp
        src/FrameBuilder.cpp
        src/FrameInfo.cpp
        src/FrameInfoVisualizer.cpp
        src/GammaFontRenderer.cpp
        src/GlLayer.cpp
        src/GlopBuilder.cpp
        src/GpuMemoryTracker.cpp
        src/GradientCache.cpp
        src/Interpolator.cpp
        src/JankTracker.cpp
        src/Layer.cpp
        src/LayerBuilder.cpp
        src/LayerUpdateQueue.cpp
        src/Matrix.cpp
        src/OpDumper.cpp
        src/PathCache.cpp
        src/PathParser.cpp
        src/PathTessellator.cpp
        src/PixelBuffer.cpp
        src/ProfileRenderer.cpp
        src/Program.cpp
        src/ProgramCache.cpp
        src/Properties.cpp
        src/PropertyValuesHolder.cpp
        src/RecordingCanvas.cpp
        src/RenderBufferCache.cpp
        src/RenderNode.cpp
        src/RenderProperties.cpp
        src/ResourceCache.cpp
        src/ShadowTessellator.cpp
        src/SkiaShader.cpp
        src/Snapshot.cpp
        src/SpotShadow.cpp
        src/TessellationCache.cpp
        src/TextDropShadowCache.cpp
        src/Texture.cpp
        src/TextureCache.cpp
        src/VectorDrawable.cpp
        src/VkLayer.cpp
        utils/RefBase.cpp
        utils/safe_iop.c
        utils/SharedBuffer.cpp
        utils/Static.cpp
        utils/StopWatch.cpp
        utils/String8.cpp
        utils/String16.cpp
        utils/StrongPointer.cpp
        utils/Threads.cpp
        utils/Timers.cpp
        utils/Unicode.cpp
        utils/VectorImpl.cpp
        utils/JenkinsHash.cpp
        cutils/ashmem-host.c
        cutils/properties.cpp
        util/EGLWindow.cpp
        util/OSWindow.cpp
        util/Timer.cpp
        util/system_utils.cpp
        util/system_utils_mac.cpp
        util/system_utils_posix.cpp
        util/osx/OSXPixmap.mm
        util/osx/OSXWindow.mm
        sample/SampleApplication.cpp
        sample/main.cpp
        )


target_link_libraries(hwui
        GLESv2
        EGL
        skia
        harfbuzz
        harfbuzz-icu
        "-framework Cocoa"
        "-framework CoreGraphics"
        "-framework CoreText"
        "-framework OpenGL"
        "-framework CoreFoundation"
        "-framework MediaPlayer"
        "-framework QuartzCore"
        )


target_link_directories(
        hwui PUBLIC
        /usr/local/lib/
        ${SKIA_PATH}/out/Static
        ${ANGLE_PATH}/out/Debug
        ${UV_PATH}/
)

add_definitions(
        -DHWUI_NEW_OPS
        -fno-rtti
        -DEGL_EGLEXT_PROTOTYPES=1
        -DGL_GLES_PROTOTYPES=1
        -DEGL_EGL_PROTOTYPES=1
        -DANGLE_EGL_LIBRARY_NAME=\"libEGL\"
        -DANGLE_GLESV2_LIBRARY_NAME=\"libGLESv2\"
)
