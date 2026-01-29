/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SoundComponentClass: SimpleSoundComponentClass
{
}

class SoundComponent: SimpleSoundComponent
{
	//! Play a sound from the owner entity's position
	proto external AudioHandle SoundEvent(string eventName);
	//! Play a sound from a set transformation
	proto external AudioHandle SoundEventTransform(string eventName, vector transf[]);
	//! Play a sound from the owner entity's position
	//! If boneName is different than string.Empty, then you might need to first use SetTransformation(vector transf[]) or else sound may fail the auditability test
	proto external AudioHandle SoundEventBone(string eventName, string bone);
	//! Play a sound with a given offset from the owner entity
	//! If offset is different than vector.Zero, then you might need to first use SetTransformation(vector transf[]) or else sound may fail the auditability test
	proto external AudioHandle SoundEventOffset(string eventName, vector offset);
}

/*!
\}
*/
