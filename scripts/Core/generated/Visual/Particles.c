/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Visual
\{
*/

/*!
Script API to Particle Effect Component.

Please note that some emitters may not be present under some environments.
E.g. the dedicated server will skip loading of most emitters as they are purely visual
(on the other hand, L3 emitters with non-zero occlusion cannot be skipped as they can be used by AI).
This also means some methods can take different arguments or give different results under different environments,
e.g. GetNumEmitters, GetEmitterNames, GetNumParticles or anything working with an emitter index.
*/
sealed class Particles: pointer
{
	/*!
	Sets a physical velocity source (entity with a PhysicsComponent) for the particle effect.

	If the physical velocity source is available, the effect will get parent velocity from that (which is preferred).
	Otherwise the effect calculates the velocity from current position, previous position and delta time.

	\param ent Entity with a PhysicsComponent to use as a velocity source or null
	*/
	static proto void SetVelocitySource(IEntity ent);
	/*!
	Restarts the whole effect.
	I.e. destroys all particles and resets emitting time to zero, etc.

	\param invalidatePrevPos  If true, the effect's previous position is invalidated and parent velocity set to zero.
	                          Useful e.g. for particle effect pooling where the effect is "teleported" and restarted.
	                          If false (the default), the previous position is kept so it is available right in the next frame.
	                          Useful when the parent entity's position is being continuously updated every frame
	                          even when the effect is not playing at the moment.
	*/
	proto external void Restart(bool invalidatePrevPos = false);
	/*!
	Invalidates stored position of the owner (parent) entity from previous frame.

	The previous position is used for parent velocity computation (if there's no physical velocity source set),
	distributing newly emitted particles etc.

	Should be used e.g. when owner entity is teleported or its transform is corrected by the networking code.

	\param parentVelocity Optional parent velocity vector to be used if there's no physical velocity source
	                      until the previous position is available again
	*/
	proto external void InvalidatePrevPos(vector parentVelocity = vector.Zero);
	/*!
	Manually sets a new value for the previous position of the owner (parent) entity.

	The previous position is used for parent velocity computation (if there's no physical velocity source set),
	distributing newly emitted particles etc.

	Normally, the position is set automatically by the particle system at the end of each frame.
	This allows to override the automatically stored position	or it may be used after the position has been invalidated.
	*/
	proto external void OverridePrevPos(vector pos);
	/*!
	Schedules a simulation step (update) of the particle effect.
	Expected to be called once per frame.
	The actual simulation will happen later in the same frame.

	\param deltaTime duration since the last call

	\return True iff the particle effect has finished (so it doesn't and won't have any more particles).
	        Delayed by one frame due to the async nature of the operation.
	*/
	proto external bool  Simulate(float deltaTime);
	/*!
	Schedules simulation of the particle effect over given amount of time in multiple steps.
	The actual simulation will happen later in this frame.

	Approximates simulation result after multiple frames	(i.e. after several calls to Simulate or Animate).
	Intended for "pre-warming" of the effect.

	\param duration Total time in seconds to simulate
	\param minFPS   Minimum number of simulation steps per a second of the duration.
	                Actual number of steps may be higher (e.g. due to minimum FPS specified in the effect)
									but it will never be lower.

	\return True iff the particle effect has finished (so it doesn't and won't have any more particles).
	        Delayed by one frame due to the async nature of the operation.
	*/
	proto external bool  SimulateMultiStep(float duration, float minFPS);
	/*!
	Gets the count of all emitters in the effect.
	*/
	proto external int GetNumEmitters();
	/*!
	Gets total active particle count (from all effect's emitters).
	*/
	proto external int GetNumParticles();
	/*!
	Sets a particle emitter parameter value.
	\param emitterIdx Index of the emitter to set. -1 to set it for all emitters of the effect.

	\param param      The parameter to set
	\param value      The value to set the parameter to
	*/
	proto external void SetParam(int emitterIdx, EmitterParam param, void value);
	/*!
	Gets the current value of a particle emitter parameter.

	\param emitterIdx Index of the emitter to get the value from
	\param param      The parameter to get the value of
	\param value      Output of the method - the value
	*/
	proto external void GetParam(int emitterIdx, EmitterParam param, out void value);
	/*!
	Gets a value of a particle emitter parameter from the effect definition (the default).

	\param emitterIdx Index of the emitter to get the value from
	\param param      The parameter to get the default value of
	\param value      Output of the method - the value
	*/
	proto external void GetParamOrig(int emitterIdx, EmitterParam param, out void value);
	/*!
	Sets a given parameter of a given emitter to the multiple of its ORIGINAL value from the effect's definition.

	E.g. if the emitter #2 has birthrate = 100 in its definition then
	MultParam(2, EmitterParam.BIRTH_RATE, 0.50) will set it to 50 and a successive call of
	MultParam(2, EmitterParam.BIRTH_RATE, 0.75) will set it to 75.

	\param emitterIdx Index of the emitter to set, -1 = "all emitters"
	\param param      Which parameter to set
	\param mul        The multiplier of the original value
	*/
	proto external void MultParam(int emitterIdx, EmitterParam param, float mul);
	/*!
	Gets an array with the names of all emitters of the effect.

	\param names Initialized array to which the names will be added.
	             If the array is empty, the name of emitter with index = 0 will be at names[0], etc.
	\return Number of emitters in the effect (how many items were added to the array).
	*/
	proto external int GetEmitterNames(out array<string> names);
}

/*!
\}
*/
