class SCR_NotificationMessageUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sIcon;
	
	[Attribute(desc: "Widgets that are colored by the Notification Color")]
	protected ref array<string> m_aColoredWidgets;
	
	[Attribute(desc: "Notifications widgets that are hidden if notification is GM only. These widgets should be shown by default")]
	protected ref array<string> m_aNonGameMasterWidgets;
	
	[Attribute(desc: "Notifications widgets that are shown if notification is GM only. These widgets should be hidden by default")]
	protected ref array<string> m_aGameMasterOnlyWidgets;

	[Attribute()]
	protected string m_sNotificationText;
	
	[Attribute("GoToIndicator")]
	protected string m_sGoToIndicator;

	[Attribute("GoToControlHint")]
	protected string m_sGoToControlHint;
	
	[Attribute("GotoIndicatorsHolder")]
	protected string m_sGoToIndicatorsHolder;

	//Reference
	protected Widget m_wRoot;
	protected SCR_NotificationsLogComponent m_NotificationsLog;
	protected ref SCR_NotificationData m_Data;
	protected Widget m_GoToControlHint;
	protected Widget m_GoToIndicator;
	protected Widget m_GoToIndicatorsHolder;
	
	protected ref ScriptInvoker Event_OnDeleted = new ScriptInvoker();
	
	//State
	protected bool m_bIsListeningToDoneFade;
	protected bool m_bHasPosition;
	protected bool m_bShowTeleportControlHint;
	
	//Animation speed and update	
	protected static const float ANIMATION_DONE_UPDATE_SPEED = 100;
	protected static const float FORCE_DELETE_FADE_SPEED = 3;
	protected static const float AUTO_FADE_SPEED = 1;
	protected static const float FADE_IN_SPEED = 2;
	
	//------------------------------------------------------------------------------------------------
	//! Init the message, setting the visuals
	//! \param[in] data the data of the notification
	//! \param[in] notificationLog a reference to the notificationLog
	//! \param[in] fadeDelay automatic fade delay of notification
	void Init(SCR_NotificationData data, SCR_NotificationsLogComponent notificationLog, float fadeDelay)
	{
		if (!m_wRoot)
			return;
		
		m_Data = data;
		if (!m_Data)
			return;
		
		SCR_NotificationsComponent notificationManager = notificationLog.GetNotificationManager();
		SCR_NotificationDisplayData displayData = data.GetDisplayData();
		SCR_EditorManagerEntity editorManager = notificationLog.GetEditorManager();
		
		m_NotificationsLog = notificationLog;
		
		TextWidget notificationText = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sNotificationText));
		if (!notificationText)
			return;

		UpdateText();

		SCR_UINotificationInfo uiInfo;
		ENotificationColor notificationColorEnum;
		displayData.GetDisplayVisualizationData(data, uiInfo, notificationColorEnum);
		
		if (uiInfo)
		{
			ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIcon));
			if (icon)
			{
				if (uiInfo.HasIcon())
					uiInfo.SetIconTo(icon);
				else 
					icon.SetVisible(false);
			}
			
			SetWidgetColor(m_NotificationsLog.GetNotificationWidgetColor(notificationColorEnum));
		}
		
		//Show Gm only visuals and hide non game master visuals
		if (data.GetNotificationReceiverType() == ENotificationReceiver.GM_ONLY || data.GetNotificationReceiverType() == ENotificationReceiver.LOCAL_GM_ONLY || data.GetNotificationReceiverType() == ENotificationReceiver.GM_OR_AFFECTED_PLAYER_ONLY)
		{
			Widget elementWidget;
			
			foreach (string element: m_aNonGameMasterWidgets)
			{
				elementWidget = m_wRoot.FindAnyWidget(element);
				
				if (elementWidget)
					elementWidget.SetVisible(false);
			}
			
			foreach (string element: m_aGameMasterOnlyWidgets)
			{
				elementWidget = m_wRoot.FindAnyWidget(element);
				
				if (elementWidget)
					elementWidget.SetVisible(true);
			}
		}
		
		//Set text color
		SCR_ColoredTextNotificationUIInfo coloredTextNotificationUIInfo = SCR_ColoredTextNotificationUIInfo.Cast(uiInfo);
		if (coloredTextNotificationUIInfo)
		{
			Color textColor = notificationLog.GetNotificationTextColor(displayData.GetTextColor(data));
			textColor.SetA(notificationText.GetColor().A());
			notificationText.SetColor(textColor);
		}
		
		//Get position
		vector position;
		if (m_NotificationsLog.HasNotificationInput() && displayData.GetPosition(data, position))
		{
			m_NotificationsLog.GetEditorManager().GetOnOpened().Insert(EditorOpened);
			m_NotificationsLog.GetEditorManager().GetOnClosed().Insert(EditorClosed);
			m_NotificationsLog.GetEditorManager().GetOnLimitedChange().Insert(OnLimitedChanged);
			
			m_bHasPosition = true;
			m_bShowTeleportControlHint = true;
			
			//notificationManager.GetOnNotification().Insert(OnNotification);
			m_NotificationsLog.GetOnNewMessageHasPosition().Insert(OnNewNotificationHasPosition);
			m_NotificationsLog.GetOnInputDeviceChanged().Insert(OnInputDeviceChanged);
			
			if (!editorManager.IsOpened())
				EditorClosed();
			else 
				EditorOpened();
		}
		else
		{
			//Disable teleport button
			m_wRoot.SetEnabled(false);
		}
			
		GetGame().GetCallqueue().CallLater(AutoFadeDelay, fadeDelay);
		
		m_wRoot.SetOpacity(0);
		AnimateWidget.Opacity(m_wRoot, 1, FADE_IN_SPEED);
	}

	//------------------------------------------------------------------------------------------------
	//! Update message text
	void UpdateText()
	{
		if (!m_Data)
			return;

		string notificationMessage = m_Data.GetText();
		if (notificationMessage.IsEmpty())
		{
			ForceRemoveNotification();
			return;
		}

		SCR_NotificationDisplayData displayData = m_Data.GetDisplayData();
		if (!displayData)
			return;

		TextWidget notificationText = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sNotificationText));
		if (!notificationText)
			return;

		string param1, param2, param3, param4, param5, param6;

		if (notificationMessage != string.Empty)
			m_Data.GetNotificationTextEntries(param1, param2, param3, param4, param5, param6);
		else
			notificationMessage = typename.EnumToString(ENotification, m_Data.GetID());

		if (!displayData.MergeParam1With2())
		{
			notificationText.SetTextFormat(notificationMessage, param1, param2, param3, param4, param5, param6);
		}
		else
		{
			string mergeParams = WidgetManager.Translate(param2, param1);
			notificationText.SetTextFormat(notificationMessage, param1, mergeParams, param3, param4, param5, param6);
		}
	}

	//======================== SET COLOR ========================\\

	//------------------------------------------------------------------------------------------------
	protected void SetWidgetColor(Color notificationColor)
	{
		if (!m_aColoredWidgets || m_aColoredWidgets.IsEmpty())
			return;
		
		Color color;
		
		foreach (string widgetName: m_aColoredWidgets)
		{
			Widget widget = m_wRoot.FindAnyWidget(widgetName);
			if (widget)
			{
				color = Color.FromInt(notificationColor.PackToInt());
				color.SetA(widget.GetColor().A());
				widget.SetColor(notificationColor);
			}
		}
	}
	
	//Removed for now but can be used for mods. 
