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


struct Conic {
    Conic() {}

    Conic(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        fPts[0] = p0;
        fPts[1] = p1;
        fPts[2] = p2;
        fW = w;
    }

    Conic(const SkPoint pts[3], SkScalar w) {
        memcpy(fPts, pts, sizeof(fPts));
        fW = w;
    }

    SkPoint fPts[3];
    SkScalar fW;

    void set(const SkPoint pts[3], SkScalar w) {
        memcpy(fPts, pts, 3 * sizeof(SkPoint));
        fW = w;
    }

    void set(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        fPts[0] = p0;
        fPts[1] = p1;
        fPts[2] = p2;
        fW = w;
    }

    void chop(Conic dst[2]) const;

    /**
     *  return the power-of-2 number of quads needed to approximate this conic
     *  with a sequence of quads. Will be >= 0.
     */
    int computeQuadPOW2(SkScalar tol) const;

    int chopIntoQuadsPOW2(SkPoint* pPoint, int i);
};

static SkScalar subdivide_w_value(SkScalar w) {
    return SkScalarSqrt(SK_ScalarHalf + w * SK_ScalarHalf);
}

void Conic::chop(Conic* dst) const {
    Sk2s scale = Sk2s(SkScalarInvert(SK_Scalar1 + fW));
    SkScalar newW = subdivide_w_value(fW);

    Sk2s p0 = from_point(fPts[0]);
    Sk2s p1 = from_point(fPts[1]);
    Sk2s p2 = from_point(fPts[2]);
    Sk2s ww(fW);

    Sk2s wp1 = ww * p1;
    Sk2s m = (p0 + times_2(wp1) + p2) * scale * Sk2s(0.5f);

    dst[0].fPts[0] = fPts[0];
    dst[0].fPts[1] = to_point((p0 + wp1) * scale);
    dst[0].fPts[2] = dst[1].fPts[0] = to_point(m);
    dst[1].fPts[1] = to_point((wp1 + p2) * scale);
    dst[1].fPts[2] = fPts[2];

    dst[0].fW = dst[1].fW = newW;

}

// Limit the number of suggested quads to approximate a conic
#define kMaxConicToQuadPOW2     5

int Conic::computeQuadPOW2(SkScalar tol) const {
    if (tol < 0 || !SkScalarIsFinite(tol)) {
        return 0;
    }

    SkScalar a = fW - 1;
    SkScalar k = a / (4 * (2 + a));
    SkScalar x = k * (fPts[0].fX - 2 * fPts[1].fX + fPts[2].fX);
    SkScalar y = k * (fPts[0].fY - 2 * fPts[1].fY + fPts[2].fY);

    SkScalar error = SkScalarSqrt(x * x + y * y);

    int pow2;
    for (pow2 = 0; pow2 < kMaxConicToQuadPOW2; ++pow2) {

        if (error <= tol) {
            break;
        }
        error *= 0.25f;
    }
    return pow2;
}


// This was originally developed and tested for pathops: see SkOpTypes.h
// returns true if (a <= b <= c) || (a >= b >= c)
static bool between(SkScalar a, SkScalar b, SkScalar c) {
    return (a - b) * (c - b) <= 0;
}

static SkPoint* subdivide(const Conic& src, SkPoint pts[], int level) {
    SkASSERT(level >= 0);

    if (0 == level) {
        memcpy(pts, &src.fPts[1], 2 * sizeof(SkPoint));
        return pts + 2;
    } else {
        Conic dst[2];
        src.chop(dst);
        --level;
        pts = subdivide(dst[0], pts, level);
        return subdivide(dst[1], pts, level);
    }
}

int Conic::chopIntoQuadsPOW2(SkPoint* pts, int pow2) {

    SkASSERT(pow2 >= 0);
    *pts = fPts[0];
    if (pow2 == kMaxConicToQuadPOW2) {  // If an extreme weight generates many quads ...
        abort();
    }
    subdivide(*this, pts + 1, pow2);

    return 1 << pow2;
}

#define MAX_DEPTH 15

void recursiveQuadraticBezierVertices(
        float ax, float ay,
        float bx, float by,
        float cx, float cy,
        std::vector<Vertex>& outputVertices, int depth) {
    float dx = bx - ax;
    float dy = by - ay;
    // d is the cross product of vector (B-A) and (C-B).
    float d = (cx - bx) * dy - (cy - by) * dx;

    if (depth >= MAX_DEPTH
        || d * d <= 0.25) {
        // below thresh, draw line by adding endpoint
        outputVertices.push_back(Vertex{bx, by});
    } else {
        float acx = (ax + cx) * 0.5f;
        float bcx = (bx + cx) * 0.5f;
        float acy = (ay + cy) * 0.5f;
        float bcy = (by + cy) * 0.5f;

        // midpoint
        float mx = (acx + bcx) * 0.5f;
        float my = (acy + bcy) * 0.5f;

        recursiveQuadraticBezierVertices(ax, ay, mx, my, acx, acy,
                                         outputVertices, depth + 1);
        recursiveQuadraticBezierVertices(mx, my, bx, by, bcx, bcy,
                                         outputVertices, depth + 1);
    }
}


