[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MAP_COMPASS_SELECTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MAP_COMPASS_SELECT : SCR_BaseTutorialStage
{
	Widget m_wHighlight;
	SCR_MapToolEntry m_CompassTool;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpened);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClosed);
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpened);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClosed);
		
		if (!m_TutorialComponent.GetIsMapOpen())
			OnMapClosed(null);

		HighlightIcon();
	}
	
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
			if (tool.m_sIconQuad == "compass")
			{
				toolButton = GetGame().GetWorkspace().FindAnyWidget("ToolMenuButton"+i);
				m_CompassTool = tool;
			}
		}
		
		if (m_CompassTool)
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
	override protected bool GetIsFinished()
	{	
		if (!m_CompassTool)
			return false;
		
		return m_CompassTool.IsEntryActive();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Navigation_MAP_COMPASS_SELECT()
	{
		delete m_wHighlight;
	}
};