/*
Panel which shows state of addon.

!!! This component relies on SCR_WorkshopUiCommon to be initialized to work correctly.
*/

class SCR_AddonDetailsPanelComponent : SCR_ContentDetailsPanelBase
{
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// This sort of visual stuff should be configurable in attributes
		
	protected const int MAX_ADDON_TYPE_IMAGES = 12; // Max amount of addon type images will be shown.
	
//---- REFACTOR NOTE END ----
	
	protected ref SCR_AddonInfoWidgets m_InfoWidgets = new SCR_AddonInfoWidgets();
	protected ref SCR_WorkshopItem m_Item;
	protected SCR_WorkshopItemBackendImageComponent m_BackendImageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_InfoWidgets.Init(m_CommonWidgets.m_wAdditionalInfo.GetChildren());
		
		m_BackendImageComponent = SCR_WorkshopItemBackendImageComponent.Cast(m_CommonWidgets.m_wBackendImage.FindHandler(SCR_WorkshopItemBackendImageComponent));
		
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates properties which never change at run time (addon description, size, etc)
	//! This is called only when new addon is assigned or at start
	protected void UpdateAllWidgets()
	{
		// Don't mess up widgets in workbench
		if (SCR_Global.IsEditMode())
			return;
		
		if (!m_Item)
		{
			ClearTypeDisplays();
			m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(false);
			m_CommonWidgets.m_wMainArea.SetVisible(false);
			if (m_BackendImageComponent)
				m_BackendImageComponent.SetWorkshopItemAndImage(null, null);
			return;
		}
		
		m_CommonWidgets.m_wMainArea.SetVisible(true);

		UpdateInfo();
		UpdateTypes();
		UpdateImage();
		UpdateMANWVisuals();
		UpdateDependencies();
		UpdateErrorMessage();
	}	

