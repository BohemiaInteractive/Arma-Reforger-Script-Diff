[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionStartAnimation : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play animation on")]
	ref SCR_ScenarioFrameworkGet m_ActorGetter;
	
	[Attribute(desc: "Prop to be alligned to")]
	ref SCR_ScenarioFrameworkGet m_PropAllignToGetter;
	
	[Attribute(params: "agr", desc: "Resourcename to define the animation graph file reference", category: "Asset")]
	ResourceName m_sGraphName;
	
	[Attribute(params: "asi", desc: "Resourcename to define the animation set instance file reference", category: "Asset")]
	ResourceName m_GraphInstanceName;
	
	[Attribute("0", desc: "Control variable for the animation, refers here to what npc animation we intend to use", UIWidgets.Auto)]
	int m_iNPC;
	
	[Attribute("true", UIWidgets.Auto)]
	bool m_bRootMotion;
	
	[Attribute("true", UIWidgets.Auto)]
	bool m_bHolsterWeapon;

	[Attribute("true", UIWidgets.Auto)]
	bool m_bUseCinematicCommand;
	
	[Attribute(defvalue: "1", desc: "Allow environmental exit on the animation, meaning the animation can get stopped by isSwimming or isFalling checks.\nDeny if you want to force an npc to do an animation no matter what, especially useful when attaching them to a pivot")]
	bool m_bAnimationAllowEnvironmentalExit;

	private bool m_bAlignToProp;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_ActorGetter, entity))
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;
		
		CharacterAnimationComponent animComp = controller.GetAnimationComponent();
		if (!animComp)
			return;

		SCR_CharacterCommandHandlerComponent cmdHandler = SCR_CharacterCommandHandlerComponent.Cast(animComp.GetCommandHandler());
		if (!cmdHandler)
			return;

		cmdHandler.StartCommand_Move();

		SCR_ScriptedCommandsStaticTable staticTable = cmdHandler.GetScriptedStaticTable();
		if (!staticTable)
			return;

		int commandId = -1;
		string commandName;

		if (m_bUseCinematicCommand)
		{
			commandId = staticTable.m_CustomCinematicCommand;
			commandName = SCR_CustomAnimData_Properties.BINDING_COMMAND_NAME;
		}

		SCR_LoiterCustomAnimData customAnimData = SCR_LoiterCustomAnimData.CreateInstance(
				commandID : commandId,
				commandValue: ELoiteringType.CUSTOM,
				commandName : commandName,
				graphName : m_sGraphName,
				graphInstanceName: m_GraphInstanceName,
				controlVariableName : staticTable.CONTROL_VARIABLE_NAME,
				controlVariableID: staticTable.m_OfficerMissionSpeech,
				controlVariableValue: m_iNPC,
				allowRootMotion : m_bRootMotion,
				allowEnvironmentalExit: m_bAnimationAllowEnvironmentalExit);

		IEntity propEntity;
		vector vAnimationTransform[4] = { "1 0 0", "0 1 0", "0 0 1", "0 0 0" };
		m_bAlignToProp = false;
		
		if (m_PropAllignToGetter && ValidateInputEntity(object, m_PropAllignToGetter, propEntity))
		{
			propEntity.GetWorldTransform(vAnimationTransform);	
			m_bAlignToProp = true;
		}

		controller.StartLoitering(propEntity, ELoiteringType.CUSTOM , m_bHolsterWeapon, m_bRootMotion, m_bAlignToProp, vAnimationTransform, true, customAnimData);
	}
}