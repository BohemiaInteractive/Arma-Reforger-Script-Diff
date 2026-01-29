class SCR_FuelHitZone : SCR_DestructibleHitzone
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Auto, desc: "Fuel Tank ID", category: "Fuel Damage Config")]
	protected int m_iFuelTankID;

	[Attribute(EVehicleHitZoneGroup.FUEL_TANKS.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eHitZoneGroup;

	[Attribute("0.5", desc: "Minimum factor for determining explosion scale from fuel amount", category: "Fuel Damage Config")]
	protected float m_fMinExplosionScaleFactor;

	protected SCR_FuelNode m_FuelTank;

	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		SetFuelNodeID(m_iFuelTankID);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateFuelTankState()
	{
		if (!m_FuelTank)
			return;

		EDamageState state = GetDamageState();
		float health = GetDamageStateThreshold(state);
		m_FuelTank.SetHealth(health);
	}

	//------------------------------------------------------------------------------------------------
	SCR_FuelNode GetFuelTank()
	{
		return m_FuelTank;
	}

	//------------------------------------------------------------------------------------------------
	int GetFuelTankID()
	{
		return m_iFuelTankID;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign hitzone to a simulation fuel tank
	void SetFuelNodeID(int fuelNodeID)
	{
		m_iFuelTankID = fuelNodeID;

		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return;

		SCR_FuelNode scrFuelNode;
		array<BaseFuelNode> fuelNodes = {};
		fuelManager.GetFuelNodesList(fuelNodes);
		foreach (BaseFuelNode fuelNode : fuelNodes)
		{
			scrFuelNode = SCR_FuelNode.Cast(fuelNode);
			if (scrFuelNode && scrFuelNode.GetFuelTankID() == m_iFuelTankID)
			{
				m_FuelTank = scrFuelNode;
				break;
			}
		}

#ifdef WORKBENCH
		if (!m_FuelTank)
			PrintFormat("SCR_FuelHitZone %1: fuel tank #%2 not found in %3", GetName(), m_iFuelTankID, GetOwner(), level: LogLevel.WARNING);
#endif // WORKBENCH

		UpdateFuelTankState();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		UpdateFuelTankState();
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	override float GetSecondaryExplosionScale()
	{
		if (!m_FuelTank)
			return 0;

		// Truck addon fuel cargo that does not have its own health
		float damage;
		if (m_RootDamageManager && m_RootDamageManager.GetDefaultHitZone().GetDamageState() == EDamageState.DESTROYED)
			damage = 1;
		else if (GetBaseDamageMultiplier() == 0 && GetHitZoneContainer().GetDefaultHitZone() == this)
			damage = 1;
		else
		 	damage = Math.Lerp(m_fMinExplosionScaleFactor, 1, 1 - GetDamageStateThreshold(GetDamageState()));

		// TODO: Curve based on fuel to air ratio
		return m_FuelTank.GetFuel() * damage;
	}
}
