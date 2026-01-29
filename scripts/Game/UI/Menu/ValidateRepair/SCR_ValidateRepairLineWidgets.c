// Layout file: UI/layouts/Menus/ContentBrowser/ValidateRepair/ValidateRepairLine.layout
class SCR_ValidateRepairLineWidgets
{
	protected static const ResourceName LAYOUT = "{8E94F18104D88E73}UI/layouts/Menus/ContentBrowser/ValidateRepair/ValidateRepairLine.layout";

	ImageWidget m_wAddonImg;

	ButtonWidget m_wLoadingOverlay;
	SCR_LoadingOverlay m_LoadingOverlayComponent;

	BlurWidget m_wBlur;
	ImageWidget m_wBackgroundImage;

	ImageWidget m_wAnimationImage;
	SCR_SpinningWidgetComponent m_AnimationImageComponent;

	TextWidget m_wText;
	TextWidget m_wNameText;
	HorizontalLayoutWidget m_wHorizontalButtons;

	ButtonWidget m_wRedownloadButton;
	SCR_ModularButtonComponent m_RedownloadButtonComponent0;
	SCR_DynamicIconComponent m_RedownloadButtonComponent1;

	RichTextWidget m_wDownloadStateText;
	RichTextWidget m_wDownloadErrorText;

	SizeLayoutWidget m_wProgress;
	SCR_WLibProgressBarComponent m_ProgressComponent;


	//------------------------------------------------------------------------------------------------
	bool Init(notnull Widget root)
	{
		m_wAddonImg = ImageWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.SizeImg.ScaleImg.Overlay0.m_AddonImg"));

		m_wLoadingOverlay = ButtonWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.SizeImg.ScaleImg.Overlay0.m_LoadingOverlay"));
		m_LoadingOverlayComponent = SCR_LoadingOverlay.Cast(m_wLoadingOverlay.FindHandler(SCR_LoadingOverlay));

		m_wBlur = BlurWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.SizeImg.ScaleImg.Overlay0.m_LoadingOverlay.loadingOverlayRoot1.m_Blur"));
		m_wBackgroundImage = ImageWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.SizeImg.ScaleImg.Overlay0.m_LoadingOverlay.loadingOverlayRoot1.m_BackgroundImage"));

		m_wAnimationImage = ImageWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.SizeImg.ScaleImg.Overlay0.m_LoadingOverlay.loadingOverlayRoot1.m_AnimationImage"));
		m_AnimationImageComponent = SCR_SpinningWidgetComponent.Cast(m_wAnimationImage.FindHandler(SCR_SpinningWidgetComponent));

		m_wText = TextWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.SizeImg.ScaleImg.Overlay0.m_LoadingOverlay.loadingOverlayRoot1.m_Text"));
		m_wNameText = TextWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.VerticalLayout.HorizontalLayout.m_NameText"));
		m_wHorizontalButtons = HorizontalLayoutWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.VerticalLayout.HorizontalLayout.SizeLayout0.m_HorizontalButtons"));

		m_wRedownloadButton = ButtonWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.VerticalLayout.HorizontalLayout.SizeLayout0.m_HorizontalButtons.m_RedownloadButton"));
		m_RedownloadButtonComponent0 = SCR_ModularButtonComponent.Cast(m_wRedownloadButton.FindHandler(SCR_ModularButtonComponent));
		m_RedownloadButtonComponent1 = SCR_DynamicIconComponent.Cast(m_wRedownloadButton.FindHandler(SCR_DynamicIconComponent));

		m_wDownloadStateText = RichTextWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.VerticalLayout.StatusHorizontalLayout.m_DownloadStateText"));
		m_wDownloadErrorText = RichTextWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.VerticalLayout.StatusHorizontalLayout.m_DownloadErrorText"));

		m_wProgress = SizeLayoutWidget.Cast(root.FindWidget("SizeLayout.Overlay.HorizontalLayout.VerticalLayout.m_Progress"));
		m_ProgressComponent = SCR_WLibProgressBarComponent.Cast(m_wProgress.FindHandler(SCR_WLibProgressBarComponent));


		return true;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayout()
	{
		return LAYOUT;
	}
}
