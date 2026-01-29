//! Load a proper config file for player faction and open an asset browser.
class SCR_ToolbarContextBrowserBuildingUIComponent : ScriptedWidgetComponent
{
	[Attribute("BrowserButton")]
	protected string m_sOpenContentBrowserButtonName;
	
	[Attribute()]
	protected ref SCR_EditorContentBrowserDisplayConfig m_ContentBrowserDisplayConfig;
	
	//------------------------------------------------------------------------------------------------
	protected void OpenAssetBrowserButton(Widget widget, float value, EActionTrigger actionTrigger)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		ChimeraCharacter char = ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!char)
			return;
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(char.FindComponent(FactionAffiliationComponent));
		if (!factionComp)
			return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(factionComp.GetAffiliatedFaction());
		if (!faction)
			return;
		
		// Get all display config data from attribute
		bool saveBrowserState;
		array<EEditableEntityLabel> alwaysActiveLabels = {}, whitelistLabels = {};
		m_ContentBrowserDisplayConfig.GetAlwaysActiveLabels(alwaysActiveLabels);
		m_ContentBrowserDisplayConfig.GetWhiteListeLabels(whitelistLabels);
		array<EEditableEntityLabelGroup> whitelistLabelGroups = {};
		m_ContentBrowserDisplayConfig.GetWhiteListeLabelGroups(whitelistLabelGroups);
		
		LocalizedString browserHeader = m_ContentBrowserDisplayConfig.GetHeader();
		saveBrowserState = m_ContentBrowserDisplayConfig.GetSaveContentBrowserState();
		
		// Add player's current faction as always active label
		EEditableEntityLabel factionLabel = faction.GetFactionLabel();
		alwaysActiveLabels.Insert(factionLabel);
		
		SCR_EditorContentBrowserDisplayConfig displayConfig = new SCR_EditorContentBrowserDisplayConfig(whitelistLabelGroups, whitelistLabels, alwaysActiveLabels, saveBrowserState, browserHeader);
		
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		if (contentBrowserManager)
			contentBrowserManager.OpenBrowserLabelConfigInstance(displayConfig);		
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		Widget buttonWidget = w.FindAnyWidget(m_sOpenContentBrowserButtonName);
		if (buttonWidget)
			ButtonActionComponent.GetOnAction(buttonWidget).Insert(OpenAssetBrowserButton);
	}
}
