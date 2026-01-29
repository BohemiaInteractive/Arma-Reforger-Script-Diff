/*!
Simple radial compass which will rotate based on current camera position.
Has to be attached to MenuRootBase to be updated automatically, otherwise call UpdateRotation() manually.
*/
class SCR_CompassUIComponent: MenuRootSubComponent
{
	[Attribute(defvalue: "0.8", uiwidget: UIWidgets.Slider, params: "0 1 0.01")]
	protected float m_fLabelsDistance;
	
	[Attribute()]
	protected string m_sTextureWidgetName;
	
	[Attribute()]
	protected string m_sEastWidgetName;
	
	[Attribute()]
	protected string m_sSouthWidgetName;
	
	[Attribute()]
	protected string m_sWestWidgetName;
	
	[Attribute()]
	protected string m_sNorthWidgetName;
	
	protected BaseWorld m_World;
	protected ImageWidget m_TextureWidget;
	protected ref map<Widget, float> m_Labels = new map<Widget, float>;
	
	/*!
	Update compass rtation base don the current view.
	\param timeSlice Time since the last frame
	*/
	void UpdateRotation(float timeSlice)
	{
		vector transform[4];
		m_World.GetCurrentCamera(transform);
		float yaw = -Math3D.MatrixToAngles(transform)[0];
		m_TextureWidget.SetRotation(yaw);
		
		float yawRad = yaw * Math.DEG2RAD;
		foreach (Widget label, float angle: m_Labels)
		{
			FrameSlot.SetPos(
				label,
				m_fLabelsDistance * Math.Cos(yawRad + angle),
				m_fLabelsDistance * Math.Sin(yawRad + angle)
			);
		}
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		m_World = GetGame().GetWorld();
		if (!m_World)
			return;
		
		m_TextureWidget = ImageWidget.Cast(w.FindAnyWidget(m_sTextureWidgetName));
		if (!m_TextureWidget)
			return;
		
		m_Labels.Insert(w.FindAnyWidget(m_sEastWidgetName), 0);
		m_Labels.Insert(w.FindAnyWidget(m_sSouthWidgetName), Math.PI * 0.5);
		m_Labels.Insert(w.FindAnyWidget(m_sWestWidgetName), Math.PI);
		m_Labels.Insert(w.FindAnyWidget(m_sNorthWidgetName), Math.PI * 1.5);
		
		m_fLabelsDistance *= 0.5 * FrameSlot.GetSizeY(w);
		
		menu.GetOnMenuUpdate().Insert(UpdateRotation);
	}
	override void HandlerDeattached(Widget w)
	{
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Remove(UpdateRotation);
		}
	}
};