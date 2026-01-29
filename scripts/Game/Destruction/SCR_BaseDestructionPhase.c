class SCR_BaseDestructionPhase: BaseDestructionPhase
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Model to use for the damage phase", "xob et")]
	ResourceName m_sPhaseModel;

	[Attribute("0", UIWidgets.CheckBox, "Use material remap from parent prefab when changing to this phase")]
	bool m_bUseMaterialFromParent;

	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the phase")]
	ref array<ref SCR_BaseSpawnable> m_aPhaseDestroySpawnObjects;

	[Attribute(defvalue: SCR_EDestructionResponseIndex.AUTOMATIC.ToString(), desc: "Value that will be used for the physics response index at this phase.\nWhen AUTOMATIC is used then response index will be set to the value between HUGE_DESTRUCTIBLE and TINY_DESTRUCTIBLE based on the ratio of current health to the max health", uiwidget: UIWidgets.ComboBox, enumType: SCR_EDestructionResponseIndex)]
	SCR_EDestructionResponseIndex m_ePhysicalResponseIndex;

	protected VObject m_PhaseModelCache;
	protected ref Resource m_ResourceCache;

	//------------------------------------------------------------------------------------------------
	// Resolve phase model path and remap string from m_sPhaseModel without loading resources.
	// Returns true only if a non-empty modelPath was resolved.
	// Note: Exposed for DestructibleEntity call sites.
	bool GetPhaseModelAndRemap(out ResourceName modelPath, out string remap, ResourceName parentPrefabPath = ResourceName.Empty)
	{
		modelPath = ResourceName.Empty;
		remap = string.Empty;

		if (!m_sPhaseModel || m_sPhaseModel == ResourceName.Empty)
			return false;

		SCR_Global.GetModelAndRemapFromResource(m_sPhaseModel, modelPath, remap);

		// Parent material remap inheritance (optional, controlled by phase attribute)
		if (m_bUseMaterialFromParent && parentPrefabPath && parentPrefabPath != ResourceName.Empty)
		{
			ResourceName modelPathSource;
			string remapParent;
			SCR_Global.GetModelAndRemapFromResource(parentPrefabPath, modelPathSource, remapParent);

			// If phase already has a remap, sanitize against parent; otherwise take parent remap directly
			if (!remap.IsEmpty())
			{
				remap = SCR_DestructionUtility.SanitizeRemapString(remap, remapParent);
			}
			else
			{
				remap = remapParent;
			}
		}

		return modelPath && modelPath != ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	VObject GetModel()
	{
		if (m_ResourceCache && m_PhaseModelCache)
			return m_PhaseModelCache;

		m_ResourceCache = Resource.Load(m_sPhaseModel);
		if (!m_ResourceCache)
			return null;

		BaseResourceObject resourceObject = m_ResourceCache.GetResource();
		if (!resourceObject)
			return null;

		m_PhaseModelCache = resourceObject.ToVObject();
		return m_PhaseModelCache;
	}
}