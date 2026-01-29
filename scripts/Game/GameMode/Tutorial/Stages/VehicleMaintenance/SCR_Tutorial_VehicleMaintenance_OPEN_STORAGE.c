[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_OPEN_STORAGEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_OPEN_STORAGE : SCR_BaseTutorialStage
{
	ActionsManagerComponent m_ActionsManager;
	ScriptedUserAction m_UserAction;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity storage = GetGame().GetWorld().FindEntityByName("StorageDepo");
		if (!storage)
			return;
	
		RegisterWaypoint(storage, "", "CUSTOM").SetOffsetVector("0 1 0");
		
		m_ActionsManager = ActionsManagerComponent.Cast(storage.FindComponent(ActionsManagerComponent));
		if (!m_ActionsManager)
			return;
		
		array <BaseUserAction> userActions = {};
		m_ActionsManager.GetActionsList(userActions);
		if (!userActions || userActions.IsEmpty())
			return;
		
		foreach (BaseUserAction userAction : userActions)
		{
			if (userAction.IsInherited(SCR_OpenStorageAction))
				m_UserAction = ScriptedUserAction.Cast(userAction); 
		}
		
		m_ActionsManager.AddUserActionEventListener(m_UserAction, OnInvoked);
		
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 13);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInvoked(IEntity user, ScriptedUserAction action, EUserActionEvent e)
	{
		m_bFinished = true;
		m_ActionsManager.RemoveUserActionEventListener(m_UserAction, OnInvoked);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			m_TutorialComponent.InsertStage("KMB");
		else
			m_TutorialComponent.InsertStage("GAMEPAD");
		
		SCR_TutorialLogic_VehicleMaintenance logic = SCR_TutorialLogic_VehicleMaintenance.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
		{
			GetGame().OnInputDeviceIsGamepadInvoker().Insert(logic.OnInputChanged);
		}
		
		super.OnStageFinished();
	}
};