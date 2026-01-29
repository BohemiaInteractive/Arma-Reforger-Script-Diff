[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionStartSimpleAnimation : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play animation on")]
	ref SCR_ScenarioFrameworkGet m_ActorGetter;
	
	[Attribute(params: "anm", desc: "Resource name of the animation", category: "Asset")]
	ResourceName m_sAnimName;
	
	[Attribute("0", UIWidgets.Auto)]
	int m_iStartTime;
	
	[Attribute("1", UIWidgets.Auto)]
	float m_fSpped;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bLoop;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_ActorGetter, entity))
			return;
		
		AnimationPlayerComponent animationComponent = AnimationPlayerComponent.Cast(entity.FindComponent(AnimationPlayerComponent));
		if (!animationComponent)
		{
			if (object)
				PrintFormat("ScenarioFramework Action: Animation component not found for Action %1 attached on %2.", this, object.GetName(), level: LogLevel.ERROR);
			else
				PrintFormat("ScenarioFramework Action: Animation component not found for Action %1.", this, level: LogLevel.ERROR);

			return;
		}
		
		animationComponent.Prepare(m_sAnimName, m_iStartTime, m_fSpped, m_bLoop);
		animationComponent.Play();
	}
}