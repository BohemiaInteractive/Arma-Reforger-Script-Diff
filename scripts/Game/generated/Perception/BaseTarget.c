/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Perception
\{
*/

class BaseTarget: ScriptAndConfig
{
	/*!
	Returns target entity
	*/
	proto external IEntity GetTargetEntity();
	proto external ETargetCategory GetTargetCategory();
	// Time passed since target was detected
	proto external float GetTimeSinceDetected();
	// Time passed since the target was seen (direct LoS)
	proto external float GetTimeSinceSeen();
	// Time (from perception manager) when the target was seen (direct LoS)
	proto external float GetTimeLastSeen();
	// Time (from perception manager) when the target was detected
	proto external float GetTimeLastDetected();
	// Time passed since the target's type was recognized
	proto external float GetTimeSinceTypeRecognized();
	// Time passed since the target's side was recognized
	proto external float GetTimeSinceSideRecognized();
	// Time passed since the target endangered us
	proto external float GetTimeSinceEndangered();
	// Position where the target was seen last
	proto external vector GetLastSeenPosition();
	// Position where the target was detected last
	proto external vector GetLastDetectedPosition();
	// Returns distance to target. It's a cached value, calculated by PerceptionComponent.
	proto external float GetDistance();
	// Returns unit type, same as in PerceivableComponent of that target
	proto external EAIUnitType GetUnitType();
	// Returns how we percieve whether that target is endangering us or not. The actual value is updated periodically.
	proto external bool IsEndangering();
	// Returns how we perceive whether the target is disarmed or not. The actual value is updated periodically.
	proto external bool IsDisarmed();
	/*!
	Returns value from 0 to 1.0 representing where is obstruction which blocks our visibility of the target.
	1.0 - We could fully trace to target AND exposure is above zero
	0.0 < ... < 1.0 - Target has no visible aimpoints, trace is interrupted by obstacle
	0.0 - Target is not in view cone OR is out of perception range
	*/
	proto external float GetTraceFraction();
	/*
	Returns a value from 0 to 1.0 which shows how much target is visible based on visibility of each aimpoint.
	This is affected both by amount of visible aimpoint and by how much each of them is visible.
	*/
	proto external float GetExposure();
	// Returns perceivable component of target
	proto external PerceivableComponent GetPerceivableComponent();
	proto external DamageManagerComponent GetDamageManagerComponent();
	proto external BaseWeaponManagerComponent GetWeaponManagerComponent();
	//! Returns PerceivableComponent.GetFactionAffiliationComponent()
	proto external FactionAffiliationComponent GetFactionAffiliationComponent();
	//! Returns PerceivableComponent.GetPerceivedFaction()
	proto external Faction GetPerceivedFaction();
	proto void GetAccumulatedRecognition(out float outRecognitionDetect, out float outRecognitionIdentify);
	// Sets the last seen position to provided value if the timestamp is newer than existing last seen time
	proto external void UpdateLastSeenPosition(vector pos, float perceptionMgrTimestamp);
}

/*!
\}
*/
