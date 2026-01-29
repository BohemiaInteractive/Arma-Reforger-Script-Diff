//------------------------------------------------------------------------------------------------
//! Data class for transmissions
class TransmissionData
{
	// Widgets
	ref SCR_VoNOverlay_ElementWidgets m_Widgets;
	Widget m_wBaseWidget;

	bool m_bForceUpdate;	// when transmission update is required
	bool m_bVisible;		// element visible, false if widget not visible
	bool m_bIsActive;		// active, false when widget starts fade out
	bool m_bIsAnimating;	// in process of fade in/out
	bool m_bIsAdditional;
	int m_iPlayerID;		// incoming tranmissions playerID
	float m_fAlpha;			// opacity
	float m_fActiveTimeout;	// delay before active element starts fading
	float m_fFrequency;		// current frequency
	float m_fQuality;
	IEntity m_Entity;		// transmitting character
	bool m_bIsSenderEditor;
	Faction m_Faction;
	BaseTransceiver m_RadioTransceiver;

	//------------------------------------------------------------------------------------------------
	// Hide transmission, skip animation
	void HideTransmission()
	{
		m_fAlpha = 0;
		m_bIsActive = false;
		m_bIsAnimating = false;
		m_fActiveTimeout = 0;
		m_bVisible = false;
		if (m_wBaseWidget)
			m_wBaseWidget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	void TransmissionData(Widget base, int iPlayerID)
	{
		m_wBaseWidget = base;
		m_wBaseWidget.SetOpacity(0);

		m_Widgets = new SCR_VoNOverlay_ElementWidgets();
		m_Widgets.Init(m_wBaseWidget);

		m_iPlayerID = iPlayerID;
	}
}

//------------------------------------------------------------------------------------------------
//! VON display of active outgoing and incoming transmissions
class SCR_VonDisplay : SCR_InfoDisplayExtended
{
	[Attribute(UIConstants.ICONS_IMAGE_SET)]
	protected string m_sImageSet;

	[Attribute(UIConstants.ICONS_GLOW_IMAGE_SET)]
	protected string m_sImageSetGlow;

	[Attribute("{25221F619214A722}UI/layouts/HUD/VON/VoNOverlay_Element.layout")]
	protected string m_sReceivingTransmissionLayout;
	
	[Attribute(defvalue: "1", desc: "Show enemy names in radio and direct talk?")]
	protected bool m_bShowEnemyNames;
	
	const string ICON_DIRECT_SPEECH = "VON_directspeech";
	const string ICON_RADIO_HINT = "VON_radio";
	const string ICON_RADIO = "VON_frequency";
	const string ICON_RADIO_MUTED = "sound-off";
	const string ICON_SERVER_DISABLE_HINT = "server-locked";

	const string LABEL_FREQUENCY_UNITS = "#AR-VON_FrequencyUnits_MHz";
	const string LABEL_UNKNOWN_SOURCE = "#AR-VON_UnknownSource";
	const string LABEL_CHANNEL_PLATOON = "#AR-VON_ChannelPlatoon-UC";
	const string WIDGET_INCOMING = "VonIncoming";
	const string WIDGET_TRANSMIT = "VonTransmitting";
	const string WIDGET_OVERFLOW = "VonAdditional";
	const string WIDGET_OVERFLOW_TEXT = "number";
	const string WIDGET_SELECTED_ROOT = "VonSelected";
	const string WIDGET_SELECTED_ICON = "Selected_Icon";
	const string WIDGET_SELECTED_ICONGLOW = "Selected_IconGlow";
	const string WIDGET_SELECTED_VON = "Selected_VONChannel";
	const string WIDGET_SELECTED_TEXT = "Selected_Text";

	static const ref Color COLOR_WHITE = Color.FromSRGBA(255, 255, 255, 255);

	protected int m_iTransmissionSlots = 4;			// max amount of receiving transmissions
	const float FADEOUT_TIMER_THRESHOLD = 1;
	const float FADEIN_SPEED = 10;
	const float FADEOUT_SPEED = 5;
	const int SELECTED_HINT_FADE_SPEED = 3000; // ms
	const int HEIGHT_DIVIDER = 35;

