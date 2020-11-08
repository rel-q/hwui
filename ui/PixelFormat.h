/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//

// Pixel formats used across the system.
// These formats might not all be supported by all renderers, for instance
// skia or SurfaceFlinger are not required to support all of these formats
// (either as source or destination)


#ifndef UI_PIXELFORMAT_H
#define UI_PIXELFORMAT_H

namespace android {
typedef enum {
    HAL_PIXEL_FORMAT_RGBA_8888 = 1,
    HAL_PIXEL_FORMAT_RGBX_8888 = 2,
    HAL_PIXEL_FORMAT_RGB_888 = 3,
    HAL_PIXEL_FORMAT_RGB_565 = 4,
    HAL_PIXEL_FORMAT_BGRA_8888 = 5,
    HAL_PIXEL_FORMAT_RGBA_1010102 = 43, // 0x2B
    HAL_PIXEL_FORMAT_RGBA_FP16 = 22, // 0x16
    HAL_PIXEL_FORMAT_YV12 = 842094169, // 0x32315659
    HAL_PIXEL_FORMAT_Y8 = 538982489, // 0x20203859
    HAL_PIXEL_FORMAT_Y16 = 540422489, // 0x20363159
    HAL_PIXEL_FORMAT_RAW16 = 32, // 0x20
    HAL_PIXEL_FORMAT_RAW10 = 37, // 0x25
    HAL_PIXEL_FORMAT_RAW12 = 38, // 0x26
    HAL_PIXEL_FORMAT_RAW_OPAQUE = 36, // 0x24
    HAL_PIXEL_FORMAT_BLOB = 33, // 0x21
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 34, // 0x22
    HAL_PIXEL_FORMAT_YCBCR_420_888 = 35, // 0x23
    HAL_PIXEL_FORMAT_YCBCR_422_888 = 39, // 0x27
    HAL_PIXEL_FORMAT_YCBCR_444_888 = 40, // 0x28
    HAL_PIXEL_FORMAT_FLEX_RGB_888 = 41, // 0x29
    HAL_PIXEL_FORMAT_FLEX_RGBA_8888 = 42, // 0x2A
    HAL_PIXEL_FORMAT_YCBCR_422_SP = 16, // 0x10
    HAL_PIXEL_FORMAT_YCRCB_420_SP = 17, // 0x11
    HAL_PIXEL_FORMAT_YCBCR_422_I = 20, // 0x14
    HAL_PIXEL_FORMAT_JPEG = 256, // 0x100
} android_pixel_format_t;


enum {
    //
    // these constants need to match those
    // in graphics/PixelFormat.java & pixelflinger/format.h
    //
    PIXEL_FORMAT_UNKNOWN    =   0,
    PIXEL_FORMAT_NONE       =   0,

    // logical pixel formats used by the SurfaceFlinger -----------------------
    PIXEL_FORMAT_CUSTOM         = -4,
        // Custom pixel-format described by a PixelFormatInfo structure

    PIXEL_FORMAT_TRANSLUCENT    = -3,
        // System chooses a format that supports translucency (many alpha bits)

    PIXEL_FORMAT_TRANSPARENT    = -2,
        // System chooses a format that supports transparency
        // (at least 1 alpha bit)

    PIXEL_FORMAT_OPAQUE         = -1,
        // System chooses an opaque format (no alpha bits required)

    // real pixel formats supported for rendering -----------------------------

    PIXEL_FORMAT_RGBA_8888    = HAL_PIXEL_FORMAT_RGBA_8888,    // 4x8-bit RGBA
    PIXEL_FORMAT_RGBX_8888    = HAL_PIXEL_FORMAT_RGBX_8888,    // 4x8-bit RGB0
    PIXEL_FORMAT_RGB_888      = HAL_PIXEL_FORMAT_RGB_888,      // 3x8-bit RGB
    PIXEL_FORMAT_RGB_565      = HAL_PIXEL_FORMAT_RGB_565,      // 16-bit RGB
    PIXEL_FORMAT_BGRA_8888    = HAL_PIXEL_FORMAT_BGRA_8888,    // 4x8-bit BGRA
    PIXEL_FORMAT_RGBA_5551    = 6,                             // 16-bit ARGB
    PIXEL_FORMAT_RGBA_4444    = 7,                             // 16-bit ARGB
    PIXEL_FORMAT_RGBA_FP16    = HAL_PIXEL_FORMAT_RGBA_FP16,    // 64-bit RGBA
    PIXEL_FORMAT_RGBA_1010102 = HAL_PIXEL_FORMAT_RGBA_1010102, // 32-bit RGBA
};

typedef int32_t PixelFormat;

uint32_t bytesPerPixel(PixelFormat format);
uint32_t bitsPerPixel(PixelFormat format);

}; // namespace android

#endif // UI_PIXELFORMAT_H
