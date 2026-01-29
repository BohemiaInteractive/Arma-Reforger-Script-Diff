[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_FormatUIInfo: SCR_BlockUIInfo
{
	[Attribute()]
	protected ref array<ref SCR_BaseFormatParam> m_aParams;

	//------------------------------------------------------------------------------------------------
	override bool SetNameTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		if (m_aParams && !m_aParams.IsEmpty())
			textWidget.SetTextFormat(GetName(), Param(0), Param(1), Param(2), Param(3), Param(4), Param(5), Param(6), Param(7), Param(8));
		else
			textWidget.SetText(GetName());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SetDescriptionTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		if (m_aParams && !m_aParams.IsEmpty())
			textWidget.SetTextFormat(GetDescription(), Param(0), Param(1), Param(2), Param(3), Param(4), Param(5), Param(6), Param(7), Param(8));
		else
			textWidget.SetText(GetDescription());
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param index
	//! \return
	protected string Param(int index)
	{
		if (index < m_aParams.Count())
			return m_aParams[index].GetParam();
		else
			return string.Empty;
	}
}

[BaseContainerProps()]
class SCR_BaseFormatParam
{
	//------------------------------------------------------------------------------------------------
	string GetParam();
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sActionName", "Action: '%1'")]
class SCR_ActionFormatParam: SCR_BaseFormatParam
{
	[Attribute()]
	protected string m_sActionName;
	
	//[Attribute("0.760 0.384 0.08 1")]
	//protected ref Color m_Color;

	//------------------------------------------------------------------------------------------------
	override string GetParam()
	{
		// TODO: Turn color conversion to a general method
		Color sRGBA = Color.FromIntSRGB(UIColors.CONTRAST_COLOR.PackToInt());
		
		//--- Convert to sRGBA format for rich text
		sRGBA.LinearToSRGB();
		
		//--- Convert to ints, no fractions allowed in rich text
		int colorR = sRGBA.R() * 255;
		int colorG = sRGBA.G() * 255;
		int colorB = sRGBA.B() * 255;
		int colorA = sRGBA.A() * 255;
		
		return string.Format("<color rgba='%2,%3,%4,%5'><action name='%1' scale='1.25'/></color>", m_sActionName, colorR, colorG, colorB, colorA);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_ImageSet", true, "ImageSet: '%1'")]
class SCR_ImageSetFormatParam : SCR_BaseFormatParam
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "imageset")]
	protected ResourceName m_ImageSet;
	
	[Attribute(desc: "")]
	protected string m_sName;

	//------------------------------------------------------------------------------------------------
	override string GetParam()
	{
		return string.Format("<image set='%1' name='%2'/>", m_ImageSet, m_sName);
	}
}
