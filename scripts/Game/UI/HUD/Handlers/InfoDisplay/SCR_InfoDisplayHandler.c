[BaseContainerProps()]
class SCR_InfoDisplayHandler
{
	//! Info Display which owns this Info Dispaly Handler.
	protected SCR_InfoDisplay m_OwnerDisplay;

	[Attribute(desc: "Should this Handler be updated every frame?")]
	bool m_bCanUpdate;

	protected bool m_bEnabled;

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called before OnStart and used for initializing this Handler.
	\param display The Owner Info Display
	*/
	void Initialize(notnull SCR_InfoDisplay display)
	{
		m_OwnerDisplay = display;
		m_bEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called when the Owning Info Display of this handler is started.
	\param display The Owner Info Display
	*/
	void OnStart(notnull SCR_InfoDisplay display);

	//------------------------------------------------------------------------------------------------
	/*!
	Called when the Owning Info Display of this handler is stopped.
	\param display The Owner Info Display
	*/
	void OnStop(notnull SCR_InfoDisplay display);

	//------------------------------------------------------------------------------------------------
	/*!
	Called every frame
	*/
	void OnUpdate(float timeSlice);
}
