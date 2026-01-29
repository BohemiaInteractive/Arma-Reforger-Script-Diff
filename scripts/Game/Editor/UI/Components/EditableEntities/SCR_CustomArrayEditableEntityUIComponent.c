class SCR_CustomArrayEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute(desc: "Image widget to set icon same as EditableEntity UI info icon")]
	protected ref array<string> m_sIconWidgetNames;
	
	[Attribute("0", desc: "If true will attempt to color the widget this script is attached to if the EditableEntity uses SCR_ColorUIInfo")]
	protected bool m_bColorWidget;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		//~ Set custom textures
		if (info)
		{
			ImageWidget iconWidget;
			
			//~ Set Icons
			if (m_sIconWidgetNames)
			{
				foreach (string icon : m_sIconWidgetNames)
				{
					iconWidget = ImageWidget.Cast(GetWidget().FindAnyWidget(icon));
					info.SetIconTo(iconWidget);
				}
			}
			
			//~ Color widget
			if (m_bColorWidget)
			{
				SCR_ColorUIInfo colorInfo = SCR_ColorUIInfo.Cast(info);
				
				if (colorInfo)
					GetWidget().SetColor(colorInfo.GetColor());
				else 
					Print("'SCR_CustomArrayEditableEntityUIComponent' tried to set the UI color but the EditableEntity is not using SCR_ColorUIInfo", LogLevel.WARNING);
			}
		}
	}
}
