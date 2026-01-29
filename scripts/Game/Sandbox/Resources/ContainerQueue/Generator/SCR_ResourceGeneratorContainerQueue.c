[BaseContainerProps(configRoot: true)]
class SCR_ResourceGeneratorContainerQueue : SCR_ResourceContainerQueue<SCR_ResourceGenerator>
{
	//------------------------------------------------------------------------------------------------
	override void DebugDraw()
	{
		if (!m_Interactor)
			return;

		int consumerIdx;
		float heightDiff;
		vector origin;
		int debugColour = m_Interactor.GetDebugColor().PackToInt();
		Color color1	= Color.FromInt(debugColour);
		Color color		= Color.FromInt(debugColour);
		Color color2	= Color.FromInt(debugColour);

		color.Scale(0.2);
		color.SetA(1.0);
		color2.Lerp(Color.FromInt(Color.WHITE), 0.5);
		color2.SetA(1.0);

		foreach (int idx, SCR_ResourceContainer container : m_aRegisteredContainers)
		{
			if (!container)
				continue;

			string infoText = string.Format("   GEN   Pos: %1  Distance: %2 m   ", idx, vector.Distance(m_Interactor.GetOwnerOrigin(), container.GetOwnerOrigin()));
			origin 			= container.GetOwnerOrigin();
			consumerIdx		= container.GetLinkedInteractorIndex(m_Interactor);

			if (container.IsResourceGainEnabled())
				consumerIdx++;

			if (container.IsResourceDecayEnabled())
				consumerIdx++;

			heightDiff = container.debugControlOffset * (consumerIdx + 3);

			vector offset = { 0.0, heightDiff, 0.0 };
			Shape.CreateArrow(origin + offset, origin, 0.0, 0xFFFFFFFF, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
			Shape.CreateArrow(m_Interactor.GetOwnerOrigin(), origin + offset, 0.1, color1.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER |ShapeFlags.DEPTH_DITHER);

			DebugTextWorldSpace.Create(m_Interactor.GetOwner().GetWorld(), infoText, DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.ONCE, origin[0], origin[1] + heightDiff, origin[2], 10, color.PackToInt(), color2.PackToInt(), consumerIdx + 1);

			if (!container.GetComponent().IsDebugVisualizationEnabled())
				container.DebugDraw(false);
		}
	}
}
