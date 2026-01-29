//! Wrapper for array that works as a stack
//! - items are stored as 'first in, last out'
//! - items inside are always ref, so bear that in mind
// may be replaced with cpp implementation with script API later on
class SCR_Stack<Class T>
{
	protected ref array<ref T> m_aArray = {};

	//------------------------------------------------------------------------------------------------
	//! Push an item to the end of the stack
	//! \param[in] item the item to push
	void Push(T item)
	{
		m_aArray.Insert(item);
	}

	//------------------------------------------------------------------------------------------------
	//! Pop an item from the top of the stack
	//! \return the last inserted and topmost element, null if no element is present
	T Pop()
	{
		int index = m_aArray.Count() - 1;
		if (index < 0)
			return null;

		T poppedItem = m_aArray[index];
		m_aArray.Remove(index);

		return poppedItem;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the last inserted and topmost element without popping it from the stack
	//! \return the last inserted and topmost element, null if no element is present
	T Top()
	{
		int index = m_aArray.Count() - 1;
		if (index < 0)
			return null;

		return m_aArray[index];
	}

	//------------------------------------------------------------------------------------------------
	//! Get the first inserted and topmost element without popping it from the stack
	//! \return the last inserted and topmost element, null if no element is present
	T Bottom()
	{
		if (m_aArray.IsEmpty())
			return null;

		return m_aArray[0];
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if stack is empty, false otherwise
	bool IsEmpty()
	{
		return m_aArray.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the count of elements stored in this stack
	int Count()
	{
		return m_aArray.Count();
	}
}
