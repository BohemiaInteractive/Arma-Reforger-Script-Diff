[BaseContainerProps(description: "Similar to SCR_ScenarioFrameworEntitykDamageStateCondition but with propulated selections for character and character hit zone groups.")]
class SCR_ScenarioFrameworkMedicalConditionHealth : SCR_ScenarioFrameworkMedicalConditionBase
{
	[Attribute(desc: "Minimum health of hit zone or group to satisfy condition. (Inclusive)", uiwidget: UIWidgets.Slider, defvalue: "0.0", params: "0 1 0.001")]
	float m_fHealthMinimum;

	[Attribute(desc: "Maximum health of hit zone or group to satisfy condition. (Inclusive)", uiwidget: UIWidgets.Slider, defvalue: "1.0", params: "0 1 0.001")]
	float m_fHealthMaximum;

	[Attribute(desc: "If true, uses the health hitpoint.", defvalue: "1")]
	bool m_bUseDefaultHealth;

	[Attribute(desc: "Is satisfied by health on any specified hit zone.", uiwidget: UIWidgets.EditComboBox, defvalue: "Head", enums: SCR_AttributesHelper.ParamFromTitles("Health;Blood;Resilience;Head;Chest;Abdomen;Hips;RArm;LArm;RForearm;LForearm;RHand;LHand;RThigh;LThigh;RCalf;LCalf;RFoot;LFoot;Neck;"))]
	ref array<string> m_aHitZoneNames;

	[Attribute(desc: "Is satisfied by health in any specified hit zone groups.", uiwidget: UIWidgets.ComboBox, defvalue: ECharacterHitZoneGroup.HEAD.ToString(), enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ref array<ECharacterHitZoneGroup> m_aHitZonesGroups;

	protected static const float S_F_HEALTH_EPSILON = 0.001;

	//------------------------------------------------------------------------------------------------
	override bool Init(notnull SCR_ChimeraCharacter character, notnull SCR_CharacterDamageManagerComponent damageManager)
	{
		super.Init(character, damageManager);

		if (m_fHealthMinimum > m_fHealthMaximum)
		{
			PrintFormat("[SCR_ScenarioFrameworkMedicalConditionHealth.Init] defined m_fHealthMinimum %1 is greater than m_fHealthMaximum %2", m_fHealthMinimum, m_fHealthMaximum, LogLevel.WARNING);
			return false;
		}

		if (m_bUseDefaultHealth)
			return EvaluateHealthScaled(damageManager.GetHealthScaled());

		foreach (string hitZoneName : m_aHitZoneNames)
		{
			HitZone hitZone = damageManager.GetHitZoneByName(hitZoneName);
			if (EvaluateHealthScaled(hitZone.GetHealthScaled()))
				return true;
		}

		foreach (ECharacterHitZoneGroup hitZoneGroup : m_aHitZonesGroups)
		{
			float healthScaled = damageManager.GetGroupHealthScaled(hitZoneGroup);
			if (EvaluateHealthScaled(healthScaled))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if health is within minimum and maximum range with epsilon tolerance.
	//! \param[in] health Evaluates if health is within range between minimum and maximum values, with tolerance for equality.
	//! \return Checks if health is within minimum and maximum range with epsilon tolerance.
	protected bool EvaluateHealthScaled(float health)
	{
		return
			(health >= m_fHealthMinimum || float.AlmostEqual(health, m_fHealthMinimum, S_F_HEALTH_EPSILON)) &&
			(health <= m_fHealthMaximum || float.AlmostEqual(health, m_fHealthMaximum, S_F_HEALTH_EPSILON));
	}
}
