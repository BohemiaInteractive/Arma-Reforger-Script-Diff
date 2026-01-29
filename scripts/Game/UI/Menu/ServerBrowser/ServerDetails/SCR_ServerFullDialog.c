/*
	Dialog to provide feedback for attempting to join a full server. 
	Supports
	- Enqueued
	- Server full, queue full
	- Server full, queue disabled
*/

class SCR_ServerFullDialog : SCR_ConfigurableDialogUi
{
	protected const string WIDGET_BACKGROUND_IMAGE =	"BackgroundImageBackend";
	protected const string WIDGET_SIDE_SPINNER =		"SideSpinner";

	protected const string INPUT_BUTTON_FIELD_MANUAL = 	"FieldManual";
	protected const string INPUT_BUTTON_FAVORITES = 	"Favorites";
	protected const string MOUSE_BUTTON_FAVORITES = 	"FavoriteButton";
	
	protected const int ENQUEUED_TIME_UPDATE_FREQUENCY = 1000;
	
	// Messages
	protected const string MESSAGE_SERVER_FULL = 			"#AR-ServerBrowser_FullServer";
	protected const string MESSAGE_SERVER_POPULATION =		"#AR-Scenario_Players";
	protected const string MESSAGE_QUEUE_FULL =				"#AR-ServerQueue_QueueFull_Header";
	protected const string MESSAGE_QUEUE_DISABLED =			"#AR-ServerQueue_QueueDisabled_Header";
	protected const string MESSAGE_QUEUE_POPULATION =		"#AR-ServerQueue_PlayersInQueue";
	protected const string MESSAGE_QUEUE_MAX_SIZE =			"#AR-ServerQueue_MaxSize";
	protected const string MESSAGE_ENQUEUED =				"#AR-ServerQueue_Enqueued_Header";
	protected const string MESSAGE_ENQUEUED_POSITION =		"#AR-ServerQueue_CurrentPosition";
	protected const string MESSAGE_QUEUE_WAIT_TIME =		"#AR-ServerQueue_AvgWaitTime";
	protected const string MESSAGE_QUEUE_EXPLANATION =		"#AR-ServerQueue_Enqueued_Message";
	protected const string MESSAGE_QUEUE_EXPLANATION_TIME =	"#AR-ServerQueue_Enqueued_Message_Time";
	
	protected const string LABEL_LEAVE_QUEUE = 	"#AR-ServerQueue_LeaveQueue_Button";
	protected const string LABEL_CANCEL =		"#AR-Workshop_ButtonCancel";
	
	protected ref SCR_ServerFullDialogContentWidgets m_Widgets = new SCR_ServerFullDialogContentWidgets();
	
	protected SCR_InputButtonComponent m_NavConfirm;
	protected SCR_InputButtonComponent m_NavCancel;
	protected SCR_InputButtonComponent m_NavFavorites;
	protected SCR_InputButtonComponent m_NavFieldManual;
	protected SCR_ModularButtonComponent m_BtnFavorites;
	protected SCR_ScenarioBackendImageComponent m_BackendImageComp;
	protected Widget m_wSideSpinner;

	protected Room m_Room;
	protected SCR_EJoinFailUI m_eMode;
	
	protected ref ScriptInvokerRoom m_OnRetryFullServerJoin;
	protected ref ScriptInvokerVoid m_OnLeaveQueueRequest;
	protected ref ScriptInvokerVoid m_OnFavorite;

	// --- Override ---
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		m_Widgets.Init(GetContentLayoutRoot());
		
		m_NavConfirm = FindButton(SCR_ConfigurableDialogUi.BUTTON_CONFIRM);
		m_NavCancel = FindButton(SCR_ConfigurableDialogUi.BUTTON_CANCEL);

		m_NavFieldManual = FindButton(INPUT_BUTTON_FIELD_MANUAL);
		if (m_NavFieldManual)
			m_NavFieldManual.m_OnActivated.Insert(OnFieldManual);
		
		m_NavFavorites = FindButton(INPUT_BUTTON_FAVORITES);
		if (m_NavFavorites)
			m_NavFavorites.m_OnActivated.Insert(OnFavorite);

		Widget mouseFavorites = m_wRoot.FindAnyWidget(MOUSE_BUTTON_FAVORITES);
		if (mouseFavorites)
		{
			m_BtnFavorites = SCR_ModularButtonComponent.FindComponent(mouseFavorites);
			if (m_BtnFavorites)
				m_BtnFavorites.m_OnClicked.Insert(OnFavorite);
		}
		
