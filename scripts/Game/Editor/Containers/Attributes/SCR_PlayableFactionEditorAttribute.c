[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayableFactionEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{				
		SCR_EditableFactionComponent editableFaction = SCR_EditableFactionComponent.Cast(item);
		if (!editableFaction)
			return null;
		
		Faction faction = editableFaction.GetFaction();
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if(!scrFaction)
			return null;
 		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager || !factionManager.CanChangeFactionsPlayable())
			return null;
		
		vector value = Vector(scrFaction.IsPlayable(), factionManager.GetFactionIndex(faction), 0);
		return SCR_BaseEditorAttributeVar.CreateVector(value);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableFactionComponent editableFaction = SCR_EditableFactionComponent.Cast(item);
		if (!editableFaction)
			return;
		
		editableFaction.SetFactionPlayableServer(var.GetBool());
		
		if (item)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_FACTION_CHANGED, playerID);
	}
};