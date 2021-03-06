#include <babylon/materials/node/node_material.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/core/json_util.h>
#include <babylon/core/logging.h>
#include <babylon/engines/engine.h>
#include <babylon/engines/scene.h>
#include <babylon/materials/effect.h>
#include <babylon/materials/ieffect_creation_options.h>
#include <babylon/materials/image_processing_configuration.h>
#include <babylon/materials/node/blocks/dual/reflection_texture_block.h>
#include <babylon/materials/node/blocks/dual/texture_block.h>
#include <babylon/materials/node/blocks/fragment/fragment_output_block.h>
#include <babylon/materials/node/blocks/input/input_block.h>
#include <babylon/materials/node/blocks/transform_block.h>
#include <babylon/materials/node/blocks/vertex/vertex_output_block.h>
#include <babylon/materials/node/inode_material_options.h>
#include <babylon/materials/node/node_material_block.h>
#include <babylon/materials/node/node_material_build_state.h>
#include <babylon/materials/node/node_material_build_state_shared_data.h>
#include <babylon/materials/node/node_material_connection_point.h>
#include <babylon/materials/node/node_material_defines.h>
#include <babylon/materials/node/optimizers/node_material_optimizer.h>
#include <babylon/materials/textures/base_texture.h>
#include <babylon/materials/textures/texture.h>
#include <babylon/meshes/sub_mesh.h>
#include <babylon/meshes/vertex_buffer.h>
#include <babylon/misc/file_tools.h>
#include <babylon/misc/string_tools.h>

