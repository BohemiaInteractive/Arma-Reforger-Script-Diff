/*!
Details with mod list to display all server mods.
There should be displayed missing server.
Extra information about server can be added.
*/

class SCR_ServerDetailsDialog : SCR_AddonListDialog
{
	protected const string WIDGET_SCROLL = "ScrollSize";
	protected const string WIDGET_ADDON_LIST = "AddonList";

	protected const string WIDGET_IPADDRESS_TEXT = "IPAddressText";
	protected const string WIDGET_DISCORD_TEXT = "DiscordText";
	protected const string WIDGET_MODS_NUMBER_TEXT = "ModsNumberText";
	protected const string WIDGET_MODS_SIZE_TEXT = "ModsSizeAmount";
	protected const string WIDGET_ALERT_TEXT = "AlertText";
	protected const string WIDGET_ALERT_WRAPPER = "AlertOverlay"; //TODO: move alerts handling to a specific component on the button itself
	protected const string WIDGET_MODS_SIZE_TO_DOWNLOAD_TEXT = "ModsSizeToDownload";

	protected const string WIDGET_MODS_SIZE_LAYOUT = "ModsSizeLayout";
	protected const string WIDGET_DETAIL_ICONS_LAYOUT = "DetailIconsLayout";
	protected const string WIDGET_MODS_SIZE_TO_DOWNLOAD_LAYOUT = "ModsSizeToDownloadLayout";

	protected const string IMG_WRONG_VERSION = "DetailIcon_WrongVersion";
	protected const string IMG_PASSWORD_PROTECTED = "Detailicon_PasswordProtected";
	protected const string IMG_CROSS_PLATFORM = "DetailIcon_CrossPlatform";
	protected const string IMG_MODDED = "DetailIcon_Modded";
	protected const string IMG_VERSION_ALERT = "VersionAlertIconButton";

	protected const string BTN_CONFIRM = "Confirm";
	protected const string BTN_FAVORITES = "favorites";
	protected const string BTN_FAVORITES_STAR = "FavoriteButton";
	protected const string BTN_COPY_IPADDRESS = "IPAddressButton";
	protected const string BTN_DISCORD = "DiscordButton";

	protected Widget m_wScroll;
	protected Widget m_wAddonList;
	protected Widget m_wModsSizeLayout;
	protected Widget m_wDetailIconsLayout;
	protected Widget m_wVersionAlertIcon;
	protected Widget m_wModsSizeToDownloadLayout;
	protected Widget m_wAlertWrapper;

	protected RichTextWidget m_wIPAddressText;
	protected RichTextWidget m_wDiscordText;
	protected RichTextWidget m_wModsNumberText;
	protected RichTextWidget m_wModsSizeText;
	protected RichTextWidget m_wAlertText;
	protected RichTextWidget m_wModsSizeToDownloadText;

	protected SCR_InputButtonComponent m_NavConfirm;
	protected SCR_InputButtonComponent m_NavFavorites;

	protected SCR_ModularButtonComponent m_BtnFavorites;
	protected SCR_ButtonComponent m_BtnCopyIPAddress;
	protected SCR_ButtonComponent m_BtnDiscord;

	protected SCR_ScenarioBackendImageComponent m_BackendImageComp;
	protected Widget m_wBackgroundImageBackend;

	ref ScriptInvoker m_OnFavorites = new ScriptInvoker();

	static Room s_Room;

	protected float m_fVersionAlertIconPaddingLeft;

	protected const ResourceName ADDON_LINE_LAYOUT_SERVER_BROWSER = "{3BC78F295971FD3D}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineConfirmation_ServerBrowser.layout";
	protected const string STR_VERSION_MISMATCH = "#AR-ServerBrowser_JoinModVersionMissmatch";
	
	protected const string STR_HIGH_PING = "#AR-ServerBrowser_HighPingWarning_Title";
	protected const string STR_HIGH_PING_ICON = "ping-low";

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		super.InitWidgets();

