//------------------------------------------------------------------------------------------------
//[EntityEditorProps(category: "GameScripted/Tasks", description: "Campaign defend task support entity.", color: "0 0 255 255")]
//class SCR_CampaignDefendTaskSupportEntityClass: SCR_RequestedTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//void OnCharacterDeath(SCR_ChimeraCharacter character);
//typedef func OnCharacterDeath;
//typedef ScriptInvokerBase<OnCharacterDeath> OnCharacterDeathInvoker;

//------------------------------------------------------------------------------------------------
//class SCR_CampaignDefendTaskSupportEntity : SCR_RequestedTaskSupportEntity
//{
//	[Attribute("300", desc: "From how far can an ally request reinforcements. (in m)")]
//	protected float m_fMinAllyDistance;
//	
//	[Attribute("400", desc: "How far an ally can be to receive rewards for this task. (in m)")]
//	protected float m_fMaxAllyDistance;
//	
//	[Attribute("300", desc: "How far an enemy has to be, for an ally to be able to request reinforcements. (in m)")]
//	protected float m_fMinEnemyDistance;
//	
//	[Attribute("400", desc: "How far an enemy has to be, to be outside of the radius of an existing defend task. (in m)")]
//	protected float m_fMaxEnemyDistance;
//	
//	// This only holds living characters, no need to check whether they are alive!!
//	protected ref map<SCR_CampaignFaction, ref array<SCR_ChimeraCharacter>> m_mFactionSortedCharacters = new map<SCR_CampaignFaction, ref array<SCR_ChimeraCharacter>>();
//	
//	protected ref OnCharacterDeathInvoker m_OnCharacterDeath;
//	
//	protected SCR_CampaignMilitaryBaseComponent m_ClosestBase;
//	
//	//------------------------------------------------------------------------------------------------
//	OnCharacterDeathInvoker GetOnCharacterDeath()
//	{
//		if (!m_OnCharacterDeath)
//			m_OnCharacterDeath = new OnCharacterDeathInvoker();
//		
//		return m_OnCharacterDeath;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool CanRequest()
//	{
//		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
//		if (!playerController)
//			return false;
//		
//		if (!SCR_CampaignDefendTask.CheckDefendRequestConditions(playerController, m_ClosestBase))
//			return false;
//		
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Request()
//	{
//		PlayerController playerController = GetGame().GetPlayerController();
//		if (!playerController)
//			return;
//		
//		SCR_UIRequestEvacTaskComponent.RequestReinforcements(playerController);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetRequestButtonText(notnull TextWidget textWidget)
//	{
//		if (!m_ClosestBase)
//			return;
//		
//		textWidget.SetTextFormat(m_sRequestButtonText, m_ClosestBase.GetBaseName());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTargetBase(int taskID, int baseID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_CampaignBaseTask task = SCR_CampaignBaseTask.Cast(GetTaskManager().GetTask(taskID));
//		if (!task)
//			return;
//		
//		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseID);
//		if (!base)
//			return;
//		
//		task.SetTargetBase(base);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetBase(notnull SCR_CampaignBaseTask task, notnull SCR_CampaignMilitaryBaseComponent base)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		int taskID, baseID;
//		
//		taskID = task.GetTaskID();
//		baseID = base.GetCallsign();
//		
//		Rpc(RPC_SetTargetBase, taskID, baseID);
//		RPC_SetTargetBase(taskID, baseID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void CreateCampaignDefendTask(SCR_CampaignMilitaryBaseComponent targetBase, Faction targetFaction)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_CampaignDefendTask defendTask = SCR_CampaignDefendTask.Cast(CreateTask());
//		if (!defendTask)
//			return;
//		
//		SetTargetBase(defendTask, targetBase); // Replicated internally
//		SetTargetFaction(defendTask, targetFaction); // Replicated internally
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnControllableCreated(IEntity controllable)
//	{
//		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controllable);
//		if (!character)
//			return;
//		
//		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!faction)
//			return;
//		
//		array<SCR_ChimeraCharacter> factionCharacters = GetCharactersByFaction(faction);
//		if (!factionCharacters)
//		{
//			factionCharacters = new array<SCR_ChimeraCharacter>();
//			m_mFactionSortedCharacters.Insert(faction, factionCharacters);
//		}
//		
//		factionCharacters.Insert(character);
//		
//		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
//		if (!characterControllerComponent)
//			return;
//		
//		characterControllerComponent.GetOnPlayerDeathWithParam().Insert(OnCharacterDeath);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnCharacterDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity killerEntity, Instigator killer)
//	{
//		SCR_ChimeraCharacter character = characterControllerComponent.GetCharacter();
//		if (!character)
//			return;
//		
//		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!faction)
//			return;
//		
//		array<SCR_ChimeraCharacter> factionCharacters = m_mFactionSortedCharacters.Get(faction);
//		if (!factionCharacters)
//			return;
//		
//		factionCharacters.RemoveItem(character);
//		
//		if (m_OnCharacterDeath)
//			m_OnCharacterDeath.Invoke(character);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//Only for cases when character is removed before dying
//	void OnControllableDeleted(IEntity controllable)
//	{
//		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controllable);
//		if (!character)
//			return;
//		
//		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!faction)
//			return;
//		
//		array<SCR_ChimeraCharacter> factionCharacters = m_mFactionSortedCharacters.Get(faction);
//		if (!factionCharacters)
//			return;
//		
//		if (factionCharacters.Find(character) > -1)
//			factionCharacters.RemoveItem(character);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	array<SCR_ChimeraCharacter> GetCharactersByFaction(SCR_CampaignFaction faction)
//	{
//		array<SCR_ChimeraCharacter> characters = m_mFactionSortedCharacters.Get(faction);
//		
//		if (!characters)
//			return characters;
//		
//		// TODO matousvoj1: Proper removal of these items once OnDelete is called on entities
//		// Added to avoid nulls in arrays
//		for (int i = characters.Count() - 1; i >= 0; i--)
//		{
//			if (!characters[i])
//				characters.Remove(i);
//		}
//		
//		return characters;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMinAllyDistance()
//	{
//		return m_fMinAllyDistance;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMaxAllyDistance()
//	{
//		return m_fMaxAllyDistance;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMinEnemyDistance()
//	{
//		return m_fMinEnemyDistance;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMaxEnemyDistance()
//	{
//		return m_fMaxEnemyDistance;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[Obsolete("Use new SCR_TaskManagerUIComponent instead")]
//	override void OnTaskListOpen(notnull SCR_TaskManagerUIComponent uiTaskManagerComponent)
//	{	/*
//		if (!CanRequest())
//			return;
//		
//		if (!m_ClosestBase)
//			return;
//		
//		array<Widget> widgets = uiTaskManagerComponent.GetWidgetsArray();
//		if (!widgets)
//			return;
//		
//		Widget button = CreateButtonAndSetPadding(uiTaskManagerComponent.GetParentWidget(), widgets);
//		if (!button)
//			return;
//		
//		TextWidget textWidget = TextWidget.Cast(button.FindAnyWidget("Text"));
//		if (textWidget)
//			SetRequestButtonText(textWidget);
//		*/
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Initialize()
//	{
//		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//		if (!gameMode)
//			return;
//		
//		gameMode.GetOnControllableSpawned().Insert(OnControllableCreated);
//		gameMode.GetOnControllableDeleted().Insert(OnControllableDeleted);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_CampaignDefendTaskSupportEntity()
//	{
//		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//		if (gameMode)
//		{
//			gameMode.GetOnControllableSpawned().Remove(OnControllableCreated);
//			gameMode.GetOnControllableDeleted().Remove(OnControllableDeleted);
//		}
//		
//		SCR_CharacterControllerComponent characterControllerComponent;
//		array<SCR_ChimeraCharacter> characters;
//		for (int i = m_mFactionSortedCharacters.Count() - 1; i >= 0; i--)
//		{
//			characters = m_mFactionSortedCharacters.GetElement(i);
//			if (!characters)
//				continue;
//			
//			for (int j = characters.Count() - 1; j >= 0; j--)
//			{
//				if (!characters[j])
//					continue;
//				
//				characterControllerComponent = SCR_CharacterControllerComponent.Cast(characters[j].FindComponent(SCR_CharacterControllerComponent));
//				if (!characterControllerComponent)
//					return;
//				
//				characterControllerComponent.GetOnPlayerDeathWithParam().Remove(OnCharacterDeath);
//			}
//		}
//	}
//};