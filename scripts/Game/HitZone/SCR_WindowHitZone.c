class SCR_WindowHitZone : SCR_HitZone
{
	override protected void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		
		if (!GetGame().InPlayMode())
			return;
		
		// Update the associated portal when the window is destroyed / restored
		if (newState == EDamageState.DESTROYED)
			SetSoundPassing(true);
		else if (newState == EDamageState.UNDAMAGED)
			SetSoundPassing(false);
	}
	
	private void SetSoundPassing(bool value)
	{	
		HitZoneContainerComponent container = GetHitZoneContainer();
		if (container == null)
			return;
		
		IEntity owner = container.GetOwner();
		
		// Find the entity with the subscene
		WorldSubsceneComponent subscene;
		IEntity parent = owner;
		while (parent != null)
		{
			subscene = WorldSubsceneComponent.Cast(parent.FindComponent(WorldSubsceneComponent));
			if (subscene != null)
				break;

			parent = parent.GetParent();
		}
		
		if (subscene == null)
			return;
		
		// window world position
		vector posHZ[4];
		owner.GetWorldTransform(posHZ);
		// subscene entity world position
		vector matWS[4];
		parent.GetWorldTransform(matWS);
		
		// Invert the transform so we get the local position of the window within the parent
		vector matLS[4];
		Math3D.MatrixInvMultiply4(matWS, posHZ, matLS);
		
		// Find the portal and update the sound passing flag as necessary
		vector mins = matLS[3] - { 0.2, 0.2, 0.2 };
		vector maxs = matLS[3] + { 0.2, 0.2, 0.2 };
		int portalIdx = subscene.FindPortalByOBB(mins, maxs, matWS);
		if (portalIdx < 0 || portalIdx >= 0xFFFF)
			return;
		
		subscene.SetPortalPassingSound(portalIdx, value);
	}
}
