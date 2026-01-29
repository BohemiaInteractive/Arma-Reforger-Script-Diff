// Script File 
//Only works with SCR_ButtonBoxMultiSelectAttributeUIComponent layout
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseMultiSelectPresetsEditorAttribute: SCR_BasePresetsEditorAttribute
{
	[Attribute("0", "If multiselect uses custom flags. If false it will use the default system and sets states of flags either true or false")]
	protected bool m_bCustomFlags;
	
	//Which state will be checked
	private int m_iFlagChecker = 1;
	
	//Saves the true/false states
	protected vector m_vFlags;
	
	//Which vector index the states are saved
	private int m_iStateVectorIndex;
	
	//How many flags can be added to each vector index
	private int m_iFlagsPerIndex = 31;
	
	//Current flags for the current vector index
	private int m_iCurrentFlagsPerIndex;
	
	//---- REFACTOR NOTE START: Not ideal handling flag manipulation ----
	
	//Saves the next entity state
	protected void AddOrderedState(bool state)
	{
		if (m_iStateVectorIndex > 2)
		{
			Debug.Error2(Type().ToString(), string.Format("Attribute multiselect has reach the max element limit of '%1'!", (31 * 3).ToString()));
			return;
		}
		
		if (state)
		{
			int stateValue = m_vFlags[m_iStateVectorIndex];
			stateValue |= m_iFlagChecker;
			m_vFlags[m_iStateVectorIndex] = stateValue;
		}
			
		m_iFlagChecker *= 2;
		m_iCurrentFlagsPerIndex++;
		
		if (m_iCurrentFlagsPerIndex >= m_iFlagsPerIndex)
			ResetLoopValues(true);
	}
	
	//Gets the next entity state
	protected bool GetOrderedState()
	{
		if (m_iStateVectorIndex > 2)
		{
			Print(string.Format("Attribute multiselect has reach the max element limit of '%1' and is therefore set to false!", (31 * 3).ToString()), LogLevel.WARNING);
			return false;
		}
		
		int stateValue = m_vFlags[m_iStateVectorIndex];
		bool returnState = (stateValue & m_iFlagChecker);
		
		m_iFlagChecker *= 2;
		m_iCurrentFlagsPerIndex++;
		
		if (m_iCurrentFlagsPerIndex >= m_iFlagsPerIndex)
			ResetLoopValues(true);
		
		return returnState;
	}
	
	protected int GetStateVectorIndex()
	{
		return m_iStateVectorIndex;
	}
	
	//Add flag in specific vector index. Returns false if invalid index
	protected bool AddFlag(int flag, int vectorIndex = 0)
	{		
		if (vectorIndex > 2 || vectorIndex < 0)
			return false;
		
		int flagValue = m_vFlags[vectorIndex];
		flagValue |= flag;
		m_vFlags[vectorIndex] = flagValue;
		
		return true;
	}
	
	//Sets all flags of the specific index. Returns false if vector indext is invalid
	protected bool SetFlags(int flags, int vectorIndex = 0)
	{		
		if (vectorIndex > 2 || vectorIndex < 0)
			return false;
		
		m_vFlags[vectorIndex] = flags;
		return true;
	}
	
	//Gets all flags of index
	protected int GetFlags(int vectorIndex)
	{
		if (vectorIndex > 2 || vectorIndex < 0)
			return 0;
		
		return m_vFlags[vectorIndex];
	}
	
	//Check if Var has flag
	protected bool HasFlag(int flagToCheck)
	{		
		int flagValue;
		int vectorIndex = 0;
		
		while (vectorIndex < 3)
		{
			flagValue = m_vFlags[vectorIndex];
			if (flagValue & flagToCheck)
				return true;
				
			vectorIndex++;
		}
		
		return false;
	}
	
	protected void ResetLoopValues(bool increaseVectorIndex)
	{
		m_iFlagChecker = 1;
		m_iCurrentFlagsPerIndex = 0;
		
		if (increaseVectorIndex)
			m_iStateVectorIndex++;
		else
			m_iStateVectorIndex = 0;
	}
	
	//---- REFACTOR NOTE END ----
	
	protected vector GetFlagVector()
	{
		return m_vFlags;
	}
	
	//Reset vars
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		ResetLoopValues(false);
		m_vFlags = Vector(0,0,0);
		
		return null;
	}
	
	//Reset bit check and get states
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		ResetLoopValues(false);
		m_vFlags = var.GetVector();
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{	
		CreatePresets();
		outEntries.Insert(new SCR_EditorAttributePresetMultiSelectEntry(m_iButtonsOnRow, m_bHasRandomizeButton, m_sRandomizeButtonIcon, m_bUseIcon, m_bHasButtonDescription, m_sButtonDescription, m_fButtonHeight, m_bCustomFlags));
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
};