/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Attributes
\{
*/

class EntityEditorProps: CommonEditorProps
{
	/*!
	\param category Typically a "folder/item" path for placing the type into a tree view, e.g., "StaticEntities/Walls".
	\param description A description of the type's usage.
	\param color Color of an entity visualizer shape outline
	\param visible Indicates whether the entity visualizer is visible when not selected.
	\param insertable Indicates whether the type is available for insertion from the tree view.
	\param configRoot Indicates whether the type can be used as the root object of a config file (.conf).
	\param icon A path to an icon file for better visualization in a tree view.
	\param style Style of entity visualizer, can be: "none", "box", "sphere", "cylinder", "capsule", "pyramid", "diamond", "icon". Anything else is custom. Empty is none.
	\param sizeMin Minimum dimensions of an entity visualizer shape
	\param sizeMax Maximum dimensions of an entity visualizer shape
	\param color2 Color of an entity visualizer shape
	\param dynamicBox Is the entity visualizer using custom dimensions (provided by _WB_GetBoundBox)
	\param iconPrefab Scene icon definition. It must be a resource name in string form like "{AC931A989866A2BA}system/wbdata/WorldEditor/EntityIcons/Rectangle/Rectangle_ground.et"
	\param billboard Behaves like a 2D billboard that faces the icon model to be oriented to camera (for icons only)
	\param elevation Extra elevation above an entity. Currently works only with icons.
	\param scale Scale of the visualizer. Currently works only with icons.
	*/
	proto void EntityEditorProps(
		string category = "",
		string description = "",
		string color = "255 0 0 255",
		bool visible = true,
		bool insertable = true,
		bool configRoot = false,
		string icon = "",
		string style = "box",
		vector sizeMin = "-0.25 -0.25 -0.25",
		vector sizeMax = "0.25 0.25 0.25",
		string color2 = "0 0 0 0",
		bool dynamicBox = false,
		string iconPrefab = "",
		bool billboard = true,
		float elevation = 0,
		float scale = 1
	);

}

/*!
\}
*/
