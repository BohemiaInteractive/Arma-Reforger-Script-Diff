//------------------------------------------------------------------------------------------------
class SCR_PopupMessage
{
	string m_sText;
	string m_sSubtitle;
	float m_fDuration;
	int m_iPriority;
	string m_sSound;
	string m_aTextParams[4] = {"", "", "", ""};
	string m_aSubtitleParams[4] = {"", "", "", ""};
	SCR_EPopupMsgFilter m_eFilter;
	WorldTimestamp m_fProgressStart;
	WorldTimestamp m_fProgressEnd;
	WorldTimestamp m_fHideTimestamp;
};

//------------------------------------------------------------------------------------------------
class SCR_PopUpNotificationClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! Takes care of dynamic and static onscreen popups
class SCR_PopUpNotification : GenericEntity
{
	protected static const int INVALID_ID = -1;
	protected static const float FADE_DURATION = 1;
	protected static const ResourceName LAYOUT_NAME = "{8EF935F196AADE33}UI/layouts/Common/PopupUI.layout";
	protected static const int POPUP_OFFSET = 35;

	protected static SCR_PopUpNotification s_Instance = null;
	protected static SCR_EPopupMsgFilter s_eFilter;

	static const float DEFAULT_DURATION = 4;
	static const string TASKS_KEY_IMAGE_FORMAT = "<color rgba='226, 168, 79, 200'><shadow mode='image' color='0, 0, 0' size='1' offset='1, 1' opacity = '0.5'><action name='TasksOpen'/></shadow></color>";

	protected ref array<ref SCR_PopupMessage> m_aQueue = {};

	protected Widget m_wRoot;
	protected RichTextWidget m_wPopupMsg;
	protected RichTextWidget m_wPopupMsgSmall;
	protected ProgressBarWidget m_wStatusProgress;

	protected float m_fDefaultAlpha;
	protected float m_fDefaultHorizontalOffset = -1;

	protected bool m_bInventoryOpen;
	protected bool m_bOffset;
	protected bool m_bIsEnabledInSettings = true;

	protected IEntity m_Player;

	protected SCR_PopupMessage m_ShownMsg;
	protected string m_sCachedLanguage;

	protected static const string INTERFACE_SETTINGS_NAME = "m_bShowNotifications";

