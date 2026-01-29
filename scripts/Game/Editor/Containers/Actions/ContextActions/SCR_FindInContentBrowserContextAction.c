[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_FindInContentBrowserContextAction : SCR_BaseContextAction
{
	[Attribute(desc: "Display config, only header is used, labels are filled by action")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_ContentBrowserConfig;
	
	protected bool GetEntityLabels(SCR_EditableEntityComponent editableEntity, out array<EEditableEntityLabel> entityLabels)
	{
		if (editableEntity)
		{
			SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
			if (editableUiInfo)
			{
				editableUiInfo.GetEntityLabels(entityLabels);
				return !entityLabels.IsEmpty();
			}
		}
		return false;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return hoveredEntity != null && selectedEntities.Contains(hoveredEntity) && !SCR_EditableFactionComponent.Cast(hoveredEntity);
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (hoveredEntity && contentBrowserManager)
		{
			SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(hoveredEntity.GetInfo());

			int itemIndex = contentBrowserManager.FindIndexOfInfo(uiInfo);
			if (itemIndex == -1) return;
			
			int pageIndex = Math.AbsInt(itemIndex / contentBrowserManager.GetPageEntryCount());
			
			SCR_EditorContentBrowserSaveStateData resultState = new SCR_EditorContentBrowserSaveStateData();
			//resultState.SetPageIndex(pageIndex);
			
			array<EEditableEntityLabel> prefabLabels = {};
			uiInfo.GetEntityLabels(prefabLabels);
			
			resultState.SetLabels(prefabLabels);
			string localizedEntityName = WidgetManager.Translate(uiInfo.GetName());
			
			//array<string> localizedWords = {};
			//localizedEntityName.Split(" ", localizedWords, true);
			//resultState.SetSearchString(string.Format("%1 %2", localizedWords[0] + localizedWords[1]));
			
			resultState.SetSearchString(localizedEntityName);
			
			contentBrowserManager.OpenBrowserState(resultState, m_ContentBrowserConfig);
		}
	}
};