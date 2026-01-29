//------------------------------------------------------------------------------------------------
class SCR_LadderUserAction : ScriptedUserAction
{
	static const float MAX_LADDER_TEST_DISTANCE = 5.0;
	static const float MAX_LADDER_ENTRY_ANGLE = 75.0;
	
	[Attribute("0", UIWidgets.EditBox, "Which ladder component to use", "0 inf")]
	protected int m_iLadderComponentIndex;
	
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		array<Managed> ladderComponents = {};
		pOwnerEntity.FindComponents(LadderComponent, ladderComponents);
		if (!ladderComponents.IsIndexValid(m_iLadderComponentIndex))
		{
			Print("m_iLadderComponentIndex : " + m_iLadderComponentIndex + " is not an index valid", LogLevel.ERROR);
			return;
		}
	}
	
	protected SCR_CharacterCommandHandlerComponent FindCommandHandler(IEntity pUser)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUser);
		if (!character)
			return null;
		
		CharacterAnimationComponent animationComponent = character.GetCharacterController().GetAnimationComponent();
		if (!animationComponent)
			return null;
		
		return SCR_CharacterCommandHandlerComponent.Cast(animationComponent.FindComponent(SCR_CharacterCommandHandlerComponent));
	}
	
	protected override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		CharacterControllerComponent controller = character.GetCharacterController();
		controller.TryUseLadder(GetOwner(), m_iLadderComponentIndex, MAX_LADDER_TEST_DISTANCE, MAX_LADDER_ENTRY_ANGLE);
	}	
	
	protected override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		return controller && controller.CanUseLadder(GetOwner(), m_iLadderComponentIndex, MAX_LADDER_TEST_DISTANCE, MAX_LADDER_ENTRY_ANGLE);
	}
	
	protected override bool CanBeShownScript(IEntity user)
	{		
		// Already on a ladder
		SCR_CharacterCommandHandlerComponent cmdHandler = FindCommandHandler(user);
		return cmdHandler && !cmdHandler.GetCommandLadder();
	}
	
	//! Action is performed through input actions of CharacterController, 
	//! therefore it shan't be replicated via user action itself
	protected override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};