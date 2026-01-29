/*
Panel which shows state of a scenario.
It refreshes itself periodically.
*/

class SCR_ScenarioDetailsPanelComponent : SCR_ContentDetailsPanelBase
{		
	protected ref MissionWorkshopItem m_Scenario;
	protected SCR_MissionHeader m_Header;
	
	protected SCR_ScenarioBackendImageComponent m_BackendImageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		
		m_BackendImageComponent = SCR_ScenarioBackendImageComponent.Cast(m_CommonWidgets.m_wBackendImage.FindHandler(SCR_ScenarioBackendImageComponent));

		UpdateAllWidgets();
	}
	
	// -------- Public API -----------
	//-----------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem scenario)
	{
		SCR_MissionHeader header;
		if (scenario)
			header = SCR_MissionHeader.Cast(scenario.GetHeader());

		m_Header = header;
		m_Scenario = scenario;
		
		UpdateAllWidgets();
	}	
	
	// -------- Protected / Private -----------
	//-----------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!GetGame().InPlayMode())
			return;
		
		if (!m_Scenario && !m_Header)
		{
			m_CommonWidgets.m_wNameText.SetText(string.Empty);
			m_CommonWidgets.m_wAuthorNameText.SetText(string.Empty);
			SetDescriptionText(string.Empty);
			
			if (m_BackendImageComponent)
				m_BackendImageComponent.SetImage(null);
			
			return;
		}
		
		// Name
		string title;
		if (m_Header)
			title = m_Header.m_sName;
		else if (m_Scenario)
			title = m_Scenario.Name();

		m_CommonWidgets.m_wNameText.SetText(title);
		
		// Author name
		// We don't have author name in scenario header now.
		UpdateAuthorNameText();
		
		// Description
		if (m_Header)
			SetDescriptionText(m_Header.m_sDescription);
		else if(m_Scenario)
			SetDescriptionText(m_Scenario.Description());
		
		// Image
		if (m_BackendImageComponent)
		{
			BackendImage image;
			
			if (m_Scenario)
				image = m_Scenario.Thumbnail();
			
			m_BackendImageComponent.SetImage(image);
		}

		// Error message
		SCR_ERevisionAvailability availability = SCR_ScenarioUICommon.GetOwnerRevisionAvailability(m_Scenario);
		bool isInError = availability != SCR_ERevisionAvailability.ERA_AVAILABLE && availability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		bool restricted = SCR_ScenarioUICommon.IsOwnerRestricted(m_Scenario);
		
		m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(isInError || restricted, false);
		m_CommonWidgets.m_WarningOverlayComponent.SetBlurUnderneath(restricted);
		
		if (restricted)
			m_CommonWidgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.MESSAGE_RESTRICTED_GENERIC, SCR_WorkshopUiCommon.ICON_REPORTED);
		else
			m_CommonWidgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessageVerbose(availability), SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(availability));
	
		if (m_BackendImageComponent)
		{
			if (isInError)
				m_BackendImageComponent.SetImageSaturation(UIConstants.DISABLED_WIDGET_SATURATION);
			else
				m_BackendImageComponent.SetImageSaturation(UIConstants.ENABLED_WIDGET_SATURATION);
		}
	}
	
	//-----------------------------------------------------------------------------------
	protected void UpdateAuthorNameText()
	{
		if (!m_Scenario)
			return;

		if (m_Scenario.GetOwner() && m_Scenario.GetOwner().Author())
			m_CommonWidgets.m_wAuthorNameText.SetText(m_Scenario.Author());
		else
			m_CommonWidgets.m_wAuthorNameText.SetText(UIConstants.BOHEMIA_INTERACTIVE);
	}
}