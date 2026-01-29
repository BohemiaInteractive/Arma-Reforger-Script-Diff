class SCR_PhysicsObserverAttribute : BaseItemAttributeData
{
	//------------------------------------------------------------------------------------------------
	//! Callback method used to inform that provided entity physics state has changed
	//! \param[in] owner
	//! \param[in] isActive
	void OnPhysicsStateChanged(notnull IEntity owner, bool isActive);
}
