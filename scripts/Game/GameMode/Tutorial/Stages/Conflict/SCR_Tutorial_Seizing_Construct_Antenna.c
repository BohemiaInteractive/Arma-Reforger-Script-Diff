[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Construct_AntennaClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_Construct_Antenna : SCR_BaseTutorialStage
{
	SCR_CampaignBuildingCompositionComponent m_CampaignBuildingComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity bunker = GetGame().GetWorld().FindEntityByName("BUILDING_ANTENNA");
		if (!bunker)
			return;
		
		RegisterWaypoint(bunker, "", "BUILD");
		
		m_CampaignBuildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(bunker.FindComponent(SCR_CampaignBuildingCompositionComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		IEntity fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseLighthouseNorth");
		if (fakeBase)
		{
			SCR_TutorialFakeBaseComponent fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				//TODO: SUBOPTIMAL
				fakeBaseComponent.AddLinkName("FakeBaseSignalHill");
				fakeBaseComponent.CreateLinks();
				fakeBaseComponent.m_bAntennaWidgetVisible = false;
				SCR_Enum.SetFlag(fakeBaseComponent.m_eServices, SCR_EFakeBaseServices.RADIO_ANTENNA);
			}
		}
		
		super.OnStageFinished();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_CampaignBuildingComponent)
			return false;
		
		return m_CampaignBuildingComponent.IsCompositionSpawned();
	}
};