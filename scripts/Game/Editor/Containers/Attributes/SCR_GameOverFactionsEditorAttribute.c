[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_GameOverFactionsEditorAttribute: SCR_BaseMultiSelectPresetsEditorAttribute
{
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(item);
		if (!respawnComponent)
			return null;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(gameMode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverManager)
			return null;
		
		super.ReadVariable(item, manager);
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return null;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return null;
		
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>;
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
		
		
		SCR_Faction scrFaction;
		int playableFactionsCount = 0;
		
		for(int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			
			if (!scrFaction || !scrFaction.IsPlayable())
				continue;
			
			playableFactionsCount++;
			AddOrderedState(false);
		}
		
		if (playableFactionsCount <= 0)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateVector(GetFlagVector());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		super.WriteVariable(item, var, manager, playerID);
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverManager)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return;
		
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>;
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
		
		array <int> selectedFactions = new array<int>;
		SCR_Faction scrFaction;
		
		//Get all selected factions
		for(int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			
			if (!scrFaction || !scrFaction.IsPlayable() || !GetOrderedState())
				continue;
			
			selectedFactions.Insert(factionManager.GetFactionIndex(scrFaction));
		}
		
		gameOverManager.SetEditorGameOverFactions(selectedFactions);
	}
	
	protected int GetEditorGameOverInfoArray(notnull array <SCR_BaseGameOverScreenInfo> gameOverScreens, notnull out array <SCR_BaseGameOverScreenInfo> editorGameOverScreens)
	{
		SCR_BaseGameOverScreenInfoEditor editorOptional;
		
		foreach (SCR_BaseGameOverScreenInfo screen: gameOverScreens)
		{
			editorOptional = screen.GetEditorOptionalParams();
			
			if (!editorOptional || !editorOptional.m_bCanBeSetByGameMaster)
				continue;
			
			editorGameOverScreens.Insert(screen);
		}

		return editorGameOverScreens.Count();
	}
	
	override protected void CreatePresets()
	{
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		m_aValues.Clear();	
		
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>;
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
			
		SCR_Faction scrFaction;
		
		for(int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			
			if (!scrFaction || !scrFaction.IsPlayable())
				continue;

			SCR_EditorAttributeFloatStringValueHolder value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetWithUIInfo(scrFaction.GetUIInfo(), factionManager.GetFactionIndex(factionDelegates.Get(i).GetFaction()));
			m_aValues.Insert(value);
		}
	}
};