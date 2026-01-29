[BaseContainerProps()]
class SCR_ResourceGeneratorActionBase
{
	//------------------------------------------------------------------------------------------------
	//! Returns true if the action has constrained generation of resources.
	bool HasConstrainedGeneration()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the amount of possible generated resources.
	//! \param[in] generator The generator that the action is being computed on.
	//! \param[in] resourceValue The resource value that is to be generated.
	float ComputeGeneratedResources(notnull SCR_ResourceGenerator generator, float resourceValue)
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Performs the action for generation of resources.
	//! Override this method in order to decide how to generate resources.
	//!
	//! \param[in] generator The generator that the action is being performed on.
	//! \param[inout] resourceValue The resource value that is to be used with the action.
	void PerformAction(notnull SCR_ResourceGenerator generator, inout float resourceValue);
}