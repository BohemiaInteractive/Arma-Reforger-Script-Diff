class SCR_AIPerformSmartHealing : AITaskScripted
{
	// instead of class-local enum :(
	private const int USAGE_NONE = 0;
	private const int USAGE_WAITING_TO_START = 1;
	private const int USAGE_IN_PROGRESS = 2;
	private const int USAGE_DONE_SUCCESS = 3;
	private const int USAGE_DONE_FAILURE = 4;
	
	protected const float TIME_OF_HEALING_MS = 12000.0;	// how long the node waits before FAILS (time to trigger and execute healing action)
	protected const string PORT_USER_ACTION = "UserAction";
	protected const string PORT_TARGET_ENTITY = "TargetEntity";
	protected const string PORT_ITEM = "ItemToUse";
	
	protected typename m_sUserAction;
	protected IEntity m_targetEntity, m_item;
	protected SCR_CharacterControllerComponent m_targetContrComp;
	protected bool m_bAborted;
		
	private int m_iItemUsageInProgress;	
	private float m_fTimeout_ms;
	
	[Attribute( defvalue: "SCR_BandageUserAction", uiwidget: UIWidgets.EditBox, desc: "Insert UserAction class name" )]
	protected string m_userActionString;
	
	override void OnEnter(AIAgent owner)
	{
		m_fTimeout_ms = 0;
		m_bAborted = false;
		m_iItemUsageInProgress = USAGE_NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		switch (m_iItemUsageInProgress)
		{
			case USAGE_NONE: 
			{
			
				string userActionString;
				
				if (!GetVariableIn(PORT_USER_ACTION, userActionString))
					userActionString = m_userActionString;
				GetVariableIn(PORT_ITEM, m_item);
				
				IEntity controlledEntity = owner.GetControlledEntity();
				if (!controlledEntity)
					return ENodeResult.FAIL;
				
				if (!GetVariableIn(PORT_TARGET_ENTITY, m_targetEntity))
					m_targetEntity = controlledEntity;
						
				m_sUserAction = userActionString.ToType();
				if (m_sUserAction == typename.Empty)
					return ENodeResult.FAIL;
				
				ChimeraCharacter character = ChimeraCharacter.Cast(m_targetEntity);
				if (character)
				{
					SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
					if (!damageMan && !damageMan.CanBeHealed())
						return ENodeResult.FAIL;
				}
				character = ChimeraCharacter.Cast(controlledEntity);
				m_targetContrComp = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (!m_targetContrComp)
					return ENodeResult.FAIL;
							
				array<BaseUserAction> outActions = {};
				SCR_HealingUserAction action;
				GetActions(m_targetEntity, outActions);
				foreach (BaseUserAction baseAction : outActions)
				{
					// Check if action is healing action type
					action = SCR_HealingUserAction.Cast(baseAction);
					if (!action)
						continue;
					
					// Find the healing action on the correct hitzone group
					if (action.GetUserActionGroup() != GetTargetGroup(m_targetEntity))
						continue;
					
					// If healing action is of the desired healing-type, and can be performed, PerformAction
					if (action && m_sUserAction == action.Type() && action.CanBePerformedScript(controlledEntity))
					{
						m_targetContrComp.m_OnItemUseBeganInvoker.Insert(OnItemUseBegan);
						m_targetContrComp.m_OnItemUseEndedInvoker.Insert(OnItemUseEnded);
						m_iItemUsageInProgress = USAGE_WAITING_TO_START;
						action.PerformAction(m_targetEntity, controlledEntity);
						return ENodeResult.RUNNING;
					}
				}
				return ENodeResult.FAIL;
			};	
			case USAGE_WAITING_TO_START:
			{
				float worldTime = GetGame().GetWorld().GetWorldTime();
				if (m_fTimeout_ms == 0)
					m_fTimeout_ms = worldTime + TIME_OF_HEALING_MS;
				//else if (worldTime > m_fTimeout_ms)
					//return ENodeResult.FAIL;
				return ENodeResult.RUNNING;
			};
			case USAGE_IN_PROGRESS:
			{
				float worldTime = GetGame().GetWorld().GetWorldTime();
				if (m_fTimeout_ms == 0)
					m_fTimeout_ms = worldTime + TIME_OF_HEALING_MS;
				//else if (worldTime > m_fTimeout_ms)
					//return ENodeResult.FAIL;
				return ENodeResult.RUNNING;
			};
			case USAGE_DONE_SUCCESS:
			{
				return ENodeResult.SUCCESS;
			};
			case USAGE_DONE_FAILURE:
			{
				return ENodeResult.FAIL;
			};
		};
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bAborted)
			return;
		
		if (m_targetContrComp)
		{
			m_targetContrComp.m_OnItemUseBeganInvoker.Remove(OnItemUseBegan);
			m_targetContrComp.m_OnItemUseEndedInvoker.Remove(OnItemUseEnded);
			if (m_iItemUsageInProgress < USAGE_DONE_SUCCESS) // we are ending before the action has finished
				m_targetContrComp.GetInputContext().SetCancelItemAction(true);
		}
		m_bAborted = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnItemUseBegan(IEntity item, ItemUseParameters animParams)
	{
		if (item != m_item)
			return;
		m_iItemUsageInProgress = USAGE_IN_PROGRESS;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnItemUseEnded(IEntity item, bool actionCompleted, ItemUseParameters animParams)
	{
		if (item != m_item)
			return;
		
		if (actionCompleted)
			m_iItemUsageInProgress = USAGE_DONE_SUCCESS;
		else 
			m_iItemUsageInProgress = USAGE_DONE_FAILURE;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetActions(IEntity targetEntity, notnull out array<BaseUserAction> outActions)
	{
		if (!targetEntity)
			return;
		
		ActionsManagerComponent actionOnEntity = ActionsManagerComponent.Cast(targetEntity.FindComponent(ActionsManagerComponent));
		
		if (!actionOnEntity)
			return;
		
		actionOnEntity.GetActionsList(outActions);
	}
	
	//------------------------------------------------------------------------------------------------
	protected ECharacterHitZoneGroup GetTargetGroup(IEntity targetEntity)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(targetEntity);
		if (!char)
			return null;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMan)
			return null;
		
		array<ECharacterHitZoneGroup> limbs = {};
		damageMan.GetAllLimbs(limbs);
		SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(damageMan.GetMostDOTHitZone(EDamageType.BLEEDING, false, limbs));
		if (!charHitZone)
			return null;
		
		return charHitZone.GetHitZoneGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_USER_ACTION,
		PORT_TARGET_ENTITY,
		PORT_ITEM
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning() { return true; }
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Uses smart healing action and is running until it finishes.";
	}
};