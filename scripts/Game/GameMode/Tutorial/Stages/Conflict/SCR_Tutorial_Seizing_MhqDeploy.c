[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_MhqDeployClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_MhqDeploy : SCR_BaseTutorialStage
{
	SCR_TutorialDeployMobileAssembly m_UserAction;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity box = GetGame().GetWorld().FindEntityByName("MHQ_TRUNK");
		if (!box)
			return;
		
		RegisterWaypoint(box, "", "CUSTOM");
		
		ActionsManagerComponent actionMan = ActionsManagerComponent.Cast(box.FindComponent(ActionsManagerComponent));
		if (!actionMan)
			return;
		
		array <BaseUserAction> userActions = {};
		
		actionMan.GetActionsList(userActions);
		foreach (BaseUserAction userAction : userActions)
		{
			if (userAction.IsInherited(SCR_TutorialDeployMobileAssembly))
				m_UserAction = SCR_TutorialDeployMobileAssembly.Cast(userAction);
		}
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_B", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (!m_UserAction)
			return false;
		
		if (m_UserAction.IsActive())
		{
			SCR_TutorialFakeBaseComponent fakeBaseComponent;
			IEntity fakeBase = GetGame().GetWorld().FindEntityByName("Tutorial_MobileHQ");
			if (fakeBase)
			{
				fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
				if (fakeBaseComponent)
				{
					fakeBaseComponent.m_bVisible = true;
					fakeBaseComponent.CreateLinks();
				}
			}
		
			fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseLighthouseNorth");
			if (fakeBase)
			{
				fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
				if (fakeBaseComponent)
				{
					fakeBaseComponent.m_BaseColor = Color.FromRGBA(218,8,7,255);
					fakeBaseComponent.m_sHighlight = "Hostile_Installation_Focus_Land";
					fakeBaseComponent.m_eMilitarySymbolIdentity = EMilitarySymbolIdentity.OPFOR;
					fakeBaseComponent.m_bAntennaWidgetVisible = true;
					fakeBaseComponent.m_bAllowServices = true;
					fakeBaseComponent.m_bAllowServicesSizeOverlay = true;
					fakeBaseComponent.CreateLinks();
				}
			}
			
			fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseSignalHill");
			if (fakeBase)
			{
				fakeBaseComponent.AddLinkName("Tutorial_MobileHQ");
				fakeBaseComponent.ClearLinks();
				fakeBaseComponent.CreateLinks();
			}
			
			return true;
		}
		
		return false;
	}
};