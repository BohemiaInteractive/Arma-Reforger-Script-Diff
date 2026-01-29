#ifdef WORKBENCH
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Component", true)]
class EditablePrefabsComponent_PreviewEntity : EditablePrefabsComponent_Base
{
	[Attribute("1", desc: "When enabled, preview prefab can be used also in runtime, not only when spawning from prefab.")]
	protected bool m_bRuntime;

	//------------------------------------------------------------------------------------------------
	override void EOnCreate(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource instanceEntitySource, IEntityComponentSource componentSource, IEntityComponentSource componentCurrent)
	{
		SCR_PreviewEntityComponent.GeneratePreviewEntries(api, entitySource);
		api.SetVariableValue(entitySource, { ContainerIdPathEntry(componentSource.GetClassName()) }, "m_bRuntime", m_bRuntime.ToString());
	}
}
#endif // WORKBENCH
