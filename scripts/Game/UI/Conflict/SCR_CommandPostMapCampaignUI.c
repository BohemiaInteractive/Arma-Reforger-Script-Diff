class SCR_CommandPostMapCampaignUI : SCR_MapUIBaseComponent
{
	[Attribute("{E1D446FCFA9DF389}UI/layouts/Campaign/CommandPostMapUI.layout", params: "layout")]
	protected ResourceName m_sBaseElement;

	protected InputManager m_InputManager;

	protected const int MAP_CLOSE_DELAY = 300;

	//------------------------------------------------------------------------------------------------
	protected void InitCommander()
	{
		m_RootWidget = m_MapEntity.GetMapConfig().RootWidgetRef; // Needs to be refreshed here

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_RootWidget);

		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MapEscape", EActionTrigger.DOWN, OnMapEscape);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		InitCommander();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);

		if (m_InputManager)
			m_InputManager.RemoveActionListener("MapEscape", EActionTrigger.DOWN, OnMapEscape);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapEscape()
	{
		// hotfix to suppress the pausemenu display, needs the same delay logic as for opening/closing the map in SCR_MapGadgetComponent
		// after press Escape must first call game->OnMenuOpen() to check if it can open the pausemenu
		GetGame().GetCallqueue().CallLater(OnMapEscapeDelayed, MAP_CLOSE_DELAY, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapEscapeDelayed()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		menuManager.CloseMenuByPreset(ChimeraMenuPreset.CommandPostMapMenu);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);

		m_InputManager.ActivateContext("CommandPostMapContext");
	}
}
