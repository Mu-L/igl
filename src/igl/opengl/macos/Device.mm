/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <igl/opengl/macos/Device.h>

#include <AppKit/AppKit.h>
#include <igl/opengl/IContext.h>

namespace igl::opengl::macos {

Device::Device(std::unique_ptr<IContext> context) :
  opengl::Device(std::move(context)), platformDevice_(*this) {}

const PlatformDevice& Device::getPlatformDevice() const noexcept {
  return platformDevice_;
}

} // namespace igl::opengl::macos
