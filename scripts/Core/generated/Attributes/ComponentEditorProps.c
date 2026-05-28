/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Attributes
\{
*/

class ComponentEditorProps: CommonEditorProps
{
	/*!
	\param category Typically a "folder/item" path for placing the type into a tree view, e.g., "StaticEntities/Walls".
	\param description A description of the type's usage.
	\param color Color of an entity visualizer shape outline
	\param visible Indicates whether the entity visualizer is visible when not selected.
	\param insertable Indicates whether the type is available for insertion from the tree view.
	\param configRoot Indicates whether the type can be used as the root object of a config file (.conf).
	\param icon A path to an icon file for better visualization in a tree view.
	*/
	proto void ComponentEditorProps(
		string category = "",
		string description = "",
		string color = "255 0 0 255",
		bool visible = true,
		bool insertable = true,
		bool configRoot = false,
		string icon = "",
	);

}

/*!
\}
*/
