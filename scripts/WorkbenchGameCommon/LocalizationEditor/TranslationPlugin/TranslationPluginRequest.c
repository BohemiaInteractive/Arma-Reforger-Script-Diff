#ifdef WORKBENCH
class TranslationPluginRequest : JsonApiStruct
{
	ref array<string> locales; //!< null if each query has a different locales request
	ref array<ref TranslationPluginQuery> queries = {};

	//------------------------------------------------------------------------------------------------
	override event void OnPack()
	{
		super.OnPack();

		UnregV("locales");
		if (locales && !locales.IsEmpty())
		{
			StartArray("locales");

			foreach (string locale : locales)
			{
				ItemString(locale);
			}

			EndArray();
		}

		UnregV("queries");
		StartArray("queries");

		if (queries)
		{
			foreach (TranslationPluginQuery query : queries)
			{
				ItemObject(query);
			}
		}

		EndArray();
	}

	//------------------------------------------------------------------------------------------------
	override event void OnExpand()
	{
		super.OnExpand();

		if (locales && !locales.IsEmpty())
			RegV("locales");

		RegV("queries");
	}
}

class TranslationPluginQuery : JsonApiStruct
{
	string id;
	string text;
	ref array<string> locales = {}; //!< can be null when sent (TranslationPluginRequest then holding common locales)
	ref TranslationRequestMeta meta;

	//------------------------------------------------------------------------------------------------
	override event void OnPack()
	{
		super.OnPack();

		UnregV("id");
		StoreString("id", id);

		UnregV("text");
		StoreString("text", text);

		UnregV("locales");
		if (locales)
		{
			StartArray("locales");

			foreach (string locale : locales)
			{
				ItemString(locale);
			}

			EndArray();
		}

		UnregV("meta");
		if (meta && !meta.IsEmpty())
			StoreObject("meta", meta);
	}

	//------------------------------------------------------------------------------------------------
	override event void OnExpand()
	{
		super.OnExpand();

		RegV("id");
		RegV("text");
		RegV("locales");

		if (meta && !meta.IsEmpty())
			RegV("meta");
	}
}

class TranslationRequestMeta : JsonApiStruct
{
	string comment;
	string gender;
	int maxLength;

	//------------------------------------------------------------------------------------------------
	//! \return true if properties are empty, false if there is content (and can be serialised)
	bool IsEmpty()
	{
		return maxLength < 1 && comment.IsEmpty() && gender.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override event void OnPack()
	{
		super.OnPack();

		UnregV("comment");
		if (!comment.IsEmpty())
			StoreString("comment", comment);

		UnregV("gender");
		if (!gender.IsEmpty())
			StoreString("gender", gender);

		UnregV("maxLength");
		if (maxLength > 0)
			StoreInteger("maxLength", maxLength);
	}

	//------------------------------------------------------------------------------------------------
	override event void OnExpand()
	{
		super.OnExpand();

		if (!comment.IsEmpty())
			RegV("comment");

		if (!gender.IsEmpty())
			RegV("gender");

		if (maxLength > 0)
			RegV("maxLength");
	}
}
#endif // WORKBENCH
