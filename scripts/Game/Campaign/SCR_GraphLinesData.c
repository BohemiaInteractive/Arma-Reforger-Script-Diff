//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_GraphLinesData
{
	[Attribute("0.3", UIWidgets.Slider, "Default alpha of the lines.", "0, 1, 0.1")]
	protected float m_fAlphaDefault;
	
	[Attribute("1", UIWidgets.Slider, "Alpha of the lines, when highlighted or selected.", "0, 1, 0.1")]
	protected float m_fAlphaHighlight;
	
	[Attribute("3", UIWidgets.Slider, "Alpha of the lines, when highlighted or selected.", "0.1, 5, 0.1")]
	protected float m_fLineWidth;
	
	//------------------------------------------------------------------------------------------------
	float GetDefaultAlpha()
	{
		return m_fAlphaDefault;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetHighlightedAlpha()
	{
		return m_fAlphaHighlight;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLineWidth()
	{
		return m_fLineWidth;
	}
};
