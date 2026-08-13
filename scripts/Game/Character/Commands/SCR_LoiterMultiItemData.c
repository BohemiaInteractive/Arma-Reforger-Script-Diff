[BaseContainerProps()]
class SCR_LoiterMultiItemData : SCR_LoiterItemDataBase
{
	[Attribute(desc: "List of items which are going to be spawned during the loiter animation.")]
	protected ref array<ref SCR_LoiterItemDataBase> m_aItems;

	[Attribute(defvalue: "1", desc: "Determines if for spawning of all items seed will be the same, or deterministicly iterated.\nThis has only the effect if this entry has multiple item definitions using SCR_LoiterRandomItemData.")]
	protected bool m_bUseSameSeedForAllItems;

	protected const int PRIME = 2137;

	//------------------------------------------------------------------------------------------------
	override array<IEntity> SpawnPrefabs(notnull IEntity animationOwner, int seed)
	{
		array<IEntity> output = {};
		array<IEntity> tmp;
		int entrySeed = seed;
		foreach(int i, SCR_LoiterItemDataBase entry : m_aItems)
		{
			if (!m_bUseSameSeedForAllItems)
			{
				int sign = 1;
				if (i % 2)
					sign = -1;

				entrySeed = seed * sign + seed % ((i + 1) * PRIME);
			}

			tmp = entry.SpawnPrefabs(animationOwner, entrySeed);
			if (tmp)
				output.InsertAll(tmp);
		}

		return output;
	}
}
