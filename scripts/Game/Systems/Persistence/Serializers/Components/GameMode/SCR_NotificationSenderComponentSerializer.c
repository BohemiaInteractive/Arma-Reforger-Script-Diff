class SCR_NotificationSenderComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_NotificationSenderComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_NotificationSenderComponent notifcationSender = SCR_NotificationSenderComponent.Cast(component);
		const BaseContainer source = notifcationSender.GetComponentSource(owner);

		const EKillFeedType killFeedType = notifcationSender.GetKillFeedType();
		EKillFeedType killFeedTypeDefault = EKillFeedType.UNKNOWN_KILLER;
		if (source)
			source.Get("m_iKillFeedType", killFeedTypeDefault);

		const EKillFeedReceiveType receivekillFeedType = notifcationSender.GetReceiveKillFeedType();
		EKillFeedReceiveType receivekillFeedTypeDefault = EKillFeedReceiveType.GROUP_ONLY;
		if (source)
			source.Get("m_iReceiveKillFeedType", receivekillFeedTypeDefault);

		const SCR_EFriendlyFireKillFeedType friendlyFireKillFeedType = notifcationSender.GetFriendlyFireKillFeedType();
		SCR_EFriendlyFireKillFeedType friendlyFireKillFeedTypeDefault = EKillFeedReceiveType.GROUP_ONLY;
		if (source)
			source.Get("m_eFriendlyFireKillFeedType", friendlyFireKillFeedTypeDefault);

		if (killFeedType == killFeedTypeDefault &&
			receivekillFeedType == receivekillFeedTypeDefault &&
			friendlyFireKillFeedType == friendlyFireKillFeedTypeDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(killFeedType, killFeedTypeDefault);
		context.WriteDefault(receivekillFeedType, receivekillFeedTypeDefault);
		context.WriteDefault(friendlyFireKillFeedType, friendlyFireKillFeedTypeDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_NotificationSenderComponent notifcationSender = SCR_NotificationSenderComponent.Cast(component);

		int version;
		context.Read(version);

		EKillFeedType killFeedType;
		if (context.Read(killFeedType))
			notifcationSender.SetKillFeedType(killFeedType);

		EKillFeedReceiveType receivekillFeedType;
		if (context.Read(receivekillFeedType))
			notifcationSender.SetReceiveKillFeedType(receivekillFeedType);

		SCR_EFriendlyFireKillFeedType friendlyFireKillFeedType;
		if (context.Read(friendlyFireKillFeedType))
			notifcationSender.SetFriendlyFireKillFeedType(friendlyFireKillFeedType);

		return true;
	}
}
