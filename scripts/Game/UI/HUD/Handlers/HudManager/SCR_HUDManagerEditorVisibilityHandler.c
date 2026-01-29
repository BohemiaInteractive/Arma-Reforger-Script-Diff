[BaseContainerProps()]
class SCR_HUDManagerEditorVisibilityHandler : SCR_HUDManagerHandler
{
	[Attribute(desc: "Which HUD Layout should be opened when a player exits Editor.")]
	protected string m_sExitingEditorLayout;
	[Attribute(desc: "Which HUD Layout should be opened when a player enters Editor.")]
	protected string m_sEnteringEditorLayout;

	protected bool m_bIsEditorOpen;

	//------------------------------------------------------------------------------------------------
	protected void OnEditorExit()
	{
		m_bIsEditorOpen = false;

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(m_HUDManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (layoutHandler)
			layoutHandler.ChangeActiveHUDLayout(m_sExitingEditorLayout);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorEnter()
	{
		m_bIsEditorOpen = true;

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(m_HUDManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (layoutHandler)
			layoutHandler.ChangeActiveHUDLayout(m_sEnteringEditorLayout);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCharacterPossesed(IEntity entity)
	{
		if (!m_bIsEditorOpen)
			return;

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(m_HUDManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (layoutHandler)
			layoutHandler.ChangeActiveHUDLayout(m_sExitingEditorLayout);
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(notnull SCR_HUDManagerComponent owner)
	{
		RegisterEvents();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterEvents()
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();

		if (!pc || !editorManager || m_HUDManager.GetInfoDisplayCount() == 0)
			return;

		pc.m_OnBeforePossess.Insert(OnCharacterPossesed);
		editorManager.GetOnOpened().Insert(OnEditorEnter);
		editorManager.GetOnClosed().Insert(OnEditorExit);

		m_HUDManager.RemoveHandlerFromUpdatableHandlers(this);

		if (editorManager.IsOpened())
			OnEditorEnter();
	}
}
