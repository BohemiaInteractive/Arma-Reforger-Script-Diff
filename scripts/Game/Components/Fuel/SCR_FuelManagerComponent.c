[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_FuelManagerComponentClass : FuelManagerComponentClass
{
}

class SCR_FuelManagerComponent : FuelManagerComponent
{
	protected ref ScriptInvokerFloat m_OnFuelChanged;
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] outScriptedNodes
	//! \return
	int GetScriptedFuelNodesList(out notnull array<SCR_FuelNode> outScriptedNodes)
	{
		outScriptedNodes.Clear();
		
		array<BaseFuelNode> fuelNodes = {};
		GetFuelNodesList(fuelNodes);
		SCR_FuelNode scrNode;
		
		foreach(BaseFuelNode node : fuelNodes)
		{
			scrNode = SCR_FuelNode.Cast(node);
			if (!scrNode)
				continue;
			
			outScriptedNodes.Insert(scrNode);		
		}
		
		return outScriptedNodes.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] outScriptedNodes
	//! \param[in] hasFuelNodeFlags
	//! \param[in] ignoreIfHasFlags
	//! \param[in] queryType
	//! \return
	int GetScriptedFuelNodesList(out notnull array<SCR_FuelNode> outScriptedNodes, SCR_EFuelNodeTypeFlag hasFuelNodeFlags, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0, SCR_EFuelNodeFlagQueryType queryType = SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES)
	{
		//~ No need to check flags if none are given
		if (hasFuelNodeFlags <= 0 && ignoreIfHasFlags <= 0)
			return GetScriptedFuelNodesList(outScriptedNodes);
		
		outScriptedNodes.Clear();
		
		array<BaseFuelNode> fuelNodes = {};
		GetFuelNodesList(fuelNodes);
		SCR_FuelNode scrNode;
		
		foreach(BaseFuelNode node : fuelNodes)
		{
			scrNode = SCR_FuelNode.Cast(node);
			if (!scrNode)
				continue;
			
			if (ignoreIfHasFlags > 0 && scrNode.HasAnyTypeFlag(ignoreIfHasFlags))
				continue;
			
			if (hasFuelNodeFlags > 0)
			{
				bool validNode;
			
				switch (queryType)
				{
					case SCR_EFuelNodeFlagQueryType.EXACT_TYPE:
					{
						validNode = scrNode.HasExactTypeFlags(hasFuelNodeFlags);
						break;
					}
					case SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES:
					{
						validNode = scrNode.HasAllTypeFlags(hasFuelNodeFlags);
						break;
					}
					case SCR_EFuelNodeFlagQueryType.ANY_OF_GIVEN_TYPES:
					{
						validNode = scrNode.HasAnyTypeFlag(hasFuelNodeFlags);
						break;
					}
				}
				
				if (!validNode)
					continue;
			}
						
			outScriptedNodes.Insert(scrNode);		
		}
		
		return outScriptedNodes.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if fuel manager can be refueled. Will only check fuel nodes with the CAN_RECEIVE_FUEL and without any of the given ignoreIfHasFlags
	//! \param[in] fuelTanksReceivers optional if you want to only check specific fuel tanks. Leave null or empty to ignore
	//! \param[in] ignoreIfHasFlags ignore any fuel nodes that have the ignore flags. Leave 0 to ignore
	//! \return true if at least one fuel node can be refueled
	bool CanBeRefueledScripted(array<int> fuelTanksReceivers = null, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0)
	{
		array<SCR_FuelNode> nodes = {};	
		SCR_FuelNode node;
		for (int i = GetScriptedFuelNodesList(nodes, SCR_EFuelNodeTypeFlag.CAN_RECEIVE_FUEL, ignoreIfHasFlags) - 1; i >= 0; i--)
		{
			node = nodes.Get(i);
			
			if (!node || node.GetFuel() >= node.GetMaxFuel())
				continue;
			
			if (fuelTanksReceivers && !fuelTanksReceivers.IsEmpty())
			{
				if (!fuelTanksReceivers.Contains(node.GetFuelTankID()))
					continue;
			}
			//~ Passes all tests
			return true;
		}
		
		//~ Has no nodes to refuel
		return false;
	}
	
	//~ Todo: move logic from SCR_FuelSupportStationComponent to SCR_FuelManagerComponent
	//------------------------------------------------------------------------------------------------
	//! Transfer fuel to fuel nodes within the fuel manager (server only)
	//! \param[in] provider If fuel should be taken from another fuel manager
	//! \param[in] fuelTanksReceivers Only transfer fuel into the given fuel managers. If empty array all are used. Note that the fuel tanks still need the CAN_RECEIVE_FUEL flag in order to receive fuel
	//! \param[in] ignoreIfHasFlags Ignore any fuel nodes of provider that have the ignore flags. Leave 0 to ignore
	//! \return Fuel amount that was left over as all fuel nodes that can receive full are full
//	float TransferFuelWithFlow(float fuelToTransfer, SCR_FuelManagerComponent provider = null, array<int> fuelTanksReceivers = null, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0)
//	{
//	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[out] fuelManagers
	//! \return
	static int GetAllFuelManagers(notnull IEntity entity, out notnull array<SCR_FuelManagerComponent> fuelManagers)
	{
		fuelManagers.Clear();
		
		SCR_FuelManagerComponent fuelManager = SCR_FuelManagerComponent.Cast(entity.FindComponent(SCR_FuelManagerComponent));
		if (fuelManager)
			fuelManagers.Insert(fuelManager);
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return fuelManagers.Count();
			
		array<EntitySlotInfo> slotInfos = {};
		slotManager.GetSlotInfos(slotInfos);
		
		IEntity slotEntity;
		
		foreach (EntitySlotInfo slotInfo: slotInfos)
		{
			slotEntity = slotInfo.GetAttachedEntity();
			
			if (!slotEntity)
				continue;
			
			fuelManager = SCR_FuelManagerComponent.Cast(slotEntity.FindComponent(SCR_FuelManagerComponent));
			if (fuelManager)
				fuelManagers.Insert(fuelManager);
		}
		
		return fuelManagers.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if fuelmanager has any fuel in fuel nodes that can provide fuel (SCR_FuelNode only)
	//! \param[in] ignoreIfHasFlags any nodes with this flag is skipped. Leave 0 to ignore
	//! \return true if it can provide fuel
	bool HasFuelToProvide(SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0)
	{		
		//~ Get all fuel providers
		array<SCR_FuelNode> nodes = {};
		GetScriptedFuelNodesList(nodes, SCR_EFuelNodeTypeFlag.CAN_PROVIDE_FUEL, ignoreIfHasFlags);
		
		//~ No nodes
		if (nodes.IsEmpty())
			return false;
			
		//~ Loop through fuel providers and check if it has any fuel
		foreach(SCR_FuelNode node: nodes)
		{
			//~ Can provide and has fuel. 
			if (!float.AlmostEqual(node.GetFuel(), 0))
				return true;
		}
		
		//~ No valid fuel nodes found that can provide fuel
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get to currentFuel, totalfuel and fuel percentage of all nodes with the given flags
	//! \param[out] totalFuel Total current fuel of all fuel nodes
	//! \param[out] totalMaxFuel Total max fuel of all fuel nodes
	//! \param[out] totalFuelPercentage Total fuel percentage of all fuel nodes. Gives value from 0 to 1
	//! \param[in] fuelNodeFlags Only checks fuel nodes with given type (or types depending if all or one flag is required). Leave 0 to get all node types even non SCR_FuelNodes. Otherwise only checks SCR_FuelNodes
	//! \param[in] ignoreIfHasFlags Any nodes with this flag is skipped. Leave 0 to ignore
	//! \param[in] queryType If the obtained data is from fuelnodes with: The exact given type, has all given flags, has at least one given flag
	void GetTotalValuesOfFuelNodes(out float totalFuel, out float totalMaxFuel, out float totalFuelPercentage, SCR_EFuelNodeTypeFlag hasFuelNodeFlags = 0, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0, SCR_EFuelNodeFlagQueryType queryType = SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES)
	{
		totalFuel = 0;
		totalMaxFuel = 0;
		totalFuelPercentage = 0;
		
		//~ No flags are set no need to loop through all nodes
		if (hasFuelNodeFlags <= 0 && ignoreIfHasFlags <= 0)
		{
			totalFuel = GetTotalFuel();
			totalMaxFuel = GetTotalMaxFuel();
		}
		else 
		{
			array<BaseFuelNode> fuelNodes = {};
			GetFuelNodesList(fuelNodes);
			SCR_FuelNode scrFuelNode;
			
			foreach(BaseFuelNode node: fuelNodes)
			{
				//~ If flags set
				scrFuelNode = SCR_FuelNode.Cast(node);
			
				//~ Get valid fuel node (non-SCR are always valid)
				if (scrFuelNode)
				{
					//~ Has a flag it should not
					if (ignoreIfHasFlags > 0 && scrFuelNode.HasAnyTypeFlag(ignoreIfHasFlags))
						continue;
					
					//~ Check if node is valid
					if (hasFuelNodeFlags > 0)
					{
						bool validNode;
					
						switch (queryType)
						{
							case SCR_EFuelNodeFlagQueryType.EXACT_TYPE:
							{
								validNode = scrFuelNode.HasExactTypeFlags(hasFuelNodeFlags);
								break;
							}
							case SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES:
							{
								validNode = scrFuelNode.HasAllTypeFlags(hasFuelNodeFlags);
								break;
							}
							case SCR_EFuelNodeFlagQueryType.ANY_OF_GIVEN_TYPES:
							{
								validNode = scrFuelNode.HasAnyTypeFlag(hasFuelNodeFlags);
								break;
							}
						}
						
						if (!validNode)
							continue;
					}
				}
				
				totalFuel += node.GetFuel();
				totalMaxFuel += node.GetMaxFuel();
			}
		}
		
		//~ Safty
		if (totalMaxFuel <= 0)
		{
			totalFuel = 0;
			totalMaxFuel = 0;
			totalFuelPercentage = 0;
			return;
		}
		
		totalFuelPercentage = totalFuel / totalMaxFuel;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get to currentFuel, totalfuel and fuel percentage of all nodes with the given flags
	//! \param[in] fuelManagers FuelManagers
	//! \param[out] totalFuel Total current fuel of all fuel nodes
	//! \param[out] totalMaxFuel Total max fuel of all fuel nodes
	//! \param[out] totalFuelPercentage Total fuel percentage of all fuel nodes. Gives value from 0 to 1
	//! \param[in] hasfuelNodeFlags Only checks fuel nodes with given type (or types depending if all or one flag is required). Leave 0 to get all node types even non SCR_FuelNodes. Otherwise only checks SCR_FuelNodes
	//! \param[in] ignoreIfHasFlags Any nodes with this flag is skipped. Leave 0 to ignore
	//! \param[in] queryType If the obtained data is from fuelnodes with: The exact given type, has all given flags, has at least one given flag
	static void GetTotalValuesOfFuelNodesOfFuelManagers(notnull array<SCR_FuelManagerComponent> fuelManagers, out float totalFuel, out float totalMaxFuel, out float totalFuelPercentage, SCR_EFuelNodeTypeFlag hasfuelNodeFlags = 0, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0, SCR_EFuelNodeFlagQueryType queryType = SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES)
	{
		totalFuel = 0;
		totalMaxFuel = 0;
		totalFuelPercentage = 0;
		
		if (fuelManagers.IsEmpty())
			return;
		
		float nodeTotalFuel, nodeTotalMaxFuel, nodeTotalFuelPercentage;
		
		//~ Get values of all fuelmanagers
		foreach (SCR_FuelManagerComponent fuelManager: fuelManagers)
		{
			if (!fuelManager)
				continue;
			
			fuelManager.GetTotalValuesOfFuelNodes(nodeTotalFuel, nodeTotalMaxFuel, nodeTotalFuelPercentage, hasfuelNodeFlags, ignoreIfHasFlags, queryType);
			
			totalFuel += nodeTotalFuel;
			totalMaxFuel += nodeTotalMaxFuel;
		}
		
		//~ Safty
		if (totalMaxFuel <= 0)
		{
			totalFuel = 0;
			totalMaxFuel = 0;
			totalFuelPercentage = 0;
			return;
		}
		
		//~ Calculate total percentage
		totalFuelPercentage = totalFuel / totalMaxFuel;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the total fuel percentage to a value between 0 and 1. (Server only)
	//! Distributes the fuel correctly between all fuel nodes
	//! \param[in] percentage Percentage of fuel to set
	//! \param[in] hasFuelNodeFlags Flag of what type (or types depending if all or one flag is required) of fuel nodes can be set. Leave 0 to set all node types. Non SCR_Nodes are always valid
	//! \param[in] ignoreIfHasFlags Any nodes with this flag is skipped. Leave 0 to ignore
	//! \param[in] queryType If the nodes that are set have: The exact given type, has all given flags, has at least one given flag
	void SetTotalFuelPercentage(float percentage, SCR_EFuelNodeTypeFlag hasFuelNodeFlags = 0, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0, SCR_EFuelNodeFlagQueryType queryType = SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES)
	{
		//~ Safty clamp
		percentage = Math.Clamp(percentage, 0 , 1);
		
		array<BaseFuelNode> fuelNodes = {};
		array<BaseFuelNode> validFuelNodes = {};
		SCR_FuelNode scrFuelNode;
		
		float totalFuel = 0;
		float totalMaxFuel 0;
		
		GetFuelNodesList(fuelNodes);
	
		//~ Get total fuel and max fuel of all non providers
		foreach(BaseFuelNode node: fuelNodes)
		{
			//~ If flags set
			if (hasFuelNodeFlags > 0 || ignoreIfHasFlags > 0)
			{
				scrFuelNode = SCR_FuelNode.Cast(node);
				
				//~ Get valid fuel node (non-SCR are always valid)
				if (scrFuelNode)
				{
					//~ Has a flag it should not
					if (ignoreIfHasFlags > 0 && scrFuelNode.HasAnyTypeFlag(ignoreIfHasFlags))
						continue;
					
					//~ Check if node is valid
					if (hasFuelNodeFlags > 0)
					{
						bool validNode; 
					
						switch (queryType)
						{
							case SCR_EFuelNodeFlagQueryType.EXACT_TYPE:
							{
								validNode = scrFuelNode.HasExactTypeFlags(hasFuelNodeFlags);
								break;
							}
							case SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES:
							{
								validNode = scrFuelNode.HasAllTypeFlags(hasFuelNodeFlags);
								break;
							}
							case SCR_EFuelNodeFlagQueryType.ANY_OF_GIVEN_TYPES:
							{
								validNode = scrFuelNode.HasAnyTypeFlag(hasFuelNodeFlags);
								break;
							}
						}
						
						if (!validNode)
							continue;
					}
				}
			}
			
			validFuelNodes.Insert(node);
			totalFuel += node.GetFuel();
			totalMaxFuel += node.GetMaxFuel();
		}
		
		float newFuel = totalMaxFuel * percentage;
		float diff = newFuel - totalFuel;
		
		foreach(BaseFuelNode node: validFuelNodes)
		{
			float fuel = node.GetFuel();
			float maxFuel = node.GetMaxFuel();

			if (diff > 0)
			{
				// Distribute remaining capacity
				if (totalMaxFuel > totalFuel)
					fuel += diff * (maxFuel - fuel) / (totalMaxFuel - totalFuel);
			}
			else if (diff < 0)
			{
				// Distribute remaining fuel
				if (totalFuel > 0)
					fuel += diff * fuel / totalFuel;
			}

			node.SetFuel(fuel);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the total fuel percentage to a value between 0 and 1 of all given fuelmanagers (Server Only)
	//! Distributes the fuel correctly between all fuel nodes
	//! \param[in] fuelManagers FuelManagers
	//! \param[in] percentage Percentage of fuel to set
	//! \param[in] hasFuelNodeFlags Flag of what type (or types depending if all or one flag is required) of fuel nodes can be set. Leave 0 to set all node types. Non SCR_Nodes are always valid
	//! \param[in] ignoreIfHasFlags Any nodes with this flag is skipped. Leave 0 to ignore
	//! \param[in] queryType If the nodes that are set have: The exact given type, has all given flags, has at least one given flag
	static void SetTotalFuelPercentageOfFuelManagers(notnull array<SCR_FuelManagerComponent> fuelManagers, float percentage, SCR_EFuelNodeTypeFlag hasFuelNodeFlags = 0, SCR_EFuelNodeTypeFlag ignoreIfHasFlags = 0, SCR_EFuelNodeFlagQueryType queryType = SCR_EFuelNodeFlagQueryType.ALL_GIVEN_TYPES)
	{
		if (fuelManagers.IsEmpty())
			return;
		
		foreach (SCR_FuelManagerComponent fuelManager: fuelManagers)
		{
			if (!fuelManager)
				continue;
			
			fuelManager.SetTotalFuelPercentage(percentage, hasFuelNodeFlags, ignoreIfHasFlags, queryType);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return script invoker for on fuel changed.
	//! Note that this creates fuel invokers for all SCR_FuelNodes and there is no way currently to destroy them so can be costly
	//! \return scriptInvoker on fuel changed. Includes current total fuel amount
	ScriptInvokerFloat GetOnFuelChanged()
	{
		if (!m_OnFuelChanged)
		{
			m_OnFuelChanged = new ScriptInvokerFloat();
			
			array<SCR_FuelNode> fuelNodes = {};
			
			GetScriptedFuelNodesList(fuelNodes);
			foreach (SCR_FuelNode scrNode : fuelNodes)
			{
				scrNode.GetOnFuelChanged().Insert(OnFuelNodeAmountChanged);
			}
		}
	
		return m_OnFuelChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFuelNodeAmountChanged(float newFuel)
	{
		m_OnFuelChanged.Invoke(GetTotalFuel());
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_FuelManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		#ifdef WORKBENCH
		//~ Safty check if the Fuelmanager has any non SCR_FuelNode's in the array
		array<BaseFuelNode> fuelNodes = {};
		GetFuelNodesList(fuelNodes);
		
		foreach(BaseFuelNode node : fuelNodes)
		{
			if (!node.IsInherited(SCR_FuelNode))
			{
				Print("'SCR_FuelManagerComponent' has non SCR_FuelNode in the array! This is not supported with many of the SCR_Fuelmanager logics!", LogLevel.WARNING);
				break;
			}
		}
		#endif
	}
}

//~ Fuel node type flags
enum SCR_EFuelNodeFlagQueryType
{
	EXACT_TYPE,			//!< Will only get fuel node if flag type is the same as given flag
	ALL_GIVEN_TYPES,	//!< Will only get fuelnode if all the given types are present on the fuel node. (More than the given can be present but non can be missing)
	ANY_OF_GIVEN_TYPES,	//!< Will only get fuelnode if at least one of the given flags is present on the fuel node. (More than the given can be present)
}
