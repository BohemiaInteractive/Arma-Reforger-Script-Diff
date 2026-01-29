//Generic components from GameLib (script side of c++ classes)

//!Builtin component types
//TypeID MeshObjectTypeID;
//TypeID HierarchyTypeID;
//TypeID RigidBodyTypeID;
//TypeID SphereGeometryTypeID;
//TypeID BoxGeometryTypeID;

class SignalInput
{
	string m_name;
	float m_value;
	
	void SignalInput()
	{
		m_value = 0;
	}
}

enum EItemInsertionResult
{
	Ok,
	ErrorNullItem,
	ErrorLateInsertion,
	ErrorItemHasNoLayout
}

class RplSchedulerInsertionCtx
{
	RplStateOverride StateOverride;
}



