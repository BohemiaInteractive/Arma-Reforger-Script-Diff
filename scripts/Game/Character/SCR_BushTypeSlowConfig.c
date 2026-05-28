//! Config file to keep track of the different foliage types and the weight of the slowdown effect
[BaseContainerProps(configRoot: true)]
class SCR_BushTypeSlowConfig
{
	[Attribute("1", uiwidget: UIWidgets.Auto, "The default if a specific value cannot be found")]
	protected float m_fDefaultSlowHeightModifier;

	[Attribute(desc: "Specific foliage type weight ratios")]
	protected ref array<ref SCR_BushContainer> m_aFoliageTypeSlowWeight;

	//------------------------------------------------------------------------------------------------
	//! Get the weight of the slowdown effect of the given tree type
	float GetHeightSlowEffectModifier(ETreeSoundTypes treeType)
	{
		foreach (SCR_BushContainer bushContainer : m_aFoliageTypeSlowWeight)
		{
			if(bushContainer.GetTreeType() == treeType)
				return bushContainer.GetSlowdownHeightModifier();
		}
		
		return m_fDefaultSlowHeightModifier;
	}
}
