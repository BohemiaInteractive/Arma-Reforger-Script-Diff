class SCR_JoinRequestEntry : SCR_SubMenuBase
{
	protected int m_iPlayerID;
	protected SCR_RequestToJoinSubmenu m_ParentMenu;
	protected SCR_InputButtonComponent m_RefuseButton;
	protected SCR_InputButtonComponent m_AcceptButton;

	//------------------------------------------------------------------------------------------------
	//! \param[in] menu
	void SetParentMenu(SCR_RequestToJoinSubmenu menu)
	{
		m_ParentMenu = menu;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetPlayerID(int value)
	{
		m_iPlayerID = value;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPlayerID()
	{
		return m_iPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to accept the reqeust
	protected void AcceptToJoinPrivateGroup()
	{
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(GetGame().GetPlayerController().GetPlayerId());
		if (!playerGroupComponent)
			return;

		playerGroupComponent.AcceptJoinPrivateGroup(GetPlayerID(), true);

		if (m_ParentMenu)
			GetGame().GetCallqueue().CallLater(m_ParentMenu.InvokeOnJoinRequestRespond, 200); //call later because requesters are updated before array is clear
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to reject the reqeust
	protected void RefuseJoinPrivateGroup()
	{
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(GetGame().GetPlayerController().GetPlayerId());
		if (!playerGroupComponent)
			return;

		playerGroupComponent.AcceptJoinPrivateGroup(GetPlayerID(), false);

		if (m_ParentMenu)
			GetGame().GetCallqueue().CallLater(m_ParentMenu.InvokeOnJoinRequestRespond, 200); //call later because requesters are updated before array is clear
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();

		m_RefuseButton.SetEnabled(true);
		m_AcceptButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();

		m_RefuseButton.SetEnabled(false);
		m_AcceptButton.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Method triggered manually after widget creation process was finished
	//! \param[in] w widget on which this handler is initalized
	void Init(Widget w)
	{
		SCR_ModularButtonComponent buttonComp = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
		if (!buttonComp)
			return;

		buttonComp.m_OnFocus.Insert(OnMenuFocusGained);
		buttonComp.m_OnFocusLost.Insert(OnMenuFocusLost);
		OnTabShow();
	}

	//------------------------------------------------------------------------------------------------
	//! Initialization of the component, after it was attached to the widet.
	//! Note: at this point other components on this widget may not exist yet, and for interacting with them use Init(w)
	//! \param[in] w widget to which this handler was attached
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		ButtonWidget refuseWidget = ButtonWidget.Cast(w.FindAnyWidget("Refuse"));
		if (!refuseWidget)
			return;

		ButtonWidget acceptWidget = ButtonWidget.Cast(w.FindAnyWidget("Accept"));
		if (!acceptWidget)
			return;

		m_RefuseButton = SCR_InputButtonComponent.Cast(refuseWidget.FindHandler(SCR_InputButtonComponent));
		if (!m_RefuseButton)
			return;

		m_AcceptButton = SCR_InputButtonComponent.Cast(acceptWidget.FindHandler(SCR_InputButtonComponent));
		if (!m_AcceptButton)
			return;

		m_RefuseButton.SetEnabled(false);
		m_AcceptButton.SetEnabled(false);
		m_RefuseButton.m_OnActivated.Insert(RefuseJoinPrivateGroup);
		m_AcceptButton.m_OnActivated.Insert(AcceptToJoinPrivateGroup);
	}
}
