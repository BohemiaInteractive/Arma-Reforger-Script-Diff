[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterControllerComponent_TestClass : SCR_CharacterControllerComponentClass
{
}

class SCR_CharacterControllerComponent_Test : SCR_CharacterControllerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		am.SetActionValue("CharacterForward", 1);
		super.OnPrepareControls(owner, am, dt, player);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CharacterControllerComponent_Test(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
}
