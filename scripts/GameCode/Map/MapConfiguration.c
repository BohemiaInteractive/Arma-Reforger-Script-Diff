// Map configuration object created from SCR_MapConfig through SCR_MapEntity API
class MapConfiguration
{
	EMapEntityMode MapEntityMode;
	Widget RootWidgetRef;
	int LayerCount;
	
	ref SCR_MapPropsBase MapPropsConfig;
	ref SCR_MapLayersBase LayerConfig;
	ref SCR_MapDescriptorVisibilityBase DescriptorVisibilityConfig;
	ref SCR_MapDescriptorDefaults DescriptorDefsConfig;
	
	ref array<ref SCR_MapModuleBase> Modules = {}; 			// Must be child of SCR_MapModuleBase, NO constructor arguments
	ref array<ref SCR_MapUIBaseComponent> Components = {};	// Must be child of SCR_MapUIBaseComponent, NO constructor arguments
	EMapOtherComponents OtherComponents = 0; 				// Flags for components that are part of the map system, but not inherited from SCR_MapModuleBase or SCR_MapUIBaseComponent, such as MapLegend
};