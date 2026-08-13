//------------------------------------------------------------------------------------------------
//! AI spawn-budget importance tier. Higher tier may consume a larger fraction of AIWorld's active-AI
//! limit before CanActivateGroup refuses. Mirrors the integer constants AI_IMPORTANCE_* on the C++
//! side (AILib/Entities/AIGroup.h). Caps enforced by AIWorld::CanActivateGroup:
//!
//!   LOW       capRatio = 0.50 - ambient patrols, filler
//!   NORMAL    capRatio = 0.70 - scripted patrols, remnants
//!   HIGH      capRatio = 0.90 - base defenders
//!   CRITICAL  capRatio = 1.00 - hired / mission-critical / player-led AI
//!
enum SCR_EAISpawnImportance
{
	LOW      = 0,
	NORMAL   = 1,
	HIGH     = 2,
	CRITICAL = 3
};
