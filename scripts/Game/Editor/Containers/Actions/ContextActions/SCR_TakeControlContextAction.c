[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TakeControlContextAction : SCR_BaseEditorAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!hoveredEntity || !SCR_PossessingManagerComponent.GetInstance())
			return false;
				
		GenericEntity owner = hoveredEntity.GetOwner();
		if (!owner) 
			return false;
		
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));
		
		if (!compartmentManager)
		{
			SCR_EditableEntityComponent aiEntity = hoveredEntity.GetAIEntity();
			if (!aiEntity || aiEntity.IsDestroyed())
				return false;
			
			//--- Check if the entity is not a player
			return SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(aiEntity.GetOwner()) <= 0;
		}
		else 
		{
			return true;
		}
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!hoveredEntity || !SCR_PossessingManagerComponent.GetInstance())
			return false;
		
		GenericEntity owner = hoveredEntity.GetOwner();
		if (!owner) 
			return false;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));

		if (!compartmentManager)
		{
			return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
		}
		else 
		{
			array<IEntity> pilots = new array<IEntity>;
			compartmentManager.GetOccupantsOfType(pilots, ECompartmentType.PILOT);
		
			if (pilots.IsEmpty() || !pilots[0])
				return false;
			
			SCR_EditableEntityComponent pilotEditableEntity = SCR_EditableEntityComponent.Cast(pilots[0].FindComponent(SCR_EditableEntityComponent));
			if (!pilotEditableEntity)
				return false;
			 
			SCR_EditableEntityComponent aiEntity = pilotEditableEntity.GetAIEntity();
			if (!aiEntity || aiEntity.IsDestroyed())
				return false;
			
			//--- Check if the entity is not a player
			return SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(aiEntity.GetOwner()) <= 0;
		}
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!hoveredEntity || !SCR_PossessingManagerComponent.GetInstance())
			return;
			
		GenericEntity owner = hoveredEntity.GetOwner();
		if (!owner) 
			return;
		
		SCR_EditableEntityComponent entityToControl;
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		
		if (!compartmentManager)
		{
			entityToControl = hoveredEntity;
		}
		else 
		{			
			array<IEntity> pilots = new array<IEntity>;
			compartmentManager.GetOccupantsOfType(pilots, ECompartmentType.PILOT);
		
			if (pilots.IsEmpty() || !pilots[0])
				return;
			
			entityToControl = SCR_EditableEntityComponent.Cast(pilots[0].FindComponent(SCR_EditableEntityComponent));
		}
		
		if (!entityToControl)
			return;
		
		SCR_EditableEntityComponent aiEntity = entityToControl.GetAIEntity();
		if (!aiEntity || aiEntity.IsDestroyed())
			return;
		
		//--- Find player controller (param is playerID)
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(param));
		if (!playerController)
			return;

		playerController.SetPossessedEntity(aiEntity.GetOwner());
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		//--- Set the speed of the character to the default speed
		characterController.SetDynamicSpeed(1);
		
		//--- Close the editor (ToDo: Move to lobby's OnPossessed event)
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
		{
			SCR_EditorManagerEntity manager = core.GetEditorManager(param);
			if (manager)
				manager.Close();
		}
	}
	override int GetParam()
	{
		return GetGame().GetPlayerController().GetPlayerId();
	}
	override bool IsServer()
	{
		return true;
	}
};