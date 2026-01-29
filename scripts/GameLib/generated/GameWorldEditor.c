/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Generic ingame editor implementation.
*/
class GameWorldEditor: Managed
{
	proto external WorldEditorAPI GetEditorAPI();
	proto external bool SaveWorld();
	/*!
	\param savePath		Can have max 256 characters, if there are more than 256 characters it will return false.
	If overrideWorld is true, and if the path already exists it will be overridden
	*/
	proto external bool SaveWorldAs(string savePath, bool overridePath = false);
	proto external void Undo();
	proto external void Redo();
	proto external bool SwitchToGameMode(ResourceName worldSystemsConfig);
	proto external void SwitchToEditMode();
}
