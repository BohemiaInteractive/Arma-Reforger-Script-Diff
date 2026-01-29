class SCR_DestroyedTreesData : Managed
{
	EntityID treeID;
	ref array<int> m_aDestroyedPartsIndexes = new array<int>();
	ref array<vector> m_aPositions = new array<vector>();
	ref array<float> m_aRotations = new array<float>();
	ref array<bool> m_aIsTreePartDynamic = new array<bool>();
	ref array<bool> m_aIsParented = new array<bool>();
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DestroyedTreesData()
	{
		m_aDestroyedPartsIndexes = null;
		m_aPositions = null;
		m_aRotations = null;
		m_aIsTreePartDynamic = null;
		m_aIsParented = null;
	}
};