	protected bool m_bIsVONUIDisabled;
	protected bool m_bIsVONDirectDisabled;
	protected bool m_bIsDirectToggled;			// direct speech toggle state
	protected bool m_bIsChannelToggled;			// channel toggle EFireState

	protected ref TransmissionData m_OutTransmission;												// outgoing transmission data class
	protected ref array<Widget> m_aWidgetsIncomingVON = {};											// available widgets for receiving transmission display
	protected ref array<ref TransmissionData> m_aTransmissions = {}; 								// array of existing incoming transmission data classes
	protected ref map<int, TransmissionData> m_aTransmissionMap = new map<int, TransmissionData>;	// map for lookup when transmission is received
	protected ref array<ref TransmissionData> m_aAdditionalSpeakers = {};							// array of additional speakers that cant be displayed

	protected PlayerManager m_PlayerManager;
	protected SCR_VONController m_VONController;
	protected SCR_InfoDisplaySlotHandler m_SlotHandler;
	protected SCR_HUDSlotUIComponent m_HUDSlotComponent;

	protected Widget m_wVerticalLayout;
	protected Widget m_wSelectedHint;
	protected ImageWidget m_wSelectedHintIcon;
	protected ImageWidget m_wSelectedHintIconGlow;
	protected TextWidget m_wSelectedVON;
	protected TextWidget m_wSelectedText;
	protected Widget m_wTalkingAmountWidget;
	protected Widget m_wAdditionalSpeakersWidget;
	protected RichTextWidget m_wAdditionalSpeakersText;
	
	protected const string LABEL_ROLE_FORMAT = "(%1)";

