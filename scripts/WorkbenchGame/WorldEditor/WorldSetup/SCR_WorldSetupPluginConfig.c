#ifdef WORKBENCH // this config is only related to a Workbench tool (SCR_WorldSetupPlugin_Entities)
[BaseContainerProps(configRoot: true)]
class SCR_WorldSetupPluginConfig
{
	[Attribute(desc: "Entities to create in the world")]
	ref array<ref SCR_WorldSetupPluginConfig_Entity> m_aEntities;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sPrefab")]
class SCR_WorldSetupPluginConfig_Entity
{
	[Attribute(desc: "Which Prefab to create - if Class Name above is provided, this value is ignored", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sPrefab;

	[Attribute(desc: "Only one entity of that classname (e.g GenericTerrainEntity, etc) can be created (works with both Class Name and Prefab above)")]
	bool m_bMustBeUniqueByClassName;

	[Attribute(desc: "Only one entity of that Prefab (e.g \"{guid}/MyPrefab.et\"}) can be created (inheritance check not supported)")]
	bool m_bMustBeUniqueByPrefab;

	[Attribute(defvalue: "0 0 0", desc: "The entity's creation position in world coordinates")]
	vector m_vPosition;

	[Attribute(defvalue: "0 0 0", desc: "The entity's angles, in degrees", params: "0 360")]
	vector m_vAngles;

	[Attribute(desc: "Additional Values are applied last; a key (variable name) that does not exist in the Entity will simply be ignored")]
	ref array<ref SCR_WorldSetupPluginConfig_EntitySourceKeyValue> m_aAdditionalValues;
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_sKey")]
class SCR_WorldSetupPluginConfig_EntitySourceKeyValue
{
	// this could be extended with ContainerIdPathEntry if needed
	// [Attribute()]
	// ref array<string> m_aVariablePath;

	[Attribute(desc: "Name of the variable to be set")]
	string m_sKey;

	[Attribute(desc: "Variable value")]
	string m_sValue;
}
#endif // WORKBENCH
