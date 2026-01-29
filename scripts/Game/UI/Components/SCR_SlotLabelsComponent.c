class SCR_SlotLabelsComponent : ScriptedWidgetComponent
{
    override void HandlerAttached(Widget w)
    {
        #ifdef WORKBENCH
			Game g = GetGame();
			GenericEntity world = GetGame().GetWorldEntity();
			
			if (world)
				return;
			
	        Widget child = w.GetChildren();
		
	        while (child)
	        {
				if (OverlayWidget.Cast(child))
				{
					TextWidget text = TextWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE, new Color(1,1,1,1), 0, child));
		            text.SetText(child.GetName());
					text.SetFont("{3E7733BAC8C831F6}UI/Fonts/RobotoCondensed/RobotoCondensed_Regular.fnt");
					text.SetExactFontSize(18);	
				}
					
	            child = child.GetSibling();
	        }
        #endif
    }
};