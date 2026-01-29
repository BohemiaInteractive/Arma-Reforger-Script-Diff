//------------------------------------------------------------------------------------------------
// Attached to layout used for radial menu VON entries
class SCR_VONEntryComponent : SCR_SelectionMenuEntryComponent
{
	protected ImageWidget m_wPowerIcon;
	protected ImageWidget m_wActiveIcon;
	protected ImageWidget m_wMuteIcon;
	protected TextWidget m_wTransceiverText;
	protected TextWidget m_wFrequencyText;
	protected TextWidget m_wChannelText;
	
	//------------------------------------------------------------------------------------------------
	void SetPowerIcon(bool state)
	{
		if (state)
			m_wPowerIcon.SetColor(Color.FromInt(Color.GREEN));
		else 
			m_wPowerIcon.SetColor(Color.FromInt(Color.GRAY));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActiveIcon(bool state)
	{
		if (state)
			m_wActiveIcon.SetOpacity(1);
		else 
			m_wActiveIcon.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMuteIcon(bool state)
	{
		if (state)
			m_wMuteIcon.SetOpacity(1);
		else 
			m_wMuteIcon.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTransceiverText(string text)
	{
		m_wTransceiverText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTransceiverOpacity(float alpha)
	{
		m_wTransceiverText.SetOpacity(alpha);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFrequencyText(string text)
	{
		m_wFrequencyText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFrequencyColor(Color color)
	{
		m_wFrequencyText.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetChannelText(string text)
	{
		if (text == string.Empty)
		{
			m_wChannelText.SetOpacity(0);
		}
		else 
		{
			m_wChannelText.SetText(text);
			m_wChannelText.SetOpacity(1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wPowerIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PowerIcon"));
		m_wActiveIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ActiveIcon"));
		m_wMuteIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MuteIcon"));
		m_wTransceiverText = TextWidget.Cast(m_wRoot.FindAnyWidget("TransceiverText"));
		m_wFrequencyText = TextWidget.Cast(m_wRoot.FindAnyWidget("FrequencyText"));
		m_wChannelText = TextWidget.Cast(m_wRoot.FindAnyWidget("ChannelText"));
	}
};
