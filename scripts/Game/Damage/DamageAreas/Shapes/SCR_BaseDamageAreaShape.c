class SCR_BaseDamageAreaShape : BaseDamageAreaShape
{
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//! Method used to execute the logic when some property is changed in the parent entity. This method is only called in workbench.
	//! \param[in] owner
	//! \param[in] src
	//! \param[in] key name of the property that was changed
	//! \param[in] ownerContainers
	//! \param[in] parent
	//! \return true if something was chagned by this method
	bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent);

	//------------------------------------------------------------------------------------------------
	//! Method used to draw custom debug shapes using owner's shape manager. This is only present in workbench or debug version of clients.
	//! \param[in] owner
	//! \param[in,out] shapeMgr
	//! \param[in] runtime
	void DrawDebug(IEntity owner, inout SCR_DebugShapeManager shapeMgr, bool runtime = false);

	//------------------------------------------------------------------------------------------------
	//! Method used to proces the shape in order to optimize its usage in runtime
	//! \param[in] ownerSource of entity that contains this shape
	//! \param[in,out] api pointer to update entity properties
	//! \param[in] shapePath in the entity source where data of this shape is located
	//! \param[out] offset difference from current position and center of optimized shape
	//! \return true if anything was changed in this entity source, otherwise false
	bool OptimizeShape(notnull IEntitySource ownerSource, inout notnull WorldEditorAPI api, notnull array<ref ContainerIdPathEntry> shapePath, out vector offset);
#endif

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Init(IEntity owner);
}
