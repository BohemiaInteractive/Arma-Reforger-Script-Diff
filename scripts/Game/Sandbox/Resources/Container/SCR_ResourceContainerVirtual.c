[BaseContainerProps()]
class SCR_ResourceContainerVirtual : SCR_ResourceContainer
{
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, params: "inf inf purpose=coords space=entity")]
	protected vector m_vBoundingVolumeMins;
	
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, params: "inf inf purpose=coords space=entity")]
	protected vector m_vBoundingVolumeMaxs;
	
	//------------------------------------------------------------------------------------------------
	override void GetBoundingVolume(inout vector mins, inout vector maxs)
	{
		mins = m_vBoundingVolumeMins;
		maxs = m_vBoundingVolumeMaxs;
	}
	
	override void GetAxisAlignedBoundingVolume(inout vector mins, inout vector maxs)
	{
		if (!m_Owner)
			return;
		
		vector mat[3];
		
		m_Owner.GetWorldTransform(mat);
		
		vector p1	= m_vBoundingVolumeMins;
		vector p2	= m_vBoundingVolumeMaxs;
		vector p3	= p1;
		vector p4	= p2;
		
		p3[2]	= p3[2] + p2[2];
		p4[2]	= p4[2] + p1[2];
		
		p1	= p1.Multiply3(mat);
		p2	= p2.Multiply3(mat);
		p3	= p3.Multiply3(mat);
		p4	= p4.Multiply3(mat);
		
		float minX =  Math.Min(Math.Min(Math.Min(p1[0], p2[0]), p3[0]), p4[0]);
		float maxX =  Math.Max(Math.Max(Math.Max(p1[0], p2[0]), p3[0]), p4[0]);
		float minZ =  Math.Min(Math.Min(Math.Min(p1[2], p2[2]), p2[2]), p4[2]);
		float maxZ =  Math.Max(Math.Max(Math.Max(p1[2], p2[2]), p2[2]), p4[2]);
		
		mins = Vector(minX, p1[1], minZ);
		maxs = Vector(maxX, p2[1], maxZ);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CopyFromContainer(notnull SCR_ResourceContainer container)
	{
		super.CopyFromContainer(container);
		
		SCR_ResourceContainerVirtual containerCasted = SCR_ResourceContainerVirtual.Cast(container);
		
		m_vBoundingVolumeMins = containerCasted.m_vBoundingVolumeMins;
		m_vBoundingVolumeMaxs = containerCasted.m_vBoundingVolumeMaxs;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsInRange(vector origin, float range)
	{		
		return Math3D.IntersectionSphereAABB(origin - m_Owner.GetOrigin(), range, m_vBoundingVolumeMins, m_vBoundingVolumeMaxs);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DebugDraw(bool shouldShowRange = true)
	{
		super.DebugDraw(shouldShowRange);
		
		if (!m_Owner)
			return;
		
		Color color1 = GetDebugColor();
		Color color2 = GetDebugColor();
		vector containerMat[4];
		
		m_Owner.GetWorldTransform(containerMat);
		color1.SetA(0.5);
		color2.SetR(0.0);
		color2.SetG(0.0);
		color2.SetB(0.0);
		color2.SetA(1.0);
		Shape.Create(ShapeType.BBOX, color1.PackToInt(), ShapeFlags.ONCE | ShapeFlags.DEPTH_DITHER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE, m_vBoundingVolumeMins, m_vBoundingVolumeMaxs).SetMatrix(containerMat);
		containerMat[0] = containerMat[0] * 1.05;
		containerMat[1] = containerMat[1] * 1.05;
		containerMat[2] = containerMat[2] * 1.05;
		Shape.Create(ShapeType.BBOX, color2.PackToInt(), ShapeFlags.ONCE | ShapeFlags.DEPTH_DITHER | ShapeFlags.WIREFRAME, m_vBoundingVolumeMins, m_vBoundingVolumeMaxs).SetMatrix(containerMat);
	}
}