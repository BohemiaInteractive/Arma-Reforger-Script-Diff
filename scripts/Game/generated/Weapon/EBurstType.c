/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

enum EBurstType
{
	BT_Uninterruptable,
	// Releasing during the burst "pauses" it, and the next trigger depress continues the same burst. Used on (for example) an M16A2
	BT_Interruptable,
	// Releasing during the burst interrupts it, and resets so that the next burst is a full one. Used on (for example) a H&K G3 and MP5
	BT_InterruptableAndResetting,
}

/*!
\}
*/
