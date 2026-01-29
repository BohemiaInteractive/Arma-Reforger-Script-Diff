class SCR_TutorialGamemodeComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_TutorialGamemodeComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.Cast(component);

		const SCR_ETutorialCourses courses = tutorial.GetFinishedCourses();
		const int freeRoamActivations = tutorial.GetFreeRoamActivations();

		if (courses == 0 && freeRoamActivations == 0)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(courses);
		context.Write(freeRoamActivations);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto tutorial = SCR_TutorialGamemodeComponent.Cast(component);

		int version;
		context.Read(version);

		SCR_ETutorialCourses courses;
		context.Read(courses);

		int freeRoamActivations;
		context.Read(freeRoamActivations);

		tutorial.SetFreeRoamActivation(freeRoamActivations);
		tutorial.LoadProgress(courses);
		return true;
	}
}