		m_Widgets.m_IPAddressButtonComponent.m_OnClicked.Insert(OnCopyIPAddress);

		Widget backgroundImageBackend = GetRootWidget().FindAnyWidget(WIDGET_BACKGROUND_IMAGE);
		if (backgroundImageBackend)
			m_BackendImageComp = SCR_ScenarioBackendImageComponent.Cast(backgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
		
		m_wSideSpinner = m_wRoot.FindAnyWidget(WIDGET_SIDE_SPINNER);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetCallqueue().Remove(UpdateEnqueuedTimeDisplay);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		super.OnConfirm();
		
		if (m_OnRetryFullServerJoin && (m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL || m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED))
			m_OnRetryFullServerJoin.Invoke(m_Room);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCancel()
	{
		super.OnCancel();
		
		if (m_eMode == SCR_EJoinFailUI.ENQUEUED && m_OnLeaveQueueRequest)
			m_OnLeaveQueueRequest.Invoke();
	}

	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	protected void OnFavorite()
	{
		if ((m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL || m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED) && m_OnFavorite)
			m_OnFavorite.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFieldManual()
	{
		SCR_FieldManualUI fieldmenu = SCR_FieldManualUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.FieldManualDialog));
		if (fieldmenu)
			fieldmenu.ShowQueueMessage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplayFavoriteAction(bool isFavorite)
	{
		if (m_NavFavorites && m_NavFavorites.IsVisible())
			m_NavFavorites.SetLabel(UIConstants.GetFavoriteLabel(isFavorite));

		// Star Button
		if (m_BtnFavorites)
			m_BtnFavorites.SetToggled(isFavorite, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRoomSetFavoriteResponseDialog()
	{
		DisplayFavoriteAction(m_Room.IsFavorite());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCopyIPAddress()
	{
		System.ExportToClipboard(m_Room.HostAddress());
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIPAddressText(string text)
	{
		m_Widgets.m_wIPAddressText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetDiscordText(string text)
	{
		m_Widgets.m_wDiscordText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDetailIcons()
	{
		if (!m_Room)
			return;

		m_Widgets.m_wDetailIcon_PasswordProtected.SetVisible(m_Room.PasswordProtected());
		m_Widgets.m_wDetailIcon_CrossPlatform.SetVisible(m_Room.IsCrossPlatform());
		m_Widgets.m_wDetailIcon_Modded.SetVisible(m_Room.IsModded());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFavoritesButtonsEnabled(bool enabled)
	{	
		// Input button
		if (m_NavFavorites)
			m_NavFavorites.SetVisible(enabled, false);
		
		// Star mouse button
		if (!m_BtnFavorites)
			return;
		
		Widget widget = m_BtnFavorites.GetRootWidget();
		if (!widget)
			return;

		WidgetFlags flags = widget.GetFlags();
		if (enabled)
			widget.SetFlags(SCR_Enum.RemoveFlag(flags, WidgetFlags.IGNORE_CURSOR));
		else
			widget.SetFlags(SCR_Enum.SetFlag(flags, WidgetFlags.IGNORE_CURSOR));
	}
	
	// --- Displayed info changes based on mode ---
	//------------------------------------------------------------------------------------------------
	// Display information about the queue and the player's position in it
	protected void UpdateInfo_Enqueued()
	{
		// --- Queue position
		int count = m_Room.GetQueueUserPosition() + 1; // backend value starts from 0
		int limit = m_Room.GetQueueMaxSize();
		bool showPosition = count > 0;
		
		m_Widgets.m_wQueueState.SetVisible(showPosition);
		if (showPosition)
			m_Widgets.m_wQueueState.SetText(WidgetManager.Translate(MESSAGE_ENQUEUED_POSITION, SCR_RichTextTags.TagColor(count.ToString(), UIColors.CopyColor(UIColors.CONTRAST_COLOR))));
		
		// --- Queue max size
		m_Widgets.m_wQueueMaxSizeOverlay.SetVisible(showPosition);
		if (showPosition)
			m_Widgets.m_wQueueMaxSize.SetText(WidgetManager.Translate(MESSAGE_QUEUE_MAX_SIZE, limit));
		
		// --- Wait time
		/*
		int waitTime = m_Room.GetQueueAvgWaitTime();
		bool showWaitTime = showPosition && waitTime > 0;

		m_Widgets.m_wQueueWaitTimeOverlay.SetVisible(showWaitTime);
		if (showWaitTime)
			m_Widgets.m_wQueueWaitTime.SetText(WidgetManager.Translate(MESSAGE_QUEUE_WAIT_TIME, UIConstants.FormatSeconds(waitTime * count)));
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	// Display a warning about the queue being full, or if it isn't anymore, display it's size and allow rejoining
	protected void UpdateInfo_QueueFull()
	{
		// --- Queue position
		int count = m_Room.GetQueueSize();
		int limit = m_Room.GetQueueMaxSize();
		bool fullQueue = count == limit;
		bool emptyQueue = count <= 0;
		
		if (fullQueue)
		{
			m_Widgets.m_wQueueState.SetText(MESSAGE_QUEUE_FULL);
			m_Widgets.m_wQueueState.SetColor(UIColors.CopyColor(UIColors.NEUTRAL_ACTIVE_STANDBY));
		}
		else
		{
			m_Widgets.m_wQueueState.SetText(WidgetManager.Translate(MESSAGE_QUEUE_POPULATION, count));
			m_Widgets.m_wQueueState.SetColor(UIColors.CopyColor(UIColors.NEUTRAL_INFORMATION));
		}
		
		// --- Queue max size
		m_Widgets.m_wQueueMaxSize.SetText(WidgetManager.Translate(MESSAGE_QUEUE_MAX_SIZE, limit));
		
		// --- State message
		count = m_Room.PlayerCount();
		limit = m_Room.PlayerLimit();
		bool fullServer = count == limit || !emptyQueue;
		
		if (fullServer)
		{
			m_Widgets.m_wCurrentState.SetText(MESSAGE_SERVER_FULL);
			m_Widgets.m_wCurrentState.SetColor(UIColors.CopyColor(UIColors.WARNING));
		}
		else
		{
			m_Widgets.m_wCurrentState.SetText(WidgetManager.Translate(MESSAGE_SERVER_POPULATION, UIConstants.FormatValueOutOf(count, limit, false)));
			m_Widgets.m_wCurrentState.SetColor(UIColors.CopyColor(UIColors.NEUTRAL_INFORMATION));
		}
		
		// --- Wait time
		/*
		int waitTime = m_Room.GetQueueAvgWaitTime();
		bool showWaitTime = !emptyQueue && waitTime > 0;
		
		m_Widgets.m_wQueueWaitTimeOverlay.SetVisible(showWaitTime);
		if (showWaitTime)
			m_Widgets.m_wQueueWaitTime.SetText(WidgetManager.Translate(MESSAGE_QUEUE_WAIT_TIME, UIConstants.FormatSeconds(waitTime * m_Room.GetQueueSize())));
		*/
		
		// --- Confirm button
		if (m_NavConfirm)
			m_NavConfirm.SetEnabled(!fullServer || !fullQueue, false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Display a warning about the server being full without the ability to queue, or if it isn't anymore, display it's population and allow rejoining
	protected void UpdateInfo_QueueDisabled()
	{
		// --- Queue position
		m_Widgets.m_wQueueState.SetText(MESSAGE_QUEUE_DISABLED);
		m_Widgets.m_wQueueState.SetColor(UIColors.CopyColor(UIColors.NEUTRAL_ACTIVE_STANDBY));
		
		// --- State message
		int count = m_Room.PlayerCount();
		int limit = m_Room.PlayerLimit();
		bool fullServer = count == limit;
		
		if (fullServer)
		{
			m_Widgets.m_wCurrentState.SetText(MESSAGE_SERVER_FULL);
			m_Widgets.m_wCurrentState.SetColor(UIColors.CopyColor(UIColors.WARNING));
		}
		else
		{
			m_Widgets.m_wCurrentState.SetText(WidgetManager.Translate(MESSAGE_SERVER_POPULATION, UIConstants.FormatValueOutOf(count, limit, false)));
			m_Widgets.m_wCurrentState.SetColor(UIColors.CopyColor(UIColors.NEUTRAL_INFORMATION));
		}
		
		// --- Confirm button
		if (m_NavConfirm)
			m_NavConfirm.SetEnabled(!fullServer, false);
	}

	//------------------------------------------------------------------------------------------------
	// Time since joining queue
	protected void UpdateEnqueuedTimeDisplay()
	{
		if (!m_Room || m_Room.GetQueueJoinTime() <= 0)
		{
			m_Widgets.m_wTimeSinceJoiningQueue.SetText(MESSAGE_QUEUE_EXPLANATION);
			return;
		}
		
		// --- Time since join
		string time = SCR_RichTextTags.TagColor(UIConstants.FormatSeconds(m_Room.GetQueueJoinTime()), UIColors.CopyColor(UIColors.CONTRAST_COLOR));
		m_Widgets.m_wTimeSinceJoiningQueue.SetTextFormat(WidgetManager.Translate(MESSAGE_QUEUE_EXPLANATION_TIME, time));
	}
	
	// --- Public ---
	//------------------------------------------------------------------------------------------------
	void Init(Room room, SCR_EJoinFailUI mode, MissionWorkshopItem scenario, ScriptInvokerVoid onFavoritesResponse = null)
	{
		m_Room = room;
		m_eMode = mode;
		
		SetTitle(room.Name());
		SetScenarioImage(scenario);
		SetIPAddressText(SCR_WorkshopUiCommon.LABEL_IP_ADDRESS + " " + room.HostAddress());
		DisplayFavoriteAction(m_Room.IsFavorite());
		UpdateDetailIcons();
		
		UpdateInfo();

		if (onFavoritesResponse)
			onFavoritesResponse.Insert(OnRoomSetFavoriteResponseDialog);
		
		// --- Static widgets
		switch (m_eMode)
		{
			case SCR_EJoinFailUI.ENQUEUED:
			{
				m_Widgets.m_wCurrentState.SetText(MESSAGE_ENQUEUED);
				m_Widgets.m_wCurrentState.SetColor(UIColors.CopyColor(UIColors.CONTRAST_COLOR));
				
				m_Widgets.m_wQueueState.SetColor(UIColors.CopyColor(UIColors.NEUTRAL_INFORMATION));
				
				m_Widgets.m_wExplanationEnqueuedWrapper.SetVisible(true);
				m_Widgets.m_wExplanationFull.SetVisible(false);
				
				UpdateEnqueuedTimeDisplay();
				GetGame().GetCallqueue().CallLater(UpdateEnqueuedTimeDisplay, ENQUEUED_TIME_UPDATE_FREQUENCY, true);
				
				break;
			}
			case SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL:
			{
				m_Widgets.m_wQueueState.SetVisible(true);
				m_Widgets.m_wQueueMaxSizeOverlay.SetVisible(true);
			
				m_Widgets.m_wExplanationEnqueuedWrapper.SetVisible(false);
				m_Widgets.m_wExplanationFull.SetVisible(true);
				
				break;
			}
			case SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED:
			{
				m_Widgets.m_wQueueState.SetVisible(true);
				m_Widgets.m_wQueueMaxSizeOverlay.SetVisible(false);
			
				m_Widgets.m_wExplanationEnqueuedWrapper.SetVisible(false);
				m_Widgets.m_wExplanationFull.SetVisible(true);

				m_Widgets.m_wQueueWaitTimeOverlay.SetVisible(false);
				
				break;
			}
		}
		
		// --- Favorites buttons
		SetFavoritesButtonsEnabled(m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL || m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED);
	
		// --- Confirm button
		if (m_NavConfirm)
			m_NavConfirm.SetVisible(m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL || m_eMode == SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED, false);
		
		// --- Cancel button
		if (m_NavCancel)
		{
			string label = LABEL_CANCEL;
			if (m_eMode == SCR_EJoinFailUI.ENQUEUED)
				label = LABEL_LEAVE_QUEUE;
			
			m_NavCancel.SetLabel(label);
		}
		
		// --- Field Manual button
		if (m_NavFieldManual)
			m_NavFieldManual.SetVisible(m_eMode == SCR_EJoinFailUI.ENQUEUED, false);
		
		// --- SideSpinner
		if (m_wSideSpinner)
			m_wSideSpinner.SetVisible(m_eMode == SCR_EJoinFailUI.ENQUEUED);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateInfo()
	{
		if (!m_Room)
			return;
		
		switch (m_eMode)
		{
			case SCR_EJoinFailUI.ENQUEUED:
			{
				UpdateInfo_Enqueued(); 
				break;
			}
			case SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL:
			{
				UpdateInfo_QueueFull(); 
				break;
			}
			case SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED:
			{
				UpdateInfo_QueueDisabled(); 
				break;
			}
		}
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
	ScriptInvokerRoom GetOnRetryFullServerJoin()
	{
		if (!m_OnRetryFullServerJoin)
			m_OnRetryFullServerJoin = new ScriptInvokerRoom();

		return m_OnRetryFullServerJoin;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnLeaveQueueRequest()
	{
		if (!m_OnLeaveQueueRequest)
			m_OnLeaveQueueRequest = new ScriptInvokerVoid();

		return m_OnLeaveQueueRequest;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnFavorite()
	{
		if (!m_OnFavorite)
			m_OnFavorite = new ScriptInvokerVoid();
		
		return m_OnFavorite;
	}
}