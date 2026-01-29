[ComponentEditorProps(category: "GameScripted/Debug", description: "debug")]
class SCR_WBAfterWorldUpdateTestClass : ScriptComponentClass
{
}

class SCR_WBAfterWorldUpdateTest : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_WBAfterWorldUpdateTest(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		ent.SetFlags(EntityFlags.ACTIVE, false);
	}
	
	#ifdef WORKBENCH
		override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
		{
			return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
		}
		//------------------------------------------------------------------------------------------------
		override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
		{
			if (!owner)
				Print("Owner is NULL!!!");
			else
				Print(owner);
		}
	#endif
}
