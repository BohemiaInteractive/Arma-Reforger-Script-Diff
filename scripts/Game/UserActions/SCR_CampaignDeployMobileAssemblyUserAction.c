class SCR_CampaignDeployMobileAssemblyUserAction : ScriptedUserAction
{
	protected IEntity m_TruckBed;
	protected SCR_CampaignMobileAssemblyComponent m_AssemblyComponent;
	protected DamageManagerComponent m_DamageManagerComponent;
	
	static const float MAX_TERRAIN_HEIGHT = 0.5;
	static const float TERRAIN_SLOPE_THRESHOLD = 0.9;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_TruckBed = pOwnerEntity;
		
		if (!m_TruckBed)
			return;
		
		m_AssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(m_TruckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	DamageManagerComponent GetDamageManagerComponent()
	{
		if (m_DamageManagerComponent)
			return m_DamageManagerComponent;
		
		if (!m_TruckBed)
			return null;
		
		IEntity truck = m_TruckBed.GetParent();
		
		if (!truck)
			return null;
		
		m_DamageManagerComponent = DamageManagerComponent.Cast(truck.FindComponent(DamageManagerComponent));
		
		return m_DamageManagerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		float depth;
		if (SCR_WorldTools.IsObjectUnderwater(GetOwner(), vector.Zero, -1, depth) && depth > SCR_CampaignMobileAssemblyComponent.MAX_WATER_DEPTH)
			return false;
		
		if (!m_AssemblyComponent)
			return false;
		
		Faction f = SCR_FactionManager.SGetLocalPlayerFaction();
		
		if (!f)
			return false;
		
		if (f != m_AssemblyComponent.GetParentFaction())
			return false;
		
		if (GetDamageManagerComponent() && GetDamageManagerComponent().GetState() == EDamageState.DESTROYED)
			return false;
		
		if (m_TruckBed)
		{
			IEntity truck = m_TruckBed.GetParent();
			
			if (truck)
			{
				Physics physicsComponent = truck.GetPhysics();
				vector vel = physicsComponent.GetVelocity();
				vel[1] = 0; // Ignore Y velocity
				
				if (physicsComponent && vel.LengthSq() > 0.01)
					return false;
			}
		}
		
		return !m_AssemblyComponent.IsDeployed();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_AssemblyComponent)
			return false;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		SCR_CampaignFaction f = m_AssemblyComponent.GetParentFaction();
		
		if (!f)
			return false;
		
		if (f.GetMobileAssembly())
		{
			SetCannotPerformReason("#AR-Campaign_Action_AnotherAssemblyActive-UC");
			return false;
		}
		
		SCR_CampaignFeedbackComponent feedbackComponent = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!feedbackComponent)
			return false;
		
		if (feedbackComponent.GetBaseWithPlayer())
		{
			SetCannotPerformReason("#AR-Campaign_Action_AnotherHQNearby-UC");
			return false;
		}
		
		bool isInRange = m_AssemblyComponent.IsInRadioRange();
		
		if (!isInRange)
			SetCannotPerformReason("#AR-Campaign_Action_NoSignal");
		
		IEntity truck = m_TruckBed.GetParent();
		
		if (truck)
		{
			vector pos = truck.GetOrigin();
			
			if (SCR_TerrainHelper.GetHeightAboveTerrain(pos) > MAX_TERRAIN_HEIGHT)
			{
				SetCannotPerformReason("#AR-Campaign_Action_UnevenTerrain-UC");
				return false;
			}
			
			/*if (SCR_TerrainHelper.GetTerrainNormal(pos)[1] < TERRAIN_SLOPE_THRESHOLD)
			{
				SetCannotPerformReason("#AR-Campaign_Action_UnevenTerrain-UC");
				return false;
			}*/
		}
		
		return isInRange;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.DeployMobileAsembly(m_AssemblyComponent, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};