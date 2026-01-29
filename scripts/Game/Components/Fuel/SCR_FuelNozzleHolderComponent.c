[ComponentEditorProps(category: "GameScripted", description: "THIS IS THE SCRIPT DESCRIPTION.", icon: HYBRID_COMPONENT_ICON)]
class SCR_FuelNozzleHolderComponentClass : ScriptComponentClass
{
}

class SCR_FuelNozzleHolderComponent : ScriptComponent
{
	private SCR_Nozzle m_Nozzle = null;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] Nozzle
	void StoreNozzle( SCR_Nozzle Nozzle )
	{
		m_Nozzle = Nozzle;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] Nozzle
	//! \return
	bool NozzleStored( SCR_Nozzle Nozzle )
	{
		if( !Nozzle ) 
			return false;
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RemoveNozzle()
	{
		m_Nozzle = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_Nozzle GetNozzle()
	{
		return m_Nozzle;
	}
		
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_FuelNozzleHolderComponent( IEntityComponentSource src, IEntity ent, IEntity parent )
	{
	}
}
