//! A system to help with easing the slowdown effect applied to the character
class SCR_CharacterSlowdownEasingSystem : GameSystem
{
	protected ref array<SCR_ChimeraCharacter> m_aCharacters = {};
	
	[Attribute(defvalue: "3", desc: "Transition Strength", params: "0 inf 0.01")]
	protected float m_fTransitionStrength;

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Both);
		outInfo.AddPoint(WorldSystemPoint.FixedFrame);
	}
		
	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float dt = args.GetTimeSliceSeconds();
		
		for (int idx = m_aCharacters.Count() - 1; idx >= 0; idx--)
		{
			SCR_ChimeraCharacter character = m_aCharacters[idx];
			if (!character)
			{
				m_aCharacters.Remove(idx);
				continue;			
			}
			
			character.UpdateSlowdown(dt * m_fTransitionStrength);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add a new character to the system so they get updated. 
	//! \param[in] character Character that will be subscribed to the system.
	void Register(notnull SCR_ChimeraCharacter character)
	{
		if (!m_aCharacters.Contains(character))
			m_aCharacters.Insert(character);
		
		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove a character in the system so they no longer get updated. 
	//! \param[in] character Character that will be removed from the system.
	void Unregister(notnull SCR_ChimeraCharacter character)
	{
		if (m_aCharacters.Contains(character))
			m_aCharacters.RemoveItem(character);
		
		if (m_aCharacters.IsEmpty())
			Enable(false);
	}
}