[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableEntityComponentClass : ScriptComponentClass
{
	[Attribute("0", UIWidgets.ComboBox, category: "Editable Entity", desc: "System type of the entity.", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected EEditableEntityType m_EntityType;
	
	[Attribute(category: "Visualization", desc: "GUI representation")]
	protected ref SCR_UIInfo m_UIInfo;
	
	[Attribute("", UIWidgets.Auto, category: "Visualization", desc: "Bone to which the icon is attached to")]
	protected string m_sIconBoneName;
	
	[Attribute(category: "Editable Entity")]
	protected ref SCR_EditableEntityInteraction m_EntityInteraction;
	
	[Attribute(desc: "When spawning the editable entity and the data is set it will randomly grab a variant depending on the weight set. Any variant needs to have an editable entity component.\n\nLeave class null if there are no variants that need to be randomized", category: "Variants")]
	protected ref SCR_EditableEntityVariantData m_VariantData;
	
	//------------------------------------------------------------------------------------------------
	//! Get random variant of the given ResourceName (needs to have a editable entity component)
	//! \param[in] prefab The default variant. It will be used to get the variants from as well as potentially adding it to the randomization pool depending on the settings
	//! \return Prefab variant or default if no variant found (or randomly selected)
	static ResourceName GetRandomVariant(ResourceName prefab)
	{
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource.IsValid())
			return prefab;
		
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(prefabResource, SCR_EditableEntityComponent);
		if (!componentSource)
			return prefab;
		
		SCR_EditableEntityVariantData variantData;
		componentSource.Get("m_VariantData", variantData);
		
		if (!variantData)
			return prefab;
		
		array<SCR_EditableEntityVariant> variants = {};
		if (variantData.GetVariants(variants) == 0)
			return prefab;
		
		//~ Created weighted array for randomization
		SCR_WeightedArray<ResourceName> weightedArray = new SCR_WeightedArray<ResourceName>();
		
		foreach (SCR_EditableEntityVariant variant : variants)
		{
			prefabResource = Resource.Load(variant.m_sVariantPrefab);
			if (!prefabResource.IsValid())
				continue;
			
			//~ Editable entities only
			if (!GetEditableEntitySource(prefabResource))
				continue;
			
			//~ Add to randomizer
			weightedArray.Insert(variant.m_sVariantPrefab, variant.m_iRandomizerWeight);
		}
		
		//~ No variants to randomize
		if (weightedArray.IsEmpty())
			return prefab;
		
		//~ Add default variant to randomizer
		if (variantData.m_bRandomizeDefaultVariant)
			weightedArray.Insert(prefab, variantData.m_iDefaultVariantRandomizerWeight);
		
		//~ Get random variant
		ResourceName randomVariant;
		weightedArray.GetRandomValue(randomVariant);
		
		//~ Return random variant
		return randomVariant;
	}
	
	//------------------------------------------------------------------------------------------------
	//! If any variants were assigned to the prefab (Does not check if the variants are valid)
	//! \param[in] prefab Prefab to check default variants of
	//! \return True if any variants were assigned
	static bool HasVariants(ResourceName prefab)
	{
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource.IsValid())
			return false;
		
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(prefabResource, SCR_EditableEntityComponent);
		if (!componentSource)
			return false;
		
		SCR_EditableEntityVariantData variantData;
		componentSource.Get("m_VariantData", variantData);
		
		if (!variantData)
			return false;
		
		array<SCR_EditableEntityVariant> variants = {};
		return variantData.GetVariants(variants) > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get entity type.
	//! \return Type
	EEditableEntityType GetEntityType()
	{
		return m_EntityType;
	}

	//------------------------------------------------------------------------------------------------
	//! Get information about the entity. When none exist, create a dummy one.
	//! \return Info class
	SCR_UIInfo GetInfo()
	{
		return m_UIInfo;
	}

	//------------------------------------------------------------------------------------------------
	//! Get bone name on which entity icon will be rendered.
	//! \return Bone name
	string GetIconBoneName()
	{
		return m_sIconBoneName;
	}

	//------------------------------------------------------------------------------------------------
	//! Get entity interaction rules of this entity. If it doesn't contain any custom rules, those for its type will be used.
	//! \return Interaction rules
	SCR_EditableEntityInteraction GetEntityInteraction()
	{
		if (m_EntityInteraction)
			return m_EntityInteraction;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			return core.GetEntityInteraction(GetEntityType());
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get component source from prefab resource.
	//! \param[in] entityResource loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	//! \return Component source
	static IEntityComponentSource GetEditableEntitySource(Resource entityResource)
	{
		if (!entityResource)
			return null;
		
		BaseResourceObject entityBase = entityResource.GetResource();
		if (!entityBase)
			return null;
			
		IEntitySource entitySource = entityBase.ToEntitySource();
		if (!entitySource)
			return null;
		
		return GetEditableEntitySource(entitySource);
	}

	//------------------------------------------------------------------------------------------------
	//! Get component source from entity source.
	//! \param[in] entitySource Entity source - cannot be null
	//! \return Component source
	static IEntityComponentSource GetEditableEntitySource(IEntitySource entitySource)
	{
		if (!entitySource)
			return null;
		
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(SCR_EditableEntityComponent))
				return componentSource;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get UI info from SCR_EditableEntityComponent source
	//! \param[in] componentSource Component source - cannot be null
	//! \return UI info
	static SCR_EditableEntityUIInfo GetInfo(IEntityComponentSource componentSource)
	{		
		BaseContainer infoSource = componentSource.GetObject("m_UIInfo");
		if (!infoSource)
			return null;
		
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(infoSource));
		info.InitFromSource(componentSource);
		return info;
	}

	//------------------------------------------------------------------------------------------------
	//! Get entity system type
	//! \param[in] componentSource Component source - cannot be null
	//! \return Type
	static EEditableEntityType GetEntityType(IEntityComponentSource componentSource)
	{
		EEditableEntityType type;
		componentSource.Get("m_EntityType", type);
		return type;
	}

	//------------------------------------------------------------------------------------------------
	//! Get entity interaction rules of this entity prefab. If it doesn't contain any custom rules, those for its type will be used.
	//! \param[in] componentSource Component source - cannot be null
	//! \return Interaction rules
	static SCR_EditableEntityInteraction GetEntityInteraction(IEntityComponentSource componentSource)
	{
		BaseContainer container = componentSource.GetObject("m_EntityInteraction");
		if (container)
			return SCR_EditableEntityInteraction.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			return core.GetEntityInteraction(GetEntityType(componentSource));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get slot prefab from SCR_EditableEntityComponent source
	//! \param[in] componentSource Component source - cannot be null
	//! \return Slot prefab
	static ResourceName GetSlotPrefab(IEntityComponentSource componentSource)
	{
		BaseContainer info = componentSource.GetObject("m_UIInfo");
		if (!info)
			return ResourceName.Empty;
		
		ResourceName slotPrefab;
		info.Get("m_SlotPrefab", slotPrefab);
		return slotPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Get entity flags
	//! \param[in] componentSource Component source
	//! \return Flags
	static EEditableEntityFlag GetEntityFlags(IEntityComponentSource componentSource)
	{
		EEditableEntityFlag flags;
		componentSource.Get("m_Flags", flags);
		return flags;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the SCR_EditableEntityComponent source has given flag
	//! \param[in] componentSource Component source
	//! \param[in] flag Queried flag
	//! \return true if it has the flag
	static bool HasFlag(IEntityComponentSource componentSource, EEditableEntityFlag flag)
	{
		EEditableEntityFlag flags;
		componentSource.Get("m_Flags", flags);
		return flags & flag;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets entity budget cost values from EditableEntity component source
	//! \param[in] editableEntitySource Component source of the SCR_EditableEntityComponent
	//! \param[out] budgetValues Output array filled with budget cost values
	//! \return true if the component source is valid and component source has budget costs defined
	//!
	static bool GetEntitySourceBudgetCost(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		if (!editableEntitySource)
			return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
		if (editableEntityUIInfo)
			return editableEntityUIInfo.GetEntityBudgetCost(budgetValues);
		
		return !budgetValues.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets entity *+ entity children* budget cost values from EditableEntity component source
	//! \param[in] editableEntitySource Component source of the SCR_EditableEntityComponent
	//! \param[out] budgetValues Output array filled with budget cost values
	//! \return true if the component source is valid and component source has budget costs defined
	static bool GetEntitySourceChildrenBudgetCosts(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		if (!editableEntitySource)
			return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
		if (editableEntityUIInfo)
			editableEntityUIInfo.GetEntityChildrenBudgetCost(budgetValues);
		
		return !budgetValues.IsEmpty();
	}
}
