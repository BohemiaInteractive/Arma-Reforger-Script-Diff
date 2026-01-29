[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_GameOverTypeEditorAttribute: SCR_BaseFloatValueHolderEditorAttribute
{
	protected ref SCR_GameOverScreenConfig m_GameOverScreenConfig;
	
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
		
		if (gameOverManager.GetGameOverConfig() == null)
			return null;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(int.MIN);
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_GameOverFactionsEditorAttribute);
		
		array <SCR_BaseGameOverScreenInfo> gameOverScreens = new array <SCR_BaseGameOverScreenInfo>;
		array <SCR_BaseGameOverScreenInfo> editorGameOverScreens = new array <SCR_BaseGameOverScreenInfo>;
		
		if (!SetGameOverConfig())
			return;
		
		m_GameOverScreenConfig.GetGameOverInfoArray(gameOverScreens);
		GetEditorGameOverInfoArray(gameOverScreens, editorGameOverScreens);
		
		int index = var.GetInt();
		if (index >= editorGameOverScreens.Count() || index < 0)
			return;

		SCR_BaseGameOverScreenInfoEditor editorOptional = editorGameOverScreens[index].GetEditorOptionalParams(); 
		manager.SetAttributeEnabled(SCR_GameOverFactionsEditorAttribute, (editorOptional != null && editorOptional.m_MirroredState));
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverManager)
			return;
		
		int index = var.GetInt();
		
		array <SCR_BaseGameOverScreenInfo> gameOverScreens = new array <SCR_BaseGameOverScreenInfo>;
		array <SCR_BaseGameOverScreenInfo> editorGameOverScreens = new array <SCR_BaseGameOverScreenInfo>;
		if (!SetGameOverConfig())
			return;
		
		m_GameOverScreenConfig.GetGameOverInfoArray(gameOverScreens);
		GetEditorGameOverInfoArray(gameOverScreens, editorGameOverScreens);
		
		gameOverManager.SetEditorGameOverType(editorGameOverScreens[index].GetInfoId());
	}
	
	protected int GetEditorGameOverInfoArray(notnull array <SCR_BaseGameOverScreenInfo> gameOverScreens, notnull out array <SCR_BaseGameOverScreenInfo> editorGameOverScreens)
	{
		SCR_BaseGameOverScreenInfoEditor editorOptional;
		bool hasPlayableFactions = false;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (delegateFactionManager)
		{
			hasPlayableFactions = (delegateFactionManager.GetPlayableFactionDelegateCount() > 0);
		}
		
		foreach (SCR_BaseGameOverScreenInfo screen: gameOverScreens)
		{
			editorOptional = screen.GetEditorOptionalParams();
			
			if (!editorOptional || !editorOptional.m_bCanBeSetByGameMaster || (editorOptional.m_bNeedsPlayableFactions && !hasPlayableFactions))
				continue;
			
			editorGameOverScreens.Insert(screen);
		}

		return editorGameOverScreens.Count();
	}
	
	protected bool SetGameOverConfig()
	{
		if (m_GameOverScreenConfig)
			return true;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return false;

		SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(gameMode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverManager)
			return false;
		
		m_GameOverScreenConfig = gameOverManager.GetGameOverConfig();
		
		return true;
	}
	
	
	protected void CreatePresets()
	{
		if (!SetGameOverConfig())
			return;
		
		m_aValues.Clear();
		
		bool hasPlayableFactions = false;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (delegateFactionManager)
		{
			hasPlayableFactions = (delegateFactionManager.GetPlayableFactionDelegateCount() > 0);
		}
		
		array <SCR_BaseGameOverScreenInfo> gameOverScreens = new array <SCR_BaseGameOverScreenInfo>;
		array <SCR_BaseGameOverScreenInfo> editorGameOverScreens = new array <SCR_BaseGameOverScreenInfo>;
		m_GameOverScreenConfig.GetGameOverInfoArray(gameOverScreens);
		GetEditorGameOverInfoArray(gameOverScreens, editorGameOverScreens);
		
		SCR_EditorAttributeFloatStringValueHolder value;	
		SCR_BaseGameOverScreenInfoEditor editorOptional = null;
		
		foreach (SCR_BaseGameOverScreenInfo screen: editorGameOverScreens)
		{
			editorOptional = screen.GetEditorOptionalParams();
			
			if (editorOptional.m_bNeedsPlayableFactions && !hasPlayableFactions)
				continue;
			
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetName(editorOptional.m_sDisplayName);
			value.SetFloatValue(screen.GetInfoId());
			
			m_aValues.Insert(value);
		}
	}
	
	override bool IsSerializable()
	{
		return false;
	}
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		CreatePresets();		
		return super.GetEntries(outEntries);
	}

};