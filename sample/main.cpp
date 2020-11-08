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

#include "sample/SampleApplication.h"
#include "Matrix.h"

#define WIDTH 1200
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
            mRootNode->setPropertyFieldsDirty(RenderNode::X | RenderNode::Y);
        }
        mRootNode->setName("Root");
        auto canvas = android::Canvas::create_recording_canvas(100, 100, mRootNode);
        canvas->drawColor(SK_ColorCYAN, SkBlendMode::kSrc);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFF6F00);
        canvas->drawRect(0, 0, 300, 300, paint);
        // canvas->drawCircle(500, 300, 100, paint);
        sk_sp<SkData> data = SkData::MakeFromFileName(
                "../res/image.jpeg");
        std::unique_ptr<SkImageGenerator> gen = SkImageGenerator::MakeFromEncoded(data);
        SkBitmap dst;
        bool result = dst.tryAllocPixels(gen->getInfo()) &&
                      gen->getPixels(gen->getInfo(), dst.getPixels(), dst.rowBytes());
        sk_sp<android::Bitmap> bmp = android::Bitmap::createFrom(dst.info(), *dst.pixelRef());
        canvas->drawBitmap(*(bmp.get()), 100, 100, &paint);

        auto icu_text = icu::UnicodeString::fromUTF8("Hello HWUI");
        // Translate and rotate
        canvas->translate(300, 300);
        // Draw the text:
        android::Paint p(paint);
        p.setTextSize(50);
        p.setColor(SK_ColorWHITE);

        canvas->drawText(icu_text.getBuffer(), 0, icu_text.length(), icu_text.length(), 0, 0, 0, p,
                         android::Typeface::resolveDefault(nullptr));
        DisplayList* displayList = canvas->finishRecording();
        mRootNode->setStagingDisplayList(displayList);
        mProxy = new android::uirenderer::renderthread::RenderProxy(false, mRootNode);
        mOSWindow->initialize(mName, mWidth, mHeight);
        mOSWindow->setVisible(true);
        mOSWindow->resetNativeWindow();
        mProxy->initialize(mOSWindow->getNativeWindow());

        mProxy->syncAndDrawFrame();

        return true;
    }

    void destroy() override {
    }

    void draw() override {
        printf("main-draw\n");
        // Clear the color buffer
        // glClear(GL_COLOR_BUFFER_BIT);
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //
        // mProxy->syncAndDrawFrame();
        usleep(1000 * 1000);
        // const char* version = (const char*) glGetString(GL_VERSION);
        // printf("Initialized GLES, version %s \n", version);

    }

private:
    android::uirenderer::renderthread::RenderProxy* mProxy;
    android::uirenderer::RenderNode* mRootNode;
};

int main(int argc, char** argv) {
    android::Typeface::setRobotoTypefaceForTest();
    printf("start\n");
    Sample app(argc, argv);
    app.run();
    return 0;
}
