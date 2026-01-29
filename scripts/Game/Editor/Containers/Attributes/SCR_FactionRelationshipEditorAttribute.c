[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_FactionRelationshipEditorAttribute : SCR_BaseMultiSelectPresetsEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableFactionComponent editableFaction = SCR_EditableFactionComponent.Cast(item);
		if (!editableFaction)
			return null;
		
		SCR_Faction selectedFaction = SCR_Faction.Cast(editableFaction.GetFaction());
		if (!selectedFaction)
			return null;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return null;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return null;
		
		super.ReadVariable(item, manager);
		
		// Get delegate faction manager
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
		SCR_Faction scrFaction;
		
		// Loop through all factions
		for (int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			if (!scrFaction)
				continue;
			
			// Set the state based on friendly (highlighted) or hostile(non-highlighted)
			AddOrderedState(selectedFaction.DoCheckIfFactionFriendly(scrFaction));
		}
		
		return SCR_BaseEditorAttributeVar.CreateVector(GetFlagVector());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		SCR_EditableFactionComponent editableFaction = SCR_EditableFactionComponent.Cast(item);
		if (!editableFaction)
			return;
		
		SCR_Faction selectedFaction = SCR_Faction.Cast(editableFaction.GetFaction());
		if (!selectedFaction)
			return;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		super.WriteVariable(item, var, manager, playerID);
		
		// Get delegate faction manager
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
		SCR_Faction scrFaction;
		
		// Loop through all factions
		for (int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			if (!scrFaction)
				continue;
			
			// Based on the state, set the faction to friendly (highlighted) or hostile(non-highlighted)
			if (GetOrderedState())
				factionManager.SetFactionsFriendly(selectedFaction, scrFaction, playerID, false);
			else 
				factionManager.SetFactionsHostile(selectedFaction, scrFaction, playerID, false);
		}
		
		//~ Update AIs
		SCR_FactionManager.RequestUpdateAllTargetsFactions();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void CreatePresets()
	{
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		m_aValues.Clear();	
		
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
			
		SCR_Faction scrFaction;
		SCR_EditorAttributeFloatStringValueHolder value;
		
		for (int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			
			if (!scrFaction)
				continue;
			
			value = new SCR_EditorAttributeFloatStringValueHolder();
			
			value.SetWithUIInfo(scrFaction.GetUIInfo(), factionManager.GetFactionIndex(factionDelegates.Get(i).GetFaction()));
			m_aValues.Insert(value);
		}
	}
};
