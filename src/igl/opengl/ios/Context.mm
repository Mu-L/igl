/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <igl/opengl/ios/Context.h>

#import <Foundation/Foundation.h>

#include <OpenGLES/EAGL.h>
#include <QuartzCore/CAEAGLLayer.h>
#import <objc/runtime.h>
#include <igl/opengl/Texture.h>

namespace igl::opengl::ios {
namespace {
EAGLContext* createEAGLContext(BackendVersion backendVersion, EAGLSharegroup* sharegroup) {
  IGL_DEBUG_ASSERT(backendVersion.flavor == BackendFlavor::OpenGL_ES);
  IGL_DEBUG_ASSERT(backendVersion.majorVersion == 3 || backendVersion.majorVersion == 2);
  IGL_DEBUG_ASSERT(backendVersion.minorVersion == 0);

  if (backendVersion.majorVersion == 3 && backendVersion.minorVersion == 0) {
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                                 sharegroup:sharegroup];
    if (context == nullptr) {
      return [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:sharegroup];
    }
    return context;
  } else {
    IGL_DEBUG_ASSERT(backendVersion.majorVersion == 2,
                     "IGL: unacceptable enum for rendering API for iOS\n");
    return [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:sharegroup];
  }
}

void* getOrGenerateContextUniqueID(EAGLContext* context) {
  static const void* uniqueIdKey = &uniqueIdKey;
  static uint64_t idCounter = 0;
  NSNumber* key = objc_getAssociatedObject(context, &uniqueIdKey);
  uint64_t contextId = 0;
  if (key == nullptr) {
    // Generate and set id if it doesn't exist
    contextId = idCounter++;
    objc_setAssociatedObject(context, &uniqueIdKey, @(contextId), OBJC_ASSOCIATION_RETAIN);
  } else {
    contextId = key.integerValue;
  }
  return (void*)contextId; // NOLINT(performance-no-int-to-ptr)
}
} // namespace

Context::Context(BackendVersion backendVersion) : context_(createEAGLContext(backendVersion, nil)) {
  if (context_ != nil) {
    IContext::registerContext(getOrGenerateContextUniqueID(context_), this);
  }

  initialize();
}

Context::Context(BackendVersion backendVersion, Result* result) :
  context_(createEAGLContext(backendVersion, nil)) {
  if (context_ != nil) {
    IContext::registerContext(getOrGenerateContextUniqueID(context_), this);
  } else {
    Result::setResult(result, Result::Code::ArgumentInvalid);
  }

  initialize(result);
}

Context::Context(EAGLContext* context) : context_(context) {
  if (context_ != nil) {
    IContext::registerContext(getOrGenerateContextUniqueID(context_), this);
  }
  initialize();
}

Context::Context(EAGLContext* context, Result* result) : context_(context) {
  if (context_ != nil) {
    IContext::registerContext(getOrGenerateContextUniqueID(context_), this);
  } else {
    Result::setResult(result, Result::Code::ArgumentInvalid);
  }
  initialize(result);
}

Context::~Context() {
  // Release CVOpenGLESTextureCacheRef
  if (textureCache_ != nullptr) {
    CVOpenGLESTextureCacheFlush(textureCache_, 0);
    CFRelease(textureCache_);
  }
  willDestroy(context_ == nil ? nullptr : getOrGenerateContextUniqueID(context_));

  // Unregister EAGLContext
  if (context_ != nil) {
    if (context_ == [EAGLContext currentContext]) {
      [EAGLContext setCurrentContext:nil];
    }
  }
}

void Context::present(std::shared_ptr<ITexture> surface) const {
  static_cast<Texture&>(*surface).bind();
  [context_ presentRenderbuffer:GL_RENDERBUFFER];
}

void Context::setCurrent() {
  [EAGLContext setCurrentContext:context_];
  flushDeletionQueue();
}

void Context::clearCurrentContext() const {
  [EAGLContext setCurrentContext:nil];
}

bool Context::isCurrentContext() const {
  return [EAGLContext currentContext] == context_;
}

bool Context::isCurrentSharegroup() const {
  return [EAGLContext currentContext].sharegroup == context_.sharegroup;
}

std::unique_ptr<IContext> Context::createShareContext(Result* outResult) {
  EAGLContext* sharedContext = [[EAGLContext alloc] initWithAPI:context_.API
                                                     sharegroup:context_.sharegroup];
  if (!sharedContext) {
    Result::setResult(outResult, Result::Code::RuntimeError, "Failed to create shared context");
    return nullptr;
  }
  Result::setOk(outResult);
  return std::make_unique<Context>(sharedContext);
}

CVOpenGLESTextureCacheRef Context::getTextureCache() {
  if (textureCache_ == nullptr) {
    CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, nullptr, context_, nullptr, &textureCache_);
  }
  return textureCache_;
}

} // namespace igl::opengl::ios
