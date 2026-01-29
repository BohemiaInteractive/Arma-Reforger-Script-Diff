[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MAPTOOLSClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MAPTOOLS : SCR_BaseTutorialStage
{
	Widget m_wHighlight;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 5;
		
		Widget toolMenu = GetGame().GetWorkspace().FindAnyWidget("ToolMenu");
		
		if (toolMenu)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(toolMenu, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Navigation_MAPTOOLS()
	{
		delete m_wHighlight;
	}
};