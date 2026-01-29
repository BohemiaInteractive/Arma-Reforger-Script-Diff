[BaseContainerProps(configRoot: true)]
class SCR_PlayMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Tutorial mission", "conf")]
	ResourceName m_TutorialScenario;

	[Attribute("", UIWidgets.ResourceAssignArray, "Featured scenarios headers", "conf")]
	ref array<ResourceName> m_aFeaturedScenarios;	
		
	[Attribute("", UIWidgets.ResourceAssignArray, "Recommended scenarios headers", "conf")]
	ref array<ResourceName> m_aRecommendedScenarios;
};