	//------------------------------------------------------------------------------------------------
	//! Count of active incoming transmissions
	int GetActiveTransmissionsCount()
	{
		return m_aTransmissions.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Whether outgoing transmission is currently active
	bool IsCapturingTransmisionActive()
	{
		return m_OutTransmission != null && m_OutTransmission.m_bIsActive;
	}

	//------------------------------------------------------------------------------------------------
	//! VONComponent event
	event void OnCapture(BaseTransceiver transmitter)
	{
		if (!m_wRoot)
			return;

		int frequency;

		if (transmitter)	// can be null when using direct speech
			frequency = transmitter.GetFrequency();

		// update only when first activation / device changed / frequency changed
		if (m_OutTransmission.m_bForceUpdate == true
			|| m_OutTransmission.m_bIsActive == false
			|| m_OutTransmission.m_RadioTransceiver != transmitter
			|| (transmitter && m_OutTransmission.m_fFrequency != transmitter.GetFrequency())
		)
		{
			UpdateTransmission(m_OutTransmission, transmitter, frequency, false);
		}

		m_OutTransmission.m_bIsActive = true;
		m_OutTransmission.m_fActiveTimeout = 0;
	}

	protected Widget GetWidget()
	{
		Widget widget;
		int iWidgetIdx = 0;

		while (iWidgetIdx < m_iTransmissionSlots)
		{
			widget = m_aWidgetsIncomingVON.Get(iWidgetIdx);
			if (!widget.IsVisible())
				return widget;

			iWidgetIdx++;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! VONComponent event
	event void OnReceive(int playerId, bool isSenderEditor, BaseTransceiver receiver, int frequency, float quality)
	{
		if (!m_wRoot || m_bIsVONUIDisabled) // ignore receiving transmissions if VON UI is off
			return;

		// Check if there is an active transmission from given player
		TransmissionData pTransmission = m_aTransmissionMap.Get(playerId);

		// Active transmission from the player not found, create it
		if (!pTransmission)
		{
			if (!receiver && m_bIsVONDirectDisabled)	// direct UI off
				return;

			// No free widget
			if (!GetWidget())
			{
				//Insert new widget to display other transmissions as number
				pTransmission = new TransmissionData(m_wAdditionalSpeakersWidget, playerId);
				m_aAdditionalSpeakers.Insert(pTransmission);
				m_wAdditionalSpeakersText.SetText("+" + m_aAdditionalSpeakers.Count().ToString());
				m_wAdditionalSpeakersWidget.SetVisible(true);
				m_wAdditionalSpeakersWidget.SetOpacity(1);
				pTransmission.m_bIsAdditional = true;
				pTransmission.m_bVisible = true;
			}
			else
			{
				pTransmission = new TransmissionData(GetWidget(), playerId);
				pTransmission.m_bIsAdditional = false;
			}

			pTransmission.m_fQuality = quality;

			m_aTransmissions.Insert(pTransmission);
			m_aTransmissionMap.Insert(playerId, pTransmission);
		}

		// update only when first activation / device changed / frequency changed
		if (pTransmission.m_bIsActive == false
			|| pTransmission.m_RadioTransceiver != receiver
			|| (receiver && pTransmission.m_fFrequency != frequency)
		)
		{
			pTransmission.m_bIsSenderEditor = isSenderEditor;
			bool filtered = UpdateTransmission(pTransmission, receiver, frequency, true);
			if (!filtered)
			{
				pTransmission.HideTransmission();
				return;
			}
			SCR_LogitechLEDManager.ActivateState(ELogitechLEDState.COMMS);
		}

		pTransmission.m_bIsActive = true;
		pTransmission.m_fActiveTimeout = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Update transmission data
	//! \param TransmissionData is the subject
	//! \param radioTransceiver is the used transceiver for the transmission
	//! \param IsReceiving is true when receiving transmission, false when transmitting
	//! \param isAdditionalSpeaker is true when all incomming transmission widgets are full
	//! \return false if the transimission is filtered out to not be visible
	protected bool UpdateTransmission(TransmissionData data, BaseTransceiver radioTransceiver, int frequency, bool IsReceiving)
	{
		data.m_RadioTransceiver = radioTransceiver;
		data.m_bForceUpdate = false;

		if (!radioTransceiver && m_bIsVONDirectDisabled && IsReceiving)	// can happen when existing RADIO transmission switches to direct
			return false;

		// faction
		Faction playerFaction;
		int controlledID = GetGame().GetPlayerController().GetPlayerId();

		SCR_FactionManager factionMgr = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (controlledID && factionMgr)
		{
			playerFaction = factionMgr.SGetPlayerFaction(controlledID);
			data.m_Faction = factionMgr.SGetPlayerFaction(data.m_iPlayerID);
		}
		
		bool enemyTransmission;
		if (IsReceiving && playerFaction && playerFaction.IsFactionEnemy(data.m_Faction) && !m_bShowEnemyNames)       // enemy transmission case
		{
			if (!radioTransceiver)
				return false;           // if direct ignore
			
			enemyTransmission = true;
 	  	}

		if (!data.m_wBaseWidget)
			return false;

		if (data.m_bIsAdditional)
			return true;

		string sDeviceIcon;
		data.m_bIsAnimating = true;		// start anim
		data.m_wBaseWidget.SetVisible(true);
		data.m_Widgets.m_wIcon.SetVisible(true);
		data.m_Widgets.m_wMicFrame.SetVisible(false);
		data.m_Widgets.m_wChannelFrame.SetVisible(false);
		data.m_Widgets.m_wSeparator.SetVisible(false);
		data.m_Widgets.m_wPlatformImage.SetVisible(false);
		data.m_Widgets.m_wPlatformImageGlow.SetVisible(false);
		data.m_Widgets.m_wRecievers.SetVisible(false);
		data.m_Widgets.m_wNosignal.SetVisible(false);

		// radio
		if (radioTransceiver)
		{
			if (radioTransceiver.IsMuted())
			{
				sDeviceIcon = ICON_RADIO_MUTED;
			}
			else
			{
				sDeviceIcon = ICON_RADIO;
			}

			float adjustedFreq;
			data.m_fFrequency = frequency;
			adjustedFreq = Math.Round(data.m_fFrequency * 0.1) / 100;
			data.m_Widgets.m_wFrequency.SetText(adjustedFreq.ToString() + " " + LABEL_FREQUENCY_UNITS);
			data.m_Widgets.m_wFrequency.SetVisible(!data.m_bIsSenderEditor || frequency != 0);
			
			if (!data.m_bIsSenderEditor && adjustedFreq == 0) 
				Print("SCR_VonDisplay: Incoming frequency 0 | base: " + frequency + " | adjusted: " + adjustedFreq, LogLevel.WARNING);
		}
		// direct speech
		else
		{
			data.m_Widgets.m_wFrequency.SetVisible(false);
			sDeviceIcon = ICON_DIRECT_SPEECH;
		}

		if (sDeviceIcon != string.Empty)
		{
			data.m_Widgets.m_wIcon.LoadImageFromSet(0, m_sImageSet, sDeviceIcon);
			data.m_Widgets.m_wIconBackground.LoadImageFromSet(0, m_sImageSetGlow, sDeviceIcon);
		}

		data.m_Widgets.m_wGameMaster.SetVisible(data.m_bIsSenderEditor);

		// incoming
		if (IsReceiving)
		{
			data.m_Widgets.m_wIcon.SetColor(COLOR_WHITE);
			data.m_Widgets.m_wGameMaster.SetColor(COLOR_WHITE);

			//always show the player name, keep the check if we need it again
			if (!enemyTransmission || m_bShowEnemyNames)
			{			
				data.m_Entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(data.m_iPlayerID);
				SCR_PossessingManagerComponent possMgr =  SCR_PossessingManagerComponent.GetInstance();
				if (data.m_Entity && possMgr && possMgr.IsPossessing(data.m_iPlayerID))		// if possessing, use name of the possessed character instead of player
				{					
					SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(data.m_Entity.FindComponent(SCR_CharacterIdentityComponent));
					if (scrCharIdentity)
					{
						string name;
						array<string> nameParams = {};
						scrCharIdentity.GetFormattedFullName(name, nameParams);
						data.m_Widgets.m_wName.SetTextFormat(name, nameParams[0], nameParams[1], nameParams[2])
					}
					else	// char might not be using scripted identity component
					{
						CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(data.m_Entity.FindComponent(CharacterIdentityComponent));
						if (charIdentity && charIdentity.GetIdentity())
							data.m_Widgets.m_wName.SetText(charIdentity.GetIdentity().GetName());	 
						else 
							data.m_Widgets.m_wName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(data.m_iPlayerID));
					}
				}
				else
					data.m_Widgets.m_wName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(data.m_iPlayerID));
			}
			else
			{
				data.m_Widgets.m_wName.SetText(LABEL_UNKNOWN_SOURCE);
			}

			data.m_Widgets.m_wName.SetVisible(true);

			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageTo(data.m_iPlayerID, data.m_Widgets.m_wPlatformImage, data.m_Widgets.m_wPlatformImageGlow);

			bool sameFaction = playerFaction == data.m_Faction && !data.m_bIsSenderEditor;
			if (!sameFaction)
			{
				data.m_Widgets.m_wRole.SetVisible(false);
				data.m_Widgets.m_wSquadLeaderIcon.SetVisible(false);
				return true; // if this isn't a member of your team then you shouldn't see their roles, but you should still see them
			}

			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupsManager)
				return false; // this will completly hide this VoN entry, but if this is the case, then there are probably more problems at hand

			bool isSquadLeader;
			SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(data.m_iPlayerID);
			if (playerGroup)
			{
				data.m_Widgets.m_wRole.SetTextFormat(LABEL_ROLE_FORMAT, playerGroup.GetGroupRoleName());

				SCR_Faction faction = SCR_Faction.Cast(data.m_Faction);
				isSquadLeader = data.m_iPlayerID == playerGroup.GetLeaderID();
				
				if (faction && data.m_iPlayerID == faction.GetCommanderId())
				{
					data.m_Widgets.m_wRole.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));
					data.m_Widgets.m_wSquadLeaderIcon.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));
				}
				else
				{
					data.m_Widgets.m_wRole.SetColor(COLOR_WHITE);
					data.m_Widgets.m_wSquadLeaderIcon.SetColor(COLOR_WHITE);
				}
			}

