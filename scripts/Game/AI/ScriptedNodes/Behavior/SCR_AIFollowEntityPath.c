/**
* Task that takes group and uses it path to move
*/

class SCR_AIFollowEntityPath : SCR_AIActionTask
{
	static const string PORT_ENTITY_IN = "GroupIn";

	//------------------------------------------------------------------------------------------------
	ChimeraCharacter m_pCharacter;
	GenericEntity m_pFollowedEntity;
	bool setupResult;
	AIBaseMovementComponent m_pMyMovementComponent;
	
	static override bool VisibleInPalette()
    {
        return true;
    }
	
	override void OnEnter(AIAgent owner)
	{
		GetVariableIn(PORT_ENTITY_IN, m_pFollowedEntity);
		if (!m_pFollowedEntity)
		{
			m_pFollowedEntity = owner.GetParentGroup();
		}

		m_pMyMovementComponent = owner.GetMovementComponent();
		if (!m_pMyMovementComponent)
			return;

		setupResult = m_pMyMovementComponent.RequestFollowPathOfEntity(m_pFollowedEntity);
	}

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_pMyMovementComponent || !setupResult)
			return ENodeResult.FAIL;
		
		if (m_pMyMovementComponent.HasCompletedRequest(true))
			return ENodeResult.SUCCESS;
		return ENodeResult.RUNNING;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENTITY_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
};