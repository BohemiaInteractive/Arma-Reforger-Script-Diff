[ComponentEditorProps(category: "GameScripted/Character", description: "Stamina component for character.", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterStaminaComponentClass : CharacterStaminaComponentClass
{
}

class SCR_CharacterStaminaComponent : CharacterStaminaComponent
{
}

[BaseContainerProps()]
class StaminaEventParams
{
}

[BaseContainerProps()]
class MetabolismEventParams : StaminaEventParams
{
	[Attribute("0", UIWidgets.EditBox, "Defines how many calories will be added/subtracted when triggering OnStaminaDrain event.")]
	float 		m_fCaloriesDrain;

	[Attribute("0", UIWidgets.EditBox, "Defines how much hydration will be added/subtracted when triggering OnStaminaDrain event.")]
	float 		m_fHydrationDrain;

	[Attribute("0", UIWidgets.EditBox, "Defines how much energy will be added/subtracted when triggering OnStaminaDrain event.")]
	float 		m_fEnergyDrain;
}
