class DSGameConfig: JsonApiStruct
{
	string name;
	string scenarioId;
	string hostedScenarioModId;
	int maxPlayers;
	string password;
	string passwordAdmin;
	bool visible;

	bool crossPlatform;

	ref DSGameProperties gameProperties;
	ref array<ref DSMod> mods;

	void DSGameConfig()
	{
		RegV("name");
		RegV("password");
		RegV("passwordAdmin");
		RegV("maxPlayers");
		RegV("scenarioId");
		RegV("gameProperties");
		RegV("mods");
	}

	override void OnPack()
	{
		UnregV("visible");
		StoreBoolean("visible", visible);

		UnregV("crossPlatform");
		StoreBoolean("crossPlatform", crossPlatform);
	}

	override void OnExpand()
	{
		RegV("visible");
		RegV("crossPlatform");
	}
}

class DSGameProperties: JsonApiStruct
{
	bool battlEye;

	override void OnPack()
	{
		UnregV("battlEye");
		StoreBoolean("battlEye", battlEye);
	}

	override void OnExpand()
	{
		RegV("battlEye");
	}
}

class DSMod: JsonApiStruct
{
	string modId;
	string name;
	string version;

	void DSMod()
	{
		RegV("modId");
		RegV("name");
		RegV("version");
	}
}

class DSConfig: JsonApiStruct
{
	string bindAddress;
	int bindPort;

	string publicAddress;
	int publicPort;

	ref DSGameConfig game;

	void DSConfig()
	{
		RegV("bindAddress");
		RegV("bindPort");
		RegV("publicAddress");
		RegV("publicPort");
		RegV("game");
	}
}