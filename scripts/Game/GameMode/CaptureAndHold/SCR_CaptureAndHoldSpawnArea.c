[EntityEditorProps(category: "GameScripted/GameMode/CaptureAndHold", description: "This area trigger detects enemies not belonging to assigned faction, and raises callbacks which allow penalizing abusive behaviour.")]
class SCR_CaptureAndHoldSpawnAreaClass : SCR_SpawnAreaClass
{
}

//! This area trigger detects enemies not belonging to assigned faction,
//! and raises callbacks which allow penalizing abusive behaviour.
class SCR_CaptureAndHoldSpawnArea : SCR_SpawnArea
{
	[Attribute("#AR-CAH-SpawnArea_LocalPlayer_Alert_Title", UIWidgets.EditBox, "The string that is shown as the main message to the local player upon entering this area as enemy faction.")]
	protected LocalizedString m_sLocalPlayerAlertTitle;

	[Attribute("#AR-CAH-SpawnArea_LocalPlayer_Alert_Text", UIWidgets.EditBox, "The additional string that is shown as the main message to the local player upon entering this area as enemy faction.")]
	protected LocalizedString m_sLocalPlayerAlertText;

	[Attribute("10.0", UIWidgets.EditBox, "Time spent in the are in seconds for an enemy to be killed after.")]
	protected float m_fPenaltyTime;

	//! Last entity stored as the "local offender" or null if none.
	protected IEntity m_pLastLocalEntity;

	//! Authority map for time spent in the trigger for individual entities (offending ones)
	protected ref map<IEntity, float> m_mTimeStamps;

	//! RplComponent attached to this are or null if none
	protected RplComponent m_pRplComponent;

	//------------------------------------------------------------------------------------------------
	//! Initializes and registers this area into parent manager.
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		// Supress messages out of playmode, order of things is not quite guaranteed here
		if (!GetGame().InPlayMode())
			return;

		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_pRplComponent)
			Print("SCR_CaptureAndHoldSpawnArea cannot find RplComponent! Functionality will be limited!", LogLevel.WARNING);

		// Register self in manager
		SCR_CaptureAndHoldManager parentManager = SCR_CaptureAndHoldManager.GetAreaManager();
		if (!parentManager)
		{
			Print("SCR_CaptureAndHoldSpawnArea cannot find SCR_CaptureAndHoldManager! Functionality might be limited!", LogLevel.WARNING);
			return;
		}

		parentManager.RegisterSpawnArea(this);

		// Authority only
		if (!m_pRplComponent || !m_pRplComponent.IsMaster())
			return;

		m_mTimeStamps = new map<IEntity, float>();
		SetEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	//! Iterate through occupant entities and filter those of opposite faction.
	//! Punish enemy occupants by killing them in a preset duration.
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		// Authority only
		if (!m_pRplComponent || !m_pRplComponent.IsMaster() || !m_mTimeStamps)
			return;

		array<IEntity> toRemove = {};
		float now = GetWorld().GetWorldTime();
		ChimeraCharacter character;
		CharacterControllerComponent controller;
		foreach (IEntity occupant, float timeStamp : m_mTimeStamps)
		{
			if (now >= timeStamp + (m_fPenaltyTime * 1000.0))
			{
				character = ChimeraCharacter.Cast(occupant);
				if (!character) // ???
				{
					toRemove.Insert(occupant);
					continue;
				}
				
				controller = character.GetCharacterController();
				if (!controller.IsDead())
				{
					controller.ForceDeath();
					toRemove.Insert(character);
				}
			}
		}

		// Remove killer occupants
		foreach (IEntity occupant : toRemove)
		{
			if (!occupant)
				continue;

			if (m_mTimeStamps.Contains(occupant))
				m_mTimeStamps.Remove(occupant);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called when a character enters this area.
	protected override event void OnCharacterEnter(IEntity character, bool isFriendly)
	{
		super.OnCharacterEnter(character, isFriendly);

		// Raise callback for local player
		if (character == SCR_PlayerController.GetLocalControlledEntity())
			OnLocalPlayerEnter(character, isFriendly);

		// Authority only
		if (!m_pRplComponent || !m_pRplComponent.IsMaster())
			return;

		// Enemy occupants only
		if (isFriendly)
			return;

		if (!m_mTimeStamps.Contains(character))
			m_mTimeStamps.Insert(character, GetWorld().GetWorldTime());
	}

	//------------------------------------------------------------------------------------------------
	//! Called when the local character enters this area.
	protected event void OnLocalPlayerEnter(IEntity character, bool isFriendly)
	{
		// Store the local player entity,
		// death occuring and similar things come unexpectedly,
		// this way we can always make sure we clear the previous state
		m_pLastLocalEntity = character;

		if (isFriendly)
			return;

		SCR_PopUpNotification popupNotifications = SCR_PopUpNotification.GetInstance();
		if (!popupNotifications)
			return;

		popupNotifications.PopupMsg(text: m_sLocalPlayerAlertTitle, duration: m_fPenaltyTime, text2: m_sLocalPlayerAlertText);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when a character leaves this area.
	protected override event void OnCharacterExit(IEntity character, bool isFriendly)
	{
		super.OnCharacterExit(character, isFriendly);

		// Raise callback for local player
		if (character == m_pLastLocalEntity || character == SCR_PlayerController.GetLocalControlledEntity())
		{
			m_pLastLocalEntity = null;
			OnLocalPlayerExit(character, isFriendly);
		}

		// Authority only
		if (!m_pRplComponent || !m_pRplComponent.IsMaster())
			return;

		// Enemy occupants only
		if (isFriendly)
			return;

		if (m_mTimeStamps.Contains(character))
			m_mTimeStamps.Remove(character);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when the local character leaves this area.
	protected event void OnLocalPlayerExit(IEntity character, bool isFriendly)
	{
		if (isFriendly)
			return;

		SCR_PopUpNotification popupNotifications = SCR_PopUpNotification.GetInstance();
		if (!popupNotifications)
			return;

		if (popupNotifications.GetCurrentMsg())
			popupNotifications.HideCurrentMsg();
	}

	//------------------------------------------------------------------------------------------------
	//! Unregister self from parent manager.
	protected void ~SCR_CaptureAndHoldSpawnArea()
	{
		// Far from ideal, OnDelete would be better

		// Register self in manager
		SCR_CaptureAndHoldManager parentManager = SCR_CaptureAndHoldManager.GetAreaManager();
		if (!parentManager)
			return;

		parentManager.UnregisterSpawnArea(this);
	}
}
