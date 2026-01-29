[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_MenuOverlayEditorAttribute: SCR_BaseDuplicatableEditorAttribute
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMenuOverlayLayer))]
	private EEditorMenuOverlayLayer m_Layer;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!item.IsInherited(SCR_CameraBase)) return null;
		
		SCR_MenuOverlaysEditorComponent menuOverlaysManager = SCR_MenuOverlaysEditorComponent.Cast(SCR_MenuOverlaysEditorComponent.GetInstance(SCR_MenuOverlaysEditorComponent));
		if (!menuOverlaysManager) return null;
		
		SCR_EditorMenuOverlayLayer layer = menuOverlaysManager.GetOverlayLayer(m_Layer);
		if (!layer) return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(layer.GetCurrentOverlayIndex());
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		if (!var)
			return;
			
		SCR_MenuOverlaysEditorComponent menuOverlaysManager = SCR_MenuOverlaysEditorComponent.Cast(SCR_MenuOverlaysEditorComponent.GetInstance(SCR_MenuOverlaysEditorComponent));
		if (!menuOverlaysManager) return;
		
		SCR_EditorMenuOverlayLayer layer = menuOverlaysManager.GetOverlayLayer(m_Layer);
		if (!layer) return;
		
		layer.SetCurrentOverlay(var.GetInt());
	}
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		WriteVariable(null, GetVariable(), manager, -1);
	}
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Clear();
		
		SCR_MenuOverlaysEditorComponent menuOverlaysManager = SCR_MenuOverlaysEditorComponent.Cast(SCR_MenuOverlaysEditorComponent.GetInstance(SCR_MenuOverlaysEditorComponent));
		if (!menuOverlaysManager) return 0;
		
		SCR_EditorMenuOverlayLayer layer = menuOverlaysManager.GetOverlayLayer(m_Layer);
		if (!layer) return 0;
		
		array<SCR_EditorMenuOverlay> overlays = new array<SCR_EditorMenuOverlay>;
		int overlaysCount = layer.GetOverlays(overlays);
		if (overlaysCount == 0) return 0;
		
		for (int i = 0; i < overlaysCount; i++)
		{
			outEntries.Insert(new SCR_BaseEditorAttributeEntryText(overlays[i].GetDisplayName()));
		}
		return overlaysCount;
	}
};