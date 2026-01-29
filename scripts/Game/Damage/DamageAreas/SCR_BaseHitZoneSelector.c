class SCR_BaseHitZoneSelector : ScriptAndConfig
{
	[Attribute(defvalue: "1", desc: "Should the game abort damage handling when no compatible hit zones were found, or should it try to use default area logic.")]
	protected bool m_bAbortWhenNoCompatibleHitZones;
	
	//------------------------------------------------------------------------------------------------
	//! Method used to find a specific hit zone. Override this to have custom logic for picking a hit zone
	//! \param[in] dmgMgr
	//! \param[in] hitZoneSelectionMode
	//! \return
	HitZone SelectHitZone(SCR_DamageManagerComponent dmgMgr, SCR_EHitZoneSelectionMode hitZoneSelectionMode);

	//------------------------------------------------------------------------------------------------
	//!
	//! \return true when damage application should be aborted due the fact that no compatible hit zones were found
	bool ShouldAbortWhenNoCompatibleHitZonesWereFound()
	{
		return m_bAbortWhenNoCompatibleHitZones;
	}
}