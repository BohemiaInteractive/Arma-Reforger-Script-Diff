//------------------------------------------------------------------------------------------------
class SCR_VoiceoverSubtitles : SCR_InfoDisplayExtended
{
	protected RichTextWidget m_wSubtitle;

	protected static float s_fLingerDuration = 0.5;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wSubtitle = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Subtitle"));
	}

	//------------------------------------------------------------------------------------------------
	//! How long should subtitles stay visible after the sound ended (seconds)
	static void SetLingerDuration(float duration)
	{
		if (duration < 0)
			duration = 0;

		s_fLingerDuration = duration;
	}

	//------------------------------------------------------------------------------------------------
	static float GetLingerDuration()
	{
		return s_fLingerDuration;
	}

	//------------------------------------------------------------------------------------------------
	void ShowSubtitle(string text)
	{
		if (!m_wSubtitle)
			return;

		m_wSubtitle.SetTextFormat(text);
		Show(true);
	}
}
