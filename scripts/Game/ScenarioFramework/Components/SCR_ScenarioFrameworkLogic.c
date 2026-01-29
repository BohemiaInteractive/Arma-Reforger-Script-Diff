[BaseContainerProps()]
class SCR_ScenarioFrameworkLogicInput
{
	[Attribute(desc: "Input connector", category: "Input")]
	ref SCR_ScenarioFrameworkActionInputBase m_InputAction;
	
	SCR_ScenarioFrameworkLogic m_MasterLogic;
	
	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework logic and optionally initializes input action.
	//! \param[in] logic
	void Init(SCR_ScenarioFrameworkLogic logic)
	{
		m_MasterLogic = logic;
		if (m_InputAction)
			m_InputAction.Init(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activates logic on input signal change, passing signal and entity to master logic.
	//! \param[in] bSignal Boolean flag indicating whether the input signal is active or not.
	//! \param[in] entity Entity representation: Input parameter for receiving activation signal from another entity.
	void OnActivate(bool bSignal, IEntity entity)
	{
		if (m_MasterLogic)
			m_MasterLogic.OnInput(bSignal, entity);
	}
}

class SCR_ScenarioFrameworkLogicClass : GenericEntityClass
{
}

class SCR_ScenarioFrameworkLogic : GenericEntity
{
	[Attribute(desc: "If set to true, when this Logic gets activated, it will break the breakpoint in the Script Editor in respective methods. This can be also set during runtime via Debug Menu > ScenarioFramework > Logic Inspector")]
	bool m_bDebug;
	
	[Attribute(desc: "What causes the increase", category: "Input")]
	ref array<ref SCR_ScenarioFrameworkLogicInput> m_aInputs;
		
	[Attribute(desc: "What to do once counter is reached", category: "OnActivate")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;
	
	bool m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework logic and inputs.
	void Init()
	{
		if (m_bIsTerminated)
			return;
		
		foreach (SCR_ScenarioFrameworkLogicInput input : m_aInputs)
		{
			input.Init(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state of termination.
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return whether the object is terminated or not.
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInput(bool pSignal = true, IEntity entity = null);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity which activates this action
	void OnActivate(IEntity entity)
	{
		// Here you can debug specific Logic instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Logic Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkLogic.OnActivate] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			action.OnActivate(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	void RestoreToDefault()
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
		{
			action.RestoreToDefault();
		}
	}
	
#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	//! Rename entity with default name on creation in World Editor.
	//! \param[in] src Source entity representing the object being created in the world editor.
	override void _WB_OnCreate(IEntitySource src)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource thisSrc = api.EntityToSource(this);
		api.RenameEntity(thisSrc, api.GenerateDefaultEntityName(thisSrc));
	}
#endif
}

class SCR_ScenarioFrameworkLogicCounterClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicCounter : SCR_ScenarioFrameworkLogic
{	
	[Attribute(defvalue: "1", desc: "Threshold", UIWidgets.Graph, category: "Counter")]
	int	m_iCountTo;
	
	[Attribute(desc: "What to do once value is increased", category: "OnIncrease")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aOnIncreaseActions;
	
	[Attribute(desc: "What to do once value is decreased", category: "OnDecrease")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aOnDecreaseActions;
			
	int m_iCnt = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Increases entity or self based on input signal.
	//! \param[in] pSignal Boolean flag indicating whether input signal is active or not.
	//! \param[in] entity Increases entity's value if entity is provided, else increases this entity's value.
	override void OnInput(bool pSignal = true, IEntity entity = null)
	{
		if (entity)
			Increase(entity);
		else
			Increase(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Counter value.
	int GetCounterValue()
	{
		return m_iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value Sets counter value.
	void SetCounterValue(int value)
	{
		m_iCnt = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increases the counter value by 1
	//! \param[in] entity
	void Increase(IEntity entity)
	{
		// Here you can debug specific Logic instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Logic Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkLogicCounter.Increase] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		m_iCnt++;
		if (m_iCnt == m_iCountTo)
		{
			OnActivate(entity);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase increaseAction : m_aOnIncreaseActions)
		{
			increaseAction.OnActivate(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Decreases the counter value by 1
	//! \param[in] entity
	void Decrease(IEntity entity)
	{
		// Here you can debug specific Logic instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Logic Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkLogicCounter.Decrease] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		m_iCnt--;
		if (m_iCnt == m_iCountTo)
		{
			OnActivate(entity);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase decreaseAction : m_aOnDecreaseActions)
		{
			decreaseAction.OnActivate(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resets counter to 0.
	void Reset()
	{
		m_iCnt = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void RestoreToDefault()
	{
		super.RestoreToDefault();
		
		Reset();
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aOnIncreaseActions)
		{
			action.RestoreToDefault();
		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aOnDecreaseActions)
		{
			action.RestoreToDefault();
		}
	}
}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class SCR_ScenarioFrameworkLogicORClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicOR : SCR_ScenarioFrameworkLogic
{
	int	 m_iActivations = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Controls input activation count.
	//! \param[in] pSignal pSignal is a boolean parameter indicating whether an input signal is received or not. If true, it increments activation count;
	//! \param[in] entity Input entity represents an optional parameter in the method, which can be an instance of any entity in the game world. It is
	override void OnInput(bool pSignal = true, IEntity entity = null)
	{
		if (pSignal)
			m_iActivations = Math.Min(++m_iActivations, m_aInputs.Count());
		else
			m_iActivations--;
	}	
}

class SCR_ScenarioFrameworkLogicSwitchClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicSwitch : SCR_ScenarioFrameworkLogic
{
}
//---- REFACTOR NOTE END ----
