class SCR_2DPiPScopeZeroingGenerator : ScriptedBaseZeroingGenerator
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Reference projectile", params: "et")]
	protected ResourceName m_sReferenceProjectile;

	protected SCR_2DPIPSightsComponent m_SightsComponent;
	protected float m_fFovZoomed;

	//------------------------------------------------------------------------------------------------
	override bool WB_InitGenerator()
	{
		SetPointInfoGeneration(false);
		SetProjectileResource(m_sReferenceProjectile);
		
		BaseSightsComponent bsc = GetSights();
		m_SightsComponent = SCR_2DPIPSightsComponent.Cast(bsc);
		if (m_SightsComponent)
		{
			float magnification = m_SightsComponent.GetMagnification();
			const float s_fReferenceFOV = 38; // Hardcoded, depends on player settings but this should work here
			m_fFovZoomed = Math.RAD2DEG * 2 * Math.Atan2(Math.Tan(Math.DEG2RAD * (s_fReferenceFOV / 2)), magnification);
			return true;
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool WB_GetZeroingData(float weaponAngle, float distance, out vector offset, out vector angles)
	{
		angles[0] = weaponAngle;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override float WB_CalculateWeaponZeroingAnimationValue(float pitch, float distance, out vector offset, out vector angles)
	{
		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(m_SightsComponent.GetComponentData(m_SightsComponent.GetOwner()));
		if (!data)
			return 0;

		if (data.GetZeroType() == SCR_EPIPZeroingType.EPZ_CAMERA_TURN)
		{
			return angles[0];
		}
		else
		{
			// Unsupported at this point.
			Print("The Reticle Offset zeroing method is currently unsupported pending Refactor of scopes", LogLevel.ERROR);
			return 0;
		}
	}
}
