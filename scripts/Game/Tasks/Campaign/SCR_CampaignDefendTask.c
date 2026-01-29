//------------------------------------------------------------------------------------------------
//class SCR_CampaignDefendTaskClass: SCR_CampaignBaseTaskClass
//{
//};

//------------------------------------------------------------------------------------------------
//class SCR_CampaignDefendTask : SCR_CampaignBaseTask
//{
//	static const float DEFEND_TASK_DEFAULT_MIN_ALLY_DISTANCE = 300;
//	static const float DEFEND_TASK_DEFAULT_MAX_ALLY_DISTANCE = 400;
//	static const float DEFEND_TASK_DEFAULT_MIN_ENEMY_DISTANCE = 300;
//	static const float DEFEND_TASK_DEFAULT_MAX_ENEMY_DISTANCE = 400;
//	
//	//**************************//
//	//PROTECTED STATIC VARIABLES//
//	//**************************//
//	protected static SCR_CampaignMilitaryBaseComponent s_ClosestBase; //This is the closest base to the local player's character (only updated when task list is open)
//	
//	//*********************//
//	//PUBLIC STATIC METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetMinAllyDistance()
//	{
//		if (!GetTaskManager())
//			return DEFEND_TASK_DEFAULT_MIN_ALLY_DISTANCE;
//		
//		SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetMinAllyDistance();
//		
//		return DEFEND_TASK_DEFAULT_MIN_ALLY_DISTANCE;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetMaxAllyDistance()
//	{
//		if (!GetTaskManager())
//			return DEFEND_TASK_DEFAULT_MAX_ALLY_DISTANCE;
//		
//		SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetMaxAllyDistance();
//		
//		return DEFEND_TASK_DEFAULT_MAX_ALLY_DISTANCE;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetMinEnemyDistance()
//	{
//		if (!GetTaskManager())
//			return DEFEND_TASK_DEFAULT_MIN_ENEMY_DISTANCE;
//		
//		SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetMinEnemyDistance();
//		
//		return DEFEND_TASK_DEFAULT_MIN_ENEMY_DISTANCE;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetMaxEnemyDistance()
//	{
//		if (!GetTaskManager())
//			return DEFEND_TASK_DEFAULT_MAX_ENEMY_DISTANCE;
//		
//		SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetMaxEnemyDistance();
//		
//		return DEFEND_TASK_DEFAULT_MAX_ENEMY_DISTANCE;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static SCR_CampaignDefendTask FindDefendTask(SCR_CampaignMilitaryBaseComponent targetBase, SCR_CampaignFaction targetFaction)
//	{
//		array<SCR_BaseTask> tasks = new array<SCR_BaseTask>();
//		GetTaskManager().GetTasks(tasks);
//		
//		if (!tasks)
//			return null;
//		
//		for (int i = tasks.Count() - 1; i >= 0; i--)
//		{
//			SCR_CampaignDefendTask defendTask = SCR_CampaignDefendTask.Cast(tasks[i]);
//			if (!defendTask)
//				continue;
//			
//			if (defendTask.GetTargetBase() == targetBase && defendTask.GetTargetFaction() == targetFaction)
//				return defendTask;
//		}
//		
//		return null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static SCR_CampaignMilitaryBaseComponent FindClosestFriendlyBase(notnull IEntity controlledEntity, notnull Faction faction)
//	{
//		
//		array<SCR_MilitaryBaseComponent> bases = {};
//		SCR_MilitaryBaseSystem.GetInstance().GetBases(bases);
//		
//		if (!bases)
//			return null;
//		
//		int closestBaseIndex = -1;
//		float closestBaseDistance = float.MAX;
//		float minAllyDistanceSq = Math.Pow(GetMinAllyDistance(), 2);
//		vector controlledEntityOrigin = controlledEntity.GetOrigin();
//		
//		for (int i = bases.Count() - 1; i >= 0; i--)
//		{
//			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(bases[i]);
//			
//			if (!campaignBase)
//				continue;
//			
//			if (!campaignBase.IsInitialized())
//				continue;
//			
//			if (campaignBase.GetFaction() != faction)
//				continue;
//			
//			float distance = vector.DistanceSq(campaignBase.GetOwner().GetOrigin(), controlledEntityOrigin);
//			
//			if (distance < minAllyDistanceSq && distance < closestBaseDistance)
//			{
//				closestBaseDistance = distance;
//				closestBaseIndex = i;
//			}
//		}
//		
//		if (closestBaseIndex != -1)
//			return SCR_CampaignMilitaryBaseComponent.Cast(bases[closestBaseIndex]);
//		return null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static bool CheckDefendRequestConditions(notnull SCR_PlayerController playerController, out SCR_CampaignMilitaryBaseComponent closestBase)
//	{
//		IEntity locallyControlledEntity = playerController.GetMainEntity();
//		
//		if (!locallyControlledEntity)
//			return false;
//		
//		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(locallyControlledEntity.FindComponent(FactionAffiliationComponent));
//		if (!factionAffiliationComponent)
//			return false;
//		
//		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetAffiliatedFaction());
//		if (!faction)
//			return false;
//		
//		closestBase = FindClosestFriendlyBase(locallyControlledEntity, faction);
//		
//		if (!closestBase)
//			return false;
//		else
//		{
//			if (FindDefendTask(closestBase, faction))
//				return false;
//			
//			if (!FindEnemyPresence(closestBase, faction))
//				return false;
//			
//			return true;
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Set initialCheck to false if the task is already created for this base
//	static bool FindEnemyPresence(notnull SCR_CampaignMilitaryBaseComponent base, notnull SCR_CampaignFaction alliedFaction, bool initialCheck = true)
//	{
//		SCR_CampaignFaction enemyFaction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetEnemyFaction(alliedFaction);
//		if (!enemyFaction)
//			return false;
//		
//		SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (!supportEntity)
//			return false;
//		
//		float distanceSq;
//		if (initialCheck)
//			distanceSq = supportEntity.GetMinEnemyDistance();
//		else
//			distanceSq = supportEntity.GetMaxEnemyDistance();
//		
//		distanceSq *= distanceSq;
//		
//		array<SCR_ChimeraCharacter> enemyCharacters = supportEntity.GetCharactersByFaction(enemyFaction);
//		if (!enemyCharacters)
//			return false;
//		
//		vector baseOrigin = base.GetOwner().GetOrigin();
//		for (int i = enemyCharacters.Count() - 1; i >= 0; i--)
//		{
//			if (vector.DistanceSq(baseOrigin, enemyCharacters[i].GetOrigin()) < distanceSq)
//				return true;
//		}
//		
//		return false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static bool IsCharacterInAnyDefendTaskRange(SCR_ChimeraCharacter character)
//	{
//		array<SCR_BaseTask> tasks = new array<SCR_BaseTask>();
//		GetTaskManager().GetTasks(tasks);
//		
//		if (!tasks)
//			return false;
//		
//		SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (!supportEntity)
//			return false;
//		
//		for (int i = tasks.Count() - 1; i >= 0; i--)
//		{
//			SCR_CampaignDefendTask defendTask = SCR_CampaignDefendTask.Cast(tasks[i]);
//			if (!defendTask)
//				continue;
//			
//			if (defendTask.IsCharacterInDefendTaskRange(character, supportEntity))
//				return true;
//		}
//		
//		return false;
//	}
//	
//	//*********************************//
//	//PROTECTED OVERRIDE MEMBER METHODS//
//	//*********************************//
//	
//	//------------------------------------------------------------------------------------------------
//	protected override void UpdateMapInfo()
//	{
//		//Insert task icon into base's map UI
//		//m_TargetBase cannot be null here
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Shows a message about this task being available again
//	protected override void ShowAvailableTask(bool afterAssigneeRemoved = false)
//	{
//		SCR_ECannotAssignReasons reason;
//		if (CanBeAssigned(reason))
//		{
//			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//			if (campaign)
//			{
//				string text = TASK_AVAILABLE_TEXT + " " + GetTitle();
//				string baseName;
//				
//				if (m_TargetBase)
//					baseName = m_TargetBase.GetBaseNameUpperCase();
//
//				SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
//			}
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override protected void ShowTaskProgress(bool showMsg = true)
//	{
//		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//		if (campaign && showMsg)
//		{
//			string baseName;
//			
//			if (m_TargetBase)
//				baseName = m_TargetBase.GetBaseNameUpperCase();
//			
//			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_PROGRESS_TEXT + " " + GetTitle() + " " + TASK_AMOUNT_COMPLETED_TEXT, prio: SCR_ECampaignPopupPriority.TASK_PROGRESS, param1: baseName);
//		}
//	}
//	
//	//******************************//
//	//PUBLIC OVERRIDE MEMBER METHODS//
//	//******************************//
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		string baseName;
//		
//		if (m_TargetBase)
//			baseName = m_TargetBase.GetBaseName();
//		
//		textWidget.SetTextFormat(taskText, baseName);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Finish(bool showMsg = true)
//	{
//		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
//		super.Finish(showMsg);
//		
//		if (!m_TargetBase)
//			return;
//		
//		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//		if (!campaign)
//			return;
//		
//		if (showMsg)
//		{
//			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_TargetBase.GetBaseNameUpperCase(), sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
//		}
//		
//		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
//		
//		if (!comp)
//			return;
//		
//		// Reward XP for reconfiguring a relay
//		if (!GetTaskManager().IsProxy())
//		{
//			SCR_CampaignDefendTaskSupportEntity supportClass = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//			if (!supportClass)
//				return;
//			
//			array<SCR_ChimeraCharacter> characters = supportClass.GetCharactersByFaction(SCR_CampaignFaction.Cast(GetTargetFaction()));
//			if (!characters)
//				return;
//			
//			for (int i = characters.Count() - 1; i >= 0; i--)
//			{
//				if (IsCharacterInDefendTaskRange(characters[i]))
//					comp.AwardXP(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(characters[i]), SCR_EXPRewards.TASK_DEFEND, 1.0);
//			}
//		};
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Fails the task.
//	override void Fail(bool showMsg = true)
//	{
//		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
//		super.Fail(showMsg);
//		
//		if (!m_TargetBase)
//			return;
//		
//		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//		if (campaign && showMsg)
//		{
//			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_TargetBase.GetBaseNameUpperCase(), sound: SCR_SoundEvent.TASK_FAILED);
//		}
//	}
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	bool IsCharacterInDefendTaskRange(notnull SCR_ChimeraCharacter character, SCR_CampaignDefendTaskSupportEntity supportClass = null)
//	{
//		if (!supportClass)
//		{
//			supportClass = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//			if (!supportClass)
//				return false;
//		}
//		
//		float maxDistanceSq;
//		// Is ally, use ally distance
//		if (character.GetFaction() == GetTargetFaction())
//			maxDistanceSq = supportClass.GetMaxAllyDistance();
//		else
//			maxDistanceSq = supportClass.GetMaxEnemyDistance();
//		
//		maxDistanceSq *= maxDistanceSq;
//		
//		float distanceSq = vector.DistanceSq(character.GetOrigin(), GetTargetBase().GetOwner().GetOrigin());
//		
//		return (distanceSq < maxDistanceSq);
//	}
//	
//	//***************************//
//	//PUBLIC MEMBER EVENT METHODS//
//	//***************************//
//	
//	//------------------------------------------------------------------------------------------------
//	void PeriodicalCheck()
//	{
//		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(m_TargetFaction);
//		if (!m_TargetBase || !campaignFaction)
//			return;
//		
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		// Enemy presence not found == success
//		if (!FindEnemyPresence(m_TargetBase, campaignFaction, false))
//			supportEntity.FinishTask(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnCharacterDeath(SCR_ChimeraCharacter character)
//	{
//		// Enemy died in range of this tasks target base, let's check if the town is safe
//		if (character.GetFaction() != GetTargetFaction() && IsCharacterInDefendTaskRange(character))
//			PeriodicalCheck();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a base has been captured.
//	void OnBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
//	{
//		if (!base || base != m_TargetBase || !GetTaskManager())
//			return;
//		
//		if (base.GetFaction() == m_TargetFaction)
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		supportEntity.FailTask(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override string GetMapDescriptorText()
//	{
//		return GetTaskListTaskText();
//	}
//	
//	//------------------------------------------------------------------------------------------------
// 	void SCR_CampaignDefendTask(IEntitySource src, IEntity parent)
//	{
//		if (SCR_Global.IsEditMode(this))
//			return;
//		
//		if (GetTaskManager().IsProxy())
//			return;
//		
//		if (SCR_BaseTaskManager.s_OnPeriodicalCheck60Second)
//			SCR_BaseTaskManager.s_OnPeriodicalCheck60Second.Insert(PeriodicalCheck);
//		
//		SCR_MilitaryBaseSystem.GetInstance().GetOnBaseFactionChanged().Insert(OnBaseCaptured);
//		
//		SCR_CampaignDefendTaskSupportEntity supportClass = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (supportClass)
//			supportClass.GetOnCharacterDeath().Insert(OnCharacterDeath);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_CampaignDefendTask(string description)
//	{
//		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
//			return;
//		
//		if (GetTaskManager().IsProxy())
//			return;
//		
//		if (SCR_BaseTaskManager.s_OnPeriodicalCheck60Second)
//			SCR_BaseTaskManager.s_OnPeriodicalCheck60Second.Remove(PeriodicalCheck);
//		
//		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
//		
//		if (baseManager)
//			baseManager.GetOnBaseFactionChanged().Remove(OnBaseCaptured);
//		
//		SCR_CampaignDefendTaskSupportEntity supportClass = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//		if (supportClass)
//			supportClass.GetOnCharacterDeath().Remove(OnCharacterDeath);
//	}
//};
