//! Collection of wall pair data objects into a single wall group.
//! Wall group is meant to group walls of the same length. It's possible to add shorter wall(s) to a wall group as well(a feature required by map designers), but the wall group as a whole is considered to have only a single length(that of the longest wall in the wallgroup) and when considering which wall group is selected during generation,
//! it's the wall group length as a whole that's being considered, not individual walls within the group. When placing the walls during wall generation, after it's already been decided which wall group will satisfy the length requirenment,
//! and after a particular wall pair was selected based on the weights, the actual prefab is placed in the world and the exact length of this prefab is then deducted from the remaining segment(so no gap is created by possibly using a shorter variance).
class SCR_WallGroup
{
	ref array<ref SCR_WallPair> m_aWallPairs = {};
	ref array<float> m_aWeights = {};

	float m_fWallLength;

	//------------------------------------------------------------------------------------------------
	//! Get a random wall piece, respecting piece weights
	//! \return a weighted, random wall pair
	SCR_WallPair GetRandomWall(float value01)
	{
		int index = SCR_ArrayHelper.GetWeightedIndex(m_aWeights, value01);
		return m_aWallPairs.Get(index);
	}
}
