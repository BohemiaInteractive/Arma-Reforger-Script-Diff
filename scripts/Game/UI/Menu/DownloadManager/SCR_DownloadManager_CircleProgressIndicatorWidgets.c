class SCR_DownloadManager_CircleProgressIndicatorWidgets
{
	static const ResourceName s_sLayout = "{1F7E87FEFF13C817}UI/layouts/Menus/MainMenu/CoreMenuHeader_CircleProgressButton.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ImageWidget m_ProgressCircle;

	ImageWidget m_DownloadDoneImage;

	TextWidget m_QueueSizeText;

	ButtonWidget m_DownloadButton;
	SCR_ModularButtonComponent m_DownloadButtonComponent;

	bool Init(Widget root)
	{
		m_ProgressCircle = ImageWidget.Cast(root.FindAnyWidget("m_ProgressCircle"));

		m_DownloadDoneImage = ImageWidget.Cast(root.FindAnyWidget("m_DownloadDoneImage"));

		m_QueueSizeText = TextWidget.Cast(root.FindAnyWidget("CountText"));

		m_DownloadButton = ButtonWidget.Cast(root.FindAnyWidget("m_DownloadButton"));
		m_DownloadButtonComponent = SCR_ModularButtonComponent.Cast(m_DownloadButton.FindHandler(SCR_ModularButtonComponent));

		return true;
	}
}
