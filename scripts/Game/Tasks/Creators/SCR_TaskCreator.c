[BaseContainerProps()]
class SCR_TaskCreator
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sTaskPrefab;

	//------------------------------------------------------------------------------------------------
	//! Init task creator
	//! \param[in] taskCreatorComponent
	void Init(SCR_TaskCreatorComponent taskCreatorComponent);

	//------------------------------------------------------------------------------------------------
	//! Deinit task creator
	void Deinit();
}
