class SCR_TimePresetsAttributeUIComponent: SCR_ButtonBoxAttributeUIComponent
{
	protected bool m_bIsReseting = false;
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{						
		if (!var)
			return;
		
		if (!m_bButtonValueInitCalled)
			super.SetFromVar(var);
		else //Time slider sets is changed thus setting the button disabled. This will set it enabled again on reset.
			GetGame().GetCallqueue().CallLater(DelayedReset, 1, false, var);
	}
	
	protected void DelayedReset(SCR_BaseEditorAttributeVar var)
	{
		m_bIsReseting = true;
		m_ToolBoxComponent.SetItemSelected(var.GetInt(), true);
		m_bIsReseting = false;
	}
	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{	
		if (m_bIsReseting)
			return false;
		else 
			return super.OnChangeInternal(w, x, y, finished);
	}
};