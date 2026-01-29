[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_CaptureBaseClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_CaptureBase : SCR_BaseTutorialStage
{
	SCR_FactionAffiliationComponent m_AffiliationComp;
	SCR_PopUpNotification m_PopUpNotification;
	SCR_SeizingComponent m_SeizingComponent;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PopUpNotification = SCR_PopUpNotification.GetInstance();
		
		IEntity base = GetGame().GetWorld().FindEntityByName("FakeBaseLighthouseNorth");
		RegisterWaypoint(base);
		m_AffiliationComp = SCR_FactionAffiliationComponent.Cast(base.FindComponent(SCR_FactionAffiliationComponent));
		m_AffiliationComp.SetFaction(base ,GetGame().GetFactionManager().GetFactionByKey("USSR"));
		
		BaseGameTriggerEntity trigger;
		IEntity child = base.GetChildren();
		while (child)
		{
			if (child.IsInherited(BaseGameTriggerEntity))
			{
				trigger = BaseGameTriggerEntity.Cast(child);
				break;
			}
				
			child = child.GetSibling();
		}
			
		if (trigger)
			trigger.SetSphereRadius(12);
		
		m_SeizingComponent = SCR_SeizingComponent.Cast(base.FindComponent(SCR_SeizingComponent));
		m_SeizingComponent.GetOnCaptureStart().Insert(OnSeizingStarted);
		m_SeizingComponent.GetOnCaptureInterrupt().Insert(OnSeizingInterrupted);
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_C", 5);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSeizingStarted(SCR_Faction faction, SCR_SeizingComponent seizingComponent)
	{	
		m_PopUpNotification.PopupMsg("#AR-Campaign_SeizingFriendly-UC", -1, prio: SCR_ECampaignSeizingMessagePrio.SEIZING_YOU, progressStart: m_SeizingComponent.GetSeizingStartTimestamp(), progressEnd: m_SeizingComponent.GetSeizingEndTimestamp(), category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSeizingInterrupted(SCR_Faction faction, SCR_SeizingComponent seizingComponent)
	{
		m_PopUpNotification.HideCurrentMsg();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_AffiliationComp)
		{
			if (m_AffiliationComp.GetAffiliatedFaction() == GetGame().GetFactionManager().GetFactionByKey("USSR"))
				return false;
			
			IEntity fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseLighthouseNorth");
			if (fakeBase)
			{
				SCR_TutorialFakeBaseComponent fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
				if (fakeBaseComponent)
				{
					//TODO: SUBOPTIMAL
					fakeBaseComponent.m_BaseColor = Color.FromRGBA(4,141,231,255);
					fakeBaseComponent.m_eMilitarySymbolIdentity = EMilitarySymbolIdentity.BLUFOR;
					fakeBaseComponent.m_sHighlight = "Friend_Installation_Focus_Land";
					fakeBaseComponent.CreateLinks();
				}
				
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Seizing_CaptureBase()
	{
		m_PopUpNotification.HideCurrentMsg();
		m_SeizingComponent.GetOnCaptureStart().Remove(OnSeizingStarted);
		m_SeizingComponent.GetOnCaptureInterrupt().Remove(OnSeizingInterrupted);
	}
};