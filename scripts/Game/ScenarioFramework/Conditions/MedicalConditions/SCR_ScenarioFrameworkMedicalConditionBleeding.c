[BaseContainerProps()]
class SCR_ScenarioFrameworkMedicalConditionBleeding : SCR_ScenarioFrameworkMedicalConditionBase
{
	[Attribute(desc: "If true, is satisfied by bleeding on any HitZone.", defvalue: "1")]
	bool m_bCanBleedAnywhere;

	[Attribute(desc: "Is satisfied by bleeding on any specified HitZone.", uiwidget: UIWidgets.EditComboBox, enums: SCR_AttributesHelper.ParamFromTitles("Health;Blood;Resilience;Head;Chest;Abdomen;Hips;RArm;LArm;RForearm;LForearm;RHand;LHand;RThigh;LThigh;RCalf;LCalf;RFoot;LFoot;Neck;"))]
	ref array<string> m_aHitZoneNames;

	[Attribute(desc: "Is satisfied by bleeding in any specified HitZoneGroups.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ref array<ECharacterHitZoneGroup> m_aHitZonesGroups;

	protected ref set<string> m_AllCheckedHitZones;

	//------------------------------------------------------------------------------------------------
	override bool Init(notnull SCR_ChimeraCharacter character, notnull SCR_CharacterDamageManagerComponent damageManager)
	{
		super.Init(character, damageManager);

		if (m_bCanBleedAnywhere)
			return damageManager.IsBleeding();

		array<HitZone> bleedingHitZones = damageManager.GetBleedingHitZones();
		if (!bleedingHitZones)
			return false;
		
		set<string> allCheckedHitZones = GetAllCheckedHitZones(damageManager);
		foreach (HitZone hitZone : bleedingHitZones)
		{
			if (allCheckedHitZones.Contains(hitZone.GetName()))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return A set of all checked hit zones' names based on m_aHitZoneNames and m_aHitZonesGroups.
	protected set<string> GetAllCheckedHitZones(notnull SCR_CharacterDamageManagerComponent damageManager)
	{
		if (!m_AllCheckedHitZones)
		{
			array<HitZone> hitZonesFromGroups = {};
			foreach (ECharacterHitZoneGroup hitZoneGroup : m_aHitZonesGroups)
			{
				damageManager.GetHitZonesOfGroup(hitZoneGroup, hitZonesFromGroups, clearArray: false);
			}

			m_AllCheckedHitZones = new set<string>();
			m_AllCheckedHitZones.Reserve(m_aHitZoneNames.Count() + hitZonesFromGroups.Count());

			foreach (string hitZoneName : m_aHitZoneNames)
			{
				m_AllCheckedHitZones.Insert(hitZoneName);
			}
			foreach (HitZone hitZone : hitZonesFromGroups)
			{
				m_AllCheckedHitZones.Insert(hitZone.GetName());
			}
		}
		return m_AllCheckedHitZones;
	}
}
