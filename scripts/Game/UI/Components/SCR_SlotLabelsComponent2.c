class SCR_SlotLabelsComponent2 : ScriptedWidgetComponent
{
    override void HandlerAttached(Widget w)
    {
        #ifdef WORKBENCH
			Game g = GetGame();
			GenericEntity world = GetGame().GetWorldEntity();
			
			if (world)
				return;
			
	        Widget slot = w.GetChildren();
		
	        while (slot)
	        {
				if (SizeLayoutWidget.Cast(slot))
				{
					OverlayWidget overlay = OverlayWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.OverlayWidgetTypeID, WidgetFlags.VISIBLE, new Color(1,1,1,1), 0, slot));
					SizeLayoutSlot.SetHorizontalAlign(overlay, LayoutHorizontalAlign.Stretch);
					SizeLayoutSlot.SetVerticalAlign(overlay, LayoutHorizontalAlign.Stretch);
									
					ImageWidget img = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE, new Color(Math.RandomFloat(0,0.8),Math.RandomFloat(0,0.8),Math.RandomFloat(0,0.8),1), 0, overlay));
					OverlaySlot.SetHorizontalAlign(img, LayoutHorizontalAlign.Stretch);
					OverlaySlot.SetVerticalAlign(img, LayoutHorizontalAlign.Stretch);
				
					TextWidget text = TextWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE, new Color(1,1,1,1), 0, overlay));
		            text.SetText(slot.GetName());
					text.SetFont("{3E7733BAC8C831F6}UI/Fonts/RobotoCondensed/RobotoCondensed_Regular.fnt");
					text.SetExactFontSize(18);	
				}
					
	            slot = slot.GetSibling();
	        }
        #endif
    }
};