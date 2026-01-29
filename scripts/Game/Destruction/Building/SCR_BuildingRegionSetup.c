[BaseContainerProps()]
class SCR_BuildingRegionSetup
{
	[Attribute("500", UIWidgets.EditBox, "Maximum damage the region can take before being destroyed")]
	float m_MaxHealth;
	[Attribute("0.55", UIWidgets.EditBox, "Percentage of structurally linked parents that must be destroyed for this region to fall")]
	float m_StructuralSupportPct;
	[Attribute("{B6CBD40830C3F0A4}graphics/particle/legacy/tkom/weapon/destruct_wall.ptc", UIWidgets.ResourcePickerThumbnail, "Particle effect to use when the region is destroyed")]
	ResourceName m_DestructFX_PTC;
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sound effect to play when the region is destroyed")]
	ResourceName m_DestructFX_SND;
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab to spawn when the region is destroyed")]
	ResourceName m_DestructFX_PFB;
};
