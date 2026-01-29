class SCR_DoubleClickAction: SCR_BaseEditorAction
{
	[Attribute("1", desc: "When enabled, action will not be performed if its shotcut is mouse double-click and mouse moved between two clicks.")]
	protected bool m_bIsDoubleClick;
	
	protected const string LMB_ACTION = "MouseLeft";
	protected const int MAX_CURSOR_DIST_SQ = 100;
	
	protected vector m_vMouseLeftPos;
	
	protected void CachePosition()
	{
		m_vMouseLeftPos = GetMousePosVector();
	}
	protected void OnLMB()
	{
		GetGame().GetCallqueue().CallLater(CachePosition, 1);
	}
	protected vector GetMousePosVector()
	{
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		return Vector(mouseX, mouseY, 0);
	}
	protected bool DidCursorMoveDuringDoubleClick()
	{
		if (!m_bIsDoubleClick || !GetGame().GetInputManager().IsUsingMouseAndKeyboard() || m_vMouseLeftPos == vector.Zero)
			return false;
		
		bool didMove = vector.DistanceSq(m_vMouseLeftPos, GetMousePosVector()) > MAX_CURSOR_DIST_SQ;
		m_vMouseLeftPos = vector.Zero;
		return didMove;
	}
	
	override void AddShortcut(SCR_BaseActionsEditorComponent actionsManager)
	{
		super.AddShortcut(actionsManager);
		
		if (m_ActionsManager)
			GetGame().GetInputManager().AddActionListener(LMB_ACTION, EActionTrigger.DOWN, OnLMB);
	}
	override void RemoveShortcut()
	{
		if (m_ActionsManager)
			GetGame().GetInputManager().RemoveActionListener(LMB_ACTION, EActionTrigger.DOWN, OnLMB);
		
		super.RemoveShortcut();
	}
};
