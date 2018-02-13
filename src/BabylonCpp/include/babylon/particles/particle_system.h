#ifndef BABYLON_PARTICLES_PARTICLE_SYSTEM_H
#define BABYLON_PARTICLES_PARTICLE_SYSTEM_H

#include <babylon/animations/ianimatable.h>
#include <babylon/babylon_global.h>
#include <babylon/math/color4.h>
#include <babylon/math/vector3.h>
#include <babylon/particles/iparticle_emitter_Type.h>
#include <babylon/particles/iparticle_system.h>
#include <babylon/tools/observable.h>
#include <babylon/tools/observer.h>

namespace BABYLON {

/**
 * @brief This represents a particle system in Babylon.
 * Particles are often small sprites used to simulate hard-to-reproduce
 * phenomena like fire, smoke, water, or abstract visual effects like magic
 * glitter and faery dust. Particles can take different shapes while emitted
 * like box, sphere, cone or you can write your custom function.
 * Example: https://doc.babylonjs.com/babylon101/particles
 */
class BABYLON_SHARED_EXPORT ParticleSystem : public IAnimatable,
                                             public IParticleSystem {

public:
  /** Statics **/
  /**
   * Source color is added to the destination color without alpha affecting the
   * result.
   */
  static constexpr int BLENDMODE_ONEONE = 0;
  /**
   * Blend current color and particle color using particle’s alpha.
   */
  static constexpr int BLENDMODE_STANDARD = 1;

public:
  /**
   * @brief Instantiates a particle system.
   * Particles are often small sprites used to simulate hard-to-reproduce
   * phenomena like fire, smoke, water, or abstract visual effects like magic
   * glitter and faery dust.
   * @param name The name of the particle system
   * @param capacity The max number of particles alive at the same time
   * @param scene The scene the particle system belongs to
   * @param customEffect a custom effect used to change the way particles are
   * rendered by default
   * @param isAnimationSheetEnabled Must be true if using a spritesheet to
   * animate the particles texture
   * @param epsilon Offset used to render the particles
   */
  ParticleSystem(const string_t& name, size_t capacity, Scene* scene,
                 Effect* customEffect         = nullptr,
                 bool isAnimationSheetEnabled = false, float epsilon = 0.01f);
  virtual ~ParticleSystem() override;

  virtual IReflect::Type type() const override;

  /**
   * @brief Sets a callback that will be triggered when the system is disposed.
   */
  void setOnDispose(
    const ::std::function<void(ParticleSystem*, EventState&)>& callback);

  /**
   * @brief Gets wether an animation sprite sheet is enabled or not on the
   * particle system.
   */
  bool isAnimationSheetEnabled() const;

  /**
   * @brief "Recycles" one of the particle by copying it back to the "stock" of
   * particles and removing it from the active list. Its lifetime will start
   * back at 0.
   * @param particle The particle to recycle
   */
  void recycleParticle(Particle* particle);

  /**
   * @brief Gets the maximum number of particles active at the same time.
   * @returns The max number of active particles.
   */
  size_t getCapacity() const;

  /**
   * @brief Gets Wether there are still active particles in the system.
   * @returns True if it is alive, otherwise false.
   */
  bool isAlive() const;

  /**
   * @brief Gets Wether the system has been started.
   * @returns True if it has been started, otherwise false.
   */
  bool isStarted() const override;

  /**
   * @brief Starts the particle system and begins to emit.
   */
  void start();

  /**
   * @brief Stops the particle system.
   */
  void stop();

  /**
   * @brief ignore (for internal use only)
   */
  void _appendParticleVertex(unsigned int index, Particle* particle,
                             int offsetX, int offsetY);

  /**
   * @brief ignore (for internal use only)
   */
  void _appendParticleVertexWithAnimation(unsigned int index,
                                          Particle* particle, int offsetX,
                                          int offsetY);

  /**
   * @brief Animates the particle system for the current frame by emitting new
   * particles and or animating the living ones.
   */
  void animate() override;

  /**
   * @brief Rebuilds the particle system.
   */
  void rebuild() override;

  /**
   * @brief Renders the particle system in its current state.
   * @returns the current number of particles.
   */
  size_t render() override;

  /**
   * @brief Disposes the particle system and free the associated resources.
   */
  void dispose(bool doNotRecurse = false) override;

  vector_t<Animation*> getAnimations() override;

  /**
   * @brief Creates a Sphere Emitter for the particle system. (emits along the
   * sphere radius)
   * @param radius The radius of the sphere to emit from
   * @returns the emitter
   */
  SphereParticleEmitter* createSphereEmitter(float radius = 1.f);

  /**
   * @brief Creates a Directed Sphere Emitter for the particle system. (emits
   * between direction1 and direction2)
   * @param radius The radius of the sphere to emit from
   * @param direction1 Particles are emitted between the direction1 and
   * direction2 from within the sphere
   * @param direction2 Particles are emitted between the direction1 and
   * direction2 from within the sphere
   * @returns the emitter
   */
  SphereDirectedParticleEmitter* createDirectedSphereEmitter(
    float radius = 1.f, const Vector3& direction1 = Vector3(0.f, 1.f, 0.f),
    const Vector3& direction2 = Vector3(0.f, 1.f, 0.f));

  /**
   * @brief Creates a Cone Emitter for the particle system. (emits from the cone
   * to the particle position)
   * @param radius The radius of the cone to emit from
   * @param angle The base angle of the cone
   * @returns the emitter
   */
  ConeParticleEmitter* createConeEmitter(float radius = 1.f,
                                         float angle  = Math::PI_4);

  // this method needs to be changed when breaking changes will be allowed
  // to match the sphere and cone methods and properties direction1,2 and
  // minEmitBox,maxEmitBox to be removed from the system.
  /**
   * @brief Creates a Box Emitter for the particle system. (emits between
   * direction1 and direction2 from withing the box defined by minEmitBox
   * and maxEmitBox)
   * @param direction1 Particles are emitted between the direction1 and
   * direction2 from within the box
   * @param direction2 Particles are emitted between the direction1 and
   * direction2 from within the box
   * @param minEmitBox Particles are emitted from the box between minEmitBox
   * and maxEmitBox
   * @param maxEmitBox  Particles are emitted from the box between
   * minEmitBox and maxEmitBox
   * @returns the emitter
   */
  BoxParticleEmitter* createBoxEmitter(const Vector3& direction1,
                                       const Vector3& direction2,
                                       const Vector3& minEmitBox,
                                       const Vector3& maxEmitBox);

  /**
   * @brief Clones the particle system.
   * @param name The name of the cloned object
   * @param newEmitter The new emitter to use
   * @returns the cloned particle system
   */
  IParticleSystem* clone(const string_t& name, Mesh* newEmitter) override;

  /**
   * @brief Serializes the particle system to a JSON object.
   * @returns the JSON object
   */
  Json::object serialize() const override;

  /**
   * @brief Parses a JSON object to create a particle system.
   * @param parsedParticleSystem The JSON object to parse
   * @param scene The scene to create the particle system in
   * @param rootUrl The root url to use to load external dependencies like
   * texture
   * @returns the Parsed particle system
   */
  static ParticleSystem* Parse(const Json::value& parsedParticleSystem,
                               Scene* scene, const string_t& url);

private:
  void _createIndexBuffer();
  void _update(int newParticles);
  Effect* _getEffect();
  void _appenedParticleVertexesWithSheet(unsigned int offset,
                                         Particle* particle);
  void _appenedParticleVertexesNoSheet(unsigned int offset, Particle* particle);

public:
  /**
   * List of animations used by the particle system.
   */
  vector_t<Animation*> animations;

  /**
   * The density of particles, the rate of particle flow
   */
  int emitRate;

  /**
   * If you want to launch only a few particles at once, that can be done, as
   * well.
   */
  int manualEmitCount;

  /**
   * The overall motion speed (0.01 is default update speed, faster updates =
   * faster animation)
   */
  float updateSpeed;

  /**
   * The amount of time the particle system is running (depends of the overall
   * speed above).
   */
  int targetStopDuration;

  /**
   * Specifies whether the particle system will be disposed once it reaches the
   * end of the animation.
   */
  bool disposeOnStop;

  /**
   * Minimum power of emitting particles.
   */
  float minEmitPower;
  /**
   * Maximum power of emitting particles.
   */
  float maxEmitPower;

  /**
   * Minimum life time of emitting particles.
   */
  float minLifeTime;
  /**
   * Maximum life time of emitting particles.
   */
  float maxLifeTime;

  /**
   * Minimum Size of emitting particles.
   */
  float minSize;
  /**
   * Maximum Size of emitting particles.
   */
  float maxSize;

  /**
   * Minimum angular speed of emitting particles (Z-axis rotation for each
   * particle).
   */
  float minAngularSpeed;
  /**
   * Maximum angular speed of emitting particles (Z-axis rotation for each
   * particle).
   */
  float maxAngularSpeed;

  /**
   * The texture used to render each particle. (this can be a spritesheet)
   */
  Texture* particleTexture;

  /**
   * This can help using your own shader to render the particle system.
   * The according effect will be created
   */
  string_t customShader;

  /**
   * By default particle system starts as soon as they are created. This
   * prevents the automatic start to happen and let you decide when to start
   * emitting particles.
   */
  bool preventAutoStart;

  /**
   * This function can be defined to provide custom update for active particles.
   * This function will be called instead of regular update (age, position,
   * color, etc.). Do not forget that this function will be called on every
   * frame so try to keep it simple and fast :)
   */
  ::std::function<void(vector_t<Particle*>& particles)> updateFunction;

  /**
   * Callback triggered when the particle animation is ending.
   */
  ::std::function<void()> onAnimationEnd;

  /**
   * Blend mode use to render the particle, it can be either
   * ParticleSystem.BLENDMODE_ONEONE or ParticleSystem.BLENDMODE_STANDARD.
   */
  unsigned int blendMode;

  /**
   * Forces the particle to write their depth information to the depth buffer.
   * This can help preventing other draw calls to override the particles.
   */
  bool forceDepthWrite;

  /**
   * You can use gravity if you want to give an orientation to your particles.
   */
  Vector3 gravity;

  /**
   * Random direction of each particle after it has been emitted, between
   * direction1 and direction2 vectors.
   */
  Vector3 direction1;
  /**
   * Random direction of each particle after it has been emitted, between
   * direction1 and direction2 vectors.
   */
  Vector3 direction2;

  /**
   * Minimum box point around our emitter. Our emitter is the center of
   * particles source, but if you want your particles to emit from more than one
   * point, then you can tell it to do so.
   */
  Vector3 minEmitBox;
  /**
   * Maximum box point around our emitter. Our emitter is the center of
   * particles source, but if you want your particles to emit from more than one
   * point, then you can tell it to do so.
   */
  Vector3 maxEmitBox;

  /**
   * Random color of each particle after it has been emitted, between color1 and
   * color2 vectors.
   */
  Color4 color1;
  /**
   * Random color of each particle after it has been emitted, between color1 and
   * color2 vectors.
   */
  Color4 color2;
  /**
   * Color the particle will have at the end of its lifetime.
   */
  Color4 colorDead;

  /**
   * An optional mask to filter some colors out of the texture, or filter a part
   * of the alpha channel.
   */
  Color4 textureMask;

  /**
   * The particle emitter type defines the emitter used by the particle system.
   * It can be for example box, sphere, or cone...
   */
  unique_ptr_t<IParticleEmitterType> particleEmitterType;

  /**
   * This function can be defined to specify initial direction for every new
   * particle. It by default use the emitterType defined function.
   */
  ::std::function<void(float emitPower, const Matrix& worldMatrix,
                       Vector3& directionToUpdate, Particle* particle)>
    startDirectionFunction;
  /**
   * This function can be defined to specify initial position for every new
   * particle. It by default use the emitterType defined function.
   */
  ::std::function<void(const Matrix& worldMatrix, Vector3& positionToUpdate,
                       Particle* particle)>
    startPositionFunction;

  /**
   * If using a spritesheet (isAnimationSheetEnabled), defines if the sprite
   * animation should loop between startSpriteCellID and endSpriteCellID or not.
   */
  bool spriteCellLoop;
  /**
   * If using a spritesheet (isAnimationSheetEnabled) and spriteCellLoop defines
   * the speed of the sprite loop.
   */
  float spriteCellChangeSpeed;
  /**
   * If using a spritesheet (isAnimationSheetEnabled) and spriteCellLoop defines
   * the first sprite cell to display.
   */
  unsigned int startSpriteCellID;
  /**
   * If using a spritesheet (isAnimationSheetEnabled) and spriteCellLoop defines
   * the last sprite cell to display.
   */
  unsigned int endSpriteCellID;
  /**
   * If using a spritesheet (isAnimationSheetEnabled), defines the sprite cell
   * width to use.
   */
  unsigned int spriteCellWidth;
  /**
   * If using a spritesheet (isAnimationSheetEnabled), defines the sprite cell
   * height to use.
   */
  unsigned int spriteCellHeight;

  /**
   * An event triggered when the system is disposed.
   */
  Observable<ParticleSystem> onDisposeObservable;

  unsigned int _vertexBufferSize;

private:
  Observer<ParticleSystem>::Ptr _onDisposeObserver;
  vector_t<Particle*> _particles;
  float _epsilon;
  size_t _capacity;
  Scene* _scene;
  vector_t<Particle*> _stockParticles;
  int _newPartsExcess;
  Float32Array _vertexData;
  unique_ptr_t<Buffer> _vertexBuffer;
  unordered_map_t<string_t, unique_ptr_t<VertexBuffer>> _vertexBuffers;
  unique_ptr_t<GL::IGLBuffer> _indexBuffer;
  Effect* _effect;
  Effect* _customEffect;
  string_t _cachedDefines;

  Color4 _scaledColorStep;
  Color4 _colorDiff;
  Vector3 _scaledDirection;
  Vector3 _scaledGravity;
  int _currentRenderId;

  bool _alive;
  bool _started;
  bool _stopped;
  int _actualFrame;
  int _scaledUpdateSpeed;

  bool _isAnimationSheetEnabled;

  ::std::function<void(unsigned int offset, Particle* particle)>
    _appendParticleVertexes;

}; // end of class ParticleSystem

} // end of namespace BABYLON

#endif // end of BABYLON_PARTICLES_PARTICLE_SYSTEM_H
