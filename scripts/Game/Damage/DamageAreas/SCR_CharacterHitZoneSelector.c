class SCR_CharacterHitZoneSelector : SCR_BaseHitZoneSelector
{
	[Attribute(defvalue: ECharacterHitZoneGroup.VIRTUAL.ToString(), desc: "Character hit zone group.\nThis will try to return compatible hit zone, but only for characters, and for other entities it will return null.\nWhen radom selection is enabled, then game will choose random hit zone from the ones found for this group.", uiwidget: UIWidgets.ComboBox, enumType: ECharacterHitZoneGroup)]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;

	//------------------------------------------------------------------------------------------------
	override HitZone SelectHitZone(SCR_DamageManagerComponent dmgMgr, SCR_EHitZoneSelectionMode hitZoneSelectionMode)
	{
		SCR_CharacterDamageManagerComponent characterDmgMgr = SCR_CharacterDamageManagerComponent.Cast(dmgMgr);
		if (!characterDmgMgr)
			return null;

		array<HitZone> groupedHitZones = {};
		int count = dmgMgr.GetHitZonesOfGroup(m_eHitZoneGroup, groupedHitZones);
		if (count < 1)
			return null;

		if (count == 1 || hitZoneSelectionMode == SCR_EHitZoneSelectionMode.DEFAULT || hitZoneSelectionMode == SCR_EHitZoneSelectionMode.FIRST_PHYSICAL)
			return groupedHitZones[0];

		return groupedHitZones.GetRandomElement();
	}
}