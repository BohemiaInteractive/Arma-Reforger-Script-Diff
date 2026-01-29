//------------------------------------------------------------------------------------------------
class SCR_MatrixRow
{
	
	private ref array<int>			m_iRow = new array<int>();
	private string 					m_sDebugOutput = "";
//	private int						m_iDefaultElementValue;
	
	
	//------------------------------------------------------------------------------------------------
	// ! checks for the free place of the size sizeCols in the row starting on position col. Returns true if succeeded
	bool CheckFreePlaceInRow( int sizeCols, int col )
	{
		if( ( col + sizeCols ) > m_iRow.Count() )
			return false;
	
		for( int i = col; i < ( col + sizeCols ); i++ )
		{
			if( m_iRow.Get( i ) != 0 )
				return false;
		}
			
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! reserves the place of the size sizeCols in the row starting on position col. Returns column if succeeded
	bool ReservePlaceInRow( int sizeCols, int col )
	{
		if( ( col + sizeCols ) > m_iRow.Count() )
			return false;
	
		for( int i = col; i < ( col + sizeCols ); i++ )
			m_iRow.Set( i, 1 );
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! reserves the place of the size sizeCols in the row starting on position col. Returns true if succeeded
	bool ClearPlaceInRow( int sizeCols, int col )
	{
		if( ( col + sizeCols ) <= m_iRow.Count() )
			return false;
	
		for( int i = col; i < ( col + sizeCols ); i++ )
			m_iRow.Set( i, 1 );
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Debug()
	{
		m_sDebugOutput = "| ";
			for( int i = 0; i < m_iRow.Count(); i++ )
			{
				m_sDebugOutput += (m_iRow.Get( i ) ).ToString();
				m_sDebugOutput += " | ";
			}
			Print( m_sDebugOutput );
	}
	
	//------------------------------------------------------------------------------------------------
	int GetElement( int index )
	{
		int i = m_iRow.Get( index );
		return i;		
	}
	
	//------------------------------------------------------------------------------------------------
	void InsertElement( int element )
	{
		m_iRow.Insert( element );
	}
	
	//------------------------------------------------------------------------------------------------
	void SetElement( int index, int element )
	{
		m_iRow.Set( index, element );
	}
	
	
	//------------------------------------------------------------------------------------------------
/*
	void SCR_Array( out notnull array<int> from )
	{
		m_iRow.Copy( from );
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	void SCR_MatrixRow( int size, int defaulElementValue = 0 )
	{
		if( !size )
			size = 4;
		//m_iRow.Resize( size );
		for( int i = 0; i < size; i++)
			m_iRow.Insert( defaulElementValue );
	}
	

	//------------------------------------------------------------------------------------------------
	void ~SCR_MatrixRow()
	{
	}

};
