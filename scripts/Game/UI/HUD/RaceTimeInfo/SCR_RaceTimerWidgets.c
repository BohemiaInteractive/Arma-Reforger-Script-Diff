// Layout file: UI/layouts/RaceTimer.layout
class SCR_RaceTimerWidgets
{
	protected static const ResourceName LAYOUT = "{48DC3EE5C0C184E8}UI/layouts/RaceTimer.layout";

	SizeLayoutWidget m_wSizeLayoutContent;
	TextWidget m_wTimer;
	TextWidget m_wCheckpoints;
	TextWidget m_wTitleTime;
	TextWidget m_wTitleCheckpoints;

	//------------------------------------------------------------------------------------------------
	bool Init(notnull Widget root)
	{
		m_wSizeLayoutContent = SizeLayoutWidget.Cast(root.FindAnyWidget("m_wSizeLayoutContent"));
		m_wTimer = TextWidget.Cast(root.FindAnyWidget("m_wTimer"));
		m_wCheckpoints = TextWidget.Cast(root.FindAnyWidget("m_wCheckpoints"));
		m_wTitleTime = TextWidget.Cast(root.FindAnyWidget("m_wTitleTime"));
		m_wTitleCheckpoints = TextWidget.Cast(root.FindAnyWidget("m_wTitleCheckpoints"));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayout()
	{
		return LAYOUT;
	}
}
