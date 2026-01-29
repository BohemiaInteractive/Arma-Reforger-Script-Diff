//#define SHOW_DMG_INDICATORS_WARNING
//#define SHOW_DMG_INDICATORS_ERROR

class SCR_FuelTankInfo : SCR_BaseVehicleInfo
{
	[Attribute(uiwidget: UIWidgets.Auto, desc: "Fuel Tank IDs\nLeave empty to collect total fuel")]
	protected ref array<int> m_aFuelTankIDs;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Auto, desc: "Reserve fuel error level\n[%]", params: "0 100 0")]
	protected float m_fReserveFuelLevel;

	[Attribute(defvalue: "25", uiwidget: UIWidgets.Auto, desc: "Low fuel warning level\n[%]", params: "0 100 0")]
	protected float m_fLowFuelLevel;

	// TODO: Specific fuel tank IDs, -1 for total fuel - can track fuel leaks when IDs are provided.
	protected FuelManagerComponent m_pFuelManager;
	protected ref array<BaseFuelNode> m_aFuelTanks = {};

	//------------------------------------------------------------------------------------------------
	//! Blink conditions
	override bool IsBlinking()
	{
		SCR_FuelNode scrFuelTank;
		foreach (BaseFuelNode fuelTank : m_aFuelTanks)
		{
			scrFuelTank = SCR_FuelNode.Cast(fuelTank);
			if (scrFuelTank && scrFuelTank.GetLeakableFuel() > 0)
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		#ifdef SHOW_DMG_INDICATORS_ERROR
		return EVehicleInfoState.ERROR;
		#endif
		
		#ifdef SHOW_DMG_INDICATORS_WARNING
		return EVehicleInfoState.WARNING;
		#endif
		
		if (!m_pFuelManager)
			return EVehicleInfoState.DISABLED;

		// BLINKING: currently leaking
		// register to an event. If leaking, return blinking

		float fuel = 1;
		float maxFuel;
		if (m_aFuelTanks.IsEmpty())
		{
			fuel = m_pFuelManager.GetTotalFuel();
			maxFuel = m_pFuelManager.GetTotalMaxFuel();
		}
		else
		{
			// Analyze all connected fuel tanks
			foreach (BaseFuelNode fuelTank : m_aFuelTanks)
			{
				fuel += fuelTank.GetFuel();
				maxFuel += fuelTank.GetMaxFuel();
			}
		}

		if (maxFuel > 0)
			fuel /= maxFuel;

		// ERROR: < reserve fuel
		// WARNING: < low fuel or leaking
		if (fuel <= m_fReserveFuelLevel * 0.01)
			return EVehicleInfoState.ERROR;
		else if (fuel <= m_fLowFuelLevel * 0.01)
			return EVehicleInfoState.WARNING;
		else if (IsBlinking())
			return EVehicleInfoState.WARNING;

		return EVehicleInfoState.DISABLED;
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no fuel manager
		if (!m_pFuelManager)
			return false;

		return super.DisplayStartDrawInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);

		m_pFuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));

		array<BaseFuelNode> fuelTanks = {};
		int count = m_pFuelManager.GetFuelNodesList(fuelTanks);

		if (m_aFuelTankIDs.IsEmpty())
		{
			m_aFuelTanks.InsertAll(fuelTanks);
			return;
		}

		foreach (int fuelTankID : m_aFuelTankIDs)
		{
			if (fuelTanks.IsIndexValid(fuelTankID))
				m_aFuelTanks.Insert(fuelTanks[fuelTankID]);
		}
	}
};