void blurImage(uint8_t* image, int32_t width, int32_t height, float radius) {
    uint32_t intRadius = Blur::convertRadiusToInt(radius);
    std::unique_ptr<float[]> gaussian(new float[2 * intRadius + 1]);
    Blur::generateGaussianWeights(gaussian.get(), radius);

    std::unique_ptr<uint8_t[]> scratch(new uint8_t[width * height * 4]);
    Blur::horizontal(gaussian.get(), intRadius, image, scratch.get(), width, height);
    Blur::vertical(gaussian.get(), intRadius, scratch.get(), image, width, height);
}


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
        // canvas->drawColor(SK_ColorRED, SkBlendMode::kSrc);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFF6F00);
        // mRootNode->mutateStagingProperties().setClipToBounds(true);
        // mRootNode->mutateStagingProperties().setClipBounds(Rect(0, 0, 100, 100));
        // mRootNode->mutateStagingProperties().mutableRevealClip().set(true, 100, 100, 50);

        // canvas->drawRect(0, 0, 300, 300, paint);


        // canvas->drawCircle(500, 300, 100, paint);


        // auto sc = canvas->saveLayer(0, 0, WIDTH, 100, nullptr, android::SaveFlags::HasAlphaLayer);

        auto icu_text = icu::UnicodeString::fromUTF8("Hello HWUI");
        // Translate and rotate
        // canvas->translate(100, 100);
        // Draw the text:
        android::Paint p(paint);
        p.setTextSize(30);
        p.setColor(SK_ColorWHITE);
        p.setLooper(SkBlurDrawLooper::Make(SK_ColorBLACK, 10, 0, 0));


        canvas->translate(0, 50);
        canvas->drawText(icu_text.getBuffer(), 0, icu_text.length(), icu_text.length(), 0, 0, 0, p,
                         android::Typeface::resolveDefault(nullptr));
        //
        // canvas->translate(0, 50);
        // canvas->drawText(icu_text.getBuffer(), 0, icu_text.length(), icu_text.length(), 0, 0, 0, p,
        //                  android::Typeface::resolveDefault(nullptr));

        // canvas->drawColor(SK_ColorRED, SkBlendMode::kSrc);
        // canvas->drawColor(SK_ColorBLUE, SkBlendMode::kSrc);
        // canvas->drawColor(SK_ColorGREEN, SkBlendMode::kSrc);

        // canvas->translate(0, 50);
        // canvas->drawRect(0, 0, 300, 300, paint);
        // canvas->drawText(icu_text.getBuffer(), 0, icu_text.length(), icu_text.length(), 0, 0, 0, p,
        //                  android::Typeface::resolveDefault(nullptr));

        sk_sp<SkData> data = SkData::MakeFromFileName("../res/image.jpeg");
        std::unique_ptr<SkImageGenerator> gen = SkImageGenerator::MakeFromEncoded(data);
        SkBitmap dst;
        bool result = dst.tryAllocPixels(gen->getInfo()) &&
                      gen->getPixels(gen->getInfo(), dst.getPixels(), dst.rowBytes());

        SkBitmap dst2;
        bool result2 = dst2.tryAllocPixels(gen->getInfo()) &&
                       gen->getPixels(gen->getInfo(), dst2.getPixels(), dst2.rowBytes());

        SkBitmap dst3;
        dst2.copyTo(&dst3);

        auto id1 = dst.getGenerationID();
        auto id2 = dst2.getGenerationID();
        auto id3 = dst3.getGenerationID();

        float radius = 10.0f;
        uint32_t intRadius = Blur::convertRadiusToInt(radius);
        uint32_t paddedWidth = dst3.width() + 2 * intRadius;
        uint32_t paddedHeight = dst3.height() * intRadius;
        // blurImage((uint8_t*) dst3.getPixels(), paddedWidth, paddedHeight, radius);

        sk_sp<android::Bitmap> bmp = android::Bitmap::createFrom(dst3.info(), *dst3.pixelRef());
        canvas->drawBitmap(*(bmp.get()), 0, 0, &paint);


        {
            // SkPath::ConvertConicToQuads(conic[0], conic[1], conic[2], weight, quads, 1);

            canvas->translate(100, 100);
            SkPath path;
            // path.moveTo(0, 100);
            // path.conicTo(0, 200, 100, 200, SK_ScalarRoot2Over2);
            // path.conicTo(200, 200, 200, 100, SK_ScalarRoot2Over2);
            // path.conicTo(200, 0, 100, 0, SK_ScalarRoot2Over2);
            // path.conicTo(0, 0, 0, 100, SK_ScalarRoot2Over2);
            path.addRoundRect(SkRect::MakeIWH(400, 400), 200, 200);
            path.close();

            std::vector<Vertex> outputVertices;

            SkPath::Iter iter(path, false);
            SkPoint pts[4];
            SkPath::Verb v;
            while (SkPath::kDone_Verb != (v = iter.next(pts))) {
                switch (v) {
                    case SkPath::kMove_Verb:
                        ALOGV("Move to pos %f %f", pts[0].x(), pts[0].y());
                        outputVertices.push_back(Vertex{pts[0].x(), pts[0].y()});
                        break;
                    case SkPath::kLine_Verb:
                        ALOGV("kLine_Verb %f %f -> %f %f", pts[0].x(), pts[0].y(), pts[1].x(), pts[1].y());
                        outputVertices.push_back(Vertex{pts[1].x(), pts[1].y()});
                        break;
                    case SkPath::kConic_Verb: {
                        ALOGV("kConic_Verb  0x=%f 0y=%f 1x=%f 1y=%f 2x=%f 2y=%f", pts[0].x(), pts[0].y(), pts[1].x(),
                              pts[1].y(), pts[2].x(), pts[2].y());
                        Conic conic;
                        conic.set(pts, iter.conicWeight());
                        // int pow2 = conic.computeQuadPOW2(0.25);
                        int pow2 = conic.computeQuadPOW2(0.25);
                        int fQuadCount = 1 << pow2;
                        SkPoint* quads = new SkPoint[1 + 2 * fQuadCount];
                        fQuadCount = conic.chopIntoQuadsPOW2(quads, pow2);

                        for (int i = 0; i < fQuadCount; ++i) {
                            const int offset = 2 * i;
                            ALOGV("Move to pos ax=%f ay=%f bx=%f by=%f cx=%f cy=%f", quads[offset].x(),
                                  quads[offset].y(),
                                  quads[offset + 2].x(), quads[offset + 2].y(),
                                  quads[offset + 1].x(), quads[offset + 1].y());

                            recursiveQuadraticBezierVertices(
                                    quads[offset].x(), quads[offset].y(),
                                    quads[offset + 2].x(), quads[offset + 2].y(),
                                    quads[offset + 1].x(), quads[offset + 1].y(),
                                    outputVertices, 0);

                        }
                        delete[] quads;
                        break;
                    }
                }
            }


            std::vector<float> points;
            ALOGV("+++++dump+++++");
            for (auto v: outputVertices) {
                printf("{%f,%f, 1.0}, ", v.x, v.y);
                points.push_back(v.x);
                points.push_back(v.y);
            }
            canvas->drawColor(SK_ColorWHITE, SkBlendMode::kSrc);
            canvas->drawPoints(points.data(), outputVertices.size() * 2, paint);
            printf("\n");

            VertexBuffer vertexBuffer;
            Vertex* buffer = vertexBuffer.alloc<Vertex>(outputVertices.size());

            int currentIndex = 0;
            // zig zag between all previous points on the inside of the hull to create a
            // triangle strip that fills the hull
            int srcAindex = 0;
            int srcBindex = outputVertices.size() - 1;
            while (srcAindex <= srcBindex) {
                buffer[currentIndex++] = outputVertices[srcAindex];
                if (srcAindex == srcBindex) break;
                buffer[currentIndex++] = outputVertices[srcBindex];
                srcAindex++;
                srcBindex--;
            }
            {
                SkPath path;
                for (unsigned int i = 0; i < outputVertices.size(); i++) {
                    if (i == 0) {
                        path.moveTo(outputVertices[i].x, outputVertices[i].y);
                    } else {
                        path.lineTo(outputVertices[i].x, outputVertices[i].y);
                    }
                }

                // canvas->drawPath(path, paint);

            }
            ALOGV("+++++dump+++++");
            for (unsigned int i = 0; i < vertexBuffer.getVertexCount(); i++) {
                printf("{%f,%f, 1.0}, ", buffer[i].x, buffer[i].y);

            }
            printf("\n");

            // paint.setColor(SK_ColorBLUE);
            // paint.setStyle(SkPaint::kStroke_Style);
            // canvas->drawPath(path, paint);
        }

        // canvas->drawBitmap(*(bmp.get()), 0, 0, dst.width(), dst.height(), 0, 0, WIDTH, HEIGHT, &paint);

        // SkPoint pts[2] = {SkPoint::Make(0, 0), SkPoint::Make(0, 1)};
        // constexpr int count = 2;
        // const SkColor colors[count] = {0xFF000000, 0};
        // paint.setBlendMode(SkBlendMode::kDstOut);
        // SkMatrix localMatrix;
        // localMatrix.setScale(1, 100);
        // paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, count,
        //                                              SkShader::TileMode::kClamp_TileMode, 0, &localMatrix));
        //


        auto childNode = new android::uirenderer::RenderNode();
        if (childNode->mutateStagingProperties().setLeftTopRightBottom(0, 0, WIDTH / 2, HEIGHT / 2)) {
            childNode->setPropertyFieldsDirty(RenderNode::X | RenderNode::Y);
        }
        childNode->setName("Child");
        childNode->mutateStagingProperties().mutateLayerProperties().setType(LayerType::RenderLayer);

        {
            auto canvas = std::unique_ptr<android::uirenderer::RecordingCanvas>(
                    reinterpret_cast<android::uirenderer::RecordingCanvas*>(android::Canvas::create_recording_canvas(
                            WIDTH / 2, HEIGHT / 2)));

            canvas->drawColor(SK_ColorMAGENTA, SkBlendMode::kClear);
            DisplayList* displayList = canvas->finishRecording();
            childNode->setStagingDisplayList(displayList);
        }

        // canvas->drawRenderNode(childNode);

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
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        ATRACE_CALL();

        mProxy->syncAndDrawFrame();
        usleep(100 * 1000);
        return;
        //
        // {
        //     auto canvas = android::Canvas::create_recording_canvas(WIDTH, HEIGHT, mRootNode);
        //     SkPaint paint;
        //     paint.setAntiAlias(true);
        //     paint.setColor(0xFFFF6F00);
        //     canvas->drawRect(0, 0, 300, 300, paint);
        //     DisplayList* displayList = canvas->finishRecording();
        //     mRootNode->setStagingDisplayList(displayList);
        // }

        // mRootNode->mutateStagingProperties().setTranslationX(x++);
        // mRootNode->setPropertyFieldsDirty(RenderNode::TRANSLATION_X);
        // Clear the color buffer
        // glClear(GL_COLOR_BUFFER_BIT);
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        // x -= 10;

        auto canvas = std::unique_ptr<android::uirenderer::RecordingCanvas>(
                reinterpret_cast<android::uirenderer::RecordingCanvas*>(android::Canvas::create_recording_canvas(500,
                                                                                                                 500,
                                                                                                                 mRootNode)));
        // mRootNode->setPropertyFieldsDirty(android::uirenderer::RenderNode::DirtyPropertyMask::TRANSLATION_X);
        // mRootNode->mutateStagingProperties().setClipToBounds(true);
        // mRootNode->mutateStagingProperties().setClipBounds(Rect(0, 0, 100, 100));
        // mRootNode->mutateStagingProperties().setTranslationX(100);
        // mRootNode->setPropertyFieldsDirty(android::uirenderer::RenderNode::DirtyPropertyMask::TRANSLATION_X);


        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFF0000);
        SkPoint pts[2] = {SkPoint::Make(0, 0), SkPoint::Make(500, 500)};
        constexpr int count = 2;
        const SkColor colors[count] = {SK_ColorRED, SK_ColorGREEN};
        const SkScalar pos[count] = {0.0f, 1.0f};
        paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, count,
                                                     SkShader::TileMode::kClamp_TileMode));
        // paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(150, 150), 200, colors,
        //                                              nullptr, SK_ARRAY_COUNT(colors),
        //                                              SkShader::TileMode::kClamp_TileMode));

        // paint.setShader(SkGradientShader::MakeSweep(150, 150, colors, pos, count));

        canvas->drawPaint(paint);

        auto sc = canvas->saveLayer(10, 10, 300, 300, &paint, android::SaveFlags::ClipToLayer);

        // SkPath p;
        // p.moveTo(0, 0);
        // p.lineTo(100, 100);
        // p.lineTo(0, 100);
        // p.lineTo(100, 0);
        // p.lineTo(0, 0);
        // p.close();
        // canvas->clipPath(&p, SkClipOp::kIntersect);
        SkPaint p;
        p.setColor(0xFF000000);
        canvas->drawRect(0, 0, 600, 600, p);

        canvas->restore();


        // canvas->drawRect(0, 0, 300, 300, paint);
        DisplayList* displayList = canvas->finishRecording();
        mRootNode->setStagingDisplayList(displayList);
        //
        mProxy->syncAndDrawFrame();
        usleep(100 * 1000);
        // const char* version = (const char*) glGetString(GL_VERSION);
        // printf("Initialized GLES, version %s \n", version);

    }

private:
    android::uirenderer::renderthread::RenderProxy* mProxy;
    android::uirenderer::RenderNode* mRootNode;
    int x = 1;
};

int main(int argc, char** argv) {
    android::Typeface::setRobotoTypefaceForTest();
    printf("start\n");
    Sample app(argc, argv);
    app.run();
    return 0;
}