//	protected void OnNotification(SCR_NotificationData data)
//	{
//
//	}
	
	//Quickly fades out notification and deletes it. Called when max notification count is reached
	//------------------------------------------------------------------------------------------------
	//! Delete the notification with a fast fade
	void ForceRemoveNotification()
	{
		FadeDelete(FORCE_DELETE_FADE_SPEED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get script invoker for when notification is deleted
	//! \return ScriptInvoker Event_OnDeleted
	ScriptInvoker GetOnDeleted()
	{
		return Event_OnDeleted;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNewNotificationHasPosition()
	{		
		if (!m_GoToControlHint || !m_GoToIndicator || !m_GoToIndicatorsHolder)
			return;
		
		if (!m_bShowTeleportControlHint)
			return;
		
		m_bShowTeleportControlHint = false;
		
		if (!m_NotificationsLog.GetEditorManager().IsOpened())
			return;
		
		bool isLimited = m_NotificationsLog.GetEditorManager().IsLimited();
		
		//Keyboard and mouse
		if (!isLimited && m_NotificationsLog.GetIsUsingMouseAndKeyboard())
		{
			m_GoToControlHint.SetVisible(false);
			m_GoToIndicator.SetVisible(true);
			m_GoToIndicatorsHolder.SetVisible(true);
		}
		//Using gamepad
		else 
		{
			m_GoToIndicatorsHolder.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorOpened()
	{	
		bool limited = m_NotificationsLog.GetEditorManager().IsLimited();
		
		if (!m_bShowTeleportControlHint)
		{
			if (m_GoToIndicator && m_GoToIndicatorsHolder)
			{
				m_GoToIndicator.SetVisible(m_NotificationsLog.GetIsUsingMouseAndKeyboard() && !limited);
				m_GoToIndicatorsHolder.SetVisible(m_NotificationsLog.GetIsUsingMouseAndKeyboard() && !limited);
			}
				
		}
		else 
		{
			if (m_GoToControlHint && m_GoToIndicatorsHolder)
			{
				m_GoToControlHint.SetVisible(!limited);
				m_GoToIndicatorsHolder.SetVisible(!limited);
			}
				
		}
			
		if (m_wRoot)
			m_wRoot.SetEnabled(!limited);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorClosed()
	{
		if (m_GoToControlHint)
			m_GoToIndicatorsHolder.SetVisible(false);
		
		if (m_wRoot)
			m_wRoot.SetEnabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLimitedChanged(bool limited)
	{
		EditorOpened();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChanged(bool isUsingMouseAndKeyboard)
	{
		bool limited = m_NotificationsLog.GetEditorManager().IsLimited();
		
		if (!m_bHasPosition || !m_NotificationsLog.GetEditorManager().IsOpened() || limited)
			return;
		
		if (m_GoToIndicator)
			m_GoToIndicator.SetVisible(isUsingMouseAndKeyboard && !m_bShowTeleportControlHint);
		
		if (m_GoToIndicatorsHolder)
			m_GoToIndicatorsHolder.SetVisible(isUsingMouseAndKeyboard || m_bShowTeleportControlHint)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TeleportToLocation()
	{		
		if (!m_NotificationsLog.HasNotificationInput() || !m_Data)
			return;
		
		bool limited = m_NotificationsLog.GetEditorManager().IsLimited();
		if (limited)
			return;
		
		vector position;
		if (m_Data.GetDisplayData().GetPosition(m_Data, position))
		{
			SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
			if (!camera)
				return;

			SCR_TeleportToCursorManualCameraComponent cursorManualCameraComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
			if (!cursorManualCameraComponent)
				return;

			cursorManualCameraComponent.TeleportCamera(position);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FadeDelete(float fadeSpeed)
	{
		AnimateWidget.Opacity(m_wRoot, 0, fadeSpeed);
		
		if (!m_bIsListeningToDoneFade)
		{
			m_bIsListeningToDoneFade = true;
			GetGame().GetCallqueue().CallLater(AnimationDoneListenerUpdate, ANIMATION_DONE_UPDATE_SPEED, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AnimationDoneListenerUpdate()
	{
		if (!AnimateWidget.IsAnimating(m_wRoot))
			DeleteNotification();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DeleteNotification()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AutoFadeDelay()
	{
		if (m_bIsListeningToDoneFade)
			return;
		
		FadeDelete(AUTO_FADE_SPEED);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		m_wRoot = w;
		
		m_GoToIndicator = w.FindAnyWidget(m_sGoToIndicator);
		m_GoToControlHint = w.FindAnyWidget(m_sGoToControlHint);
		
		m_GoToIndicatorsHolder = m_wRoot.FindAnyWidget(m_sGoToIndicatorsHolder);	
		if (m_GoToIndicatorsHolder)
			m_GoToIndicatorsHolder.SetVisible(false);	
		
		ScriptInvoker teleportButton = ButtonActionComponent.GetOnAction(m_wRoot, false);
		if (teleportButton)
			teleportButton.Insert(TeleportToLocation);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		if (SCR_Global.IsEditMode()) 
			return;
		
		Event_OnDeleted.Invoke(this);
		
		if (!m_Data)
			return;

		if (m_bHasPosition && m_NotificationsLog && m_NotificationsLog.GetEditorManager())
		{
			m_NotificationsLog.GetEditorManager().GetOnOpened().Remove(EditorOpened);
			m_NotificationsLog.GetEditorManager().GetOnClosed().Remove(EditorClosed);
			m_NotificationsLog.GetEditorManager().GetOnLimitedChange().Remove(OnLimitedChanged);
			m_NotificationsLog.GetOnNewMessageHasPosition().Remove(OnNewNotificationHasPosition);
			//m_NotificationsLog.GetNotificationManager().GetOnNotification().Remove(OnNotification);
			m_NotificationsLog.GetOnInputDeviceChanged().Remove(OnInputDeviceChanged);
		}
			
		if (m_bIsListeningToDoneFade)
			GetGame().GetCallqueue().Remove(AnimationDoneListenerUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_NotificationData GetData()
	{
		return m_Data;
	}
}
