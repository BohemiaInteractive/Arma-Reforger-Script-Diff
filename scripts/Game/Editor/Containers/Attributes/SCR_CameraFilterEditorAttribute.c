/**
Camera filter Attribute for photomode for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_CameraFilterEditorAttribute: SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!item.IsInherited(SCR_CameraBase)) return null;
		
		SCR_CameraFiltersEditorComponent cameraFilterManager = SCR_CameraFiltersEditorComponent.Cast(SCR_CameraFiltersEditorComponent.GetInstance(SCR_CameraFiltersEditorComponent));
		if (!cameraFilterManager) return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(cameraFilterManager.GetCurrentFilter());
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_CameraFiltersEditorComponent cameraFilterManager = SCR_CameraFiltersEditorComponent.Cast(SCR_CameraFiltersEditorComponent.GetInstance(SCR_CameraFiltersEditorComponent));
		if (!cameraFilterManager) return;
		
		if (var) cameraFilterManager.SetCurrentFilter(var.GetInt());
	}
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		WriteVariable(null, GetVariable(), manager, -1);
	}
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Clear();
		
		SCR_CameraFiltersEditorComponent cameraFilterManager = SCR_CameraFiltersEditorComponent.Cast(SCR_CameraFiltersEditorComponent.GetInstance(SCR_CameraFiltersEditorComponent));
		if (!cameraFilterManager) return 0;
		
		array<SCR_CameraFilterEditor> filters = new array<SCR_CameraFilterEditor>;
		int filtersCount = cameraFilterManager.GetFilters(filters);
		if (filtersCount == 0) return 0;
		
		for (int i = 0; i < filtersCount; i++)
		{
			outEntries.Insert(new SCR_BaseEditorAttributeEntryText(filters[i].GetDisplayName()));
		}
		return filtersCount;
	}
};