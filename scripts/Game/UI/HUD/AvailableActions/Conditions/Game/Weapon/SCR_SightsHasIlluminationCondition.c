[BaseContainerProps()]
class SCR_SightsHasIlluminationCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.CheckBox, desc: "True - find illumination of controlled turret | False - find current weapon in hand illumination")]
	protected bool m_bCheckTurret;

	//------------------------------------------------------------------------------------------------
	//! Return true if currently controlled vehicle turret has illumination for sights
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_2DOpticsComponent sights = null;

		if (m_bCheckTurret)
		{
			sights = CurrentTurretSight(data);
		}
		else
		{
			sights = CurrentWeaponSights(data);
		}

		if (!sights)
			return GetReturnResult(false);

		SCR_2DOpticsComponentClass sightsData = SCR_2DOpticsComponentClass.Cast(sights.GetComponentData(sights.GetOwner()));
		if (!sightsData)
			return GetReturnResult(false);

		// Has illumination
		return GetReturnResult(sightsData.HasIllumination());
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_2DOpticsComponent CurrentWeaponSights(SCR_AvailableActionsConditionData data)
	{
		BaseSightsComponent currentSights = data.GetCurrentSights();
		if (!currentSights)
			return null;

		return SCR_2DOpticsComponent.Cast(currentSights);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_2DOpticsComponent CurrentTurretSight(SCR_AvailableActionsConditionData data)
	{
		TurretComponent turret = data.GetCurrentTurret();
		if (!turret)
			return null;

		// Sights
		return SCR_2DOpticsComponent.Cast(turret.GetSights());
	}
}
