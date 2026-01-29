class SCR_ForestGeneratorRectangle
{
	ref SCR_ForestGeneratorLine m_Line1 = new SCR_ForestGeneratorLine();
	ref SCR_ForestGeneratorLine m_Line2 = new SCR_ForestGeneratorLine();
	ref SCR_ForestGeneratorLine m_Line3 = new SCR_ForestGeneratorLine();
	ref SCR_ForestGeneratorLine m_Line4 = new SCR_ForestGeneratorLine();
	ref array<SCR_ForestGeneratorLine> m_aLines = {}; // outline lines that intersect this rectangle
	ref array<vector> m_aPoints = {}; // points of this rectangle
	ref array<IEntitySource> m_aPresentRoadShapes = {};
	float m_fWidth;
	float m_fLength;
	float m_fArea;
	int m_iX;
	int m_iY;

	//------------------------------------------------------------------------------------------------
	//! \param[out] mins "bottom-left" AABB point
	//! \param[out] maxs "top-right" AABB point
	void GetBounds(out vector mins, out vector maxs)
	{
		array<vector> points = {};
		points.Insert(m_Line1.p1.m_vPos);
		points.Insert(m_Line1.p2.m_vPos);
		points.Insert(m_Line2.p2.m_vPos);
		points.Insert(m_Line3.p2.m_vPos);
		float minX = float.MAX;
		float maxX = -float.MAX;
		float minZ = float.MAX;
		float maxZ = -float.MAX;

		foreach (vector point : points)
		{
			if (point[0] < minX)
				minX = point[0];

			if (point[0] > maxX)
				maxX = point[0];

			if (point[2] < minZ)
				minZ = point[2];

			if (point[2] > maxZ)
				maxZ = point[2];
		}

		mins[0] = minX;
		mins[2] = minZ;
		maxs[0] = maxX;
		maxs[2] = maxZ;
	}
}
