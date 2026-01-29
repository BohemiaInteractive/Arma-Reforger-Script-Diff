[EntityEditorProps(category: "GameScripted/Generators", description: "NotificationTestGeneratorEntity", dynamicBox: true, visible: false)]
class NotificationTestGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
}

class NotificationTestGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute()]
	protected string Name;

#ifdef WORKBENCH
	protected ref array<ref Shape> m_aShapes;

	//------------------------------------------------------------------------------------------------
	protected override void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes)
	{
		Print("[NotificationTestGeneratorEntity] Entity \"" + Name + "\" intersected with " + mins.Count() + " boxes", LogLevel.NORMAL);
		m_aShapes = {};
		for (int i; i < mins.Count(); ++i)
		{
			vector points[5];

			points[0] = mins[i];

			points[1][0] = maxes[i][0];
			points[1][1] = 0;
			points[1][2] = mins[i][2];

			points[2] = maxes[i];

			points[3][0] = mins[i][0];
			points[3][1] = 0;
			points[3][2] = maxes[i][2];

			points[4] = mins[i];

			m_aShapes.Insert(Shape.CreateLines(0xFFDD55FF, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, points, 5));
		}
	}
#endif // WORKBENCH
}
