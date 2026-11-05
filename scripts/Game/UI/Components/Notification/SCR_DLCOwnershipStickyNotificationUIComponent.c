class SCR_DLCOwnershipStickyNotificationUIComponent : SCR_StickyNotificationUIComponent
{
	[Attribute("#AR-DLC_Vehicle_Blocked", desc: "Warning shown when player tries to select a vehicle from DLC content they don't own", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLockedVehicle;

	[Attribute("#AR-DLC_Item_Blocked", desc: "Warning shown when player tries to select an item from DLC content they don't own", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLockedItem;

	[Attribute("#AR-DLC_Scenario_Blocked", desc: "Warning shown when player tries to select a scenario from DLC content they don't own", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLockedScenario;

	[Attribute("#AR-DLC_Object_Blocked", desc: "Warning shown when player tries to select an object from DLC content they don't own", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLockedObject;
	
	SCR_GroupsManagerComponent m_GroupManager;
	SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;

	protected SCR_EDLCInteractionType m_eLastInteraction = SCR_EDLCInteractionType.NONE;

	//How long the group invite notification exists before it gets deleted, Time in seconds
	static const int NOTIFICATION_DELETE_TIME = 20;

	protected static const float PLATFORM_ICON_SIZE = 2;
	//------------------------------------------------------------------------------------------------
	//Shows the sticky notification including player who invited you and the group callsign
	protected void ShowDLCOwnershipNotification(SCR_EDLCInteractionType interaction = SCR_EDLCInteractionType.NONE, bool isInit = false)
	{
		if (interaction != SCR_EDLCInteractionType.NONE)
			m_eLastInteraction = interaction;

		m_Text.SetTextFormat(GetLockedTextForInteraction(m_eLastInteraction));

		SetStickyActive(true, !isInit);

		GetGame().GetInputManager().RemoveActionListener("OpenNotificationDialog", EActionTrigger.DOWN, OnDialog);
		GetGame().GetInputManager().AddActionListener("OpenNotificationDialog", EActionTrigger.DOWN, OnDialog);

		//Hiding the notification after a certain amount of time has passed
		GetGame().GetCallqueue().Remove(HideNotification);
		GetGame().GetCallqueue().CallLater(HideNotification, NOTIFICATION_DELETE_TIME * 1000, false);
	}
	
	protected void OnDLCOwnershipFalse(SCR_EDLCInteractionType interaction = SCR_EDLCInteractionType.NONE)
	{
		ShowDLCOwnershipNotification(interaction, false);
	}
	
		//------------------------------------------------------------------------------------------------
	//! Picks the locked-content message matching the type of DLC interaction that was attempted
	protected LocalizedString GetLockedTextForInteraction(SCR_EDLCInteractionType interaction)
	{
		switch (interaction)
		{
			case SCR_EDLCInteractionType.VEHICLE_DRIVER:
				return m_sLockedVehicle;

			case SCR_EDLCInteractionType.ITEM_EQUIP:
				return m_sLockedItem;

			case SCR_EDLCInteractionType.SCENARIO:
				return m_sLockedScenario;

			case SCR_EDLCInteractionType.OBJECT:
				return m_sLockedObject;
		}

		return m_sLockedObject;
	}

	//------------------------------------------------------------------------------------------------
	protected void HideNotification()
	{
		SetStickyActive(false);
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		GetGame().GetInputManager().RemoveActionListener("OpenNotificationDialog", EActionTrigger.DOWN, OnDialog);
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{	
		super.OnInit(notificationLog);	
		SetVisible(false);
		
		World world = GetGame().GetWorld();
		if (!world)
			return;
		
		SCR_DLCOwnershipSystem ownershipSystem = SCR_DLCOwnershipSystem.Cast(world.FindSystem(SCR_DLCOwnershipSystem));
		if (!ownershipSystem)
			return;
		
		ownershipSystem.GetOnOwnershipNegative().Insert(OnDLCOwnershipFalse);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDialog()
	{
		if (SCR_ConfigurableDialogUi.GetCurrentDialogTag() == "dlc_not_owned")
			return;

		new SCR_DLCOwnershipDialog();
		HideNotification();
	}
	 
	//------------------------------------------------------------------------------------------------
	protected override void OnDestroy()
	{
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		GetGame().GetCallqueue().Remove(HideNotification);
		GetGame().GetInputManager().RemoveActionListener("OpenNotificationDialog", EActionTrigger.DOWN, OnDialog);

		World world = GetGame().GetWorld();
		if (!world)
			return;

		SCR_DLCOwnershipSystem ownershipSystem = SCR_DLCOwnershipSystem.Cast(world.FindSystem(SCR_DLCOwnershipSystem));
		if (ownershipSystem)
			ownershipSystem.GetOnOwnershipNegative().Remove(OnDLCOwnershipFalse);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback used to reload the text of the notification in case that language changes
	protected void OnUserSettingsChanged()
	{
		ShowDLCOwnershipNotification();
	}
}
