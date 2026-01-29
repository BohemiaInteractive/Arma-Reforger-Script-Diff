[EntityEditorProps(category: "GameScripted/Components", description: "Registers characters and holds them in an array.", color: "0 0 255 255")]
class SCR_CharacterRegistrationComponentClass : ScriptComponentClass
{
}

class SCR_CharacterRegistrationComponent : ScriptComponent
{
	protected static ref array<SCR_ChimeraCharacter> s_aChimeraCharacters = {};
	
	protected IEntity m_owner;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static array<SCR_ChimeraCharacter> GetChimeraCharacters()
	{
		return s_aChimeraCharacters;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (!s_aChimeraCharacters)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (character && s_aChimeraCharacters.Find(character) != -1)
			s_aChimeraCharacters.RemoveItem(character);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_owner = owner;
		
		if (!s_aChimeraCharacters)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (character)
			s_aChimeraCharacters.Insert(character);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CharacterRegistrationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CharacterRegistrationComponent()
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_owner);
		if (character && s_aChimeraCharacters.Find(character) != -1)
			s_aChimeraCharacters.RemoveItem(character);
	}
}
