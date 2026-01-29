[BaseContainerProps()]
class SCR_EditableEntityVariantData
{
	[Attribute("1", desc: "If true will randomize the default prefab as well. When false it will ignore the default prefab and only randomize any of the variants")]
	bool m_bRandomizeDefaultVariant;
	
	[Attribute("1", desc: "The randomization weight of the default variant if randomized (RandomizeDefaultVariant needs to be true), the higher this value the more likely it is randomly selected", params: "1 inf")]
	int m_iDefaultVariantRandomizerWeight;
	
	[Attribute()]
	protected ref array<ref SCR_EditableEntityVariant> m_aVariants;
	
	//------------------------------------------------------------------------------------------------
	//! Get a list of all variants data
	//! \param[out] variants Array of all variant data
	//! \return Count of variant data found
	int GetVariants(out notnull array<SCR_EditableEntityVariant> variants)
	{
		variants.Clear();
		
		foreach (SCR_EditableEntityVariant variant : m_aVariants)
		{
			if (!variant || !variant.m_bEnabled)
				continue;
			
			variants.Insert(variant);
		}
		
		return variants.Count();
	}
}

[BaseContainerProps(), BaseContainerCustomDoubleCheckIntResourceNameTitleField("m_bEnabled", "m_sVariantPrefab", 1, "%1", "DISABLED - %1")]
class SCR_EditableEntityVariant
{
	[Attribute(desc: "The variant to randomize, needs to have an editable entity component", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sVariantPrefab;
	
	[Attribute("1", desc: "The randomization weight of the variant, the higher this value the more likely it is randomly selected", params: "1 inf")]
	int m_iRandomizerWeight;
	
	[Attribute("1", desc: "Allows to disable the Variant. it will never randomize the variant if false.")]
	bool m_bEnabled;
}
