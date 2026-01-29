/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterHeadingAnimComponentClass: GenericComponentClass
{
}

//!
class CharacterHeadingAnimComponent: GenericComponent
{
	//! tries to align myPredictedPos so its aligned with real pos
	//! tries to rotate internally so pMyPredictedDir will align with pRealDir
	//! Use bSnap parameter if you don't want the alignment to be smooth.
	proto external void AlignPosDirWS(vector pPredictedPos, vector pPredictedDir, vector pTargetPos, vector pTargetDir, bool bSnap = false);
	proto external void AlignPosRotWS(vector pPredictedPos, float pPredictedRot[4], vector pTargetPos, float pTargetRot[4], bool bSnap = false);
	proto external void AlignRotWS(float pPredictedRot[4], float pTargetRot[4], bool bSnap = false);
	proto external void ResetAligning();
	proto external void SetFiltering(float pFilterTimeout, float pMaxVel);
	proto external bool IsAligning();
}

/*!
\}
*/
