/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RenderPipelineReflection.h"

#include <Foundation/Foundation.h>

#include <igl/Common.h>
#include <igl/metal/Texture.h>

using namespace igl;

namespace {

igl::UniformType metalDataTypeToIGLUniformType(MTLDataType type) {
  switch (type) {
  case MTLDataTypeFloat:
    return igl::UniformType::Float;
  case MTLDataTypeFloat2:
    return igl::UniformType::Float2;
  case MTLDataTypeFloat3:
    return igl::UniformType::Float3;
  case MTLDataTypeFloat4:
    return igl::UniformType::Float4;
  case MTLDataTypeBool:
    return igl::UniformType::Boolean;
  case MTLDataTypeInt:
    return igl::UniformType::Int;
  case MTLDataTypeInt2:
    return igl::UniformType::Int2;
  case MTLDataTypeInt3:
    return igl::UniformType::Int3;
  case MTLDataTypeInt4:
    return igl::UniformType::Int4;
  case MTLDataTypeFloat2x2:
    return igl::UniformType::Mat2x2;
  case MTLDataTypeFloat3x3:
    return igl::UniformType::Mat3x3;
  case MTLDataTypeFloat4x4:
    return igl::UniformType::Mat4x4;
  default:
    IGL_LOG_ERROR("Unsupported MTLDataType: %ld\n", type);
    return igl::UniformType::Invalid;
  }
}

} // namespace

namespace igl::metal {
RenderPipelineReflection::RenderPipelineReflection(MTLRenderPipelineReflection* refl) {
  if (refl != nullptr) {
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    for (MTLArgument* arg = nullptr in refl.vertexArguments) {
      if (arg.active) {
        createArgDesc(arg, ShaderStage::Vertex);
      }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    for (MTLArgument* arg = nullptr in refl.fragmentArguments) {
      if (arg.active) {
        createArgDesc(arg, ShaderStage::Fragment);
      }
    }
  }
}

RenderPipelineReflection::~RenderPipelineReflection() = default;

bool RenderPipelineReflection::createArgDesc(MTLArgument* arg, ShaderStage sh) {
  size_t loc = 0;

  if (arg.type == MTLArgumentTypeBuffer) {
    BufferArgDesc bufferDesc;
    bufferDesc.name = igl::genNameHandle(arg.name.UTF8String);
    bufferDesc.bufferAlignment = arg.bufferAlignment;
    bufferDesc.bufferDataSize = arg.bufferDataSize;
    bufferDesc.bufferIndex = static_cast<int>(arg.index);
    bufferDesc.shaderStage = sh;
    if (arg.bufferDataType == MTLDataTypeStruct) {
      // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
      for (MTLStructMember* uniform = nullptr in arg.bufferStructType.members) {
        MTLDataType elementType = uniform.dataType;
        if (elementType == MTLDataTypeArray) {
          elementType = uniform.arrayType.elementType;
        }
        igl::BufferArgDesc::BufferMemberDesc iglMemberDesc{
            .name = igl::genNameHandle(uniform.name.UTF8String),
            .type = metalDataTypeToIGLUniformType(elementType),
            .offset = (size_t)uniform.offset,
            .arrayLength = uniform.arrayType ? (size_t)uniform.arrayType.arrayLength : 1};
        bufferDesc.members.push_back(std::move(iglMemberDesc));
      }
    } else {
      igl::BufferArgDesc::BufferMemberDesc iglMemberDesc{
          .name = igl::genNameHandle(arg.name.UTF8String),
          .type = metalDataTypeToIGLUniformType(arg.bufferDataType),
          .offset = 0,
          .arrayLength = (size_t)arg.arrayLength};
      bufferDesc.members.push_back(std::move(iglMemberDesc));
    }
    bufferArguments_.push_back(std::move(bufferDesc));

    loc = bufferArguments_.size() - 1;
  } else if (arg.type == MTLArgumentTypeTexture) {
    TextureArgDesc textureDesc;
    textureDesc.name = arg.name.UTF8String;
    textureDesc.type = igl::metal::Texture::convertType(arg.textureType);
    textureDesc.textureIndex = static_cast<int>(arg.index);
    textureDesc.shaderStage = sh;
    textureArguments_.push_back(std::move(textureDesc));

    loc = textureArguments_.size() - 1;
  } else if (arg.type == MTLArgumentTypeSampler) {
    SamplerArgDesc samplerDesc;
    samplerDesc.name = arg.name.UTF8String;
    samplerDesc.samplerIndex = static_cast<int>(arg.index);
    samplerDesc.shaderStage = sh;
    samplerArguments_.push_back(std::move(samplerDesc));

    loc = samplerArguments_.size() - 1;
  } else {
    /// thread group mem and array argument type is not yet supported
    IGL_LOG_DEBUG("IGL Metal Reflection: unsupported argument type");
    /// just skip this one
    return false;
  }

  if (sh == ShaderStage::Vertex) {
    vertexArgDictionary_.insert(
        std::make_pair(std::string([arg.name UTF8String]),
                       ArgIndex(static_cast<int>(arg.index), arg.type, static_cast<int>(loc))));
  } else {
    fragmentArgDictionary_.insert(std::make_pair(
        std::string([arg.name UTF8String]), ArgIndex(static_cast<int>(arg.index), arg.type, loc)));
  }
  return true;
}

int RenderPipelineReflection::getIndexByName(const std::string& name, ShaderStage sh) const {
  const std::map<std::string, ArgIndex>& dictionary =
      (sh == ShaderStage::Vertex) ? vertexArgDictionary_ : fragmentArgDictionary_;

  auto it = dictionary.find(name);
  if (it != dictionary.end()) {
    return it->second.argumentIndex;
  }

  /// not found
  return -1;
}

const std::map<std::string, RenderPipelineReflection::ArgIndex>&
RenderPipelineReflection::getDictionary(ShaderStage sh) const {
  return (sh == ShaderStage::Vertex) ? vertexArgDictionary_ : fragmentArgDictionary_;
}

const std::vector<BufferArgDesc>& RenderPipelineReflection::allUniformBuffers() const {
  return bufferArguments_;
}

const std::vector<SamplerArgDesc>& RenderPipelineReflection::allSamplers() const {
  return samplerArguments_;
}

const std::vector<TextureArgDesc>& RenderPipelineReflection::allTextures() const {
  return textureArguments_;
}

} // namespace igl::metal
