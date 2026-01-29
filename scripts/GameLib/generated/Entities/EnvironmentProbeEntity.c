/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class EnvironmentProbeEntityClass: VolumeReceiverEntityClass
{
}

class EnvironmentProbeEntity: VolumeReceiverEntity
{
	proto external void SetProbePosition(vector position, bool invalidate);
	proto external void SetReflectionEV(float EV);
	proto external void SetDiffuseEV(float EV);
}

/*!
\}
*/
