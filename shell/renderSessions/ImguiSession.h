/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @fb-only

#pragma once

#include <IGLU/imgui/Session.h>
#include <shell/shared/platform/Platform.h>
#include <shell/shared/renderSession/RenderSession.h>
#include <igl/CommandQueue.h>
#include <igl/Framebuffer.h>

namespace igl::shell {

class ImguiSession : public RenderSession {
 public:
  explicit ImguiSession(std::shared_ptr<Platform> platform) : RenderSession(std::move(platform)) {}
  void initialize() noexcept override;
  void update(SurfaceTextures surfaceTextures) noexcept override;

 private:
  std::shared_ptr<IFramebuffer> outputFramebuffer_;
  std::unique_ptr<iglu::imgui::Session> imguiSession_;
};

} // namespace igl::shell
