class SCR_AIFindTurrets: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_TURRET_NUMBER			= "TurretNumber";
	static const string PORT_TURRET_FOUND			= "TurretsFound";
	
	protected TagSystem m_tagSystem;
	protected SCR_AIGroup m_groupOwner;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		m_tagSystem = TagSystem.Cast(world.FindSystem(TagSystem));
		if (!m_tagSystem)
		{
			NodeError(this, owner, "SCR_AIFindTurret: TagManager is not present in the world, cannot find static turrets.");			
		}
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
				NodeError(this, owner, "Node is not run on SCR_AIGroup agent or owner is not member of SCR_AIGroup!");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_tagSystem)
		{
			ClearVariable(PORT_TURRET_NUMBER);
			ClearVariable(PORT_TURRET_FOUND);
			return ENodeResult.FAIL;
		}	
		vector center;
		float radius;
		int agentsCount = m_groupOwner.GetAgentsCount();
		int turretCount;
		bool turretsFound;
		
		GetVariableIn(PORT_CENTER_OF_SEARCH, center);
		GetVariableIn(PORT_RADIUS, radius);
		
		array<IEntity> entities = {};	
		m_tagSystem.GetTagsInRange(entities, center, radius, ETagCategory.StaticTurret);
		
		foreach (IEntity ent : entities)
		{
			BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(ent.FindComponent(BaseCompartmentManagerComponent));
			if (!compComp)
				continue;
			
			SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.Cast(ent.FindComponent(SCR_AIVehicleUsageComponent));
			if (!vehicleUsageComp)
				continue;
			
			// Ignore mortars, we can't use them autonomously
			if (vehicleUsageComp.GetVehicleType() != EAIVehicleType.STATIC_WEAPON)
				continue;
			
			array<BaseCompartmentSlot> compartmentSlots = {};
			compComp.GetCompartments(compartmentSlots);
			foreach (BaseCompartmentSlot slot : compartmentSlots)
			{
				TurretCompartmentSlot turretComp = TurretCompartmentSlot.Cast(slot);
				if (turretComp)
				{
					if (!turretComp.AttachedOccupant() && turretComp.IsCompartmentAccessible() && !turretComp.IsReserved())
					{
						turretCount += 1;
						turretsFound = true;
						if (turretCount <= agentsCount) // do not occupy more turrets that you have agents
							m_groupOwner.AllocateCompartment(turretComp);
						break;
					}
				}
			}
		}
		if (turretsFound)
			SetVariableOut(PORT_TURRET_NUMBER, turretCount);
		else 
			ClearVariable(PORT_TURRET_NUMBER);
		SetVariableOut(PORT_TURRET_FOUND, turretsFound);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TURRET_NUMBER,
		PORT_TURRET_FOUND
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "SCR_AIFindTurrets: finds all static turrets within center and radius. All results are allocated (reserved) for this group.";
	}
};