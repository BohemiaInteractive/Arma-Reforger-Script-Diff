//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
class SCR_CampaignBuildingEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected const int AI_SEARCH_BUFFER_DISTANCE = 20;
	
	//------------------------------------------------------------------------------------------------
	protected bool GetComposition(IEntity owner, out SCR_EditableEntityComponent entity, out SCR_CampaignBuildingCompositionComponent composition)
	{
		composition = SCR_CampaignBuildingCompositionComponent.Cast(owner.FindComponent(SCR_CampaignBuildingCompositionComponent));
		entity = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		if (!composition || !entity)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsCompositionOwned(IEntity owner)
	{
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(owner.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return false;

		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent)
			return false;

		if (editableComponent.GetParentEntity())
			return false;

		SCR_EditorModeEntity mode = SCR_EditorModeEntity.Cast(GetManager().GetOwner());
		if (!mode)
			return false;

		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(mode.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return false;

		IEntity compositionOwner = compositionComponent.GetProviderEntity();
		if (!compositionOwner)
		{
			ScriptedGameTriggerEntity trigger = buildingComponent.GetTrigger();
			if (!trigger)
				return false;

			if (vector.DistanceXZ(trigger.GetOrigin(), owner.GetOrigin()) <= trigger.GetSphereRadius())
				return true;
		}

		if (compositionOwner != buildingComponent.GetProviderEntity())
			return false;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] char Character entity to test.
	protected bool CanAddCharacter(notnull IEntity char)
	{			
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return false;

		if (!providerComponent.IsEntityFactionSame(providerComponent.GetOwner(), char))
			return false;
		
		if (!providerComponent.CanCommandAI())
			return false;
		
		return IsInBaseArea(char, AI_SEARCH_BUFFER_DISTANCE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] waypoint waypoint entity to test.
	protected bool CanAddWaypoint(notnull IEntity waypoint)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return false;
		
		if (!providerComponent.CanCommandAI())
			return false;
		
		return IsInBaseArea(waypoint);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		IEntity entityOwner = entity.GetOwnerScripted();
		if (!entityOwner)
			return false;
		
		ChimeraCharacter char = ChimeraCharacter.Cast(entityOwner);
		if (char)
			return (CanAddCharacter(char));
		
		SCR_EditableWaypointComponent editableWaypoint = SCR_EditableWaypointComponent.Cast(entity);
		if (editableWaypoint)
			return CanAddWaypoint(editableWaypoint.GetOwner());
		
		// To show a group icon first get the leader of group. If the Group was just freshly spawned it might not be set yet. In this case, listen to an event when the leader is set and method refresh is called in the system.
		SCR_EditableGroupComponent editableGroup = SCR_EditableGroupComponent.Cast(entity);
		if (editableGroup)
		{
			SCR_EditableEntityComponent leader = editableGroup.GetAIEntity();
			if (leader)
			{
				return CanAddCharacter(leader.GetOwner());
			}
			else
			{
				SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
				if (core)
					core.Event_OnEntityRefreshed.Insert(OnEntityRefreshed);
			}
		}
		
		return (IsCompositionOwned(entityOwner));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the given entity is within a base radius.
	//! \param[in] ent Entity to be tested
	//! \param[in] buffer int Additional distance to add to the radius of the building zone where the entity can be added. For example an AI that can wander away.
	protected bool IsInBaseArea(notnull IEntity ent, int buffer = 0)
	{
		SCR_EditorModeEntity mode = SCR_EditorModeEntity.Cast(GetManager().GetOwner());
		if (!mode)
			return false;

		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(mode.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return false;
				
		ScriptedGameTriggerEntity trigger = buildingComponent.GetTrigger();
		if (!trigger)
			return false;

		return vector.DistanceXZ(trigger.GetOrigin(), ent.GetOrigin()) <= trigger.GetSphereRadius() + buffer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add given entity to the list so it's icon is shown in.
	protected void OnEntityRefreshed(SCR_EditableEntityComponent entity)
	{
		SCR_EditableGroupComponent editableGroup = SCR_EditableGroupComponent.Cast(entity);
		if (!editableGroup)
			return;
		
		SCR_EditableEntityComponent leader = editableGroup.GetAIEntity();
		if (!leader)
			return;

		if (CanAddCharacter(leader.GetOwner()))
			Add(entity, true);
	
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityRefreshed.Remove(OnEntityRefreshed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return provider component of this mode.
	protected SCR_CampaignBuildingProviderComponent GetProviderComponent()
	{
		SCR_EditorModeEntity mode = SCR_EditorModeEntity.Cast(GetManager().GetOwner());
		if (!mode)
			return null;

		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(mode.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return null;
		
		return buildingComponent.GetProviderComponent();
	} 
}