	//------------------------------------------------------------------------------------------------
	static SCR_PopUpNotification GetInstance()
	{
		if (!s_Instance)
		{
			BaseWorld world = GetGame().GetWorld();

			if (world)
				s_Instance = SCR_PopUpNotification.Cast(GetGame().SpawnEntity(SCR_PopUpNotification, world));
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	static void SetFilter(SCR_EPopupMsgFilter filter)
	{
		s_eFilter = filter;
	}
	
	//------------------------------------------------------------------------------------------------
	void Offset(bool down)
	{
		if (!m_wPopupMsg || !m_wPopupMsgSmall || !m_wStatusProgress)
			return;

		if (down == m_bOffset)
			return;
		
		m_bOffset = down;
		
		int offset = POPUP_OFFSET;
		
		if (!down)
			offset = offset * -1;
			
		FrameSlot.SetPosY(m_wPopupMsg, FrameSlot.GetPosY(m_wPopupMsg) + offset);
		FrameSlot.SetPosY(m_wPopupMsgSmall, FrameSlot.GetPosY(m_wPopupMsgSmall) + offset);
		FrameSlot.SetPosY(m_wStatusProgress, FrameSlot.GetPosY(m_wStatusProgress) + offset);
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessInit()
	{
		if (!GetGame().GetHUDManager())
			return;

		PlayerController pc = GetGame().GetPlayerController();

		if (!pc || !pc.GetControlledEntity())
			return;

		Widget root = GetGame().GetHUDManager().FindLayoutByResourceName(LAYOUT_NAME);
		if (!root)
			root = GetGame().GetHUDManager().CreateLayout(LAYOUT_NAME, EHudLayers.ALWAYS_TOP, 0);

		if (!root)
			return;

		// Init can be safely processed
		GetGame().GetCallqueue().Remove(ProcessInit);

		SCR_EditorManagerEntity.GetInstance().GetOnModeChange().Insert(OnEditorModeChanged);
		SCR_EditorManagerEntity.GetInstance().GetOnOpened().Insert(OnEditorOpened);
		SCR_EditorManagerEntity.GetInstance().GetOnClosed().Insert(OnEditorClosed);
		m_wRoot = root;

		// Initialize popups UI
		m_wPopupMsg = RichTextWidget.Cast(root.FindAnyWidget("Popup"));
		m_wPopupMsgSmall = RichTextWidget.Cast(root.FindAnyWidget("PopupSmall"));
		m_wStatusProgress = ProgressBarWidget.Cast(root.FindAnyWidget("Progress"));
		m_fDefaultAlpha = m_wPopupMsg.GetColor().A();
		m_wPopupMsg.SetVisible(false);
		m_wPopupMsgSmall.SetVisible(false);
		m_wStatusProgress.SetVisible(false);

		GetGame().GetCallqueue().CallLater(SetDefaultHorizontalPosition, 500);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (playerController)
			playerController.m_OnControlledEntityChanged.Insert(RefreshInventoryInvoker);

		RefreshQueue();

		// Popups should not be visible in map
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity)
		{
			MapConfiguration config = mapEntity.GetMapConfig();
			if (config)
			{
				Widget mapWidget = config.RootWidgetRef;
				if (mapWidget)
					root.SetZOrder(mapWidget.GetZOrder() - 1);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSettingsChanged()
	{		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(hudManager.GetInterfaceSettingsClass());
		if (!interfaceSettings)
			return;
		
		bool state;
		if (!interfaceSettings.Get(INTERFACE_SETTINGS_NAME, state))
			return;
		
		m_bIsEnabledInSettings = state;
		
		if (!m_bIsEnabledInSettings)
			HideCurrentMsg();

		string newLanguage;
		WidgetManager.GetLanguage(newLanguage);
		if (newLanguage.IsEmpty() || newLanguage == m_sCachedLanguage) // check if language is really changed
			return;

		m_sCachedLanguage = newLanguage;

		ClearMsg();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshInventoryInvoker(IEntity previousPlayer, IEntity currentPlayer)
	{
		SCR_InventoryStorageManagerComponent inventory;

		if (previousPlayer)
		{
			inventory = SCR_InventoryStorageManagerComponent.Cast(previousPlayer.FindComponent(SCR_InventoryStorageManagerComponent));

			if (inventory)
				inventory.m_OnInventoryOpenInvoker.Remove(OnInventoryToggle);
		}

		if (!currentPlayer)
			return;

		inventory = SCR_InventoryStorageManagerComponent.Cast(currentPlayer.FindComponent(SCR_InventoryStorageManagerComponent));

		if (!inventory)
			return;

		// Make absolutely sure invoker is not used multiple times in the same inventory
		inventory.m_OnInventoryOpenInvoker.Remove(OnInventoryToggle);
		inventory.m_OnInventoryOpenInvoker.Insert(OnInventoryToggle);
	}

	//------------------------------------------------------------------------------------------------
	void OnInventoryToggle(bool open)
	{
		m_bInventoryOpen = open;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpened()
	{
		if (!m_wRoot)
			return;

		if (SCR_Enum.HasFlag(SCR_EditorManagerEntity.GetInstance().GetCurrentMode(), EEditorMode.BUILDING))
			m_wRoot.SetVisible(true);
		else
			m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		if (!m_wRoot)
			return;

		m_wRoot.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChanged(SCR_EditorModeEntity currentMode, SCR_EditorModeEntity prevMode)
	{
		if (!m_wRoot || !SCR_EditorManagerEntity.GetInstance().IsOpened())
			return;

		if (SCR_Enum.HasFlag(currentMode.GetModeType(), EEditorMode.BUILDING))
			m_wRoot.SetVisible(true);
		else
			m_wRoot.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPresentPopupWithHigherPriority(int priority)
	{
		for (int i = 0, cnt = m_aQueue.Count(); i < cnt; i++)
		{
			if (m_aQueue[i].m_iPriority > priority)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetTitleText(string text)
	{
		if (m_wPopupMsg)
			m_wPopupMsg.SetTextFormat(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Queue new popup notification
	//! \param text Text to be shown
	//! \param duration How long the text should stay on screen, -1 for infinite duration (use HideCurrentMsg() to toggle off)
	//! \param text2 Secondary (smaller) text
	//! \param prio Priority (when more popups are queued)
	//! \param param1 (and subsequent) Text or secondary text parameters to be parsed
	//! \param sound Sound event to be played via SCR_UISoundEntity
	//! \param category See SCR_EPopupMsgFilter for settings
	//! \param progressStart Progress bar start value (relative to Replication.Time())
	//! \param progressEnd Progress bar end value (relative to Replication.Time()
	void PopupMsg(string text, float duration = DEFAULT_DURATION, string text2 = "", int prio = -1, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string text2param1 = "", string text2param2 = "", string text2param3 = "", string text2param4 = "", string sound = "", SCR_EPopupMsgFilter category = SCR_EPopupMsgFilter.ALL,
		WorldTimestamp progressStart = null,
		WorldTimestamp progressEnd = null
	)
	{
		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;
		
		if (!m_bIsEnabledInSettings)
			return;

		// Check filter settings, stop if the message is filtered out
		if (s_eFilter == SCR_EPopupMsgFilter.NONE || (s_eFilter == SCR_EPopupMsgFilter.TUTORIAL && category != SCR_EPopupMsgFilter.TUTORIAL))
			return;

		// Invalid params
		if ((text.IsEmpty() && text2.IsEmpty()) || duration == 0)
			return;

		SCR_PopupMessage msg = new SCR_PopupMessage;

		if (!msg)
			return;

		msg.m_sText = text;
		msg.m_sSubtitle = text2;
		msg.m_fDuration = duration;
		msg.m_iPriority = prio;
		msg.m_sSound = sound;
		msg.m_aTextParams = {param1, param2, param3, param4};
		msg.m_aSubtitleParams = {text2param1, text2param2, text2param3, text2param4};
		msg.m_eFilter = category;
		msg.m_fProgressStart = progressStart;
		msg.m_fProgressEnd = progressEnd;

		RefreshQueue(msg);
	}

	//------------------------------------------------------------------------------------------------
	SCR_PopupMessage GetCurrentMsg()
	{
		return m_ShownMsg;
	}

	//------------------------------------------------------------------------------------------------
	void HideCurrentMsg()
	{
		if (!m_ShownMsg)
			return;

		ChimeraWorld world = GetWorld();
		m_ShownMsg.m_fHideTimestamp = world.GetServerTimestamp();
	}

	//------------------------------------------------------------------------------------------------
	void ChangeProgressBarFinish(WorldTimestamp progressEnd)
	{
		if (!m_ShownMsg || m_ShownMsg.m_fProgressEnd == 0)
			return;

		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();

		// Save progress bar status so only its filling speed is changed
		float totalTime = m_ShownMsg.m_fProgressEnd.DiffMilliseconds(m_ShownMsg.m_fProgressStart);
		float elapsedTime = curTime.DiffMilliseconds(m_ShownMsg.m_fProgressStart);
		float curProgress = elapsedTime / totalTime;

		// Avoid possible division by 0 later
		if (curProgress == 1)
			return;

		m_ShownMsg.m_fProgressEnd = progressEnd;

		// Recalculate start value so the bar keeps its original progress
		float newRemainingTime = progressEnd.DiffMilliseconds(curTime);
		float newTotalTime = newRemainingTime / (1 - curProgress);
		float newElapsedTime = newTotalTime * curProgress;
		m_ShownMsg.m_fProgressStart = curTime.PlusMilliseconds(-newElapsedTime);

		// Needed to reset the progress bar limits
		m_wStatusProgress.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void FadeWidget(notnull Widget widget, bool fadeOut = false, bool instant = false)
	{
		float alpha, targetAlpha;

		if (fadeOut)
		{
			alpha = m_fDefaultAlpha;
			targetAlpha = 0;
		}
		else
		{
			alpha = 0;
			targetAlpha = m_fDefaultAlpha;
		}

		if (instant)
		{
			widget.SetOpacity(targetAlpha);
		}
		else
		{
			widget.SetOpacity(alpha);
			AnimateWidget.Opacity(widget, targetAlpha, FADE_DURATION, !fadeOut || widget.IsVisible());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshQueue(SCR_PopupMessage msg = null)
	{
		if (msg)
		{
			int index = m_aQueue.Find(msg);

			if (index == -1)
			{
				// Find the correct array index for the new message (based on priority)
				SCR_PopupMessage checkedMsg;

				for (int i = 0, cnt = m_aQueue.Count(); i < cnt; i++)
				{
					checkedMsg = m_aQueue[i];

					// Infinite duration is always considered higher prio
					if (msg.m_fDuration == -1)
					{
						if (checkedMsg.m_fDuration != -1 || checkedMsg.m_iPriority <= msg.m_iPriority)
							index = i;
					}
					else if (checkedMsg.m_fDuration != -1 && checkedMsg.m_iPriority < msg.m_iPriority)
					{
						index = i;
					}

					if (index != -1)
						break;
				}

				if (index == -1)
					m_aQueue.Insert(msg);
				else
					m_aQueue.InsertAt(msg, index);
			}
			else
			{
				m_aQueue.RemoveOrdered(index);
			}
		}

		if (m_aQueue.IsEmpty())
		{
			ClearEventMask(EntityEvent.FRAME);
		}
		else if (m_wPopupMsg)
		{
			if (m_aQueue[0] != m_ShownMsg)
				ShowMsg(m_aQueue[0]);

			SetEventMask(EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowMsg(notnull SCR_PopupMessage msg)
	{
		if (!m_bIsEnabledInSettings)
			return;
		
		if (msg == m_ShownMsg)
			return;

		if (m_ShownMsg)
			HideMsg(m_ShownMsg);

		if (msg.m_fDuration != -1)
		{
			ChimeraWorld world = GetWorld();
			msg.m_fHideTimestamp = world.GetServerTimestamp().PlusSeconds(msg.m_fDuration);
		}

		m_wPopupMsg.SetTextFormat(msg.m_sText, msg.m_aTextParams[0], msg.m_aTextParams[1], msg.m_aTextParams[2], msg.m_aTextParams[3]);
		FadeWidget(m_wPopupMsg);

		if (!msg.m_sSubtitle.IsEmpty())
		{
			m_wPopupMsgSmall.SetTextFormat(msg.m_sSubtitle, msg.m_aSubtitleParams[0], msg.m_aSubtitleParams[1], msg.m_aSubtitleParams[2], msg.m_aSubtitleParams[3]);
			FadeWidget(m_wPopupMsgSmall);
		}

		if (msg.m_fProgressStart != 0 && msg.m_fProgressEnd != 0)
		{
			m_wStatusProgress.SetMin(0);
			m_wStatusProgress.SetMax(msg.m_fProgressEnd.DiffMilliseconds(msg.m_fProgressStart));
			m_wStatusProgress.SetVisible(false);
			FadeWidget(m_wStatusProgress);
		}

		if (!msg.m_sSound.IsEmpty())
			SCR_UISoundEntity.SoundEvent(msg.m_sSound);

		m_ShownMsg = msg;
	}

	//------------------------------------------------------------------------------------------------
	protected void HideMsg(notnull SCR_PopupMessage msg)
	{
		if (msg == m_ShownMsg)
		{
			m_ShownMsg = null;

			FadeWidget(m_wPopupMsg, true);
			FadeWidget(m_wPopupMsgSmall, true);
			FadeWidget(m_wStatusProgress, true);
		}

		RefreshQueue(msg);
	}

	//------------------------------------------------------------------------------------------------
	//! Clear and instant fade all popup messages
	void ClearMsg()
	{
		if (m_wPopupMsg)
			FadeWidget(m_wPopupMsg, true, true);

		if (m_wPopupMsgSmall)
			FadeWidget(m_wPopupMsgSmall, true, true);

		if (m_wStatusProgress)
			FadeWidget(m_wStatusProgress, true, true);

		m_aQueue.Clear();
		m_ShownMsg = null;
	}

	//------------------------------------------------------------------------------------------------
	void SetDefaultHorizontalPosition()
	{
		float x, y;
		m_wPopupMsg.GetScreenPos(x, y);
		m_fDefaultHorizontalOffset = y;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_ShownMsg)
			return;

		if (m_bInventoryOpen)
		{
			m_wPopupMsg.SetVisible(false);
			m_wPopupMsgSmall.SetVisible(false);
			m_wStatusProgress.SetVisible(false);
			return;
		}

		BaseWorld world = GetGame().GetWorld();

		if (!world)
			return;

		ChimeraWorld chimWorld = world;
		WorldTimestamp curTime = chimWorld.GetServerTimestamp();
		if (m_ShownMsg.m_fHideTimestamp != 0 && curTime.GreaterEqual(m_ShownMsg.m_fHideTimestamp))
		{
			HideMsg(m_ShownMsg);
			return;
		}

		if (m_ShownMsg.m_sText && !m_wPopupMsg.IsVisible())
			m_wPopupMsg.SetVisible(true);

		if (m_ShownMsg.m_sSubtitle && !m_wPopupMsgSmall.IsVisible())
			m_wPopupMsgSmall.SetVisible(true);

		if (m_ShownMsg.m_fProgressEnd.Greater(curTime))
		{
			if (!m_wStatusProgress.IsVisible())
			{
				m_wStatusProgress.SetVisible(true);
				m_wStatusProgress.SetMin(0);
				m_wStatusProgress.SetMax(m_ShownMsg.m_fProgressEnd.DiffMilliseconds(m_ShownMsg.m_fProgressStart));
			}

			m_wStatusProgress.SetCurrent(curTime.DiffMilliseconds(m_ShownMsg.m_fProgressStart));
		}
		else if (m_wStatusProgress.IsVisible())
		{
			m_wStatusProgress.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PopUpNotification(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);

		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;

		// Make sure init can be processed properly (when HUD Manager is ready, check in ProcessInit())
		GetGame().GetCallqueue().Remove(ProcessInit);
		GetGame().GetCallqueue().CallLater(ProcessInit, 1000, true);
		
		
		// Initialize Interface settings
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(hudManager.GetInterfaceSettingsClass());
		if (!interfaceSettings)
			return;
		
		if (interfaceSettings.Get(INTERFACE_SETTINGS_NAME, m_bIsEnabledInSettings))
		{
			GetGame().OnUserSettingsChangedInvoker().Insert(OnSettingsChanged);
			WidgetManager.GetLanguage(m_sCachedLanguage);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PopUpNotification()
	{
		GetGame().OnUserSettingsChangedInvoker().Remove(OnSettingsChanged);
		
		if (m_wPopupMsg)
			m_wPopupMsg.GetParent().RemoveFromHierarchy();

		s_eFilter = SCR_EPopupMsgFilter.ALL;

		GetGame().GetCallqueue().Remove(ProcessInit);

		if (!SCR_EditorManagerEntity.GetInstance())
			return;

		SCR_EditorManagerEntity.GetInstance().GetOnModeChange().Remove(OnEditorModeChanged);
		SCR_EditorManagerEntity.GetInstance().GetOnOpened().Remove(OnEditorOpened);
		SCR_EditorManagerEntity.GetInstance().GetOnClosed().Remove(OnEditorClosed);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_EPopupMsgFilter
{
	ALL,
	NONE,
	TUTORIAL
};