		m_wScroll = m_wRoot.FindAnyWidget(WIDGET_SCROLL);
		m_wAddonList = m_wRoot.FindAnyWidget(WIDGET_ADDON_LIST);
		m_wModsSizeLayout = m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_LAYOUT);
		m_wDetailIconsLayout = m_wRoot.FindAnyWidget(WIDGET_DETAIL_ICONS_LAYOUT);
		m_wModsSizeToDownloadLayout = m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_TO_DOWNLOAD_LAYOUT);
		m_wAlertWrapper = m_wRoot.FindAnyWidget(WIDGET_ALERT_WRAPPER);
		
		m_wIPAddressText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IPADDRESS_TEXT));
		m_wDiscordText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_DISCORD_TEXT));
		m_wModsNumberText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_MODS_NUMBER_TEXT));
		m_wModsSizeText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_TEXT));
		m_wAlertText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_ALERT_TEXT));
		m_wModsSizeToDownloadText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_TO_DOWNLOAD_TEXT));
	}

	//------------------------------------------------------------------------------------------------
	override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		super.Init(root, preset, proxyMenu);

		m_NavConfirm = SCR_InputButtonComponent.Cast(root.FindAnyWidget(BTN_CONFIRM).FindHandler(SCR_InputButtonComponent));
		if (m_NavConfirm)
			BindButtonConfirm(m_NavConfirm);

		m_NavFavorites = FindButton(BTN_FAVORITES);

		m_BtnFavorites = SCR_ModularButtonComponent.FindComponent(root.FindAnyWidget(BTN_FAVORITES_STAR));

		if (m_NavFavorites)
			m_NavFavorites.m_OnActivated.Insert(OnFavorites);

		if (m_BtnFavorites)
			m_BtnFavorites.m_OnClicked.Insert(OnFavorites);

		m_BtnCopyIPAddress = SCR_ButtonComponent.Cast(root.FindAnyWidget(BTN_COPY_IPADDRESS).FindHandler(SCR_ButtonComponent));

		if (m_BtnCopyIPAddress)
			m_BtnCopyIPAddress.m_OnClicked.Insert(OnCopyIPAddress);

		m_BtnDiscord = SCR_ButtonComponent.Cast(root.FindAnyWidget(BTN_DISCORD).FindHandler(SCR_ButtonComponent));

		if (m_BtnDiscord)
			m_BtnDiscord.m_OnClicked.Insert(OnDiscord);

		m_wBackgroundImageBackend = GetRootWidget().FindAnyWidget("BackgroundImageBackend");
		if (m_wBackgroundImageBackend)
			m_BackendImageComp = SCR_ScenarioBackendImageComponent.Cast(m_wBackgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Set visibility
		m_wScroll.SetVisible(false);

		m_wModsSizeLayout.SetVisible(false);
		m_wModsSizeToDownloadLayout.SetVisible(false);

		//! Background Image
		if (!s_Room)
			return;

		SetScenarioImage(s_Room.HostScenario());
	}

	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static SCR_ServerDetailsDialog CreateServerDetailsDialog(Room room, array<ref SCR_WorkshopItem> items, string preset, ResourceName dialogsConfig = "", ScriptInvokerVoid onFavoritesResponse = null)
	{
		if (dialogsConfig == "")
			dialogsConfig = SCR_WorkshopDialogs.DIALOGS_CONFIG;

		s_Room = room;

		SCR_ServerDetailsDialog dialog = new SCR_ServerDetailsDialog(items, "");
		SCR_ConfigurableDialogUi.CreateFromPreset(dialogsConfig, preset, dialog);

		dialog.SetTitle(room.Name());
		dialog.SetIPAddressText(SCR_WorkshopUiCommon.LABEL_IP_ADDRESS + " " + room.HostAddress());
		//dialog.SetDiscordText();

		//! Favorites button
		if (onFavoritesResponse)
			onFavoritesResponse.Insert(dialog.OnRoomSetFavoriteResponseDialog);

		dialog.DisplayFavoriteAction(s_Room.IsFavorite());
		dialog.UpdateDetailIcons();
		dialog.UpdateAlertMessage();

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	void FillModList(array<ref SCR_WorkshopItem> items, SCR_RoomModsManager modsManager)
	{
		m_aItems = items;

		if (m_aItems.IsEmpty())
			return;

		//------------------------------------------------------------------------------------------------
		// The items must be sorted by size and placed in the following order: Missing, needs download -> Downloaded, needs update -> Downloaded
		array<SCR_WorkshopItem> itemsSorted = {};

		array<SCR_WorkshopItem> itemsToUpdate = {};
		array<SCR_WorkshopItem> itemsToDownload = {};
		array<SCR_WorkshopItem> itemsDownloaded = {};
		array<SCR_WorkshopItem> itemsOrdered = {};
		
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			itemsSorted.Insert(item);
		}
		
		SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemTargetSize>.HeapSort(itemsSorted, true);
		
		// Split the items based on their state
		Revision versionFrom;
		Revision versionTo;
		foreach (SCR_WorkshopItem item : itemsSorted)
		{
			versionFrom = item.GetCurrentLocalRevision();
			versionTo = item.GetItemTargetRevision();

			if (!item.GetOffline()) //Missing, needs download
				itemsToDownload.Insert(item);

			else if (versionFrom && !Revision.AreEqual(versionFrom, versionTo)) //Downloaded, needs update
				itemsToUpdate.Insert(item);

			else //Downloaded
				itemsDownloaded.Insert(item);
		}

		// Create the ordered array
		itemsOrdered.InsertAll(itemsToDownload);
		itemsOrdered.InsertAll(itemsToUpdate);
		itemsOrdered.InsertAll(itemsDownloaded);
		//------------------------------------------------------------------------------------------------

		// Setup downloaded
		foreach (SCR_WorkshopItem item : itemsOrdered)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT_SERVER_BROWSER, m_wAddonList);

			if (item == itemsOrdered[itemsOrdered.Count() - 1])
				AlignableSlot.SetPadding(w, 0, 0, 0, 0);

			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForServerBrowser(item, item.GetItemTargetRevision(), true);

			m_aDownloadLines.Insert(comp);
		}

		m_wScroll.SetVisible(true);
		UpdateModsAmountMessages(modsManager);
	}

	//------------------------------------------------------------------------------------------------
	void SetScenarioImage(MissionWorkshopItem scenario)
	{
		if (!m_BackendImageComp)
			return;

		if (scenario)
			m_BackendImageComp.SetImage(scenario.Thumbnail());
		else
			m_BackendImageComp.SetImage(null);
	}

	//------------------------------------------------------------------------------------------------
	void SetCanJoin(bool canJoin)
	{
		if (!m_NavConfirm)
			return;

		m_NavConfirm.SetEnabled(canJoin, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavorites()
	{
		m_OnFavorites.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayFavoriteAction(bool isFavorite)
	{
		if (m_NavFavorites)
			m_NavFavorites.SetLabel(UIConstants.GetFavoriteLabel(isFavorite));

		// Star Button
		if (m_BtnFavorites)
			m_BtnFavorites.SetToggled(isFavorite, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRoomSetFavoriteResponseDialog()
	{
		DisplayFavoriteAction(s_Room.IsFavorite());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCopyIPAddress()
	{
		System.ExportToClipboard(s_Room.HostAddress());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDiscord()
	{
		//TODO once we introduce the ability for the users to give a discord link on server creation
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIPAddressText(string text)
	{
		if (m_wIPAddressText)
			m_wIPAddressText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetDiscordText(string text)
	{
		if (m_wDiscordText)
			m_wDiscordText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateModsAmountMessages(SCR_RoomModsManager modsManager = null)
	{
		if (m_aItems.IsEmpty())
			return;

		m_wModsSizeLayout.SetVisible(true);

		float totalSize;
		foreach (ref SCR_WorkshopItem item : m_aItems)
		{
			totalSize += item.GetSizeBytes();
		}

		if (m_wModsNumberText)
			m_wModsNumberText.SetTextFormat(m_aItems.Count().ToString());

		if (m_wModsSizeText)
			m_wModsSizeText.SetText(SCR_ByteFormat.GetReadableSize(totalSize));

		// Check mods to update size
		if (!modsManager)
			return;

		array<ref SCR_WorkshopItem> toUpdateMods = modsManager.GetRoomItemsToUpdate();
		
		// Display size to update
		if (m_wModsSizeToDownloadLayout)
			m_wModsSizeToDownloadLayout.SetVisible(!toUpdateMods.IsEmpty());

		if (m_wModsSizeToDownloadText)
		{
			string toUpdateSize = modsManager.GetModListPatchSizeString(toUpdateMods);
			m_wModsSizeToDownloadText.SetText(toUpdateSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDetailIcons()
	{
		if (!s_Room || !m_wDetailIconsLayout)
			return;

		bool isModded = s_Room.IsModded();

		// Versions mismatch
		bool wrongVersion = s_Room.GameVersion() != GetGame().GetBuildVersion();
		bool restrictedUGC = isModded && !SCR_AddonManager.GetInstance().GetUgcPrivilege();

		//m_wDetailIconsLayout.FindWidget(IMG_WRONG_VERSION).SetVisible(wrongVersion || restrictedUGC);
		m_wDetailIconsLayout.FindWidget(IMG_WRONG_VERSION).SetVisible(false);

		// Locked with password
		m_wDetailIconsLayout.FindWidget(IMG_PASSWORD_PROTECTED).SetVisible(s_Room.PasswordProtected());

		// Crossplay
		m_wDetailIconsLayout.FindWidget(IMG_CROSS_PLATFORM).SetVisible(s_Room.IsCrossPlatform());

		// Moded
		//m_wDetailIconsLayout.FindWidget(IMG_MODDED).SetVisible(isModded);
		m_wDetailIconsLayout.FindWidget(IMG_MODDED).SetVisible(false);
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdateAlertMessage()
	{
		if (!s_Room || !m_wAlertText || !m_wAlertWrapper || !m_NavConfirm)
			return;

		bool versionMismatch = s_Room.GameVersion() != GetGame().GetBuildVersion();
		bool highPing = ServerBrowserMenuUI.IsServerPingAboveThreshold(s_Room);
		bool showMessage = versionMismatch || highPing;

		m_wAlertWrapper.SetVisible(showMessage);
		m_NavConfirm.ResetTexture();
		
		if (!showMessage)
			return;

		if (versionMismatch)
		{
			m_wAlertText.SetText(s_Room.GameVersion() + " - " + STR_VERSION_MISMATCH);
			m_NavConfirm.SetTexture(UIConstants.ICONS_IMAGE_SET, UIConstants.ICON_WARNING, Color.FromInt(UIColors.WARNING.PackToInt()));
		}
		else
		{
			string icon = string.Format("<image set='%1' name='%2' scale='%3'/>", UIConstants.ICONS_IMAGE_SET, STR_HIGH_PING_ICON, UIConstants.ACTION_DISPLAY_ICON_SCALE_HUGE);
			m_wAlertText.SetText(icon + "  " + STR_HIGH_PING + "  ");
		}
	}
}