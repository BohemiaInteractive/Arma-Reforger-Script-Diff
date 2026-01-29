//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Separate file
//! Exploding material type
enum SCR_ESecondaryExplosionType
{
	DEFAULT,
	FUEL,
	AMMO,
	TIRE,
	BATTERY,
	RESOURCE
}

//! Fire state definitions
enum SCR_ESecondaryExplosionScale
{
	NONE,
	SMALL,
	MEDIUM,
	LARGE,
	MASSIVE
}
//---- REFACTOR NOTE END ----

[BaseContainerProps(configRoot: true)]
class SCR_SecondaryExplosions
{
	[Attribute()]
	protected ref array<ref SCR_SecondaryExplosionType> m_aSecondaryExplosionTypes;

	//------------------------------------------------------------------------------------------------
	SCR_SecondaryExplosion GetSecondaryExplosionForScale(SCR_ESecondaryExplosionScale scale, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.INVALID)
	{
		foreach (SCR_SecondaryExplosionType explosions : m_aSecondaryExplosionTypes)
		{
			if (explosions.GetExplosionType() != explosionType)
				continue;
			
			if (explosionType == SCR_ESecondaryExplosionType.RESOURCE && explosions.GetResourceType() != resourceType)
				continue;

			return explosions.GetSecondaryExplosionForScale(scale);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ESecondaryExplosionScale GetSecondaryExplosionScale(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.INVALID)
	{
		foreach (SCR_SecondaryExplosionType explosions : m_aSecondaryExplosionTypes)
		{
			if (explosionType == SCR_ESecondaryExplosionType.RESOURCE && explosions.GetResourceType() != resourceType)
				continue;

			if (explosions.GetExplosionType() != explosionType)
				continue;

			return explosions.GetSecondaryExplosionScale(value);
		}

		return SCR_ESecondaryExplosionScale.NONE;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetExplosionPrefab(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.INVALID)
	{
		foreach (SCR_SecondaryExplosionType explosions : m_aSecondaryExplosionTypes)
		{
			if (explosionType == SCR_ESecondaryExplosionType.RESOURCE && explosions.GetResourceType() != resourceType)
				continue;

			if (explosions.GetExplosionType() != explosionType)
				continue;

			return explosions.GetExplosionPrefab(value);
		}
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetFireParticles(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.INVALID)
	{
		foreach (SCR_SecondaryExplosionType explosions : m_aSecondaryExplosionTypes)
		{
			if (explosionType == SCR_ESecondaryExplosionType.RESOURCE && explosions.GetResourceType() != resourceType)
				continue;

			if (explosions.GetExplosionType() != explosionType)
				continue;

			return explosions.GetFireParticles(value);
		}
		return ResourceName.Empty;
	}
}

[BaseContainerProps(), SCR_SecondaryExplosionTypeContainerTitle()]
class SCR_SecondaryExplosionType
{
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_ESecondaryExplosionType.DEFAULT), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESecondaryExplosionType))]
	protected SCR_ESecondaryExplosionType m_eSecondaryExplosionType;

	[Attribute(defvalue: SCR_Enum.GetDefault(EResourceType.INVALID), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;

	[Attribute()]
	protected ref array<ref SCR_SecondaryExplosion> m_aSecondaryExplosions;

	//------------------------------------------------------------------------------------------------
	SCR_ESecondaryExplosionType GetExplosionType()
	{
		return m_eSecondaryExplosionType;
	}

	//------------------------------------------------------------------------------------------------
	EResourceType GetResourceType()
	{
		return m_eResourceType;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SecondaryExplosion GetSecondaryExplosionForScale(SCR_ESecondaryExplosionScale scale)
	{
		foreach (SCR_SecondaryExplosion explosion : m_aSecondaryExplosions)
		{
			if (	scale == explosion.m_eSecondaryExplosionScale)
				return explosion;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ESecondaryExplosionScale GetSecondaryExplosionScale(float value)
	{
		SCR_ESecondaryExplosionScale scale;
		float topValue;

		foreach (SCR_SecondaryExplosion explosion : m_aSecondaryExplosions)
		{
			if (value < explosion.m_fMinValue)
				continue;

			if (topValue > explosion.m_fMinValue)
				continue;

			topValue = explosion.m_fMinValue;
			scale = explosion.m_eSecondaryExplosionScale;
		}

		return scale;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetExplosionPrefab(float value)
	{
		ResourceName prefab;
		float topValue;

		foreach (SCR_SecondaryExplosion explosion : m_aSecondaryExplosions)
		{
			if (value < explosion.m_fMinValue)
				continue;

			if (topValue > explosion.m_fMinValue)
				continue;

			topValue = explosion.m_fMinValue;
			prefab = explosion.m_sSecondaryExplosionPrefab;
		}

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetFireParticles(float value)
	{
		ResourceName particles;
		float topValue;

		foreach (SCR_SecondaryExplosion explosion : m_aSecondaryExplosions)
		{
			if (value < explosion.m_fMinValue)
				continue;

			if (topValue > explosion.m_fMinValue)
				continue;

			topValue = explosion.m_fMinValue;
			particles = explosion.m_sSecondaryFireParticles;
		}

		return particles;
	}
}

class SCR_SecondaryExplosionTypeContainerTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		SCR_ESecondaryExplosionType type;
		if (!source.Get("m_eSecondaryExplosionType", type))
			return false;

		title = "Type: " + SCR_Enum.GetEnumName(SCR_ESecondaryExplosionType, type);
		return true;
	}
}

[BaseContainerProps(), SCR_SecondaryExplosionContainerTitle()]
class SCR_SecondaryExplosion
{
	[Attribute()]
	float m_fMinValue;

	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_ESecondaryExplosionScale.NONE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESecondaryExplosionScale))]
	SCR_ESecondaryExplosionScale m_eSecondaryExplosionScale;

	[Attribute(desc: "Explosion prefab", UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sSecondaryExplosionPrefab;

	[Attribute(desc: "Fire particles", UIWidgets.ResourcePickerThumbnail, params: "ptc")]
	ResourceName m_sSecondaryFireParticles;
}

class SCR_SecondaryExplosionContainerTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		float minValue;
		if (source.Get("m_fMinValue", minValue))
			title = minValue.ToString(-1, 3);

		SCR_ESecondaryExplosionScale state;
		if (source.Get("m_eSecondaryExplosionScale", state))
			title += " - fire state: " + SCR_Enum.GetEnumName(SCR_ESecondaryExplosionScale, state);

		return true;
	}
}
