[EntityEditorProps(category: "GameScripted/CleanSweep", description: "This is clean sweep area entity.", color: "0 0 255 255")]
class SCR_CleanSweepAreaClass : GenericEntityClass
{
}

class SCR_CleanSweepArea : GenericEntity
{
	[Attribute(defvalue: "400", desc: "How many groups are spawned in the location")]
	float m_Range;

	[Attribute(defvalue: "0", desc: "Can this location be selected in the mission")]
	bool m_Active;

	protected static ref array<SCR_CleanSweepArea> s_aInstances = {};

	//------------------------------------------------------------------------------------------------
	static array<SCR_CleanSweepArea> GetInstances()
	{
		return SCR_ArrayHelperT<SCR_CleanSweepArea>.GetCopy(s_aInstances);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(float timeSlice)
	{
		int color;
		if (m_Active)
			color = 0x8000FF00;
		else
			color = 0x80000000;

		Shape.CreateSphere(color, ShapeFlags.ONCE | ShapeFlags.DOUBLESIDE | ShapeFlags.TRANSP, GetOrigin(), m_Range);
	}
#endif

	//------------------------------------------------------------------------------------------------
	void SCR_CleanSweepArea(IEntitySource src, IEntity parent)
	{
		if (s_aInstances)
			s_aInstances.Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CleanSweepArea()
	{
		if (s_aInstances)
			s_aInstances.RemoveItem(this);
	}
}
