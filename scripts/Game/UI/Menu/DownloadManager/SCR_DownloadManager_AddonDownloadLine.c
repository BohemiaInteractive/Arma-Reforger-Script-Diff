/*!
Component for a line which indicates addon download state in the download manager.
*/

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Common problem in UI scripts: they do not inherit from the same parent. As a result there are a myriad duplicate methods and lines of code (eg. m_wRoot)

class SCR_DownloadManager_AddonDownloadLine : ScriptedWidgetComponent
{
	// Icon setting
	protected const string ICON_UP = 	"up";
	protected const string ICON_DOWN = 	"down";

	protected const Color ICON_COLOR_DOWNLOAD = UIColors.CONTRAST_COLOR;
	protected const Color ICON_COLOR_UP = UIColors.CONFIRM;
	protected const Color ICON_COLOR_DOWN = UIColors.WARNING;

	protected const Color TEXT_SIZE_COLOR_DOWNLOAD = UIColors.CONTRAST_COLOR;
	protected const Color TEXT_SIZE_COLOR_DOWNLOADED = UIColors.NEUTRAL_INFORMATION;
	protected const Color TEXT_SIZE_COLOR_ERROR = UIColors.WARNING;

	protected ref SCR_DownloadManager_AddonDownloadLineBaseWidgets m_Widgets = new SCR_DownloadManager_AddonDownloadLineBaseWidgets();
	protected ref SCR_WorkshopItemActionDownload m_Action;
	protected ref SCR_WorkshopItem m_Item;

	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourcePickerThumbnail, "Imageset resource for icons", "imageset")]
	protected ResourceName m_IconImageSet;

	bool m_bHideButtons;
	protected bool m_bVersionChange = false;

	protected Widget m_wRoot;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_Widgets.Init(w);

		m_Widgets.m_PauseResumeButtonComponent.m_OnClicked.Insert(OnPauseButton);
		m_Widgets.m_CancelButtonComponent.m_OnClicked.Insert(OnCancelButton);
	}

	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(Update);
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Callqueue to perform continuous updates: this means a looping callqueue running for each one of these lines, and desyncronized updates (if other independent UIs display similar download progress visuals, they most likely rely on their own callqueues, so the progress bar/percentages will update at different times). This is a common issue in Workshop UI classes: they handle the same data but each one in their own sligtly different ways. Code was copy pasted all over the place, making the codebase extremely bloated and hard to mantain
	
	//------------------------------------------------------------------------------------------------
	//! Initializes the line in interactive mode.
	//! It will be able to interact with the download action.
	void InitForDownloadAction(SCR_WorkshopItemActionDownload action)
	{
		m_Action = action;
		Update();
		GetGame().GetCallqueue().CallLater(Update, SCR_WorkshopUiCommon.CONTINUOUS_UPDATE_DELAY, true);
	}


	//------------------------------------------------------------------------------------------------
	//! Initializes the line in non-interactive mode, shows basic data about a download which is already running
	void InitForCancelDownloadAction(SCR_WorkshopItemActionDownload action)
	{
		m_Action = action;
		m_bHideButtons = true;

		Update();
		GetGame().GetCallqueue().CallLater(Update, SCR_WorkshopUiCommon.CONTINUOUS_UPDATE_DELAY, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes the line in non-interactive mode, shows basic data about a download relating to joining servers
	void InitForServerDownloadAction(SCR_WorkshopItemActionDownload action)
	{
		m_Action = action;
		m_bHideButtons = true;

		if (m_Widgets && m_Widgets.m_AddonSizeIcon)
			m_Widgets.m_AddonSizeIcon.SetVisible(false);
		
		Update();
		GetGame().GetCallqueue().CallLater(Update, SCR_WorkshopUiCommon.CONTINUOUS_UPDATE_DELAY, true);
	}
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! Initializes the line in non-interactive mode, shows basic data about a download
	void InitForServerBrowser(SCR_WorkshopItem item, Revision overrideTargetVersion = null, bool showVersionAndSize = true)
	{
		InitForWorkshopItem(item, overrideTargetVersion, showVersionAndSize);

		Revision versionFrom = item.GetCurrentLocalRevision();
		Revision versionTo = overrideTargetVersion;
		if (!versionTo)
			versionTo = item.GetLatestRevision();

		if (!item.GetOffline()) //Missing
		{
			m_Widgets.m_AddonSizeText.SetColor(TEXT_SIZE_COLOR_DOWNLOAD);
			m_Widgets.m_AddonSizeIcon.SetColor(ICON_COLOR_DOWNLOAD);
			m_Widgets.m_AddonSizeIcon.LoadImageFromSet(0, m_IconImageSet, SCR_WorkshopUiCommon.ICON_DOWNLOAD);
		}
		else //Downloaded
		{
			if (versionFrom && !Revision.AreEqual(versionFrom, versionTo))
			{
				// Need version change
				m_Widgets.m_AddonSizeText.SetColor(TEXT_SIZE_COLOR_DOWNLOAD);
				m_Widgets.m_AddonSizeIcon.SetColor(ICON_COLOR_DOWNLOAD);
				m_Widgets.m_AddonSizeIcon.LoadImageFromSet(0, m_IconImageSet, SCR_WorkshopUiCommon.ICON_UPDATE);
			}
			else
			{
				// Version match
				m_Widgets.m_AddonSizeText.SetColor(TEXT_SIZE_COLOR_DOWNLOADED);
				m_Widgets.m_AddonSizeIcon.SetColor(ICON_COLOR_UP);
				m_Widgets.m_AddonSizeIcon.LoadImageFromSet(0, m_IconImageSet, UIConstants.ICON_CHECK);
				
				// Display whole size
				m_Widgets.m_AddonSizeText.SetText(SCR_ByteFormat.GetReadableSize(item.GetSizeBytes()));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes the line in passive mode. It will just show basic information once
	void InitForWorkshopItem(SCR_WorkshopItem item, Revision versionTo = null, bool showVersionAndSize = true)
	{
		m_Item = item;

		string addonName = item.GetName();
		m_Widgets.m_AddonNameText.SetText(addonName);

		if (!item.GetRestricted())
		{
			m_Widgets.m_RightWidgetGroup.SetVisible(showVersionAndSize);

			if (showVersionAndSize)
			{
				Revision versionFrom = item.GetCurrentLocalRevision();
				//Revision versionTo = overrideTargetVersion;
				if (versionTo == null)
					versionTo = item.GetLatestRevision();


				float downloadSize = item.GetTargetRevisionPatchSize();

				bool showVersionFrom = versionFrom && !Revision.AreEqual(versionFrom, versionTo);
				m_Widgets.m_VersionFromText.SetVisible(showVersionFrom);
				m_Widgets.m_VersionArrow.SetVisible(showVersionFrom);
				if (showVersionFrom)
					m_Widgets.m_VersionFromText.SetText(versionFrom.GetVersion());

				if (versionTo)
					m_Widgets.m_VersionToText.SetText(versionTo.GetVersion());

				string sizeStr = SCR_ByteFormat.GetReadableSize(downloadSize);
				m_Widgets.m_AddonSizeText.SetText(sizeStr);

				// Display what action will be done icon
				DisplayActionIcon(versionFrom, versionTo);

				// Check version change
				if (versionFrom && !Revision.AreEqual(versionFrom, versionTo))
					m_bVersionChange = true;
			}
		}
		else
		{
			// The addon is somehow restricted
			m_Widgets.m_RightWidgetGroup.SetVisible(false);

			string errorText = SCR_WorkshopUiCommon.GetRestrictedAddonStateText(item);
			m_Widgets.m_ErrorText.SetVisible(true);
			m_Widgets.m_ErrorText.SetText(errorText);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Display error massage, based on positivity message is green or red
	void DisplayError(string msg, bool positive = false)
	{
		m_Widgets.m_ErrorText.SetText(msg);
		m_Widgets.m_ErrorText.SetColor(ICON_COLOR_DOWN);
		m_Widgets.m_ErrorText.SetVisible(true);

		// Green for positive
		if (positive)
			m_Widgets.m_ErrorText.SetColor(ICON_COLOR_UP);
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionDownload GetDownloadAction()
	{
		return m_Action;
	}

	//------------------------------------------------------------------------------------------------
	protected void Update()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates all widgets. Only relevant in the mode InitForDownloadAction
	protected void UpdateAllWidgets()
	{
		if (!m_Action)
			return;

		// Name
		m_Widgets.m_AddonNameText.SetText(m_Action.GetAddonName());

		// Version from
		Revision versionFrom = m_Action.GetStartRevision();
		
		m_Widgets.m_VersionArrow.SetVisible(versionFrom != null);
		m_Widgets.m_VersionFromText.SetVisible(versionFrom != null);
		
		if (versionFrom)
			m_Widgets.m_VersionFromText.SetText(versionFrom.GetVersion());

		// Version to
		Revision versionTo = m_Action.GetTargetRevision();
		m_Widgets.m_VersionToText.SetVisible(versionTo != null);
		if (versionTo)
			m_Widgets.m_VersionToText.SetText(versionTo.GetVersion());

		// Progress text
		if (m_Action.IsCompleted() || m_Action.IsFailed() || m_Action.IsCanceled())
			m_Widgets.m_ProgressText.SetText(string.Empty);
		else
			m_Widgets.m_ProgressText.SetText(UIConstants.FormatUnitPercentage(SCR_WorkshopUiCommon.GetDownloadProgressPercentage(m_Action.GetProgress())));

		// Progress bar
		if (m_Action.IsCompleted())
			m_Widgets.m_ProgressBar.SetCurrent(1);
		else if (m_Action.IsFailed() || m_Action.IsCanceled())
			m_Widgets.m_ProgressBar.SetCurrent(0);
		else
			m_Widgets.m_ProgressBar.SetCurrent(m_Action.GetProgress());

		// Download size
		float downloadSize = m_Action.GetSizeBytes();
		string sizeStr = SCR_ByteFormat.GetReadableSize(downloadSize);
		m_Widgets.m_AddonSizeText.SetText(sizeStr);

		if (m_Action.IsCompleted())
			m_Widgets.m_AddonSizeText.SetColor(TEXT_SIZE_COLOR_DOWNLOADED);
		else if (m_Action.IsFailed() || m_Action.IsCanceled())
			m_Widgets.m_AddonSizeText.SetColor(TEXT_SIZE_COLOR_ERROR);
		else
			m_Widgets.m_AddonSizeText.SetColor(TEXT_SIZE_COLOR_DOWNLOAD);

		// Addon Size Icon
		if (m_Action.IsCompleted())
			m_Widgets.m_AddonSizeIcon.SetColor(ICON_COLOR_UP);
		else if (m_Action.IsFailed() || m_Action.IsCanceled())
			m_Widgets.m_AddonSizeIcon.SetColor(ICON_COLOR_DOWN);
		else
			m_Widgets.m_AddonSizeIcon.SetColor(ICON_COLOR_DOWNLOAD);

		// Buttons
		if (m_bHideButtons)
		{
			m_Widgets.m_CancelButton.SetVisible(false);
			m_Widgets.m_PauseResumeButton.SetVisible(false);
		}
		else
		{
			m_Widgets.m_PauseResumeButton.SetVisible(false);
			m_Widgets.m_CancelButton.SetVisible(false);
			if (!m_Action.IsCompleted() && !m_Action.IsFailed() && !m_Action.IsCanceled())
			{
				m_Widgets.m_PauseResumeButton.SetVisible(true);
				m_Widgets.m_CancelButton.SetVisible(true);

				string pauseButtonMode = "running";
				if (m_Action.IsPaused() || m_Action.IsInactive())
					pauseButtonMode = "paused";

				m_Widgets.m_PauseResumeButtonComponent.SetEffectsWithAnyTagEnabled({"all", pauseButtonMode});
			}
		}

		// Icons when the download is over
		if (m_Action.IsCompleted())
		{
			m_Widgets.m_DownloadFailedImage.SetVisible(false);
			m_Widgets.m_DownloadFinishedImage.SetVisible(true);
		}
		else if (m_Action.IsFailed() || m_Action.IsCanceled())
		{
			m_Widgets.m_DownloadFailedImage.SetVisible(true);
			m_Widgets.m_DownloadFinishedImage.SetVisible(false);
		}
		else
		{
			m_Widgets.m_DownloadFailedImage.SetVisible(false);
			m_Widgets.m_DownloadFinishedImage.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPauseButton()
	{
		if (!m_Action)
			return;

		if (m_Action.IsPaused())
			m_Action.Resume();
		else if (m_Action.IsInactive())
			m_Action.Activate();
		else
			m_Action.Pause();
	}



	//------------------------------------------------------------------------------------------------
	protected void OnCancelButton()
	{
		if (!m_Action)
			return;

		m_Action.Cancel();
	}

	//------------------------------------------------------------------------------------------------
	//! Display icon with action that need to done - download, update, downgrade
	protected void DisplayActionIcon(Revision vFrom, Revision vTo)
	{
		ImageWidget wIcon = m_Widgets.m_AddonActionIcon;
		if (!wIcon)
			return;

		// Defaul to download
		string imageName = SCR_WorkshopUiCommon.ICON_DOWNLOAD;
		Color color = ICON_COLOR_DOWNLOAD;

		// Is there current verion?
		if (vFrom)
		{
			int result = vFrom.CompareTo(vTo);

			if (result < 0)
			{
				imageName = ICON_UP;
				color = ICON_COLOR_UP;
			}
			else if (result > 0)
			{
				imageName = ICON_DOWN;
				color = ICON_COLOR_DOWN;
			}
		}

		// Setup action icon widget
		wIcon.LoadImageFromSet(0, m_IconImageSet, imageName);
		wIcon.SetColor(color);
	}
};
