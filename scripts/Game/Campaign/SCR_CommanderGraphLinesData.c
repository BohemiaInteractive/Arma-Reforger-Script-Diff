[BaseContainerProps()]
class SCR_CommanderGraphLinesData
{
	[Attribute("0.89 0.65 0.31 0.8", UIWidgets.ColorPicker)]
	protected ref Color m_Color;

	[Attribute("3", UIWidgets.Slider, "Alpha of the lines, when highlighted or selected.", "0.1 5 0.1")]
	protected float m_fLineWidth;

	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		return m_Color;
	}

	//------------------------------------------------------------------------------------------------
	float GetLineWidth()
	{
		return m_fLineWidth;
	}
}
