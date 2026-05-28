#ifdef WORKBENCH
class TranslationPluginResponse : JsonApiStruct
{
	ref array<ref TranslationPluginResult> results;
	ref TranslationPluginResponseMeta meta;

	//------------------------------------------------------------------------------------------------
	void TranslationPluginResponse()
	{
		RegAll();
	}
}

class TranslationPluginResult : JsonApiStruct
{
	string id;
	ref TranslationPluginResultHolder translations;

	//------------------------------------------------------------------------------------------------
	void TranslationPluginResult()
	{
		RegV("id");
		if (translations)
			RegV("translations");
	}
}

[BaseContainerProps()] // needed to Containerise in TranslationPlugin.GetTranslationFromLocale
class TranslationPluginResultHolder : JsonApiStruct
{
	[Attribute()] string cs_cz;
	[Attribute()] string de_de;
	[Attribute()] string en_us;
	[Attribute()] string es_es;
	[Attribute()] string fr_fr;
	[Attribute()] string it_it;
	[Attribute()] string ja_jp;
	[Attribute()] string ko_kr;
	[Attribute()] string pl_pl;
	[Attribute()] string pt_br;
	[Attribute()] string ru_ru;
	[Attribute()] string uk_ua;
	[Attribute()] string zh_cn;

	//------------------------------------------------------------------------------------------------
	void TranslationPluginResultHolder()
	{
		RegAll();
	}
}

class TranslationPluginResponseMeta : JsonApiStruct
{
	string system_prompt;
	string user_prompt;
	string model;
	ref TranslationPluginResponseMetaUsage usage;
	float total_cost;
	string currency;

	//------------------------------------------------------------------------------------------------
	void TranslationPluginResponseMeta()
	{
		RegAll();
	}
}

class TranslationPluginResponseMetaUsage : JsonApiStruct
{
	int prompt_tokens;
	int completion_tokens;
	int total_tokens;

	//------------------------------------------------------------------------------------------------
	void TranslationPluginResponseMetaUsage()
	{
		RegAll();
	}
}
#endif // WORKBENCH
