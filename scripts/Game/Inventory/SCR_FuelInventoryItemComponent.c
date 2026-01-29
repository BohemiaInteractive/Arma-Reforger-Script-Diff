[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_FuelInventoryItemComponentClass : InventoryItemComponentClass
{
}

class SCR_FuelInventoryItemComponent : InventoryItemComponent
{	
	[Attribute("1", desc: "Weight of fuel per liter to update the inventory weight", params: "0 inf", category: "Fuel")]
	protected float m_fFuelWeightPerLiter;
	
	protected SCR_FuelManagerComponent m_FuelManager;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_FuelInventoryItemComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;
		
		//~ Call a frame later so fuel manager can init correctly
		GetGame().GetCallqueue().CallLater(DelayedInit);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit()
	{
		m_FuelManager = SCR_FuelManagerComponent.Cast(GetOwner().FindComponent(SCR_FuelManagerComponent));		
		if (m_FuelManager)
		{
			m_FuelManager.GetOnFuelChanged().Insert(OnFuelAmountChanged);
			OnFuelAmountChanged(m_FuelManager.GetTotalFuel());
		}
		else 
			Debug.Error2("SCR_FuelInventoryItemComponent", "Fuel Inventory Item does not have a 'SCR_FuelManagerComponent'!");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFuelAmountChanged(float newFuelValue)
	{
		SetAdditionalWeight(newFuelValue * m_fFuelWeightPerLiter);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_FuelInventoryItemComponent()
	{
		if (m_FuelManager)
			m_FuelManager.GetOnFuelChanged().Remove(OnFuelAmountChanged);
	}
}
