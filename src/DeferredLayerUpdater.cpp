/*
 * Copyright (C) 2014 The Android Open Source Project
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
#include "DeferredLayerUpdater.h"

#include "GlLayer.h"
#include "VkLayer.h"
#include "renderstate/RenderState.h"
#include "renderthread/EglManager.h"
#include "renderthread/RenderTask.h"
#include "utils/PaintUtils.h"

namespace android {
namespace uirenderer {

DeferredLayerUpdater::DeferredLayerUpdater(RenderState& renderState, CreateLayerFn createLayerFn,
        Layer::Api layerApi)
        : mRenderState(renderState)
        , mBlend(false)
        , mTransform(nullptr)
        , mGLContextAttached(false)
        , mUpdateTexImage(false)
        , mLayer(nullptr)
        , mLayerApi(layerApi)
        , mCreateLayerFn(createLayerFn) {
    renderState.registerDeferredLayerUpdater(this);
}

DeferredLayerUpdater::~DeferredLayerUpdater() {
    SkSafeUnref(mColorFilter);
    setTransform(nullptr);
    mRenderState.unregisterDeferredLayerUpdater(this);
    destroyLayer();
}

void DeferredLayerUpdater::destroyLayer() {
    if (!mLayer) {
        return;
    }

    mLayer->postDecStrong();
    mLayer = nullptr;
}

void DeferredLayerUpdater::setPaint(const SkPaint* paint) {
    mAlpha = PaintUtils::getAlphaDirect(paint);
    mMode = PaintUtils::getBlendModeDirect(paint);
    SkColorFilter* colorFilter = (paint) ? paint->getColorFilter() : nullptr;
    SkRefCnt_SafeAssign(mColorFilter, colorFilter);
}

void DeferredLayerUpdater::apply() {
    if (!mLayer) {
        mLayer = mCreateLayerFn(mRenderState, mWidth, mHeight, mColorFilter, mAlpha, mMode, mBlend);
    }

    mLayer->setColorFilter(mColorFilter);
    mLayer->setAlpha(mAlpha, mMode);
}

void DeferredLayerUpdater::doUpdateTexImage() {
}

void DeferredLayerUpdater::doUpdateVkTexImage() {
    LOG_ALWAYS_FATAL_IF(mLayer->getApi() != Layer::Api::Vulkan,
                        "updateLayer non Vulkan backend %x, GL %x, VK %x",
                        mLayer->getApi(), Layer::Api::OpenGL, Layer::Api::Vulkan);

    static const mat4 identityMatrix;
    updateLayer(false, identityMatrix.data);

    VkLayer* vkLayer = static_cast<VkLayer*>(mLayer);
    vkLayer->updateTexture();
}

void DeferredLayerUpdater::updateLayer(bool forceFilter, const float* textureTransform) {
    mLayer->setBlend(mBlend);
    mLayer->setForceFilter(forceFilter);
    mLayer->setSize(mWidth, mHeight);
    mLayer->getTexTransform().load(textureTransform);
}

void DeferredLayerUpdater::detachSurfaceTexture() {
}

} /* namespace uirenderer */
} /* namespace android */
