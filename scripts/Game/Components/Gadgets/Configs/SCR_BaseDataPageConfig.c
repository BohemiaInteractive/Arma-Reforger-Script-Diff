class SCR_BaseDataPageConfig : ScriptAndConfig
{
	//------------------------------------------------------------------------------------------------
	//! Custom logic for drawing the data from this page
	//! \param[in] root widget inside of which data should be drawn
	//! \param[in] surfaceEntity entity on which this data is being displayed on
	//! \param[in] drawingGadget gadget which called this method
	//! \param[in] pageID id of this page, starting from 0
	void DrawPageData(notnull Widget root, notnull IEntity surfaceEntity, notnull SCR_DataDisplayGadgetComponent drawingGadget, int pageID);
}