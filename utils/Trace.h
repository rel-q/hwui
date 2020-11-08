/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ANDROID_TRACE_H
#define ANDROID_TRACE_H

#include <stdio.h>

namespace android {

class ScopedTrace {
public:
    inline ScopedTrace(const char* name) : mName(name) {
        printf("+++%s \n", mName);
    }

    inline ~ScopedTrace() {
        printf("---%s \n", mName);
    }

private:
    const char* mName;
};
};

#define _PASTE(x, y) x ## y
#define PASTE(x, y) _PASTE(x,y)
#define ATRACE_NAME(name) android::ScopedTrace PASTE(___tracer, __LINE__) (name)
#define ATRACE_CALL() ATRACE_NAME(__FUNCTION__)

#endif // ANDROID_TRACE_H
