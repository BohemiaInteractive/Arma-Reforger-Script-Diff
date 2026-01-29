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
		
		GetOwner().GetWorld().TraceMove(trace, TraceFilter);
		
		GameMaterial contactMat = trace.SurfaceProps;
		HitEffectInfo effectInfo = contactMat.GetHitEffectInfo();		
		ResourceName resourceName = effectInfo.GetBayonetHitParticleEffect();
		
		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[magnitude];
		
		EmitParticles(transform, resourceName);
	}
}