	//------------------------------------------------------------------------------------------------
	protected void UpdateImage()
	{
		if (m_BackendImageComponent)
			m_BackendImageComponent.SetWorkshopItemAndImage(m_Item, m_Item.GetThumbnail());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMANWVisuals()
	{
		if (!m_Item.GetWorkshopItem())
		{
			m_CommonWidgets.m_wMANWLogo.SetVisible(false);
			return;
		}
		
		m_CommonWidgets.m_MANWComponent.UpdateAwards(m_Item);
		
		array<WorkshopTag> items = {};
		m_Item.GetWorkshopItem().GetTags(items);
		foreach(WorkshopTag item: items)
		{
			if (item.Name() == "MANW_2025")
			{
				m_CommonWidgets.m_wMANWLogo.SetVisible(true);
				return;
			}
		}
		
		m_CommonWidgets.m_wMANWLogo.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateInfo()
	{
		if (!m_Item)
			return;
		
		// Name, description, author name
		m_CommonWidgets.m_wNameText.SetText(m_Item.GetName());
		m_CommonWidgets.m_wAuthorNameText.SetText(m_Item.GetAuthorName());
		m_CommonWidgets.m_wDescriptionText.SetText(m_Item.GetSummary());

		if (!m_bDisplayAdditionalInfo)
			return;
		
		// Rating
		m_InfoWidgets.m_wModRating.SetText(UIConstants.FormatUnitPercentage(SCR_WorkshopUiCommon.GetRatingPercentage(m_Item.GetAverageRating())));
		
		// Size
		float sizef = m_Item.GetSizeBytes();
		string sizeStr = SCR_ByteFormat.GetReadableSize(sizef);
		m_InfoWidgets.m_wModSize.SetText(sizeStr);

		// Version
		bool showUpdateText;
		string currentVersionIcon;
		Color currentVersionIconColor;
		string currentVersionText;
		Color currentVersionTextColor;
		string updateVersionText;
		
		SCR_WorkshopUiCommon.GetVersionDisplayLook(
			m_Item, 
			showUpdateText, 
			currentVersionIcon, 
			currentVersionIconColor, 
			currentVersionText, 
			currentVersionTextColor, 
			updateVersionText);
		
		m_InfoWidgets.m_wVersionUpdateHorizontalLayout.SetVisible(showUpdateText);
		m_InfoWidgets.m_wCurrentVersion.SetColor(currentVersionTextColor);
		m_InfoWidgets.m_wCurrentVersion.SetText(currentVersionText);
		m_InfoWidgets.m_wUpdateVersion.SetText(updateVersionText);
		m_InfoWidgets.m_wCurrentVersionIcon.SetColor(currentVersionIconColor);
		m_InfoWidgets.m_wCurrentVersionIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, currentVersionIcon);
		
		// Time since last downloaded
		int timeSinceDownload = m_Item.GetTimeSinceFirstDownload();
		m_InfoWidgets.m_wLastDownloadedHorizontalLayout.SetVisible(timeSinceDownload >= 0);
		
		if (timeSinceDownload >= 0)
			m_InfoWidgets.m_wLastDownloaded.SetText(SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceDownload));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTypes()
	{
		ClearTypeDisplays();
		
		if (!m_Item)
			return;
		
		WorkshopItem item = m_Item.GetWorkshopItem();
		array<WorkshopTag> tagObjects = {};	// Get tag objects from workshop item
		if (item)
			item.GetTags(tagObjects);

		foreach (WorkshopTag tag : tagObjects)
		{
			string image = SCR_WorkshopUiCommon.GetTagImage(tag.Name());
			if (!image.IsEmpty())
				AddTypeDisplay(image, UIConstants.ICONS_IMAGE_SET, UIConstants.ICONS_GLOW_IMAGE_SET);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDependencies()
	{
		if (!m_Item)
			return;
		
		array<ref SCR_WorkshopItem> dependentAddons = SCR_WorkshopUiCommon.GetDownloadedDependentAddons(m_Item);
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		
		m_InfoWidgets.m_wDependenciesHorizontalLayout.SetVisible(!dependencies.IsEmpty());
		m_InfoWidgets.m_wDependentHorizontalLayout.SetVisible(!dependentAddons.IsEmpty());
		
		if (!dependencies.IsEmpty())
		{
			if (dependencies.Count() == 1)
				m_InfoWidgets.m_wDependencies.SetText(SCR_WorkshopUiCommon.LABEL_DEPENDENCIES_NUMBER_ONE);
			else
				m_InfoWidgets.m_wDependencies.SetText(WidgetManager.Translate(SCR_WorkshopUiCommon.LABEL_DEPENDENCIES_NUMBER, dependencies.Count()));
		}

		if (!dependentAddons.IsEmpty())
		{
			if (dependentAddons.Count() == 1)
				m_InfoWidgets.m_wDependent.SetText(SCR_WorkshopUiCommon.LABEL_DEPENDENT_NUMBER_ONE);
			else
				m_InfoWidgets.m_wDependent.SetText(WidgetManager.Translate(SCR_WorkshopUiCommon.LABEL_DEPENDENT_NUMBER, dependentAddons.Count()));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateErrorMessage()
	{
		if (!m_Item)
			return;
		
		WorkshopItem item = m_Item.GetWorkshopItem();
		SCR_ERevisionAvailability revAvailability;
		
		if (item)
			revAvailability = SCR_AddonManager.ItemAvailability(item);
		
		bool localModError = revAvailability != SCR_ERevisionAvailability.ERA_AVAILABLE && revAvailability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		float saturation = UIConstants.ENABLED_WIDGET_SATURATION;
		bool restricted = m_Item.GetRestricted();
		
		m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(restricted || localModError);
		m_CommonWidgets.m_WarningOverlayComponent.SetBlurUnderneath(restricted);
		
		if (restricted)
			m_CommonWidgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.MESSAGE_RESTRICTED_GENERIC, SCR_WorkshopUiCommon.ICON_REPORTED);
		else
			m_CommonWidgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessageVerbose(item), SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(item));
		
		if (localModError)
			saturation = UIConstants.DISABLED_WIDGET_SATURATION;
		
		if (m_BackendImageComponent)
			m_BackendImageComponent.SetImageSaturation(saturation);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (!m_Item)
			return;
		
		m_Item.m_OnChanged.Remove(Callback_OnChanged);
		m_Item.m_OnGetAsset.Remove(OnGetAsset);
		m_Item.m_OnDependenciesLoaded.Remove(UpdateDependencies);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnChanged()
	{
		UpdateAllWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGetAsset(SCR_WorkshopItem item)
	{
		UpdateInfo();
		UpdateImage();
		UpdateErrorMessage();
	}
	
	// --- Public ----
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		// Clear old item
		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(Callback_OnChanged);
			m_Item.m_OnGetAsset.Remove(OnGetAsset);
			m_Item.m_OnDependenciesLoaded.Remove(UpdateDependencies);
		}

		m_Item = item;

		if (m_Item)
		{
			m_Item.m_OnChanged.Insert(Callback_OnChanged);
			m_Item.m_OnGetAsset.Insert(OnGetAsset);
			m_Item.m_OnDependenciesLoaded.Insert(UpdateDependencies);
		}

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}
}
