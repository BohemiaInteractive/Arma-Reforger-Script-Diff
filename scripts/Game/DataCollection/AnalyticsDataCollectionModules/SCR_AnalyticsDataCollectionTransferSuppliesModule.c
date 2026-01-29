class SCR_TransferSuppliesInteraction
{
	ResourceName m_sFromPrefab;
	ResourceName m_sToPrefab;
	string m_sFromType;
	string m_sToType;
	float m_fValue;
	
	//------------------------------------------------------------------------------------------------
	void SCR_TransferSuppliesInteraction(ResourceName from, ResourceName to, string fromType, string toType, float value)
	{
		m_sFromPrefab = from;
		m_sToPrefab = to;
		m_sFromType = fromType;
		m_sToType = toType;
		m_fValue = value;
	}
}

[BaseContainerProps()]
class SCR_AnalyticsDataCollectionTransferSuppliesModule : SCR_AnalyticsDataCollectionModule
{
	protected ref array<ref SCR_TransferSuppliesInteraction> m_Interactions = new array<ref SCR_TransferSuppliesInteraction>();
	protected static const string BASE = "BASE";
	
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		SCR_CampaignNetworkComponent.GetOnTransferSupplies().Insert(OnTransferSupplies);
		super.Enable();
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		SCR_CampaignNetworkComponent.GetOnTransferSupplies().Remove(OnTransferSupplies);
		super.Disable();
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetTypeFromResourceComponent(SCR_ResourceComponent resourceComponent)
	{
		IEntity parent = resourceComponent.GetOwner().GetRootParent();
		
		if (ChimeraCharacter.Cast(parent))
			return "CHARACTER";

		Vehicle vehicle = Vehicle.Cast(parent);
		if (!vehicle)
			return BASE;

		if (SCR_HelicopterControllerComponent.Cast(vehicle.GetVehicleController()))
			return "HELICOPTER";
		
		return typename.EnumToString(EVehicleType, vehicle.m_eVehicleType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetPrefabNameFromResourceComponent(SCR_ResourceComponent resourceComponent)
	{
		return resourceComponent.GetOwner().GetRootParent().GetPrefabData().GetPrefabName();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTransferSupplies(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{		
		ResourceName fromPrefab = "-";
		string fromType = BASE;
		if (interactionType != EResourcePlayerInteractionType.VEHICLE_LOAD)
		{
			fromPrefab = GetPrefabNameFromResourceComponent(resourceComponentFrom);
			fromType = GetTypeFromResourceComponent(resourceComponentFrom);
		}
		
		ResourceName toPrefab = "-";
		string toType = BASE;
		if (interactionType != EResourcePlayerInteractionType.VEHICLE_UNLOAD)
		{
			toPrefab = GetPrefabNameFromResourceComponent(resourceComponentTo);
		 	toType = GetTypeFromResourceComponent(resourceComponentTo);
		}
			
		PrintFormat("SCR_AnalyticsDataCollectionTransferSuppliesModule: Transferring %1 supplies from %2 (%3) to %4 (%5).", resourceValue, fromPrefab, fromType, toPrefab, toType);
		m_Interactions.Insert(new SCR_TransferSuppliesInteraction(fromPrefab, toPrefab, fromType, toType, resourceValue));
	}	
		
	//------------------------------------------------------------------------------------------------
	override void SessionMeasures()
	{
		if (!m_bIsEnabled)
			return;

		SCR_SessionDataEvent sessionDataEvent = GetSessionDataEvent();
		if (!sessionDataEvent)
			return;

		sessionDataEvent.array_supplies_from_prefab = "[";
		sessionDataEvent.array_supplies_to_prefab	= "[";
		sessionDataEvent.array_supplies_from_type 	= "[";
		sessionDataEvent.array_supplies_to_type 	= "[";
		sessionDataEvent.array_supplies_values		= "[";

		foreach (int i, SCR_TransferSuppliesInteraction interaction : m_Interactions)
		{
			if (i != 0)
			{
				sessionDataEvent.array_supplies_from_prefab += ",";
				sessionDataEvent.array_supplies_to_prefab 	+= ",";
				sessionDataEvent.array_supplies_from_type 	+= ",";
				sessionDataEvent.array_supplies_to_type 	+= ",";
				sessionDataEvent.array_supplies_values 		+= ",";
			}
			
			sessionDataEvent.array_supplies_from_prefab += "\"" + interaction.m_sFromPrefab + "\"";
			sessionDataEvent.array_supplies_to_prefab 	+= "\"" + interaction.m_sToPrefab + "\"";
			sessionDataEvent.array_supplies_from_type 	+= "\"" + interaction.m_sFromType + "\"";
			sessionDataEvent.array_supplies_to_type 	+= "\"" + interaction.m_sToType + "\"";
			sessionDataEvent.array_supplies_values 		+= interaction.m_fValue.ToString();
		}
		
		sessionDataEvent.array_supplies_from_prefab += "]";
		sessionDataEvent.array_supplies_to_prefab 	+= "]";
		sessionDataEvent.array_supplies_from_type 	+= "]";
		sessionDataEvent.array_supplies_to_type 	+= "]";
		sessionDataEvent.array_supplies_values 		+= "]";
		
		m_Interactions.Clear();
	}
}