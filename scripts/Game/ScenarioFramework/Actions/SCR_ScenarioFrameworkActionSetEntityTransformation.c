[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityTransformation : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to change (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;

	[Attribute(desc: "Name of the entity, which transformation will be used)")]
	ref SCR_ScenarioFrameworkGet m_TransformationEntityGetter;

	[Attribute(desc: "X rotation (Optional)")]
	vector m_vRotationX;
	
	[Attribute(desc: "Y rotation (Optional)")]
	vector m_vRotationY;
	
	[Attribute(desc: "Y rotation (Optional)")]
	vector m_vRotationZ;
	
	[Attribute(desc: "Translation (Optional)")]
	vector m_vTranslation;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;
		
		vector transformation[4];
		
		if (m_TransformationEntityGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> transformationEntityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_TransformationEntityGetter.Get());
			if (!transformationEntityWrapper)
			{
				Print(string.Format("ScenarioFramework Action: Transformation Entity Getter has issues for action %1. Action won't do anything.", this), LogLevel.ERROR);
				return;
			}

			IEntity transformationEntity = transformationEntityWrapper.GetValue();
			if (!transformationEntity)
			{
				Print(string.Format("ScenarioFramework Action: Transformation Entity could not be found for action %1. Action won't do anything.", this), LogLevel.ERROR);
				return;
			}
			
			transformationEntity.GetTransform(transformation);
		}
		else
		{
			transformation[0] = m_vRotationX;
			transformation[1] = m_vRotationY;
			transformation[2] = m_vRotationZ;
			transformation[3] = m_vTranslation;
		}
		
		entity.SetTransform(transformation);
		entity.Update();
	}
}