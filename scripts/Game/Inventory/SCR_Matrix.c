//------------------------------------------------------------------------------------------------
class SCR_Matrix
{
	
	private int 							m_iMaxColumns;
	private int								m_iMaxRows;
	
//	private ref SCR_Array					m_Array;
	private ref array<ref SCR_MatrixRow>	m_iMatrix;
		
	//------------------------------------------------------------------------------------------------
	// ! reserve first free place of the size [sizeCols, sizeRows] in the matrix
	array<int> Reserve1stFreePlace( int sizeCols, int sizeRows )
	{
		array<int> retVal;
		for( int iRow = 0; iRow < m_iMatrix.Count(); iRow++ )
		{
			for( int iCol = 0; iCol < m_iMaxColumns; iCol++ )
			{
				retVal = ReservePlace( sizeCols, sizeRows, iCol, iRow );
				if( ( retVal[0] != -1 ) && ( retVal[1] != -1 ) )
					return retVal;
			}
		}
		
		return retVal;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! reserve the place of the size [sizeCols, sizeRows] in the matrix starting on position [col, row]
	array<int> ReservePlace( int sizeCols, int sizeRows, int col, int row )
	{
		
		if( ( sizeRows + row ) > m_iMatrix.Count() )	
			return {-1,-1};	//the item doesn't fit vertically
		
		SCR_MatrixRow aRow = new SCR_MatrixRow( sizeCols );
		
		//go through all rows
		for( int iRow = row; iRow < ( sizeRows + row ); iRow++ )
		{
			GetRow( iRow, aRow );	//take the actual row
			bool bFree = aRow.CheckFreePlaceInRow( sizeCols, col );	//check for the free place
			if( !bFree )
				return {-1,-1};	
		}
		//TODO: checking and reserving can be done in one round!
		//if there's enough free place, reserve it
		for( int iRow = row; iRow < ( sizeRows + row ); iRow++ )
		{
			GetRow( iRow, aRow );	//take the actual row
			aRow.ReservePlaceInRow( sizeCols, col );
		}
		return {col, row};
	}
	
	//------------------------------------------------------------------------------------------------
	void Debug()
	{
			for( int i = 0; i < m_iMatrix.Count(); i++)
				m_iMatrix.Get( i ).Debug();
	}
	
	//------------------------------------------------------------------------------------------------
	private void GetRow( int rowIndex, out notnull SCR_MatrixRow row )
	{
		row = m_iMatrix.Get( rowIndex );
	}
	
	//------------------------------------------------------------------------------------------------
	int GetElement( int col, int row )
	{
		//ref SCR_MatrixRow tmp = new SCR_MatrixRow( m_iMaxColumns );
		
		int retVal = m_iMatrix.Get( row ).GetElement( col );
		return retVal;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetElement( int col, int row, int value )
	{
		m_iMatrix.Get( row ).SetElement( col, value );
	}
	
	
	//------------------------------------------------------------------------------------------------
	void InsertRow( SCR_MatrixRow row )
	{
		m_iMatrix.Insert( row );		
	}
	
	void Reset()
	{
		for( int iLoop = 0; iLoop < m_iMaxRows; iLoop++ )
			for( int jLoop = 0; jLoop < m_iMaxColumns; jLoop++ )
				SetElement( jLoop, iLoop, 0);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_Matrix( int cols, int rows )
	{
		m_iMaxColumns = cols;
		m_iMaxRows = rows;
		
		m_iMatrix = new array<ref SCR_MatrixRow>();
		
		for( int iLoop = 0; iLoop < rows; iLoop++ )
			m_iMatrix.Insert( SCR_MatrixRow( cols ) );
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_Matrix()
	{
	}

};
