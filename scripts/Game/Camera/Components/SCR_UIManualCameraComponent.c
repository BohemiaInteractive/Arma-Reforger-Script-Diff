//! @ingroup ManualCamera

//! Show custom UI layout.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_UIManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(params: "layout")]
	private ResourceName m_Layout;
	
	private Widget m_Widget;
	
	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_Widget = GetCameraEntity().CreateCameraWidget(m_Layout);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		if (m_Widget)
			m_Widget.RemoveFromHierarchy();
	}
}