namespace BABYLON {

bool NodeMaterial::IgnoreTexturesAtLoadTime = false;
size_t NodeMaterial::_BuildIdGenerator      = 0;

NodeMaterial::NodeMaterial(const std::string& iName, Scene* iScene,
                           const INodeMaterialOptionsPtr& options)
    : PushMaterial{iName, iScene}
    , ignoreAlpha{false}
    , maxSimultaneousLights{4}
    , options{this, &NodeMaterial::get_options, &NodeMaterial::set_options}
    , imageProcessingConfiguration{this, &NodeMaterial::get_imageProcessingConfiguration,
                                   &NodeMaterial::set_imageProcessingConfiguration}
    , _imageProcessingConfiguration{nullptr}
    , _options{nullptr}
    , _vertexCompilationState{nullptr}
    , _fragmentCompilationState{nullptr}
    , _sharedData{nullptr}
    , _buildId{NodeMaterial::_BuildIdGenerator++}
    , _buildWasSuccessful{false}
    , _animationFrame{-1}
    , _imageProcessingObserver{nullptr}
{
  _options = options;

  // Setup the default processing configuration to the scene.
  _attachImageProcessingConfiguration(nullptr);
}

NodeMaterial::~NodeMaterial() = default;

INodeMaterialOptionsPtr& NodeMaterial::get_options()
{
  return _options;
}

void NodeMaterial::set_options(const INodeMaterialOptionsPtr& value)
{
  _options = value;
}

ImageProcessingConfigurationPtr& NodeMaterial::get_imageProcessingConfiguration()
{
  return _imageProcessingConfiguration;
}

void NodeMaterial::set_imageProcessingConfiguration(const ImageProcessingConfigurationPtr& value)
{
  _attachImageProcessingConfiguration(value);

  // Ensure the effect will be rebuilt.
  _markAllSubMeshesAsTexturesDirty();
}

std::string NodeMaterial::getClassName() const
{
  return "NodeMaterial";
}

void NodeMaterial::_attachImageProcessingConfiguration(
  const ImageProcessingConfigurationPtr& configuration)
{
  if (configuration == _imageProcessingConfiguration) {
    return;
  }

  // Detaches observer.
  if (_imageProcessingConfiguration && _imageProcessingObserver) {
    _imageProcessingConfiguration->onUpdateParameters.remove(_imageProcessingObserver);
  }

  // Pick the scene configuration if needed.
  if (!configuration) {
    _imageProcessingConfiguration = getScene()->imageProcessingConfiguration();
  }
  else {
    _imageProcessingConfiguration = configuration;
  }

  // Attaches observer.
  if (_imageProcessingConfiguration) {
    _imageProcessingObserver = _imageProcessingConfiguration->onUpdateParameters.add(
      [this](ImageProcessingConfiguration* /*ipc*/, EventState & /*es*/) -> void {
        _markAllSubMeshesAsImageProcessingDirty();
      });
  }
}

NodeMaterialBlockPtr NodeMaterial::getBlockByName(const std::string& iName) const
{
  NodeMaterialBlockPtr result = nullptr;
  for (const auto& block : attachedBlocks) {
    if (block->name == iName) {
      if (!result) {
        result = block;
      }
      else {
        BABYLON_LOGF_WARN("NodeMaterial", "More than one block was found with the name '%s'",
                          name.c_str())
        return result;
      }
    }
  }

  return result;
}

NodeMaterialBlockPtr NodeMaterial::getBlockByPredicate(
  const std::function<bool(const NodeMaterialBlockPtr& block)>& predicate)
{
  for (const auto& block : attachedBlocks) {
    if (predicate(block)) {
      return block;
    }
  }

  return nullptr;
}

InputBlockPtr NodeMaterial::getInputBlockByPredicate(
  const std::function<bool(const InputBlockPtr& block)>& predicate)
{
  for (const auto& block : attachedBlocks) {
    if (block->isInput && predicate(std::static_pointer_cast<InputBlock>(block))) {
      return std::static_pointer_cast<InputBlock>(block);
    }
  }

  return nullptr;
}

std::vector<InputBlockPtr> NodeMaterial::getInputBlocks() const
{
  std::vector<InputBlockPtr> blocks;
  for (const auto& block : attachedBlocks) {
    if (block->isInput()) {
      blocks.emplace_back(std::static_pointer_cast<InputBlock>(block));
    }
  }

  return blocks;
}

NodeMaterial& NodeMaterial::registerOptimizer(const NodeMaterialOptimizerPtr& optimizer)
{
  auto index = stl_util::index_of(_optimizers, optimizer);

  if (index > -1) {
    return *this;
  }

  _optimizers.emplace_back(optimizer);

  return *this;
}

NodeMaterial& NodeMaterial::unregisterOptimizer(const NodeMaterialOptimizerPtr& optimizer)
{
  auto index = stl_util::index_of(_optimizers, optimizer);

  if (index == -1) {
    return *this;
  }

  stl_util::splice(_optimizers, index, 1);

  return *this;
}

NodeMaterial& NodeMaterial::addOutputNode(const NodeMaterialBlockPtr& node)
{
  if (node->target() == NodeMaterialBlockTargets::Undefined) {
    throw std::runtime_error(
      "This node is not meant to be an output node. You may want to explicitly set its target "
      "value.");
  }

  if ((node->target() == NodeMaterialBlockTargets::Vertex) != 0) {
    _addVertexOutputNode(node);
  }

  if ((node->target() == NodeMaterialBlockTargets::Fragment) != 0) {
    _addFragmentOutputNode(node);
  }

  return *this;
}

NodeMaterial& NodeMaterial::removeOutputNode(const NodeMaterialBlockPtr& node)
{
  if (node->target() == NodeMaterialBlockTargets::Undefined) {
    return *this;
  }

  if (node->target() == NodeMaterialBlockTargets::Vertex) {
    _removeVertexOutputNode(node);
  }

  if (node->target() == NodeMaterialBlockTargets::Fragment) {
    _removeFragmentOutputNode(node);
  }

  return *this;
}

NodeMaterial& NodeMaterial::_addVertexOutputNode(const NodeMaterialBlockPtr& node)
{
  if (stl_util::contains(_vertexOutputNodes, node)) {
    return *this;
  }

  node->target = NodeMaterialBlockTargets::Vertex;
  _vertexOutputNodes.emplace_back(node);

  return *this;
}

NodeMaterial& NodeMaterial::_removeVertexOutputNode(const NodeMaterialBlockPtr& node)
{
  auto index = stl_util::index_of(_vertexOutputNodes, node);
  if (index == -1) {
    return *this;
  }

  stl_util::splice(_vertexOutputNodes, index, 1);

  return *this;
}

NodeMaterial& NodeMaterial::_addFragmentOutputNode(const NodeMaterialBlockPtr& node)
{
  if (stl_util::contains(_fragmentOutputNodes, node)) {
    return *this;
  }

  node->target = NodeMaterialBlockTargets::Fragment;
  _fragmentOutputNodes.emplace_back(node);

  return *this;
}

NodeMaterial& NodeMaterial::_removeFragmentOutputNode(const NodeMaterialBlockPtr& node)
{
  auto index = stl_util::index_of(_fragmentOutputNodes, node);
  if (index == -1) {
    return *this;
  }

  stl_util::splice(_fragmentOutputNodes, index, 1);

  return *this;
}

bool NodeMaterial::needAlphaBlending() const
{
  if (ignoreAlpha) {
    return false;
  }
  return (alpha < 1.f) || (_sharedData && _sharedData->hints.needAlphaBlending);
}

bool NodeMaterial::needAlphaTesting() const
{
  return _sharedData && _sharedData->hints.needAlphaTesting;
}

void NodeMaterial::_initializeBlock(
  const NodeMaterialBlockPtr& node, const NodeMaterialBuildStatePtr& iState,
  std::vector<NodeMaterialBlockPtr>& nodesToProcessForOtherBuildState)
{
  node->initialize(*iState);
  node->autoConfigure(shared_from_this());
  node->_preparationId = _buildId;

  if (!stl_util::contains(attachedBlocks, node)) {
    if (node->isUnique()) {
      const auto className = node->getClassName();

      for (const auto& other : attachedBlocks) {
        if (other->getClassName() == className) {
          throw std::runtime_error(StringTools::printf(
            "Cannot have multiple blocks of type %s in the same NodeMaterial", className.c_str()));
        }
      }
    }
    attachedBlocks.emplace_back(node);
  }

  for (const auto& input : node->inputs()) {
    input->associatedVariableName = "";

    const auto& connectedPoint = input->connectedPoint();
    if (connectedPoint) {
      const auto block = connectedPoint->ownerBlock();
      if (block != node) {
        if (block->target() == NodeMaterialBlockTargets::VertexAndFragment) {
          nodesToProcessForOtherBuildState.emplace_back(block);
        }
        else if (iState->target == NodeMaterialBlockTargets::Fragment
                 && block->target() == NodeMaterialBlockTargets::Vertex
                 && block->_preparationId != _buildId) {
          nodesToProcessForOtherBuildState.emplace_back(block);
        }
        _initializeBlock(block, iState, nodesToProcessForOtherBuildState);
      }
    }
  }

  for (const auto& output : node->outputs()) {
    output->associatedVariableName = "";
  }
}

void NodeMaterial::_resetDualBlocks(const NodeMaterialBlockPtr& node, size_t iId)
{
  if (node->target() == NodeMaterialBlockTargets::VertexAndFragment) {
    node->buildId = iId;
  }

  for (const auto& inputs : node->inputs()) {
    const auto connectedPoint = inputs->connectedPoint();
    if (connectedPoint) {
      const auto block = connectedPoint->ownerBlock();
      if (block != node) {
        _resetDualBlocks(block, iId);
      }
    }
  }
}

void NodeMaterial::removeBlock(const NodeMaterialBlockPtr& block)
{
  auto attachedBlockIndex = stl_util::index_of(attachedBlocks, block);
  if (attachedBlockIndex > -1) {
    stl_util::splice(attachedBlocks, attachedBlockIndex, 1);
  }

  if (block->isFinalMerger()) {
    removeOutputNode(block);
  }
}

void NodeMaterial::build(bool verbose)
{
  _buildWasSuccessful = false;
  auto engine         = getScene()->getEngine();

  if (_vertexOutputNodes.empty()) {
    throw std::runtime_error("You must define at least one vertexOutputNode");
  }

  if (_fragmentOutputNodes.empty()) {
    throw std::runtime_error("You must define at least one fragmentOutputNode");
  }

  // Compilation state
  _vertexCompilationState                          = std::make_shared<NodeMaterialBuildState>();
  _vertexCompilationState->supportUniformBuffers   = engine->supportsUniformBuffers();
  _vertexCompilationState->target                  = NodeMaterialBlockTargets::Vertex;
  _fragmentCompilationState                        = std::make_shared<NodeMaterialBuildState>();
  _fragmentCompilationState->supportUniformBuffers = engine->supportsUniformBuffers();
  _fragmentCompilationState->target                = NodeMaterialBlockTargets::Fragment;

  // Shared data
  _sharedData                           = std::make_shared<NodeMaterialBuildStateSharedData>();
  _vertexCompilationState->sharedData   = _sharedData;
  _fragmentCompilationState->sharedData = _sharedData;
  _sharedData->buildId                  = _buildId;
  _sharedData->emitComments             = _options->emitComments;
  _sharedData->verbose                  = verbose;
  _sharedData->scene                    = getScene();

  // Initialize blocks
  std::vector<NodeMaterialBlockPtr> vertexNodes;
  std::vector<NodeMaterialBlockPtr> fragmentNodes;

  for (const auto& vertexOutputNode : _vertexOutputNodes) {
    vertexNodes.emplace_back(vertexOutputNode);
    _initializeBlock(vertexOutputNode, _vertexCompilationState, fragmentNodes);
  }

  for (const auto& fragmentOutputNode : _fragmentOutputNodes) {
    fragmentNodes.emplace_back(fragmentOutputNode);
    _initializeBlock(fragmentOutputNode, _fragmentCompilationState, vertexNodes);
  }

  // Optimize
  optimize();

  // Vertex
  for (const auto& vertexOutputNode : vertexNodes) {
    vertexOutputNode->build(*_vertexCompilationState, vertexNodes);
  }

  // Fragment
  _fragmentCompilationState->uniforms             = _vertexCompilationState->uniforms;
  _fragmentCompilationState->_uniformDeclaration  = _vertexCompilationState->_uniformDeclaration;
  _fragmentCompilationState->_constantDeclaration = _vertexCompilationState->_constantDeclaration;
  _fragmentCompilationState->_vertexState         = _vertexCompilationState;

  for (const auto& fragmentOutputNode : fragmentNodes) {
    _resetDualBlocks(fragmentOutputNode, _buildId - 1);
  }

  for (const auto& fragmentOutputNode : fragmentNodes) {
    fragmentOutputNode->build(*_fragmentCompilationState, fragmentNodes);
  }

  // Finalize
  _vertexCompilationState->finalize(*_vertexCompilationState);
  _fragmentCompilationState->finalize(*_fragmentCompilationState);

  _buildId = NodeMaterial::_BuildIdGenerator++;

  // Errors
  _sharedData->emitErrors();

  if (verbose) {
    BABYLON_LOG_INFO("NodeMaterial", "Vertex shader:")
    BABYLON_LOG_INFO("NodeMaterial", _vertexCompilationState->compilationString)
    BABYLON_LOG_INFO("NodeMaterial", "Fragment shader:")
    BABYLON_LOG_INFO("NodeMaterial", _fragmentCompilationState->compilationString)
  }

  _buildWasSuccessful = true;
  onBuildObservable.notifyObservers(this);

  // Wipe defines
  const auto& meshes = getScene()->meshes;
  for (const auto& mesh : meshes) {
    if (mesh->subMeshes.empty()) {
      continue;
    }
    for (const auto& subMesh : mesh->subMeshes) {
      if (!subMesh->getMaterial() || subMesh->getMaterial().get() != this) {
        continue;
      }

      if (!subMesh->_materialDefines) {
        continue;
      }

      auto& defines = subMesh->_materialDefines;
      defines->markAllAsDirty();
      defines.reset();
    }
  }
}

void NodeMaterial::optimize()
{
  for (const auto& optimizer : _optimizers) {
    optimizer->optimize(_vertexOutputNodes, _fragmentOutputNodes);
  }
}

void NodeMaterial::_prepareDefinesForAttributes(AbstractMesh* mesh, NodeMaterialDefines& defines)
{
  const auto oldNormal  = defines["NORMAL"];
  const auto oldTangent = defines["TANGENT"];
  const auto oldUV1     = defines["UV1"];

  defines.boolDef["NORMAL"] = mesh->isVerticesDataPresent(VertexBuffer::NormalKind);

  defines.boolDef["TANGENT"] = mesh->isVerticesDataPresent(VertexBuffer::TangentKind);

  defines.boolDef["UV1"] = mesh->isVerticesDataPresent(VertexBuffer::UVKind);

  if (oldNormal != defines["NORMAL"] || oldTangent != defines["TANGENT"]
      || oldUV1 != defines["UV1"]) {
    defines.markAsAttributesDirty();
  }
}

bool NodeMaterial::isReadyForSubMesh(AbstractMesh* mesh, BaseSubMesh* subMesh, bool useInstances)
{
  if (!_buildWasSuccessful) {
    return false;
  }

  auto scene = getScene();
  if (!_sharedData->animatedInputs.empty()) {
    const auto& frameId = scene->getFrameId();

    if (_animationFrame != frameId) {
      for (const auto& input : _sharedData->animatedInputs) {
        input->animate(scene);
      }

      _animationFrame = frameId;
    }
  }

  if (subMesh->effect() && isFrozen()) {
    if (subMesh->effect()->_wasPreviouslyReady) {
      return true;
    }
  }

  if (!subMesh->_materialDefines) {
    subMesh->_materialDefines = std::make_shared<NodeMaterialDefines>();
  }

  auto defines = std::static_pointer_cast<NodeMaterialDefines>(subMesh->_materialDefines);
  if (_isReadyForSubMesh(subMesh)) {
    return true;
  }

  auto engine = scene->getEngine();

  _prepareDefinesForAttributes(mesh, *defines);

  // Check if blocks are ready
  for (const auto& b : _sharedData->blockingBlocks) {
    if (!b->isReady(mesh, shared_from_this(), *defines, useInstances)) {
      return false;
    }
  }

  // Shared defines
  for (const auto& b : _sharedData->blocksWithDefines) {
    b->initializeDefines(mesh, shared_from_this(), *defines, useInstances);
  }

  for (const auto& b : _sharedData->blocksWithDefines) {
    b->prepareDefines(mesh, shared_from_this(), *defines, useInstances);
  }

  // Need to recompile?
  if (defines->isDirty()) {
    defines->markAsProcessed();

    // Repeatable content generators
    _vertexCompilationState->compilationString = _vertexCompilationState->_builtCompilationString;
    _fragmentCompilationState->compilationString
      = _fragmentCompilationState->_builtCompilationString;

    for (const auto& b : _sharedData->repeatableContentBlocks) {
      b->replaceRepeatableContent(*_vertexCompilationState, *_fragmentCompilationState, mesh,
                                  *defines);
    }

    // Uniforms
    std::vector<std::string> uniformBuffers;
    for (const auto& b : _sharedData->dynamicUniformBlocks) {
      b->updateUniformsAndSamples(*_vertexCompilationState, shared_from_this(), *defines,
                                  uniformBuffers);
    }

    auto mergedUniforms = _vertexCompilationState->uniforms;

    for (const auto& u : _fragmentCompilationState->uniforms) {
      if (!stl_util::contains(mergedUniforms, u)) {
        mergedUniforms.emplace_back(u);
      }
    }

    // Samplers
    auto mergedSamplers = _vertexCompilationState->samplers;

    for (const auto& s : _fragmentCompilationState->samplers) {
      if (!stl_util::contains(mergedSamplers, s)) {
        mergedSamplers.emplace_back(s);
      }
    }

    auto fallbacks = std::make_unique<EffectFallbacks>();

    for (const auto& b : _sharedData->blocksWithFallbacks) {
      b->provideFallbacks(mesh, fallbacks.get());
    }

    auto previousEffect = subMesh->effect();
    // Compilation
    auto join = defines->toString();

    std::unordered_map<std::string, std::string> baseName{
      {"vertex", "nodeMaterial" + std::to_string(_buildId)},            //
      {"fragment", "nodeMaterial" + std::to_string(_buildId)},          //
      {"vertexSource", _vertexCompilationState->compilationString},     //
      {"fragmentSource", _fragmentCompilationState->compilationString}, //
    };

    std::unordered_map<std::string, unsigned int> indexParameters{
      {"maxSimultaneousLights", maxSimultaneousLights},
      {"maxSimultaneousMorphTargets", defines->intDef["NUM_MORPH_INFLUENCERS"]}};

    IEffectCreationOptions iOptions;
    iOptions.attributes            = _vertexCompilationState->attributes;
    iOptions.uniformsNames         = std::move(mergedUniforms);
    iOptions.uniformBuffersNames   = std::move(uniformBuffers);
    iOptions.samplers              = std::move(mergedSamplers);
    iOptions.materialDefines       = defines.get();
    iOptions.defines               = std::move(join);
    iOptions.fallbacks             = std::move(fallbacks);
    iOptions.onCompiled            = onCompiled;
    iOptions.onError               = onError;
    iOptions.indexParameters       = std::move(indexParameters);
    iOptions.maxSimultaneousLights = maxSimultaneousLights;

    auto effect = scene->getEngine()->createEffect(baseName, iOptions, engine);

    if (effect) {
      /* if (_onEffectCreatedObservable()) */ {
        onCreatedEffectParameters.effect  = effect.get();
        onCreatedEffectParameters.subMesh = subMesh;
        _onEffectCreatedObservable.notifyObservers(&onCreatedEffectParameters);
      }

      // Use previous effect while new one is compiling
      if (allowShaderHotSwapping && previousEffect && !effect->isReady()) {
        effect = previousEffect;
        defines->markAsUnprocessed();
      }
      else {
        scene->resetCachedMaterial();
        subMesh->setEffect(effect, defines);
      }
    }
  }

  if (!subMesh->effect() || !subMesh->effect()->isReady()) {
    return false;
  }

  defines->_renderId                     = scene->getRenderId();
  subMesh->effect()->_wasPreviouslyReady = true;

  return true;
}

std::string NodeMaterial::compiledShaders() const
{
  return StringTools::printf("// Vertex shader\r\n%s\r\n\r\n// Fragment shader\r\n%s",
                             _vertexCompilationState->compilationString.c_str(),
                             _fragmentCompilationState->compilationString.c_str());
}

void NodeMaterial::bindOnlyWorldMatrix(Matrix& world, const EffectPtr& /*effectOverride*/)
{
  auto scene = getScene();

  if (!_activeEffect) {
    return;
  }

  const auto& hints = _sharedData->hints;

  if (hints.needWorldViewMatrix) {
    world.multiplyToRef(scene->getViewMatrix(), _cachedWorldViewMatrix);
  }

  if (hints.needWorldViewProjectionMatrix) {
    world.multiplyToRef(scene->getTransformMatrix(), _cachedWorldViewProjectionMatrix);
  }

  // Connection points
  for (const auto& inputBlock : _sharedData->inputBlocks) {
    inputBlock->_transmitWorld(_activeEffect.get(), world, _cachedWorldViewMatrix,
                               _cachedWorldViewProjectionMatrix);
  }
}

void NodeMaterial::bindForSubMesh(Matrix& world, Mesh* mesh, SubMesh* subMesh)
{
  auto scene  = getScene();
  auto effect = subMesh->effect();
  if (!effect) {
    return;
  }
  _activeEffect = effect;

  // Matrices
  bindOnlyWorldMatrix(world);

  auto mustRebind = _mustRebind(scene, effect, mesh->visibility());

  if (mustRebind) {
    const auto& sharedData = _sharedData;
    if (effect && scene->getCachedEffect() != effect) {
      // Bindable blocks
      for (const auto& block : sharedData->bindableBlocks) {
        block->bind(effect, shared_from_this(), mesh);
      }

      // Connection points
      for (const auto& inputBlock : sharedData->inputBlocks) {
        inputBlock->_transmit(effect.get(), scene);
      }
    }
  }

  _afterBind(mesh, _activeEffect);
}

std::vector<BaseTexturePtr> NodeMaterial::getActiveTextures() const
{
  auto activeTextures = PushMaterial::getActiveTextures();

  if (_sharedData) {
    for (const auto& t : _sharedData->textureBlocks) {
      if (std::holds_alternative<TextureBlockPtr>(t) && std::get<TextureBlockPtr>(t)) {
        activeTextures.emplace_back(std::get<TextureBlockPtr>(t)->texture);
      }
      else if (std::holds_alternative<ReflectionTextureBlockPtr>(t)
               && std::get<ReflectionTextureBlockPtr>(t)) {
        activeTextures.emplace_back(std::get<ReflectionTextureBlockPtr>(t)->texture);
      }
    }
  }

  return activeTextures;
}

std::vector<std::variant<TextureBlockPtr, ReflectionTextureBlockPtr>>
NodeMaterial::getTextureBlocks()
{
  if (!_sharedData) {
    return {};
  }

  return _sharedData->textureBlocks;
}

bool NodeMaterial::hasTexture(const BaseTexturePtr& texture) const
{
  if (PushMaterial::hasTexture(texture)) {
    return true;
  }

  if (!_sharedData) {
    return false;
  }

  for (const auto& t : _sharedData->textureBlocks) {
    if (std::holds_alternative<TextureBlockPtr>(t)
        && std::get<TextureBlockPtr>(t)->texture == std::static_pointer_cast<Texture>(texture)) {
      return true;
    }
    else if (std::holds_alternative<ReflectionTextureBlockPtr>(t)
             && std::get<ReflectionTextureBlockPtr>(t)->texture == texture) {
      return true;
    }
  }

  return false;
}

void NodeMaterial::dispose(bool forceDisposeEffect, bool forceDisposeTextures, bool notBoundToMesh)
{
  if (forceDisposeTextures) {
    for (const auto& tb : _sharedData->textureBlocks) {
      if (std::holds_alternative<TextureBlockPtr>(tb)) {
        auto texture = std::get<TextureBlockPtr>(tb)->texture;
        if (texture) {
          texture->dispose();
        }
      }
      else if (std::holds_alternative<ReflectionTextureBlockPtr>(tb)) {
        auto texture = std::get<ReflectionTextureBlockPtr>(tb)->texture;
        if (texture) {
          texture->dispose();
        }
      }
    }
  }

  for (const auto& block : attachedBlocks) {
    block->dispose();
  }

  onBuildObservable.clear();

  PushMaterial::dispose(forceDisposeEffect, forceDisposeTextures, notBoundToMesh);
}

void NodeMaterial::edit(const INodeMaterialEditorOptionsPtr& /*config*/)
{
}

void NodeMaterial::clear()
{
  _vertexOutputNodes.clear();
  _fragmentOutputNodes.clear();
  attachedBlocks.clear();
}

void NodeMaterial::setToDefault()
{
  clear();

  auto positionInput = InputBlock::New("Position");
  positionInput->setAsAttribute("position");

  auto worldInput = InputBlock::New("World");
  worldInput->setAsSystemValue(NodeMaterialSystemValues::World);

  auto worldPos = TransformBlock::New("WorldPos");
  positionInput->connectTo(worldPos);
  worldInput->connectTo(worldPos);

  auto viewProjectionInput = InputBlock::New("ViewProjection");
  viewProjectionInput->setAsSystemValue(NodeMaterialSystemValues::ViewProjection);

  auto worldPosdMultipliedByViewProjection
    = TransformBlock::New("WorldPos * ViewProjectionTransform");
  worldPos->connectTo(worldPosdMultipliedByViewProjection);
  viewProjectionInput->connectTo(worldPosdMultipliedByViewProjection);

  auto vertexOutput = VertexOutputBlock::New("VertexOutput");
  worldPosdMultipliedByViewProjection->connectTo(vertexOutput);

  // Pixel
  auto pixelColor   = InputBlock::New("color");
  pixelColor->value = std::make_shared<AnimationValue>(Color4(0.8f, 0.8f, 0.8f, 1.f));

  auto fragmentOutput = FragmentOutputBlock::New("FragmentOutput");
  pixelColor->connectTo(fragmentOutput);

  // Add to nodes
  addOutputNode(vertexOutput);
  addOutputNode(fragmentOutput);
}

void NodeMaterial::loadAsync(const std::string& url)
{
  FileTools::LoadFile(
    url,
    [this](const std::variant<std::string, ArrayBufferView>& data,
           const std::string & /*responseURL*/) -> void {
      if (std::holds_alternative<std::string>(data)) {
        auto serializationObject = json::parse(std::get<std::string>(data));

        loadFromSerialization(serializationObject, "");
      }
    },
    nullptr, false,
    [url](const std::string& message, const std::string & /*exception*/) -> void {
      BABYLON_LOG_ERROR("NodeMaterial", "Could not load file %s, reason: %s", url.c_str(),
                        message.c_str())
    });
}

void NodeMaterial::_gatherBlocks(const NodeMaterialBlockPtr& rootNode,
                                 std::vector<NodeMaterialBlockPtr>& list)
{
  if (stl_util::contains(list, rootNode)) {
    return;
  }
  list.emplace_back(rootNode);

  for (const auto& input : rootNode->inputs()) {
    const auto connectedPoint = input->connectedPoint();
    if (connectedPoint) {
      const auto block = connectedPoint->ownerBlock();
      if (block != rootNode) {
        _gatherBlocks(block, list);
      }
    }
  }
}

std::string NodeMaterial::generateCode()
{
  std::vector<NodeMaterialBlockPtr> alreadyDumped;
  std::vector<NodeMaterialBlockPtr> vertexBlocks;
  std::vector<std::string> uniqueNames;
  // Gets active blocks
  for (const auto& outputNode : _vertexOutputNodes) {
    _gatherBlocks(outputNode, vertexBlocks);
  }

  std::vector<NodeMaterialBlockPtr> fragmentBlocks;
  for (const auto& outputNode : _fragmentOutputNodes) {
    _gatherBlocks(outputNode, fragmentBlocks);
  }

  // Generate vertex shader
  std::string codeString = StringTools::printf("auto nodeMaterial = NodeMaterial::New(\"%s\");\r\n",
                                               !name.empty() ? name.c_str() : "node material");
  for (const auto& node : vertexBlocks) {
    if (node->isInput() && !stl_util::contains(alreadyDumped, node)) {
      codeString += node->_dumpCode(uniqueNames, alreadyDumped);
    }
  }

  // Generate fragment shader
  for (const auto& node : fragmentBlocks) {
    if (node->isInput() && !stl_util::contains(alreadyDumped, node)) {
      codeString += node->_dumpCode(uniqueNames, alreadyDumped);
    }
  }

  // Connections
  alreadyDumped = {};
  codeString += "\r\n// Connections\r\n";
  for (const auto& node : _vertexOutputNodes) {
    codeString += node->_dumpCodeForOutputConnections(alreadyDumped);
  }
  for (const auto& node : _fragmentOutputNodes) {
    codeString += node->_dumpCodeForOutputConnections(alreadyDumped);
  }

  // Output nodes
  codeString += "\r\n// Output nodes\r\n";
  for (const auto& node : _vertexOutputNodes) {
    codeString += StringTools::printf("nodeMaterial->addOutputNode(%s);\r\n",
                                      node->_codeVariableName.c_str());
  }

  for (const auto& node : _fragmentOutputNodes) {
    codeString += StringTools::printf("nodeMaterial->addOutputNode(%s);\r\n",
                                      node->_codeVariableName.c_str());
  }

  codeString += "nodeMaterial->build();\r\n";

  return codeString;
}

json NodeMaterial::serialize() const
{
  return nullptr;
}

void NodeMaterial::_restoreConnections(const NodeMaterialBlockPtr& block, const json& source,
                                       const std::unordered_map<size_t, NodeMaterialBlockPtr>& map)
{
  for (const auto& outputPoint : block->outputs()) {
    for (const auto& candidate : json_util::get_array<json>(source, "blocks")) {
      auto candidateId = json_util::get_number<size_t>(candidate, "id");
      auto target      = map.at(candidateId);

      for (const auto& input : json_util::get_array<json>(source, "inputs")) {
        auto inputTargetBlockId = json_util::get_number<size_t>(input, "targetBlockId");
        if (map.at(inputTargetBlockId) == block
            && json_util::get_string(input, "targetConnectionName") == outputPoint->name) {
          auto inputPoint = target->getInputByName(json_util::get_string(input, "inputName"));
          if (!inputPoint || inputPoint->isConnected()) {
            continue;
          }

          outputPoint->connectTo(inputPoint, true);
          _restoreConnections(target, source, map);
          continue;
        }
      }
    }
  }
}

void NodeMaterial::loadFromSerialization(const json& /*source*/, const std::string& /*rootUrl*/)
{
}

MaterialPtr NodeMaterial::clone(const std::string& /*name*/, bool /*cloneChildren*/) const
{
  return nullptr;
}

NodeMaterialPtr NodeMaterial::Parse(const json& /*source*/, Scene* /*scene*/,
                                    const std::string& /*rootUrl*/)
{
  return nullptr;
}

NodeMaterialPtr NodeMaterial::CreateDefault(const std::string& iName, Scene* scene)
{
  auto newMaterial = NodeMaterial::New(iName, scene);
  newMaterial->setToDefault();
  newMaterial->build();

  return newMaterial;
}

} // end of namespace BABYLON
