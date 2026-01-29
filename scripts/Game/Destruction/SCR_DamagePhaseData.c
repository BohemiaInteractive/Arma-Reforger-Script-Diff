//! Multi-Phase destruction phase
[BaseContainerProps(), SCR_DamagePhaseTitle()]
class SCR_DamagePhaseData
{
	[Attribute("100", UIWidgets.Slider, "Base health value for the damage phase. Upon switching to this phase, this value replaces the base health of the object", "0.01 100000 0.01")]
	float m_fPhaseHealth;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Model to use for the damage phase", params: "et xob")]
	ResourceName m_PhaseModel;
	
	[Attribute("0", desc: "Remap materials on all models defined in Phase Model array to those used on owner prefab. Remap is applied based on material slot names.")]
	bool m_bUseMaterialsFromParent;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the phase")]
	ref array<ref SCR_BaseSpawnable> m_PhaseDestroySpawnObjects;
}