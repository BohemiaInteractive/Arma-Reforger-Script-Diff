//------------------------------------------------------------------------------------------------
class SCR_VehicleWeaponActionBase : SCR_VehicleActionBase
{
	protected SCR_FireModeManagerComponent fireModeManager;
	
	//! Movement sound event name
	[Attribute(desc: "Action sound event name")]
	protected string m_sActionSoundEvent;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		fireModeManager = SCR_FireModeManagerComponent.Cast(GetOwner().FindComponent(SCR_FireModeManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! By default toggle the current state of the interaction
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		PlayActionSound();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayActionSound()
	{
		if (!m_sActionSoundEvent.IsEmpty())
			return;
		
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().GetRootParent().FindComponent(SoundComponent));
		if (!soundComp)
			return;
		
		vector contextTransform[4];	
		GetActiveContext().GetTransformationModel(contextTransform);
		soundComp.SoundEventOffset(m_sActionSoundEvent, contextTransform[3]);
	}
}
