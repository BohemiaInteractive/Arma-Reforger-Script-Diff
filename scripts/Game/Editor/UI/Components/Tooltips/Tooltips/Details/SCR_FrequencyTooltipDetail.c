[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FrequencyTooltipDetail : SCR_EntityTooltipDetail
{	
	[Attribute("")]
	protected LocalizedString m_sNoFrequency;
	
	protected TextWidget m_Text;
	protected SCR_GadgetManagerComponent m_GadgetManager;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		set<int> frequencies = new set<int>();
		SCR_Global.GetFrequencies(m_GadgetManager, frequencies);
		if (frequencies.IsEmpty())
			m_Text.SetText(m_sNoFrequency);
		else
			m_Text.SetText(SCR_FormatHelper.FormatFrequencies(frequencies));
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		if (!m_Text || entity.GetPlayerID() == 0)
			return false;
		
		m_GadgetManager = SCR_GadgetManagerComponent.Cast(entity.GetOwner().FindComponent(SCR_GadgetManagerComponent));
		return m_GadgetManager != null;
	}
}
