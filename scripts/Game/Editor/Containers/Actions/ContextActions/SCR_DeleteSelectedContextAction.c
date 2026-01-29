//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_DeleteSelectedContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(desc: "When enabled, the action will not be available if target entity is a player or a vehicle containing players.")]
	protected bool m_bCannotDeletePlayer;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return CanBePerformed(selectedEntity, cursorWorldPosition, flags);
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		//--- Prevent deleting players. Check the entity and compartments directly; don't rely on PLAYER state, as PLAYER filter may not be present in this editor mode.
		if (m_bCannotDeletePlayer)
		{
			SCR_PossessingManagerComponent possessionManager = SCR_PossessingManagerComponent.GetInstance();
			
			//--- Scan all children; player may be occupying a turret inside a composition
			set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>();
			selectedEntity.GetChildren(children);
			foreach (SCR_EditableEntityComponent child: children)
			{
				IEntity owner = child.GetOwner();
				SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
				if (compartmentManager)
				{
					//--- Entity has compartment manager, check if one of the occupants is a player
					array<IEntity> occupants = {};
					compartmentManager.GetOccupants(occupants);
					foreach (IEntity occupant: occupants)
					{
						if (possessionManager)
						{
							if (possessionManager.GetIdFromMainEntity(occupant) != 0)
								return false;
						}
						else
						{
							if ((GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(occupant) != 0))
								return false;
						}
					}
				}
				else
				{
					//--- Check if the entity itself is a player
					if (possessionManager)
					{
						if (possessionManager.GetIdFromMainEntity(owner) != 0)
							return false;
					}
					else
					{
						if ((GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner) != 0))
							return false;
					}
				}
			}
		}
		
		return selectedEntity
			&& !selectedEntity.HasEntityFlag(EEditableEntityFlag.NON_DELETABLE)
			&& !selectedEntity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE);
			//&& !selectedEntity.HasEntityState(EEditableEntityState.PLAYER);
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (selectedEntity)
			selectedEntity.Delete(true, true);
	}
};
