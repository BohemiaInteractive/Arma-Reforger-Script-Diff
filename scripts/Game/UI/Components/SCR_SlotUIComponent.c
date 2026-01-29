/*!
Scripted slot configuration.

To be used on root widget of a layout which is created by script.
*/
class SCR_SlotUIComponent: ScriptedWidgetComponent
{
	[Attribute()]
	private float AnchorLeft;
	
	[Attribute()]
	private float AnchorTop;
	
	[Attribute()]
	private float AnchorRight;
	
	[Attribute()]
	private float AnchorBottom;
	
	[Attribute()]
	private float PositionX;
	
	[Attribute()]
	private float PositionY;
	
	[Attribute()]
	private float SizeX;
	
	[Attribute()]
	private float SizeY;
	
	[Attribute()]
	private float AlignmentX;
	
	[Attribute()]
	private float AlignmentY;
	
	[Attribute()]
	private bool SizeToContent;
	
	protected Widget		m_widget;
	
	override void HandlerAttached(Widget w)
	{
		m_widget = w;
		
		if (!w.GetParent())
		{
			Print(string.Format("Cannot initialize SCR_SlotUIComponent, widget '%1' has no parent!", w.GetName()), LogLevel.WARNING);
			return;
		}
		
		if (w.GetParent().GetTypeID() != WidgetType.FrameWidgetTypeID)
		{
			Print(string.Format("Cannot initialize SCR_SlotUIComponent, parent of widget '%1' is not a FrameWidget!", w.GetName()), LogLevel.WARNING);
			return;
		}
		
		FrameSlot.SetAlignment(w, AlignmentX, AlignmentY);
		FrameSlot.SetSizeToContent(w, SizeToContent);
		FrameSlot.SetOffsets(w, AnchorLeft, AnchorTop, AnchorRight, AnchorBottom);
		FrameSlot.SetAnchorMin(w, AnchorLeft, AnchorTop);
		FrameSlot.SetAnchorMax(w, AnchorRight, AnchorBottom);
		if (AnchorLeft == AnchorRight)
		{
			FrameSlot.SetPosX(w, PositionX);
			FrameSlot.SetSizeX(w, SizeX);
		}
		if (AnchorTop == AnchorBottom)
		{
			FrameSlot.SetPosY(w, PositionY);
			FrameSlot.SetSizeY(w, SizeY);
		}
	}
	
	void SetPosX( float x )
	{
		FrameSlot.SetPosX( m_widget, x );
	}
	
	void SetPosY( float y )
	{
		FrameSlot.SetPosY( m_widget, y );
	}
	
	float GetPosX()
	{
		return FrameSlot.GetPosX( m_widget );
	}
	
	float GetPosY()
	{
		return FrameSlot.GetPosY( m_widget );
	}
	
};