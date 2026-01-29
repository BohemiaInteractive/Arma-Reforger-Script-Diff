class SCR_SourceBaseAudioComponentClass : ScriptComponentClass
{
}

class SCR_SourceBaseAudioComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	protected SCR_CampaignSourceBaseComponent m_SourceBaseComponent;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		// Spawning of base building is delayed, so we also delay the assignment of source base component
		GetGame().GetCallqueue().CallLater(SetSourceBaseComponent, 1500, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Find the source base component of owner base, caches it and subscribes to GetOnSuppliesArrivalInvoker
	protected void SetSourceBaseComponent()
	{
		IEntity ancestor = GetOwner().GetRootParent();
		if (!ancestor)
			return;

		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseSystem)
			return;

		array<SCR_MilitaryBaseComponent> bases = {};
		baseSystem.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignSourceBaseComponent sourceBaseComponent = SCR_CampaignSourceBaseComponent.Cast(base);
			if (!sourceBaseComponent)
				continue;

			if (sourceBaseComponent.GetBaseBuildingComposition() != ancestor)
				continue;

			m_SourceBaseComponent = sourceBaseComponent;
			break;
		}

		if (!m_SourceBaseComponent)
			return;

		m_SourceBaseComponent.GetOnSuppliesArrivalInvoker().Insert(PlaySupplyArrivalSound);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySupplyArrivalSound()
	{
		RpcDo_PlaySupplyArrivalSound();
		Rpc(RpcDo_PlaySupplyArrivalSound);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlaySupplyArrivalSound()
	{
		SCR_SoundManagerModule soundManagerModule = SCR_SoundManagerModule.GetInstance(GetGame().GetWorld());
		if (!soundManagerModule)
			return;

		soundManagerModule.CreateAndPlayAudioSource(GetOwner(), m_AudioSourceConfiguration);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SourceBaseAudioComponent()
	{
		if (!m_SourceBaseComponent)
			return;

		m_SourceBaseComponent.GetOnSuppliesArrivalInvoker().Remove(PlaySupplyArrivalSound);
	}
}
