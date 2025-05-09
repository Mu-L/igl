/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <unordered_map>
#include <igl/NameHandle.h>
#include <igl/RenderPipelineState.h>
#include <igl/opengl/GLIncludes.h>
#include <igl/opengl/IContext.h>
#include <igl/opengl/RenderPipelineReflection.h>
#include <igl/opengl/Shader.h>

namespace igl::opengl {

struct BlendMode {
  GLenum blendOpColor;
  GLenum blendOpAlpha;
  GLenum srcColor;
  GLenum dstColor;
  GLenum srcAlpha;
  GLenum dstAlpha;
};

class RenderPipelineState final : public WithContext, public IRenderPipelineState {
  friend class Device;

  Result create();

 public:
  explicit RenderPipelineState(IContext& context,
                               const RenderPipelineDesc& desc,
                               Result* outResult);
  ~RenderPipelineState() override;

  void bind();
  void unbind();
  Result bindTextureUnit(size_t unit, uint8_t bindTarget);

  void bindVertexAttributes(size_t bufferIndex, size_t offset);
  void unbindVertexAttributes();

  [[nodiscard]] bool matchesShaderProgram(const RenderPipelineState& rhs) const;
  [[nodiscard]] bool matchesVertexInputState(const RenderPipelineState& rhs) const;

  [[nodiscard]] int getIndexByName(const NameHandle& name, ShaderStage stage) const override;
  [[nodiscard]] int getIndexByName(const std::string& name, ShaderStage stage) const override;

  [[nodiscard]] int getUniformBlockBindingPoint(const NameHandle& uniformBlockName) const;
  [[nodiscard]] std::shared_ptr<IRenderPipelineReflection> renderPipelineReflection() override;
  void setRenderPipelineReflection(
      const IRenderPipelineReflection& renderPipelineReflection) override;

  [[nodiscard]] static GLenum convertBlendOp(BlendOp value);
  [[nodiscard]] static GLenum convertBlendFactor(BlendFactor value);
  [[nodiscard]] CullMode getCullMode() const {
    return desc_.cullMode;
  }
  [[nodiscard]] WindingMode getWindingMode() const {
    return desc_.frontFaceWinding;
  }
  [[nodiscard]] PolygonFillMode getPolygonFillMode() const {
    return desc_.polygonFillMode;
  }

  [[nodiscard]] const ShaderStages* getShaderStages() const {
    return static_cast<ShaderStages*>(desc_.shaderStages.get());
  }

  void savePrevPipelineStateAttributesLocations(RenderPipelineState& prevPipelineState) {
    prevPipelineStateAttributesLocations_ = std::move(prevPipelineState.activeAttributesLocations_);
  }

  void clearActiveAttributesLocations() {
    activeAttributesLocations_.clear();
  }

  void unbindPrevPipelineVertexAttributes();

 private:
  // Tracks a list of attribute locations associated with a bufferIndex
  std::vector<int> bufferAttribLocations_[IGL_BUFFER_BINDINGS_MAX];

  std::shared_ptr<RenderPipelineReflection> reflection_;
  std::unordered_map<size_t, size_t> vertexTextureUnitRemap;
  std::array<GLint, IGL_TEXTURE_SAMPLERS_MAX> unitSamplerLocationMap_{};
  std::unordered_map<int, size_t> uniformBlockBindingMap_;
  std::array<GLboolean, 4> colorMask_ = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
  std::vector<int> prevPipelineStateAttributesLocations_;
  std::vector<int> activeAttributesLocations_;
  BlendMode blendMode_ = {GL_FUNC_ADD, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_ONE, GL_ZERO};
  bool blendEnabled_ = false;
  bool uniformBlockBindingPointSet_ = false;
};

} // namespace igl::opengl
