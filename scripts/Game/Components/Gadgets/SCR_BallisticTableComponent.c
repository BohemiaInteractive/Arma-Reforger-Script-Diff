[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_BallisticTableComponentClass : SCR_DataDisplayGadgetComponentClass
{
}

class SCR_BallisticTableComponent : SCR_DataDisplayGadgetComponent
{
	//------------------------------------------------------------------------------------------------
	//! Interface for changing page to a different shell type
	//! \param[in] direction -1 == previous | +1 == next
	void ChangeShellType(int direction = 0)
	{
		if (direction == 0 || m_iNumberOfPages < 2)
			return;

		if (!m_wRenderTargetTexture)
			return;

		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		ResourceName projectilePrefab;
		SCR_VisualisedBallisticConfig currentData = SCR_VisualisedBallisticConfig.Cast(data.GetPage(m_iCurrentPageID));
		if (currentData)
			projectilePrefab = currentData.GetProjectilePrefab();

		int newDataId = m_iCurrentPageID + direction;
		SCR_VisualisedBallisticConfig replacementData;
		while (newDataId != m_iCurrentPageID)
		{
			if (newDataId >= m_iNumberOfPages)
				newDataId = 0;
			else if (newDataId < 0)
				newDataId = m_iNumberOfPages - 1;

			replacementData = SCR_VisualisedBallisticConfig.Cast(data.GetPage(newDataId));
			if (replacementData && replacementData.GetProjectilePrefab() != projectilePrefab)
				break;

			newDataId += direction;
		}

		if (newDataId == m_iCurrentPageID)
			return;

		m_iCurrentPageID = newDataId;
		UpdateData();
	}
}
