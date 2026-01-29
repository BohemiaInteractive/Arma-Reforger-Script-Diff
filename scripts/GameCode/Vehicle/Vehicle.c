//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Separate file, also it should have SCR_ prefix
enum EVehicleType // should be SCR_EVehicleType
{
	VEHICLE 		= 0,
	CAR 			= 2,
	TRUCK 			= 4,
	APC 			= 8,
	FUEL_TRUCK 		= 16,
	COMM_TRUCK 		= 32,
	SUPPLY_TRUCK 	= 64,
	MORTAR			= 128,
	REPAIR			= 256,
	ARSENAL			= 512,
}
//---- REFACTOR NOTE END ----

class VehicleClass : BaseVehicleClass
{
}

class Vehicle : BaseVehicle
{
	// Can be removed when EOnPhysicsActive is on components as well
	protected ref ScriptInvoker<IEntity, bool> m_OnPhysicsActive;
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Attributes should be protected and use proper getter
	[Attribute("0", UIWidgets.ComboBox, "Vehicle type:", "", ParamEnumArray.FromEnum(EVehicleType) )]
	EVehicleType m_eVehicleType;
	//---- REFACTOR NOTE END ----
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Module variable should be protected and have getter instead, also _p is wrong prefix
	SCR_VehicleFactionAffiliationComponent m_pFactionComponent;
	SCR_VehiclePerceivableComponent m_pPerceivableComponent;
	//---- REFACTOR NOTE END ----
	protected VehicleControllerComponent m_VehicleController;
	protected SCR_ResourceComponent m_ResourceComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnPhysicsActive()
	{
		if (!m_OnPhysicsActive)
			m_OnPhysicsActive = new ScriptInvoker();
		
		return m_OnPhysicsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (m_OnPhysicsActive)
			m_OnPhysicsActive.Invoke(owner, activeState);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();
		
		return null;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_VehicleFactionAffiliationComponent GetFactionAffiliation()
	{
		return m_pFactionComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetDefaultFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetDefaultAffiliatedFaction();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsOccupied()
	{	
		if (m_pFactionComponent)
			return m_pFactionComponent.IsVehicleOccupied();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get vehicle controller
	VehicleControllerComponent GetVehicleController()
	{
		return m_VehicleController;
	}

	//------------------------------------------------------------------------------------------------
	//! Get primary pilot compartment slot
	PilotCompartmentSlot GetPilotCompartmentSlot()
	{
		if (m_VehicleController)
			return m_VehicleController.GetPilotCompartmentSlot();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return pilot of the vehicle - occupant of the primary pilot compartment
	IEntity GetPilot()
	{
		PilotCompartmentSlot slot = GetPilotCompartmentSlot();
		if (slot)
			return slot.GetOccupant();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ResourceComponent GetResourceComponent()
	{
		return m_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateResourceComponent()
	{
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(this);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems()
	{
		array<ref WB_UIMenuItem> items = { new WB_UIMenuItem("Compute collision damage", 0) };

		return items;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(int id)
	{
		switch (id)
		{
			case 0:
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				if (!api)
					return;

				SCR_VehicleDamageManagerComponent dmgManagerComp = SCR_VehicleDamageManagerComponent.Cast(FindComponent(SCR_VehicleDamageManagerComponent));
				if (!dmgManagerComp)
					return;

				float criticalFrontalDamage = dmgManagerComp.CalculateCriticalCollisionDamage();
				if (criticalFrontalDamage <= 0)
				{
					Print("Error during critical collision damage callculation (returned value = " + criticalFrontalDamage + "). Check damage manager setup!", LogLevel.ERROR);
					return;
				}

				api.BeginEntityAction();

				IEntitySource ownerSource = api.EntityToSource(this);

				int componentIndex = SCR_BaseContainerTools.FindComponentIndex(ownerSource, dmgManagerComp.Type());
				array<ref ContainerIdPathEntry> path = {new ContainerIdPathEntry("components", componentIndex)};

				if (api.SetVariableValue(ownerSource, path, "m_fVehicleDestroyDamage", criticalFrontalDamage.ToString()))
					Print("Entity instance's m_fVehicleDestroyDamage set to " + criticalFrontalDamage + ". Apply this change to the prefab!", LogLevel.WARNING);
				else
					Print("Error setting m_fVehicleDestroyDamage set to " + criticalFrontalDamage, LogLevel.ERROR);

				api.EndEntityAction();
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void Vehicle(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.PHYSICSACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_VehicleController = VehicleControllerComponent.Cast(FindComponent(VehicleControllerComponent));
		m_pFactionComponent = SCR_VehicleFactionAffiliationComponent.Cast(FindComponent(SCR_VehicleFactionAffiliationComponent));
		m_pPerceivableComponent = SCR_VehiclePerceivableComponent.Cast(FindComponent(SCR_VehiclePerceivableComponent));
		UpdateResourceComponent();
	}
}
