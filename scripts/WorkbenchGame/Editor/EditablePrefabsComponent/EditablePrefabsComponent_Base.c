#ifdef WORKBENCH
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Component", true)]
class EditablePrefabsComponent_Base
{
	[Attribute(params: "ct")]
	protected ResourceName m_Component;

	protected string m_sClassname;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetClassName()
	{
		return m_sClassname;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] prefab
	//! \param[in] targetPath
	//! \param[in] entitySource
	//! \return
	IEntityComponentSource AddComponent(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource)
	{
		return api.CreateComponent(entitySource, m_Component);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] prefab
	//! \param[in] targetPath
	//! \param[in] entitySource
	//! \param[in] instanceEntitySource
	//! \param[in] componentSource
	//! \param[in] componentCurrent
	void EOnCreate(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource instanceEntitySource, IEntityComponentSource componentSource, IEntityComponentSource componentCurrent);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] prefabPath
	void EOnDelete(EditablePrefabsConfig config, WorldEditorAPI api, string prefabPath);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] currentPath
	//! \param[in] newPath
	void EOnMove(EditablePrefabsConfig config, WorldEditorAPI api, string currentPath, string newPath);

	//------------------------------------------------------------------------------------------------
	// constructor
	void EditablePrefabsComponent_Base()
	{
		if (m_Component.IsEmpty()) return;

		//--- Misconfigured
		Resource componentResource = Resource.Load(m_Component);
		if (!componentResource.IsValid())
			return;

		BaseResourceObject componentBase = componentResource.GetResource();
		if (!componentBase)
			return;

		BaseContainer componentContainer = componentBase.ToBaseContainer();
		if (!componentContainer)
			return;

		m_sClassname = componentContainer.GetClassName();
	}
}
#endif // WORKBENCH
