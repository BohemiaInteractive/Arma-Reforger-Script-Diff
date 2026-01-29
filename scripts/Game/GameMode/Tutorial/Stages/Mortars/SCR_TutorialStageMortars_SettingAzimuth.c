[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_SettingAzimuthClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_SettingAzimuth : SCR_BaseTutorialStage
{
	protected bool m_bHelp;
	protected Widget m_wHighlight;
	protected SCR_MapToolEntry m_ProtractorTool;
	
	//------------------------------------------------------------------------------------------------
	protected void HighlightIcon()
	{
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt)
			return;
		
		SCR_MapToolMenuUI toolMenuUI = SCR_MapToolMenuUI.Cast(mapEnt.GetMapUIComponent(SCR_MapToolMenuUI));
		if (!toolMenuUI)
			return;	
		
		array<ref SCR_MapToolEntry> tools = toolMenuUI.GetMenuEntries();
		
		Widget toolButton;
		foreach (int i, SCR_MapToolEntry tool : tools)
		{
			if (tool.m_sIconQuad == "ruler")
			{
				toolButton = GetGame().GetWorkspace().FindAnyWidget("ToolMenuButton"+i);
				m_ProtractorTool = tool;
			}
		}
		
		if (m_ProtractorTool)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(toolButton, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapClosed(MapConfiguration config)
	{
		GetGame().GetCallqueue().Remove(HighlightIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapOpened(MapConfiguration config)
	{
		//Delayed call, so all map widgets are properly initialized
		GetGame().GetCallqueue().CallLater(HighlightIcon, 100);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpened);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClosed);
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (!m_bHelp && m_fStageTimer > 90)
		{
			m_bHelp = true;
			SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(1));
		}

		TurretControllerComponent turret = TurretControllerComponent.Cast(GetGame().GetWorld().FindEntityByName("Mortar").FindComponent(TurretControllerComponent));
		
		if (!turret)
			return false;
		
		BaseWeaponManagerComponent weaponManager = turret.GetWeaponManager();
		
		if (!weaponManager)
			return false;
		
		vector transform[4];
		weaponManager.GetCurrentMuzzleTransform(transform);
		
		float azimuth = transform[2].VectorToAngles()[0];

		return azimuth > 181.85 && azimuth < 183.1;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialStageMortars_SettingAzimuth()
	{
		delete m_wHighlight;
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpened);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClosed);
	}
};