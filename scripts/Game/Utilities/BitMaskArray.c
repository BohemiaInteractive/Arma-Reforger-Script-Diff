//------------------------------------------------------------------------------------------------
//! SCR_BitMaskArray Class
//!
//! A script equivalent of a BitArray, where integers are used as blocks of 32 bits each
//------------------------------------------------------------------------------------------------
class SCR_BitMaskArray
{
	//! Array of integers for the bitmasks
	private ref array<int> m_BitMasks = null;
	
	//------------------------------------------------------------------------------------------------
	int GetNumBitMasks()
	{
		return m_BitMasks.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBitMask(int index)
	{
		return m_BitMasks[index];
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBitMask(int index, int bitMask)
	{
		if (index >= m_BitMasks.Count())
			return;
		
		m_BitMasks[index] = bitMask;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEmpty()
	{
		for (int b = 0; b < m_BitMasks.Count(); b++)
		{
			int bitMask = m_BitMasks.Get(b);
			if (bitMask != 0)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetBit(int index)
	{
		int bitMaskIndex = Math.Floor(index / 32);
		int bitInMask = 1 << (index % 32);
		
		int bitMask = m_BitMasks.Get(bitMaskIndex);
		
		if (bitMask & bitInMask)
			return true;
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBit(int index, bool bSet)
	{
		int bitMaskIndex = Math.Floor(index / 32);
		int bitInMask = 1 << (index % 32);
		
		int bitMask = m_BitMasks.Get(bitMaskIndex);
		if (bSet)
			bitMask |= bitInMask;
		else
			bitMask &= ~bitInMask;
		m_BitMasks.Set(bitMaskIndex, bitMask);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BitMaskArray(int maxBits)
	{
		int numBitMasks = Math.Ceil(maxBits / 32);
		m_BitMasks = new array<int>;
		
		for (int b = 0; b < numBitMasks; b++)
			m_BitMasks.Insert(0);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BitMaskArray()
	{
	}
};
