class SCR_AISetGroupFormation : AITaskScripted
{
	protected AIGroupMovementComponent m_MovementComponent;
	protected AIFormationComponent m_FormationComponent;
	protected SCR_AIGroupSettingsComponent m_SettingsComponent;
	
	
	[Attribute(SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EAIGroupFormation))]
	protected SCR_EAIGroupFormation m_eFormation;
	
	//---------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_MovementComponent = AIGroupMovementComponent.Cast(owner.FindComponent(AIGroupMovementComponent));
		m_FormationComponent = AIFormationComponent.Cast(owner.FindComponent(AIFormationComponent));
		m_SettingsComponent = SCR_AIGroupSettingsComponent.Cast(owner.FindComponent(SCR_AIGroupSettingsComponent));
	}
	
	//---------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		// Bail if it totally makes no sense
		if (!m_FormationComponent && !m_MovementComponent)
			return ENodeResult.FAIL;
		
		SCR_EAIGroupFormation formation = m_eFormation;
		
		// Check if formation is set via settings
		SCR_AIGroupFormationSettingBase formationSetting;
		if (m_SettingsComponent)
			formationSetting = SCR_AIGroupFormationSettingBase.Cast(m_SettingsComponent.GetCurrentSetting(SCR_AIGroupFormationSettingBase));
		
		// Set formation in formation component
		if (m_FormationComponent)
		{
			SCR_EAIGroupFormation formationForFormComponent = formation;
			
			// Override from settings?
			if (formationSetting)
				formationForFormComponent = formationSetting.GetFormation(0, formation);
			
			string formationName = typename.EnumToString(SCR_EAIGroupFormation, formationForFormComponent);
			m_FormationComponent.SetFormation(formationName);
		}
			
		// Set formation for all move handlers in movement component
		if (m_MovementComponent)
		{
			int handlerId = 0;
			while (m_MovementComponent.GetMoveHandlerAgentCount(handlerId) != -1)
			{
				SCR_EAIGroupFormation formationForMoveHandler = formation;
				
				// Override from settings?
				if (formationSetting)
					formationForMoveHandler = formationSetting.GetFormation(handlerId, formation);
				
				string formationName = typename.EnumToString(SCR_EAIGroupFormation, formationForMoveHandler);
				m_MovementComponent.SetFormationDefinition(handlerId, formationName);
				handlerId++;
			}
		}
		
		return ENodeResult.SUCCESS;
	}
	
	//---------------------------------------------------------------------------------
	override static bool VisibleInPalette() { return true; }
	
	//---------------------------------------------------------------------------------
	override static string GetOnHoverDescription() { return "Sets formation of group. Also checks formation in settings component."; }
}