			data.m_Widgets.m_wRole.SetVisible(true);
			data.m_Widgets.m_wSquadLeaderIcon.SetVisible(isSquadLeader);
		}
		else	// outgoing
		{
			data.m_bIsSenderEditor = false;
			SCR_EditorManagerCore editorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorCore)
			{
				SCR_EditorManagerEntity editor = editorCore.GetEditorManager();
				if (editor)
					data.m_bIsSenderEditor = editor.IsOpened();
			}
	
			data.m_Widgets.m_wName.SetText(string.Empty);
			data.m_Widgets.m_wName.SetVisible(false);
			data.m_Widgets.m_wRole.SetText(string.Empty);
			data.m_Widgets.m_wRole.SetVisible(false);
			data.m_Widgets.m_wSquadLeaderIcon.SetVisible(false);

			if (radioTransceiver)	// direct vs radio
			{
				data.m_Widgets.m_wMicFrame.SetVisible(m_bIsChannelToggled);

				if (SCR_Faction.Cast(playerFaction))	// show platoon
				{
					int factionHQFrequency = SCR_Faction.Cast(playerFaction).GetFactionRadioFrequency();
					if (factionHQFrequency == frequency)
					{
						data.m_Widgets.m_wChannelText.SetText(LABEL_CHANNEL_PLATOON);
						data.m_Widgets.m_wChannelFrame.SetVisible(true);
					}
				}

				SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
				if (groupTaskManager && groupTaskManager.IsEnabledAssigningFrequencies() && playerFaction)
				{
					SCR_Task task = groupTaskManager.GetTaskByFrequency(playerFaction, frequency);
					if (task)
					{
						data.m_Widgets.m_wChannelText.SetText(groupTaskManager.GetTaskTranslatedName(task));
						data.m_Widgets.m_wChannelFrame.SetVisible(true);
					}
				}

				if (radioTransceiver.IsMuted())
				{
					data.m_Widgets.m_wIcon.SetColor(Color.FromInt(GUIColors.RED.PackToInt()));
				}
				else
				{
					data.m_Widgets.m_wIcon.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));
				}
				
				data.m_Widgets.m_wGameMaster.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));

				PlayerController controller = GetGame().GetPlayerController();
				data.m_Entity = controller.GetControlledEntity();
				if (controller)
					data.m_Faction = factionMgr.SGetPlayerFaction(controller.GetPlayerId());

				if (data.m_Entity && data.m_Faction)
				{
					// Refactor note: Reference to specific game mode is not desired
					SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.GetInstance();
					if (gameModeCampaign && !gameModeCampaign.GetBaseManager().IsEntityInFactionRadioSignal(data.m_Entity, data.m_Faction))
					{
						data.m_Widgets.m_wRecievers.SetVisible(true);
						data.m_Widgets.m_wNosignal.SetVisible(true);
						data.m_Widgets.m_wRecievers.SetColor(Color.FromInt(GUIColors.DARK_GRAY.PackToInt()));
						data.m_Widgets.m_wNosignal.SetColor(Color.FromInt(GUIColors.DARK_GRAY.PackToInt()));
					}
					else
					{
						data.m_Widgets.m_wRecievers.SetVisible(false);
						data.m_Widgets.m_wNosignal.SetVisible(false);
					}
				}
			}
			else
			{
				data.m_Widgets.m_wMicFrame.SetVisible(m_bIsDirectToggled);
				data.m_Widgets.m_wIcon.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));
			}
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fade in/out elements
	//! \param TransmissionData is the subject
	//! \param timeSlice is the OnFrame slice
	protected void OpacityFade(TransmissionData data, float timeSlice, bool isAdditional = false)
	{
		if (!data.m_wBaseWidget)
			return;

		if (data.m_bIsActive)
		{
			data.m_wBaseWidget.SetVisible(true);
			data.m_bVisible = true;

			data.m_fAlpha = Math.Lerp(data.m_fAlpha, 1, FADEIN_SPEED * timeSlice);
			if (data.m_fAlpha > 0.99)
			{
				data.m_fAlpha = 1;
				data.m_bIsAnimating = false;
			}
		}
		else
		{
			data.m_fAlpha = Math.Lerp(data.m_fAlpha, 0, FADEOUT_SPEED * timeSlice);
			if (data.m_fAlpha < 0.01)
			{
				if (isAdditional)
				{
					data.m_fAlpha = 0;
					data.m_bIsActive = false;
					data.m_bIsAnimating = false;
					data.m_fActiveTimeout = 0;
					data.m_bVisible = false;
				}
				else
				{
					data.HideTransmission();
				}

			}
		}

		if (!isAdditional)
			data.m_wBaseWidget.SetOpacity(data.m_fAlpha);
	}

	//------------------------------------------------------------------------------------------------
	//! Show hint displaying which VON method was selected
	void ShowSelectedVONHint(SCR_VONEntry entry)
	{
		if (entry.GetVONMethod() == ECommMethod.DIRECT)
		{
			m_wSelectedVON.SetText("#AR-VON_DirectTalk");
			SetHintIcon(false);
		}
		else 
		{
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (!radioEntry || !m_wSelectedHint || !radioEntry.GetUIInfo())
			return;
						
			const string formatText = "#AR-VON_ChannelHint"; 
			m_wSelectedVON.SetTextFormat(formatText, radioEntry.GetUIInfo().GetName(), radioEntry.GetTransceiverNumber(), radioEntry.GetDisplayText());

			SetHintIcon(true);
		}

		m_wSelectedText.SetVisible(true);
		
		m_wSelectedHint.SetOpacity(1);
		GetGame().GetCallqueue().Remove(FadeSelectedVONHint);								// in case of multiple selections
		GetGame().GetCallqueue().CallLater(FadeSelectedVONHint, SELECTED_HINT_FADE_SPEED);	// start fading after a certain display time

	}
	
	//------------------------------------------------------------------------------------------------
	//! Show hint displaying which VON method was selected
	//! \param isServerDisabled determines type of disable
	void ShowSelectedVONDisabledHint(bool isServerDisabled = false)
	{
		if (isServerDisabled)
		{
			m_wSelectedHintIcon.LoadImageFromSet(0, m_sImageSet, ICON_SERVER_DISABLE_HINT);
			m_wSelectedHintIconGlow.LoadImageFromSet(0, m_sImageSetGlow, ICON_SERVER_DISABLE_HINT);
		}
		else 
		{
			m_wSelectedHintIcon.LoadImageFromSet(0, m_sImageSet, ICON_RADIO_HINT);
			m_wSelectedHintIconGlow.LoadImageFromSet(0, m_sImageSetGlow, ICON_RADIO_HINT);
		}
		
		m_wSelectedHintIcon.SetColor(Color.FromInt(GUIColors.RED.PackToInt()));
		
		m_wSelectedVON.SetText("");
		m_wSelectedText.SetVisible(false);
		
		m_wSelectedHint.SetOpacity(1);
		GetGame().GetCallqueue().Remove(FadeSelectedVONHint);								// in case of multiple selections
		GetGame().GetCallqueue().CallLater(FadeSelectedVONHint, SELECTED_HINT_FADE_SPEED);	// start fading after a certain display time
	}

	//------------------------------------------------------------------------------------------------
	protected void FadeSelectedVONHint()
	{
		AnimateWidget.Opacity(m_wSelectedHint, 0, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set VON info hint icon type
	protected void SetHintIcon(bool isRadio)
	{
		if (isRadio)
		{
			m_wSelectedHintIcon.LoadImageFromSet(0, m_sImageSet, ICON_RADIO_HINT);
			m_wSelectedHintIconGlow.LoadImageFromSet(0, m_sImageSetGlow, ICON_RADIO_HINT);
		}
		else 
		{
			m_wSelectedHintIcon.LoadImageFromSet(0, m_sImageSet, ICON_DIRECT_SPEECH);
			m_wSelectedHintIconGlow.LoadImageFromSet(0, m_sImageSetGlow, ICON_DIRECT_SPEECH);
		}
		
		m_wSelectedHintIcon.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_VONController event -> Toggle microphone indication
	protected void OnVONActiveToggled(bool directState, bool channelState)
	{
		m_bIsDirectToggled = directState;
		m_bIsChannelToggled = channelState;

		if (m_OutTransmission)
		{
			m_OutTransmission.m_bForceUpdate = true;
			OnCapture(m_OutTransmission.m_RadioTransceiver);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	protected void OnDestroyed(Instigator killer, IEntity killerEntity)
	{
		if (m_OutTransmission)
			m_OutTransmission.HideTransmission();

		int count = m_aTransmissions.Count();
		for (int i = 0; i < count; i++)
		{
			m_aTransmissions[i].HideTransmission();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize
	protected void InitDisplay()
	{
		m_bIsVONUIDisabled = GetGame().IsVONUIDisabledByServer();
		m_bIsVONDirectDisabled = GetGame().IsVONDirectSpeechUIDisabledByServer();

		m_PlayerController.m_OnDestroyed.Insert(OnDestroyed);

		m_VONController = SCR_VONController.Cast(m_PlayerController.FindComponent(SCR_VONController));
		m_VONController.GetOnVONActiveToggledInvoker().Insert(OnVONActiveToggled);
		m_VONController.SetDisplay(this);	// we set this from here instead of the other way around to avoid load order issues

		m_OutTransmission = new TransmissionData(m_wRoot.FindAnyWidget(WIDGET_TRANSMIT), 0);

		m_wVerticalLayout = m_wRoot.FindAnyWidget(WIDGET_INCOMING);
		m_wAdditionalSpeakersWidget = m_wRoot.FindAnyWidget(WIDGET_OVERFLOW);
		m_wAdditionalSpeakersText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_OVERFLOW_TEXT));

		m_wSelectedHint = m_wRoot.FindAnyWidget(WIDGET_SELECTED_ROOT);
		m_wSelectedHintIcon = ImageWidget.Cast(m_wSelectedHint.FindAnyWidget(WIDGET_SELECTED_ICON));
		m_wSelectedHintIconGlow = ImageWidget.Cast(m_wSelectedHint.FindAnyWidget(WIDGET_SELECTED_ICONGLOW));
		m_wSelectedVON = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SELECTED_VON));
		m_wSelectedText = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SELECTED_TEXT));

		m_SlotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if (!m_SlotHandler)
			return;
		
		m_HUDSlotComponent = m_SlotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;
		
		m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSlotUIResize()
	{
		// Assign it again in case the SlotUIComponent has changed
		m_HUDSlotComponent = m_SlotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;
		
		int height = m_HUDSlotComponent.GetHeight();
		m_iTransmissionSlots = ((int)height / HEIGHT_DIVIDER) - 1;
		if (m_iTransmissionSlots <= 0)
			m_iTransmissionSlots = 1;

		UpdateWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateWidgets()
	{
		int count = m_aWidgetsIncomingVON.Count();

		if (count == m_iTransmissionSlots)
			return;

		if (count > m_iTransmissionSlots)
		{
			for (int i = count - 1; i > m_iTransmissionSlots; i--)
			{
				Widget w = m_aWidgetsIncomingVON.Get(i);
				if (w)
				{
					w.RemoveFromHierarchy();
					m_aWidgetsIncomingVON.RemoveItem(w);
				}
			}
		}
		else
		{
			for (int i = count; i < m_iTransmissionSlots; i++)
			{
				Widget w = GetGame().GetWorkspace().CreateWidgets(m_sReceivingTransmissionLayout, m_wVerticalLayout);
				if (w)
				{
					w.SetVisible(false);
					m_aWidgetsIncomingVON.Insert(w);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Overrides
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		//Check if the SlotUIComponent is still valid otherwise change to the new one
		if (m_HUDSlotComponent != m_SlotHandler.GetSlotUIComponent())
		{
			if (m_HUDSlotComponent)
				m_HUDSlotComponent.GetOnResize().Remove(OnSlotUIResize);
			
			m_HUDSlotComponent = m_SlotHandler.GetSlotUIComponent();
			if (!m_HUDSlotComponent)
				return;
			
			m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
		}
		
		// update visibility timer
		if (m_OutTransmission.m_bIsActive)
		{
			m_OutTransmission.m_fActiveTimeout += timeSlice;

			if (m_OutTransmission.m_fActiveTimeout > FADEOUT_TIMER_THRESHOLD)
			{
				m_OutTransmission.m_bIsActive = false;
				m_OutTransmission.m_bIsAnimating = true;
			}
		}

		// update fade
		if (m_OutTransmission.m_bIsAnimating)
			OpacityFade(m_OutTransmission, timeSlice);

		// update incoming transmissions
		if (m_aTransmissions.IsEmpty())
		{
			return;
		}

		TransmissionData pTransmission;
		int count = m_aTransmissions.Count() - 1;

		for (int i = count; i >= 0; i--)
		{
			if (i > m_aTransmissions.Count() - 1)
			{
				count -1;
				continue;
			}

			pTransmission = m_aTransmissions[i];

			bool isAdditional = m_aAdditionalSpeakers.Contains(pTransmission);

			// update visibility timer
			if (pTransmission.m_bIsActive)
			{
				pTransmission.m_fActiveTimeout += timeSlice;

				if (pTransmission.m_fActiveTimeout > FADEOUT_TIMER_THRESHOLD)
				{
					pTransmission.m_bIsActive = false;
					pTransmission.m_bIsAnimating = true;
				}
			}

			// update fade
			if (pTransmission.m_bIsAnimating)
				OpacityFade(pTransmission, timeSlice, isAdditional);

			// remove faded transmissions
			if (!pTransmission.m_bVisible)
			{
				if (pTransmission.m_iPlayerID)
					m_aTransmissionMap.Remove(pTransmission.m_iPlayerID);

				m_aTransmissions.Remove(i);

				if (isAdditional)
				{
					m_aAdditionalSpeakers.Remove(m_aAdditionalSpeakers.Find(pTransmission));
					m_wAdditionalSpeakersText.SetText("+" + m_aAdditionalSpeakers.Count().ToString());

					if (m_aAdditionalSpeakers.IsEmpty())
						m_wAdditionalSpeakersWidget.SetVisible(false);
				}

				if (m_aAdditionalSpeakers.Count() > 0 && (m_aTransmissions.Count() - m_aAdditionalSpeakers.Count()) < m_aWidgetsIncomingVON.Count())
				{
					m_aAdditionalSpeakers[0].m_bIsActive = false;
					m_aTransmissionMap.Remove(m_aAdditionalSpeakers[0].m_iPlayerID);
					m_aTransmissions.Remove(m_aTransmissions.Find(m_aAdditionalSpeakers[0]));

					OnReceive(m_aAdditionalSpeakers[0].m_iPlayerID, m_aAdditionalSpeakers[0].m_bIsSenderEditor, m_aAdditionalSpeakers[0].m_RadioTransceiver, m_aAdditionalSpeakers[0].m_fFrequency, m_aAdditionalSpeakers[0].m_fQuality);

					m_aAdditionalSpeakers.Remove(0);
					m_wAdditionalSpeakersText.SetText("+" + m_aAdditionalSpeakers.Count().ToString());

					if (m_aAdditionalSpeakers.Count() == 0)
						m_wAdditionalSpeakersWidget.SetVisible(false);
				}

				count - 1;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		m_PlayerManager = GetGame().GetPlayerManager();

		return m_PlayerManager != null;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		InitDisplay();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		if (m_OutTransmission)
			m_OutTransmission.HideTransmission();

		int count = m_aTransmissions.Count();
		for (int i = 0; i < count; i++)
		{
			m_aTransmissions[i].HideTransmission();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_PlayerController)
			m_PlayerController.m_OnDestroyed.Remove(OnDestroyed);

		if (m_VONController)
			m_VONController.GetOnVONActiveToggledInvoker().Remove(OnVONActiveToggled);
	}
}
