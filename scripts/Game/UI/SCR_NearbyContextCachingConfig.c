[BaseContainerProps(configRoot: true)]
class SCR_NearbyContextCachingConfig
{
	[Attribute()]
	protected ref array<ref SCR_NearbyContextCachingData> m_aCachedWidgets;

	//------------------------------------------------------------------------------------------------
	//! Get all the layouts defined in config that should be precached
	//! \param[out] Array<SCR_NearbyContextCachingData> all layouts that are defined 
	void GetCachedLayouts(out array<SCR_NearbyContextCachingData> cachedLayouts)
	{
		foreach (SCR_NearbyContextCachingData data : m_aCachedWidgets)
		{
			if (data)
				cachedLayouts.Insert(data);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get the data that hold the amount of Widgets to precache and the max cache amount
	//! \param[in] string layout defined in array entry, the data is needed from
	//! \return SCR_NearbyContextCachingData data containing the precache and max cache amount
	SCR_NearbyContextCachingData GetDataFromLayout(ResourceName layout)
	{
		SCR_NearbyContextCachingData returnData;
		foreach (SCR_NearbyContextCachingData data : m_aCachedWidgets)
		{
			ResourceName dataLayout;
			data.GetLayout(dataLayout);

			if (dataLayout == layout)
			{
				returnData = data;
				break;
			}
		}
		
		return returnData;
	}
}

[BaseContainerProps(), SCR_NearbyContextTitle()]
class SCR_NearbyContextCachingData
{
	[Attribute(desc: "The Name is just used for organization reasons in the Config")]
	protected string m_sName;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout that should be cached", "layout")]
	protected ResourceName m_sLayout;

	[Attribute("5", desc: "Amount of Widgets that should be created and cached on Game start.")]
	protected int m_iPrecacheAmount;

	[Attribute("5", desc: "Max amount of Cached widgets. If more widgets are needed they will be created but not cached and deleted after not being used anymore.")]
	protected int m_iMaxCachedAmount;

	//------------------------------------------------------------------------------------------------
	//! Get defined layout
	//! \param[out] string defined layout
	void GetLayout(out ResourceName layout)
	{
		layout = m_sLayout;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the precache amount
	//! \param[out] int - amount of Widgets to precache on Init
	void GetPrecacheAmount(out int amount)
	{
		amount = m_iPrecacheAmount;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the maximum amount of cached Widgets
	//! If more widgets are cached then defined, the rest will be deleted again when they are not used anymore
	//! \param[out] int Max amount of Cached Widgets
	void GetMaxCacheAmount(out int amount)
	{
		amount = m_iMaxCachedAmount;
	}
}

class SCR_NearbyContextTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sName");
		if (index == -1)
			return false;

		source.Get("m_sName", title);

		return true;
	}
}
