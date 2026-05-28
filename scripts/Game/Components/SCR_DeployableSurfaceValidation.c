//! This is a base variant that does nothing, and thus it must NOT be used
class SCR_DeployableSurfaceValidation : ScriptAndConfig
{
	//------------------------------------------------------------------------------------------------
	//! Validates if provided entity can be considered as a valid surface for deployment
	//! \param[in] surfaceEnt entity on which the item is going to be deployed
	//! \param[in] worldPosition position in world space
	//! \param[in] surfaceNorm normal of the face on which the item is going to be deployed
	//! \param[in] nodeIndex bone id to which is attached the collider on which the item is going to be deployed
	//! \param[in] colliderIndex collider id on which the item is going to be deployed
	//! \param[in] surfaceProps properties of the surface on which the item is going to be deployed
	//! \param[in] surfaceMaterial name of the material of the surface on which the item is going to be deployed
	//! \param[in] colliderName name of the collider on which the item is going to be deployed
	//! \return true if item can be deployed on this surface, otherwise false
	bool IsSurfaceValid(IEntity surfaceEnt, vector worldPosition, vector surfaceNorm, int nodeIndex, int colliderIndex, SurfaceProperties surfaceProps, string surfaceMaterial, string colliderName);
}
