[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionStartAnimation : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play animation on")]
	ref SCR_ScenarioFrameworkGet m_ActorGetter;
	
	[Attribute(desc: "Prop to be alligned to")]
	ref SCR_ScenarioFrameworkGet m_PropAllignToGetter;
	
	[Attribute(params: "agr", desc: "Resource name of the object to be spawned", category: "Asset")]
	ResourceName m_sGraphName;
	
	[Attribute(params: "asi", desc: "Resource name of the object to be spawned", category: "Asset")]
	ResourceName m_GraphInstanceName;
	
	[Attribute("0", UIWidgets.Auto)]
	int m_iNPC;
	
	[Attribute("true", UIWidgets.Auto)]
	bool m_bRootMotion;
	
	[Attribute("true", UIWidgets.Auto)]
	bool m_bHolsterWeapon;
	
	private bool allignToProp;

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
		
		SCR_LoiterCustomAnimData customAnimData = new SCR_LoiterCustomAnimData();
		
		animComp.SetVariableInt(staticTable.m_OfficerMissionSpeech, m_iNPC);
		
		IEntity propEntity;
		vector vAnimationTransform[4] = { "1 0 0", "0 1 0", "0 0 1", "0 0 0" };
		allignToProp = false;
		
		if (m_PropAllignToGetter && ValidateInputEntity(object, m_PropAllignToGetter, propEntity))
		{
			propEntity.GetWorldTransform(vAnimationTransform);	
			allignToProp = true;
			
		}
		
		customAnimData.m_CustomCommand = staticTable.m_CustomCinematicCommand; 
		customAnimData.m_GraphName = m_sGraphName; 
		customAnimData.m_GraphInstanceName = m_GraphInstanceName;   
		controller.StartLoitering(ELoiteringType.CUSTOM, m_bHolsterWeapon, m_bRootMotion, allignToProp, vAnimationTransform, true, customAnimData);
	}
}