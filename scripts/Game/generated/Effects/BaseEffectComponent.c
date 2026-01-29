/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Effects
\{
*/

class BaseEffectComponentClass: GameComponentClass
{
}

class BaseEffectComponent: GameComponent
{
	//! Returns the particle entity associated with this effect component.
	proto external IEntity GetParticleEntity();
	proto external SignalsManagerComponent GetSignalsManager();
	proto external bool HasActiveParticles();
}

/*!
\}
*/
