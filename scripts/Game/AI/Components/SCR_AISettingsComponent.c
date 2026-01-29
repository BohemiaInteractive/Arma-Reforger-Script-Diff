[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AISettingsComponentClass : ScriptComponentClass
{
}

// Obsolete and should be removed eventually
class SCR_AISettingsComponent : ScriptComponent
{
	// rewrite if condition below if adding new lines!!!
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable debug visualization")]
	protected bool m_EnableVisualization;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow movement")]
	bool m_EnableMovement;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow reacting on danger events")]
	bool m_EnableDangerEvents;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow reacting on perceived targets")]
	bool m_EnablePerception;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow shooting and attacking in general")]
	bool m_EnableAttack;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow finding and taking cover")]
	bool m_EnableTakeCover;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow aiming and gestures in general")]
	bool m_EnableLooking;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow sending messages")]
	bool m_EnableCommunication;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow artificial aiming error for AI")]
	bool m_EnableAimError;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Allow leader to stop when formation is deformed")]
	bool m_EnableLeaderStop;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Title for world description")]
	protected string m_sDescriptionTitle;

	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "World description")]
	protected string m_sDescriptionText;
	
 	private Widget m_wHint;
	private Widget m_wOverlay;

	protected SCR_AIDebugInfoComponent m_UIComponent;
	protected static SCR_AISettingsComponent m_sInstance;

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_AISettingsComponent GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void InitVisualization()
	{
		if (!m_EnableVisualization)
			return;
		
		GetGame().GetCallqueue().CallLater(UpdateDebug,10,true);

		// Create debug layout
		WorkspaceWidget workspace = GetGame().GetWorkspace(); 
		m_wOverlay = workspace.CreateWidgets("{7C4EE3995E17845B}UI/layouts/Debug/AIInfoDebug.layout");
		if (m_wOverlay.GetNumHandlers() > 0)
			m_UIComponent = SCR_AIDebugInfoComponent.Cast(m_wOverlay.GetHandler(0));
		
		if (m_sDescriptionText != string.Empty || m_sDescriptionTitle != string.Empty)
			SCR_HintManagerComponent.ShowCustomHint(m_sDescriptionText, m_sDescriptionTitle, 12);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void UpdateDebug()
	{
		if (!m_EnableVisualization)
			return;
	
		m_UIComponent.UpdateUI();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_sInstance = this;
#ifdef AI_DEBUG
		if (m_EnableVisualization)
			InitVisualization();
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_AISettingsComponent()
	{
		if (m_wOverlay)
			m_wOverlay.RemoveFromHierarchy();
		
		if (m_wHint)
			m_wHint.RemoveFromHierarchy();
	}
}
