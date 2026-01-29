/*
Class for storing variables in key(string)-value pairs.
It is meant to be used with BT nodes.
*/

//! Base BT parameter class to be used in SCR_AIActionParams
class SCR_BTParamBase : Managed
{
	string m_sPortName;
	
	void SetVariableOut(Node node);
	void GetVariableIn(Node node);
	
	void SCR_BTParamBase(string portName)
	{
		m_sPortName = portName;
	}
};

//! Parameter with data of specific type
class SCR_BTParam<Class T> : SCR_BTParamBase
{
	T m_Value;
	
	//----------------------------------------------------------------
	override void SetVariableOut(Node node)
	{
		node.SetVariableOut(m_sPortName, m_Value);
	}
	
	//----------------------------------------------------------------
	override void GetVariableIn(Node node)
	{
		T tempVar;
		if (node.GetVariableIn(m_sPortName, tempVar))
			m_Value = tempVar;
	}
	
	//----------------------------------------------------------------
	void Init(array<SCR_BTParamBase> paramsArray, T value)
	{
		paramsArray.Insert(this);
		m_Value = value;
	}
	
	//----------------------------------------------------------------
	//! Use this in SCR_AIAction constructor.
	void Init(SCR_AIActionBase action, T value)
	{
		action.m_aParams.Insert(this);
		m_Value = value;
	}
};

// Same as SCR_BTParam, but has a strong reference to T
class SCR_BTParamRef<Class T> : SCR_BTParamBase
{
	ref T m_Value;
	
	//----------------------------------------------------------------
	override void SetVariableOut(Node node)
	{
		node.SetVariableOut(m_sPortName, m_Value);
	}
	
	//----------------------------------------------------------------
	override void GetVariableIn(Node node)
	{
		T tempVar;
		if (node.GetVariableIn(m_sPortName, tempVar))
			m_Value = tempVar;
	}
	
	//----------------------------------------------------------------
	void Init(array<SCR_BTParamBase> paramsArray, T value)
	{
		paramsArray.Insert(this);
		m_Value = value;
	}
	
	//----------------------------------------------------------------
	//! Use this in SCR_AIAction constructor.
	void Init(SCR_AIActionBase action, T value)
	{
		action.m_aParams.Insert(this);
		m_Value = value;
	}
};


//! Same as SCR_BTParam<T>, but can be assigned or not (from script side).
//! Output: When not assigned, it clears node variable. When assigned, works as usual.
//! Input: Overrides m_Value only if GetVariableIn returns true;
class SCR_BTParamAssignable<Class T> : SCR_BTParamBase
{
	T m_Value;
	
	// When true, it does SetVariableOut. When false, it does CleaarVariable
	bool m_AssignedOut;
	
	// It is set to true when data is read from node input ports into this object and data at input port is assigned
	bool m_AssignedIn;
	
	//----------------------------------------------------------------
	override void SetVariableOut(Node node)
	{
		if (m_AssignedOut)
			node.SetVariableOut(m_sPortName, m_Value);
		else
			node.ClearVariable(m_sPortName);
	}
	
	//----------------------------------------------------------------
	override void GetVariableIn(Node node)
	{
		T tempVar;
		m_AssignedIn = node.GetVariableIn(m_sPortName, tempVar);
		if (m_AssignedIn)
			m_Value = tempVar;
	}
	
	//----------------------------------------------------------------
	void Init(array<SCR_BTParamBase> paramsArray, T value, bool assignedOut = true)
	{
		paramsArray.Insert(this);
		m_Value = value;
		m_AssignedOut = assignedOut;
	}
	
	//----------------------------------------------------------------
	//! Use this in SCR_AIAction constructor.
	void Init(SCR_AIActionBase action, T value, bool assignedOut = true)
	{
		action.m_aParams.Insert(this);
		m_Value = value;
		m_AssignedOut = assignedOut;
	}
};


