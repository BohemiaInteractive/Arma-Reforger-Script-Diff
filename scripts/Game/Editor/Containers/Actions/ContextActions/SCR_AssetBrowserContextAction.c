[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_AssetBrowserContextAction : SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return hoveredEntity && hoveredEntity.HasEntityFlag(EEditableEntityFlag.SLOT); //--- ToDo: Check if the entity has an extension
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (!hoveredEntity)
			return;
		
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (contentBrowserManager)
		{
			//~ Get content browser config to hide labels and set specific labels active
			SCR_EditableLinkedContentBrowserUIInfo uiSlotInfo = SCR_EditableLinkedContentBrowserUIInfo.Cast(hoveredEntity.GetInfo());
			
			if (uiSlotInfo)
				contentBrowserManager.OpenBrowserExtended(hoveredEntity, uiSlotInfo.GetContentBrowserDisplayConfig());
			else
				contentBrowserManager.OpenBrowserExtended(hoveredEntity);
		}
	}
};