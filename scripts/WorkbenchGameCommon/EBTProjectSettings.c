// Project-specific settings that can be easily edited by artists, and read from the Enfusion Blender tools. 
// The Enfusion Blender tools will recognize a configuration file at this location: Configs/EBT/ebt_settings.conf

[BaseContainerProps(configRoot: false, description: "")]
class EBTGameMaterialSettings
{
	[Attribute(defvalue: "", desc: "Game material to specify the settings for", params: "class=GameMaterial")]
	ResourceName gameMaterial;
	
	[Attribute(defvalue: "", desc: "Visualization Color")]
	ref Color color;
}

[BaseContainerProps(configRoot: true,  description: "")]
class EBTProjectSettings
{	
    [Attribute( desc: "")]
    ref array<ref EBTGameMaterialSettings> gameMaterialSettings;
};