[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Select_AntennaClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_Seizing_Select_Antenna : SCR_BaseTutorialStage
{
	protected string m_sSpawnedEntityName = "BUILDING_ANTENNA";
	protected Widget m_wHighlight;
	protected bool m_bPreviewSelected;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_PreviewEntityEditorComponent previewEntityComponent = m_TutorialComponent.GetPreviewEntityComponent();
		if (previewEntityComponent)
			previewEntityComponent.GetOnPreviewCreate().Insert(OnPreviewCreated);
		
		GetGame().GetCallqueue().CallLater(DelayedHighlight, 1500, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPreviewCreated(SCR_EditablePreviewEntity previewEntity)
	{
		m_bPreviewSelected = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedHighlight()
	{
		Widget w = GetGame().GetWorkspace().FindAnyWidget("ContentBrowser_AssetCard");
		m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(w, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bPreviewSelected || GetGame().GetWorld().FindEntityByName(m_sSpawnedEntityName);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Seizing_Select_Antenna()
	{
		SCR_PreviewEntityEditorComponent previewEntityComponent = m_TutorialComponent.GetPreviewEntityComponent();
		if (previewEntityComponent)
			previewEntityComponent.GetOnPreviewCreate().Remove(OnPreviewCreated);
		
		GetGame().GetCallqueue().Remove(DelayedHighlight);
		delete m_wHighlight;
	}
}