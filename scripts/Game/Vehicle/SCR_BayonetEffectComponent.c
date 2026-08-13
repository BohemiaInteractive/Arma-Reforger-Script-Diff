class SCR_BayonetEffectComponentClass : SCR_ImpactEffectComponentClass
{
}

class SCR_BayonetEffectComponent : SCR_ImpactEffectComponent
{	
	//------------------------------------------------------------------------------------------------
	override void OnImpact(notnull IEntity other, float impulse, vector impactPosition, vector impactNormal, GameMaterial mat, vector velocityBefore = vector.Zero, vector velocityAfter = vector.Zero)
	{		
		vector transform[4];
		Math3D.MatrixFromUpVec(impactNormal, transform);
		transform[3] = impactPosition;

		GameMaterial material = mat;
		HitEffectInfo effectInfo = material.GetHitEffectInfo();		
		ResourceName resourceName = effectInfo.GetBayonetHitParticleEffect();

		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[0];
		
		EmitParticles(transform, resourceName);	
		Rpc(RPC_OnImpactParticlesBroadcast, impactPosition, impactNormal, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	override protected void RPC_OnImpactParticlesBroadcast(vector contactPos, vector contactNormal, int magnitude)
	{
		vector transform[4];
		Math3D.MatrixFromUpVec(contactNormal, transform);
		transform[3] = contactPos;
		
		TraceParam trace = new TraceParam();
		trace.Start = contactPos + contactNormal;
		trace.End = contactPos - contactNormal;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		array<IEntity> excludedArray = GetExcludedEntities();
		trace.ExcludeArray = excludedArray;

		GetOwner().GetWorld().TraceMove(trace, TraceFilter);
		
		GameMaterial contactMat = trace.SurfaceProps;
		HitEffectInfo effectInfo = contactMat.GetHitEffectInfo();		
		ResourceName resourceName = effectInfo.GetBayonetHitParticleEffect();

#ifdef SCR_MELEE_DEBUG
		PrintFormat("SCR_MELEE_DEBUG->SCR_BayonetEffectComponent.RPC_OnImpactParticlesBroadcast: Client detected %1 as hit entity", trace.TraceEnt);
		if (contactMat)
			PrintFormat("SCR_MELEE_DEBUG->SCR_BayonetEffectComponent.RPC_OnImpactParticlesBroadcast: Hit material is %2 ", contactMat);

		if (!resourceName.IsEmpty())
			PrintFormat("SCR_MELEE_DEBUG->SCR_BayonetEffectComponent.RPC_OnImpactParticlesBroadcast: As a result client will spawn \"%1\"", resourceName);
#endif

		if (resourceName.IsEmpty())
		{
			resourceName = GetDefaultParticles()[magnitude];

#ifdef SCR_MELEE_DEBUG
			PrintFormat("SCR_MELEE_DEBUG->SCR_BayonetEffectComponent.RPC_OnImpactParticlesBroadcast: No siutable particle was found, thus game will spawn default particles \"%1\"", resourceName);
#endif
		}
		
		EmitParticles(transform, resourceName);
	}
}

