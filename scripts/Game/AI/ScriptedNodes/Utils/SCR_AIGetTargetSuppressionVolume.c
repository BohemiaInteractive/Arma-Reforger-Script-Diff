class SCR_AIGetTargetSuppressionVolume : AITaskScripted
{
	protected static const string PORT_SUPPRESSION_VOLUME = "SuppressionVolume";
	protected static const string PORT_BASE_TARGET = "BaseTarget";
	
	protected ref TStringArray s_aVarsOut = {PORT_SUPPRESSION_VOLUME};
	protected ref TStringArray s_aVarsIn = {PORT_BASE_TARGET};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected SCR_AICombatComponent m_CombatComponent;
	protected ref SCR_AISuppressionVolumeBaseTargetBox m_SuppressionVolume;
		
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Converts baseTarget object instance to suppression volume object"; };
	
	//---------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_SuppressionVolume = new SCR_AISuppressionVolumeBaseTargetBox(vector.Zero, vector.Zero);
		
		IEntity myEntity = owner.GetControlledEntity();
		if (myEntity)
			m_CombatComponent = SCR_AICombatComponent.Cast(myEntity.FindComponent(SCR_AICombatComponent));
	}
	
	//---------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseTarget baseTarget;	
		if (!GetVariableIn(PORT_BASE_TARGET, baseTarget))
			return NodeError(this, owner, "No base target provided!");
				
		IEntity m_CharacterEntity = owner.GetControlledEntity();
		if (!m_CharacterEntity || !m_CombatComponent)
			return ENodeResult.FAIL;

		m_SuppressionVolume.SetTarget(baseTarget, m_CharacterEntity, m_CombatComponent.GetTargetDestinationPos());
		
		SetVariableOut(PORT_SUPPRESSION_VOLUME, m_SuppressionVolume);
		
		return ENodeResult.SUCCESS;
	}
}