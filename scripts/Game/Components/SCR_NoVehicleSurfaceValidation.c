class SCR_NoVehicleSurfaceValidation : SCR_DeployableSurfaceValidation
{
	//------------------------------------------------------------------------------------------------
	override bool IsSurfaceValid(IEntity surfaceEnt, vector worldPosition, vector surfaceNorm, int nodeIndex, int colliderIndex, SurfaceProperties surfaceProps, string surfaceMaterial, string colliderName)
	{
		while (surfaceEnt)
		{
			if (Vehicle.Cast(surfaceEnt))
				return false;

			if (Turret.Cast(surfaceEnt))
				return false;

			surfaceEnt = surfaceEnt.GetParent();
		}

		return true;
	}
}
