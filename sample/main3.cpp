#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkGradientShader.h>
#include <SkImageGenerator.h>
#include <SkTypeface.h>
#include <SkPixelRef.h>
#include <SkMallocPixelRef.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <renderthread/RenderThread.h>
#include <renderthread/RenderProxy.h>
#include <hwui/Canvas.h>
#include <hwui/Typeface.h>
#include <hwui/Paint.h>

#include <unicode/uchar.h>
#include <src/RecordingCanvas.h>

#include "sample/SampleApplication.h"
#include "Matrix.h"
#include <algorithm>
#include <SkGeometry.h>
#include <SkBlurDrawLooper.h>


#define WIDTH 800
#define HEIGHT 800

using namespace android::uirenderer;
typedef Matrix4 mat4;

class Sample : public SampleApplication {
public:
    Sample(int argc, char** argv)
            : SampleApplication("GLES-2.0", argc, argv, 2, 0, WIDTH, HEIGHT) {
    }

    bool initialize() override {

        mRootNode = new android::uirenderer::RenderNode();

        if (mRootNode->mutateStagingProperties().setLeftTopRightBottom(0, 0, WIDTH, HEIGHT)) {
            // mRootNode->mutateStagingProperties().setTranslationZ(100);
            mRootNode->setPropertyFieldsDirty(RenderNode::X | RenderNode::Y | RenderNode::TRANSLATION_Z);
        }
        mRootNode->setName("Root");
        // mRootNode->mutateStagingProperties().mutableOutline().setRoundRect(0, 0, 500, 500, 10, 1);
        // mRootNode->mutateStagingProperties().mutableOutline().setShouldClip(true);
        // mRootNode->mutateStagingProperties().mutateLayerProperties().setType(LayerType::RenderLayer);
        // mRootNode->mutateStagingProperties().mutateLayerProperties().setOpaque(true);

        auto canvas = static_cast<android::uirenderer::RecordingCanvas*>
        (android::Canvas::create_recording_canvas(WIDTH, HEIGHT, mRootNode));
        canvas->drawColor(SK_ColorWHITE, SkBlendMode::kSrc);


        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFF6F00);
        // mRootNode->mutateStagingProperties().setClipToBounds(true);
        // mRootNode->mutateStagingProperties().setClipBounds(Rect(0, 0, 100, 100));
        // mRootNode->mutateStagingProperties().mutableRevealClip().set(true, 100, 100, 50);

        constexpr int left = 30;
        constexpr int top = 400;
        constexpr int width = 200;
        constexpr int height = 200;
        mChildNode = new android::uirenderer::RenderNode();
        if (mChildNode->mutateStagingProperties().setLeftTopRightBottom(left, top, left + width, top + height)) {
            mChildNode->setPropertyFieldsDirty(RenderNode::X | RenderNode::Y);
        }
        mChildNode->mutateStagingProperties().setZ(20);
        mChildNode->mutateStagingProperties().mutableOutline().setRoundRect(0, 0, width, height, 0, 1);
        mChildNode->setName("Child");

        {
            auto canvas = std::unique_ptr<android::uirenderer::RecordingCanvas>(
                    reinterpret_cast<android::uirenderer::RecordingCanvas*>(android::Canvas::create_recording_canvas(
                            width, height)));

            canvas->drawColor(0xff03dac6, SkBlendMode::kSrc);
            DisplayList* displayList = canvas->finishRecording();
            mChildNode->setStagingDisplayList(displayList);
        }
        canvas->insertReorderBarrier(true);
        canvas->drawRenderNode(mChildNode);
        canvas->insertReorderBarrier(false);

        DisplayList* displayList = canvas->finishRecording();
        mRootNode->setStagingDisplayList(displayList);

        mProxy = new android::uirenderer::renderthread::RenderProxy(false, mRootNode);
        mOSWindow->initialize(mName, mWidth, mHeight);
        mOSWindow->setVisible(true);
        mOSWindow->resetNativeWindow();
        mProxy->initialize(mOSWindow->getNativeWindow());
        mProxy->setLightCenter({WIDTH / 2, 0, 900});
        mProxy->setup(1200, 20, 78);
        mProxy->syncAndDrawFrame();

        return true;
    }

    void destroy() override {
    }

    void draw() override {
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        ATRACE_CALL();
        r -= 3;
        // mProxy->setup(r, 20, 78);
        // x+=3;
        // mChildNode->mutateStagingProperties().setTranslationX(x);
        // mChildNode->setPropertyFieldsDirty(RenderNode::TRANSLATION_X);
        mProxy->syncAndDrawFrame();
        usleep(100 * 1000);
    }

private:
    android::uirenderer::renderthread::RenderProxy* mProxy;
    android::uirenderer::RenderNode* mRootNode;
    android::uirenderer::RenderNode* mChildNode;
    int x = 0;
    int r = 1200;
};

int main(int argc, char** argv) {
    android::Typeface::setRobotoTypefaceForTest();
    printf("start\n");
    Sample app(argc, argv);
    app.run();
    return 0;
}
