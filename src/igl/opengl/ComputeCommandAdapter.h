/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <bitset>
#include <functional>
#include <unordered_map>
#include <igl/Buffer.h>
#include <igl/Common.h>
#include <igl/opengl/UnbindPolicy.h>
#include <igl/opengl/UniformAdapter.h>
#include <igl/opengl/WithContext.h>

namespace igl {

class ITexture;
class IComputePipelineState;
class ISamplerState;

namespace opengl {
class Buffer;

class ComputeCommandAdapter final : public WithContext {
 private:
  using StateBits = uint8_t;
  enum class StateMask : StateBits { NONE = 0, PIPELINE = 1 << 1 };

  struct BufferState {
    Buffer* resource = nullptr;
    size_t offset = 0;
  };

  using TextureState = ITexture*;
  using TextureStates = std::array<TextureState, IGL_TEXTURE_SAMPLERS_MAX>;

 public:
  explicit ComputeCommandAdapter(IContext& context);

  void clearTextures();
  void setTexture(ITexture* texture, uint32_t index);

  void clearBuffers();
  void setBuffer(Buffer* buffer, size_t offset, uint32_t index);

  void clearUniformBuffers();
  void setBlockUniform(Buffer* buffer,
                       size_t offset,
                       size_t size,
                       int index,
                       Result* outResult = nullptr);
  void setUniform(const UniformDesc& uniformDesc, const void* data, Result* outResult = nullptr);

  void setPipelineState(const std::shared_ptr<IComputePipelineState>& newValue);
  void dispatchThreadGroups(const Dimensions& threadgroupCount,
                            const Dimensions& /*threadgroupSize*/);

  void endEncoding();

 private:
  void clearDependentResources(const std::shared_ptr<IComputePipelineState>& newValue);
  void willDispatch();
  void didDispatch();

  [[nodiscard]] bool isDirty(StateMask mask) const {
    return (dirtyStateBits_ & EnumToValue(mask)) != 0;
  }
  void setDirty(StateMask mask) {
    dirtyStateBits_ |= EnumToValue(mask);
  }
  void clearDirty(StateMask mask) {
    dirtyStateBits_ &= ~EnumToValue(mask);
  }

 private:
  std::array<BufferState, IGL_BUFFER_BINDINGS_MAX> buffers_;
  std::bitset<IGL_BUFFER_BINDINGS_MAX> buffersDirty_;
  std::bitset<IGL_TEXTURE_SAMPLERS_MAX> textureStatesDirty_;
  TextureStates textureStates_{};
  UniformAdapter uniformAdapter_;
  StateBits dirtyStateBits_ = EnumToValue(StateMask::NONE);
  std::shared_ptr<IComputePipelineState> pipelineState_;
};
} // namespace opengl
} // namespace igl
