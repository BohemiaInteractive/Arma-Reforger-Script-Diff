class SCR_RefuelAtSupportStationAction : SCR_BaseUseSupportStationAction
{
	[Attribute("#AR-SupportStation_Fuel_ActionInvalid_FuelFull", desc: "Text shown on action if fuel tank already full", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidFullTank;
	
	[Attribute("#AR-SupportStation_Fuel_ActionInvalid_FuelProviderNoFuel", desc: "There are fuel refuel stations near by but non have fuel in their tank", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidFuelActionReason;
	
	[Attribute("#AR-SupportStation_Fuel_ActionInvalid_FuelCanisterEmpty", desc: "The player is holding a fuel canister but cannot refuel as it is empty", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidReasonFuelCanisterEmpty;
	
	[Attribute(desc: "IDs of fuel tanks linked to this refuel action. Leave empty if all fuel tanks are valid")];
	protected ref array<int> m_aFuelTankIDs;
	
	[Attribute("1", desc: "Decimal count of action percentage. Put on 2 if the entity has a large fuel tank", params: "0 2")]
	protected int m_iActionDecimalCount;
	
	protected SCR_FuelManagerComponent m_FuelManager;
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.FUEL;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetFuelTankIDs(array<int> fuelTankIDs)
	{
		fuelTankIDs.Copy(m_aFuelTankIDs);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if given Fuel node is linked to action.
	Simply checks the ID of the fuel node not if the given fuel node is linked to the Fuel manager on action
	\return True if fuel node is valid or FuelTankIDs are ignored. Will return true if null as node is simply filled
	*/
	bool CheckIfFuelNodeIsValid(SCR_FuelNode fuelNode)
	{
		return !fuelNode || m_aFuelTankIDs.IsEmpty() || m_aFuelTankIDs.Contains(fuelNode.GetFuelTankID());
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	/return Count of valid fuel nodes
	*/
	int GetValidFuelNodesCount()
	{
		return m_aFuelTankIDs.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_FuelManager = SCR_FuelManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_FuelManagerComponent));
		if (!m_FuelManager)
			Print("'SCR_RefuelAtSupportStationAction': '" + pOwnerEntity.GetName() + "' is missing SCR_FuelManager!", LogLevel.WARNING);
		
		super.Init(pOwnerEntity, pManagerComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		//~ Fuel station out of fuel
		if (reasonInvalid == ESupportStationReasonInvalid.NO_FUEL_TO_GIVE)
			return m_sInvalidFuelActionReason;
		else if (reasonInvalid == ESupportStationReasonInvalid.FUEL_TANK_FULL)
			return m_sInvalidFullTank;
		else if (reasonInvalid == ESupportStationReasonInvalid.FUEL_CANISTER_EMPTY)
			return m_sInvalidReasonFuelCanisterEmpty;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}	

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_FuelManager)
			return false;
	
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ TODO: Overwrite GetClosestValidSupportStation and simply return the support station from which the player is holding the nozzle!
	//~ This way all the same functions and checks can be used and you don't need to create a whole new system for nozzle interaction
	//~ Make sure the GetNozzle action sets a reference somewhere to the Fuel SupportStation that is attached and pass it here!
	//~ Would advice to simply use the same functions as supply cost and things like flow, notifications, players in vehicle and all that has already been taken care of!
	//~ Add new ESupportStationReasonInvalid for: NO_NOZZLE if not holding any and potentially, INCORRECT FUEL TYPE
	//~ Update the GetInvalidPerformReasonString() and by extention ESupportStationReasonInvalid to include the two new ReasonInvalid and all is taken care of
	/*
	protected override SCR_BaseSupportStationComponent GetClosestValidSupportStation(IEntity actionOwner, IEntity actionUser, out ESupportStationReasonInvalid reasonInvalid = 0)
	{
		Use this function to get the SCR_FuelSupportStationComponent from which the player holds the nozzle
		
		Call: SCR_FuelSupportStationComponent.IsValid();
	
		You can also potentially check here if fuel type is correct
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	protected override bool PrioritizeHeldGadget()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{				
		bool canRefuel = true;
		
		if (!m_FuelManager)
		{
			canRefuel = false;
			return false;
		}
		
		if (m_aFuelTankIDs.IsEmpty())
		{
			if (!m_FuelManager.CanBeRefueledScripted(m_aFuelTankIDs))
				canRefuel = false;
		}
		else 
		{
			canRefuel = false;
			
			array<BaseFuelNode> nodes = {};
			m_FuelManager.GetFuelNodesList(nodes);
			SCR_FuelNode scrNode;
			
			foreach (BaseFuelNode node : nodes)
			{
				scrNode = SCR_FuelNode.Cast(node);
				
				if (!CheckIfFuelNodeIsValid(scrNode))
					continue;
				
				if (node.GetFuel() < node.GetMaxFuel())
				{
					canRefuel = true;
					break;
				}
			}
		}
		
		if (!canRefuel)
		{
			SetCanPerform(false, ESupportStationReasonInvalid.FUEL_TANK_FULL);
			return false;
		}
		
		canRefuel = super.CanBePerformedScript(user);
		return canRefuel;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int GetActionDecimalCount()
	{
		return m_iActionDecimalCount;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override float GetActionPercentage()
	{
		if (!m_bCanPerform)
			return int.MIN;
		
		float fuelPercentage;
		int validFuelNodes;
		
		array<BaseFuelNode> nodes = {};
		m_FuelManager.GetFuelNodesList(nodes);
		SCR_FuelNode scrNode;
		
		foreach (BaseFuelNode node : nodes)
		{
			scrNode = SCR_FuelNode.Cast(node);
			
			if (!CheckIfFuelNodeIsValid(scrNode))
				continue;
			
			fuelPercentage += node.GetFuel() / node.GetMaxFuel();
			validFuelNodes++;
		}
		
		if (validFuelNodes <= 0)
			return 0;
		
		return (fuelPercentage / validFuelNodes) * 100;
	}
};