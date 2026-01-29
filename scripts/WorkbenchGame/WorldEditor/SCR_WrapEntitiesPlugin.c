#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Wrap Entities", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF1B2)]
class SCR_WrapEntitiesPlugin : WorkbenchPlugin
{
	[Attribute(params: "et")]
	protected ResourceName m_sEntityPrefab;

	[Attribute(defvalue: "GenericEntity")]
	protected string m_sPrefabClass;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		int selectedCount = worldEditorAPI.GetSelectedEntitiesCount();
		if (selectedCount == 0)
			return;

		IEntity pivotEntity = worldEditorAPI.SourceToEntity(worldEditorAPI.GetSelectedEntity(selectedCount - 1));

		string entityClass;
		if (!m_sEntityPrefab.IsEmpty())
			entityClass = m_sEntityPrefab;
		else if (!m_sPrefabClass.IsEmpty())
			entityClass = m_sPrefabClass;
		else
			entityClass = "GenericEntity";

		worldEditorAPI.BeginEntityAction("WrapEntitiesPlugin");
		IEntitySource parentSrc = worldEditorAPI.CreateEntity(entityClass, "", worldEditorAPI.GetCurrentEntityLayerId(), null, pivotEntity.GetOrigin(), pivotEntity.GetAngles());
		for (int i = 0; i < selectedCount; i++)
		{
			worldEditorAPI.ParentEntity(parentSrc, worldEditorAPI.GetSelectedEntity(i), true);
		}
		worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Wrap Entities' plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return false;
	}
}
#endif // WORKBENCH
