[BaseContainerProps()]
class SCR_ScenarioFrameworkMedicalConditionTourniquet : SCR_ScenarioFrameworkMedicalConditionBase
{
	[Attribute(desc: "If true, is satisfied by tourniquet on any HitZone.", defvalue: "1")]
	bool m_bCanApplyTourniquetAnywhere;

	[Attribute(desc: "Is satisfied by tourniquet in any specified HitZoneGroups.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ref array<ECharacterHitZoneGroup> m_aHitZonesGroups;

	//------------------------------------------------------------------------------------------------
	override bool Init(notnull SCR_ChimeraCharacter character, notnull SCR_CharacterDamageManagerComponent damageManager)
	{
		super.Init(character, damageManager);

		if (m_bCanApplyTourniquetAnywhere)
			SCR_Enum.GetEnumValues(ECharacterHitZoneGroup, m_aHitZonesGroups);

		foreach (ECharacterHitZoneGroup hitZoneGroup : m_aHitZonesGroups)
		{
			if (damageManager.GetGroupTourniquetted(hitZoneGroup))
				return true;
		}

		return false;
	}
}
