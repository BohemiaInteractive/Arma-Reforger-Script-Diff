[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_PROTRACTORClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_PROTRACTOR : SCR_BaseTutorialStage
{
	Widget m_wHighlight;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
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
			if (tool.m_sIconQuad == "ruler")
			{
				toolButton = GetGame().GetWorkspace().FindAnyWidget("ToolMenuButton"+i);
				tool.GetOnEntryToggledInvoker().Insert(OnEntryToggled);
				break;
			}
		}
		
		if (toolButton)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(toolButton, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryToggled(SCR_MapToolEntry toolEntry)
	{
		delete m_wHighlight;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Navigation_PROTRACTOR()
	{
		delete m_wHighlight;
	}
};