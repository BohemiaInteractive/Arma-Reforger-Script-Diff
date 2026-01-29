#ifdef WORKBENCH
//! Data container that holds the individual wall asset related information
class SCR_WallGroupContainer
{
	bool m_bGenerated;
	float m_fMiddleObjectLength;
	float m_fSmallestWall = float.MAX;

	// for quick access, wallgroup lengths are in a seperate array (indices corresponding with those in wallgroup array)
	protected ref array<float> m_aLengths = {};

	protected ref array<ref SCR_WallGroup> m_aWallGroups = {}; // sorted by ascending length

	//------------------------------------------------------------------------------------------------
	//! \return true if empty, false otherwise
	bool IsEmpty()
	{
		return m_aWallGroups.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] biggestSmallerThan value under which the biggest wall must be (exclusive, using <)
	//! \return a random wall of the max wanted size or null if not found
	SCR_WallPair GetRandomWall(float biggestSmallerThan, float value01)
	{
		for (int i = m_aLengths.Count() - 1; i >= 0; --i) // find the longest wall which is smaller than given amount
		{
			if (m_aLengths[i] < biggestSmallerThan)
				return m_aWallGroups[i].GetRandomWall(value01);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Goes through walls and prepares a data structure that is used during wall generation
	//! \param[in] groups
	//! \param[in] forward
	//! \param[in] middleObj
	void PrepareWallGroups(notnull array<ref WallLengthGroup> groups, bool forward, string middleObj)
	{
		m_aWallGroups.Clear();

		array<ref SCR_WallGroup> wallGroupsTmp = {};

		int forwardAxis;
		if (!forward)
			forwardAxis = 2;

		if (!middleObj.IsEmpty())
			m_fMiddleObjectLength = WallGeneratorEntity.MeasureEntity(middleObj, forwardAxis);

		SCR_WallGroup wallGroup;
		SCR_WallPair wallPair;
		float wallLength;
		foreach (WallLengthGroup group : groups)
		{
			wallGroup = new SCR_WallGroup();

			foreach (WallWeightPair pair : group.m_aWallPrefabs)
			{
				wallPair = new SCR_WallPair();

				wallPair.m_sWallAsset = pair.m_sWallAsset;
				wallPair.m_fPostPadding = pair.m_fPostPadding;
				wallPair.m_fPrePadding = pair.m_fPrePadding;
				wallGroup.m_aWeights.Insert(pair.m_fWeight);

				wallLength = WallGeneratorEntity.MeasureEntity(wallPair.m_sWallAsset, forwardAxis);
				// wallLength += wallPair.m_fPostPadding;
				wallPair.m_fWallLength = wallLength;

				if (wallLength > wallGroup.m_fWallLength)
					wallGroup.m_fWallLength = wallLength; // biggest length is the one being used for the whole group

				if (!wallPair.m_sWallAsset.IsEmpty())
					wallGroup.m_aWallPairs.Insert(wallPair);
			}

			if (!wallGroup.m_aWallPairs.IsEmpty())
			{
				wallGroupsTmp.Insert(wallGroup);
				m_bGenerated = true;
			}
		}

		// order groups by length
		while (!wallGroupsTmp.IsEmpty())
		{
			float smallestSize = float.MAX;
			float smallestIndex;
			foreach (int i, SCR_WallGroup groupTmp : wallGroupsTmp)
			{
				if (groupTmp.m_fWallLength < smallestSize)
				{
					smallestSize = groupTmp.m_fWallLength;
					if (smallestSize < m_fSmallestWall)
						m_fSmallestWall = smallestSize;

					smallestIndex = i;
				}
			}

			m_aWallGroups.Insert(wallGroupsTmp[smallestIndex]);
			m_aLengths.Insert(wallGroupsTmp[smallestIndex].m_fWallLength);
			wallGroupsTmp.Remove(smallestIndex);
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_WallGroupContainer(notnull array<ref WallLengthGroup> items, bool forward, string middleObj)
	{
		PrepareWallGroups(items, forward, middleObj);
	}
}
#endif // WORKBENCH
