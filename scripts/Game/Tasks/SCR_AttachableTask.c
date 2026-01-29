[EntityEditorProps(category: "GameScripted/Tasks", description: "")]
class SCR_AttachableTaskClass : SCR_EditorTaskClass
{
};

class SCR_AttachableTask : SCR_EditorTask
{
	protected SCR_EditableEntityComponent m_Target;

	//------------------------------------------------------------------------------------------------
	//! Sets target of current task
	//! \param[in] target to attach current to
	void SetTarget(SCR_EditableEntityComponent target)
	{
		if (target == m_Target)
			return;

		m_Target = target;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current target
	//! \return m_Target
	SCR_EditableEntityComponent GetTarget()
	{
		return m_Target;
	}
}
