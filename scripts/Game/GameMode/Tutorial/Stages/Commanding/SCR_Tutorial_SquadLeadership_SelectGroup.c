[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_SelectGroupClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SquadLeadership_SelectGroup : SCR_BaseTutorialStage
{
	protected string m_sSpawnedEntityName = "REQUESTING_GROUP";
	protected Widget m_wHighlight;
	protected bool m_bPreviewSelected;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_PreviewEntityEditorComponent previewEntityComponent = m_TutorialComponent.GetPreviewEntityComponent();
		if (previewEntityComponent)
			previewEntityComponent.GetOnPreviewCreate().Insert(OnPreviewCreated);
		
		GetGame().GetCallqueue().CallLater(DelayedHighlight, 1500, false);
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 3);
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
	void ~SCR_Tutorial_SquadLeadership_SelectGroup()
	{
		SCR_PreviewEntityEditorComponent previewEntityComponent = m_TutorialComponent.GetPreviewEntityComponent();
		if (previewEntityComponent)
			previewEntityComponent.GetOnPreviewCreate().Remove(OnPreviewCreated);

		GetGame().GetCallqueue().Remove(DelayedHighlight);
		delete m_wHighlight;
	}
}