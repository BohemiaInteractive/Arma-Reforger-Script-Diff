/*!
\addtogroup AILib
\{
*/
sealed class BtVariableObject: BtVariable
{
	Managed m_Value;
}

sealed class BtVariableEntity: BtVariable
{
	IEntity m_Value;
}

sealed class BtVariableString: BtVariable
{
	string m_Value;
}

sealed class BtVariableResourceName: BtVariable
{
	ResourceName m_Value;
}

sealed class BtVariableScalar: BtVariable
{
	float m_Value;
}

sealed class BtVariableInteger: BtVariable
{
	int m_Value;
}

sealed class BtVariableBool: BtVariable
{
	bool m_Value;
}

sealed class BtVariableVector: BtVariable
{
	vector m_Value;
}

sealed class BtVariableTypename: BtVariable
{
	typename m_Value;
}
/*!
\}
*/