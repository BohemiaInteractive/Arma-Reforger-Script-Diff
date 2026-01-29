//! @ingroup Editor_UI Editor_UI_Components

class SCR_ActionTooltipEditorUIComponent : SCR_BaseTooltipEditorUIComponent
{
	[Attribute()]
	protected string m_sShortcutWidgetName;
	
	//------------------------------------------------------------------------------------------------
	override bool SetTooltip(SCR_UIInfo info, Managed instance = null)
	{
		TextWidget shortcutWidget = TextWidget.Cast(GetWidget().FindAnyWidget(m_sShortcutWidgetName));
		if (shortcutWidget)
		{
			string shortcut;
			SCR_BaseEditorAction action = SCR_BaseEditorAction.Cast(instance);
			if (action)
			{
				//--- Instance action
				shortcut = action.GetShortcut();
			}
			else
			{
				//--- UI info action
				SCR_InputActionUIInfo actionInfo = SCR_InputActionUIInfo.Cast(info);
				if (actionInfo)
					shortcut = actionInfo.GetActionName();
			}
		
			if (!shortcut.IsEmpty())
			{
				shortcutWidget.SetText(string.Format("<action name='%1'/>", shortcut));
				shortcutWidget.SetVisible(true);
			}
			else
			{
				shortcutWidget.SetVisible(false);
			}
		}
		
		return super.SetTooltip(info, instance);
	}
}
