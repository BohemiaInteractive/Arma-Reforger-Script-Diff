[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetCharacterStance : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target character entity to change stance of)")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter; 
	
	[Attribute(defvalue: "0", desc: "New stance", uiwidget: UIWidgets.ComboBox, enumType: ECharacterStance)]
	ECharacterStance m_eStance;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;
		
		CharacterControllerComponent characterController = CharacterControllerComponent.Cast(entity.FindComponent(CharacterControllerComponent));
		if (!characterController)
			return;
		
		switch (m_eStance)
		{
			case ECharacterStance.PRONE:
			{
				characterController.SetStanceChange(3);
				break;
			}
			
			case ECharacterStance.CROUCH:
			{
				characterController.SetStanceChange(2);
				break;
			}
			
			case ECharacterStance.STAND:
			{
				characterController.SetStanceChange(1);
				break;
			}
		}
	}
}