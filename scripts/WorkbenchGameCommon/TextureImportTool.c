enum MetaEddsConversion
{
	None, // CHC_NONE
	DXTCompression, // CHC_DXT
	RedHQCompression, // CHC_BC4
	RedGreenHQCompression, // CHC_BC5
	RedGreen, // CHC_RG
	Alpha8, // CHC_A8
	Red, // CHC_R
	HDRCompression, // CHC_BC6H
	ColorHQCompression, // CHC_BC7
}

enum MetaEddsColorSpaceConversion
{
	None,
	ToSrgb,
	ToLinear,
}

class TextureTypeProperty
{
	string m_Name;
	typename m_ValType;
	int m_Val;
	ref array<int> m_OtherVariants;
}

enum TextureIssueOp
{
	Ignore = 0,
	Report = 1 << 0,
	Fix = 1 << 2,
	ReportAndFix = Report | Fix,
}

enum TextureCheckResult
{
	NothingChanged = 0,
	SomethingChanged = 1,
	UnfixedFatalIssue = 2,
}

class PlatformConfig
{
	string m_Platform;
	ResourceName m_ConfigFile;
}

class TextureType
{
	// Take a look at typename.EnumToString and store typename variables for enums.
	string m_PostFix;
	ref array<ref TextureTypeProperty> m_Properties = new array<ref TextureTypeProperty>;
	ref array<ref PlatformConfig> m_PlatformConfigs = new array<ref PlatformConfig>;

	void TextureType(array<ref TextureType> container, string name)
	{
		m_PostFix = name;
		m_PostFix.ToLower();
		container.Insert(this);
	}

	void Insert(string name, typename valType, int val, array<int> otherVariants = null)
	{
		auto prop = new TextureTypeProperty;
		prop.m_Name = name;
		prop.m_ValType = valType;
		prop.m_Val = val;
		prop.m_OtherVariants = otherVariants;
		m_Properties.Insert(prop);
	}

	void AddBaseConfig(string platform, ResourceName configFile)
	{
		PlatformConfig config = new PlatformConfig;
		config.m_Platform = platform;
		config.m_ConfigFile = configFile;
		m_PlatformConfigs.Insert(config);
	}

	ResourceName GetBaseConfig(string platform)
	{
		foreach (PlatformConfig config : m_PlatformConfigs)
		{
			if(config.m_Platform == platform)
				return config.m_ConfigFile;
		}

		return "";
	}

	void CopyFrom(TextureType parent)
	{
		foreach (TextureTypeProperty property : parent.m_Properties)
		{
			m_Properties.Insert(property);
		}

		foreach (PlatformConfig config : parent.m_PlatformConfigs)
		{
			m_PlatformConfigs.Insert(config);
		}
	}

	bool IsType(string path)
	{
		string pathNoExt = FilePath.StripExtension(path);
		pathNoExt.ToLower();
		return pathNoExt.EndsWith(m_PostFix);
	}
}

class TextureTypes
{
	ref array<ref TextureType> m_Types;

	// Property names
	private static const string Conversion = "Conversion";
	private static const string ColorSpace = "ColorSpace";
	private static const string GenerateMips = "GenerateMips";
	private static const string GenerateCubemap = "GenerateCubemap";

	void TextureTypes()
	{
		m_Types = new array<ref TextureType>();

		//--------------------------------------------------------------------
		//color maps -> to sRGB
		ref TextureType COType = new TextureType(m_Types, "_co");
		COType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		COType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToSrgb);
		COType.AddBaseConfig("PC", "{EAB5DE3219F9CBA8}configs/ResourceTypes/PC/TextureColorMap.conf");
		COType.AddBaseConfig("XBOX_ONE", "{91D862F89991BFBE}configs/ResourceTypes/XBOX_ONE/TextureColorMap.conf");
		COType.AddBaseConfig("XBOX_SERIES", "{5FEAED1642ECE679}configs/ResourceTypes/XBOX_SERIES/TextureColorMap.conf");
		COType.AddBaseConfig("PS4", "{12273E1A0928F0C4}configs/ResourceTypes/PS4/TextureColorMap.conf");
		COType.AddBaseConfig("PS5", "{531A0D167B1ABD97}configs/ResourceTypes/PS5/TextureColorMap.conf");
		COType.AddBaseConfig("HEADLESS", "{BEAF5CD0C438676E}configs/ResourceTypes/HEADLESS/TextureColorMap.conf");

		ref TextureType BCType = new TextureType(m_Types, "_bc");
		BCType.CopyFrom(COType);

		ref TextureType BCRType = new TextureType(m_Types, "_bcr");
		BCRType.CopyFrom(COType);

		ref TextureType BCHType = new TextureType(m_Types, "_bch");
		BCHType.CopyFrom(COType);

		ref TextureType CAType = new TextureType(m_Types, "_ca");
		CAType.CopyFrom(COType);

		ref TextureType BCAType = new TextureType(m_Types, "_bca");
		BCAType.CopyFrom(COType);

		ref TextureType MCType = new TextureType(m_Types, "_mc");
		MCType.CopyFrom(COType);

		ref TextureType MCAType = new TextureType(m_Types, "_mca");
		MCAType.CopyFrom(COType);

		ref TextureType MLODType = new TextureType(m_Types, "_mlod");
		MLODType.CopyFrom(COType);

		//--------------------------------------------------------------------
		//EM maps
		ref TextureType EMType = new TextureType(m_Types, "_em");
		EMType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression, {MetaEddsConversion.DXTCompression});
		EMType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToSrgb);
		EMType.AddBaseConfig("PC", "{EAB5DE3219F9CBA8}configs/ResourceTypes/PC/TextureColorMap.conf");
		EMType.AddBaseConfig("XBOX_ONE", "{91D862F89991BFBE}configs/ResourceTypes/XBOX_ONE/TextureColorMap.conf");
		EMType.AddBaseConfig("XBOX_SERIES", "{5FEAED1642ECE679}configs/ResourceTypes/XBOX_SERIES/TextureColorMap.conf");
		EMType.AddBaseConfig("PS4", "{12273E1A0928F0C4}configs/ResourceTypes/PS4/TextureColorMap.conf");
		EMType.AddBaseConfig("PS5", "{531A0D167B1ABD97}configs/ResourceTypes/PS5/TextureColorMap.conf");
		EMType.AddBaseConfig("HEADLESS", "{BEAF5CD0C438676E}configs/ResourceTypes/HEADLESS/TextureColorMap.conf");

		//--------------------------------------------------------------------
		//MCR maps - rgb is linear overlay modificator of albedo
		ref TextureType MCRType = new TextureType(m_Types, "_mcr");
		MCRType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		MCRType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToLinear, {MetaEddsColorSpaceConversion.ToSrgb});
		MCRType.AddBaseConfig("PC", "{A6CC0A2F9DB86CBE}configs/ResourceTypes/PC/TextureMCRMap.conf");
		MCRType.AddBaseConfig("XBOX_ONE", "{224B78299A038E4A}configs/ResourceTypes/XBOX_ONE/TextureMCRMap.conf");
		MCRType.AddBaseConfig("XBOX_SERIES", "{B5FA7506CF384E79}configs/ResourceTypes/XBOX_SERIES/TextureMCRMap.conf");
		MCRType.AddBaseConfig("PS4", "{E7A6FCCE55593073}configs/ResourceTypes/PS4/TextureMCRMap.conf");
		MCRType.AddBaseConfig("PS5", "{E3BE705DB85AD3CA}configs/ResourceTypes/PS5/TextureMCRMap.conf");
		MCRType.AddBaseConfig("HEADLESS", "{D49C34F7354B6F47}configs/ResourceTypes/HEADLESS/TextureMCRMap.conf");

		//--------------------------------------------------------------------
		//pure normal maps
		ref TextureType NOType = new TextureType(m_Types, "_no");
		NOType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);
		NOType.AddBaseConfig("PC", "{CEA87F769FD618D2}configs/ResourceTypes/PC/TextureNormalMap.conf");
		NOType.AddBaseConfig("XBOX_ONE", "{5B84FAB5D39FB8AA}configs/ResourceTypes/XBOX_ONE/TextureNormalMap.conf");
		NOType.AddBaseConfig("XBOX_SERIES", "{1B409AA3788B720F}configs/ResourceTypes/XBOX_SERIES/TextureNormalMap.conf");
		NOType.AddBaseConfig("PS4", "{9533DFD9EDDCEDD2}configs/ResourceTypes/PS4/TextureNormalMap.conf");
		NOType.AddBaseConfig("PS5", "{30F530559C17A96E}configs/ResourceTypes/PS5/TextureNormalMap.conf");
		NOType.AddBaseConfig("HEADLESS", "{FC84ABE4B37D0DA0}configs/ResourceTypes/HEADLESS/TextureNormalMap.conf");

		ref TextureType NType = new TextureType(m_Types, "_n");
		NType.CopyFrom(NOType);

		ref TextureType NOHQType = new TextureType(m_Types, "_nohq");
		NOHQType.CopyFrom(NOType);

		// Terrain textures are caught by this and they would require swizzle to work.
		// ref TextureType normalType = new TextureType(m_Types, "_normal.");
		// normalType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);

		//--------------------------------------------------------------------
		//N types and packing with other sources
		ref TextureType NMOType = new TextureType(m_Types, "_nmo");
		NMOType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		NMOType.AddBaseConfig("PC", "{A968DA7F9A1E3A3E}configs/ResourceTypes/PC/TextureNType.conf");
		NMOType.AddBaseConfig("XBOX_ONE", "{7F6A4D372443A88D}configs/ResourceTypes/XBOX_ONE/TextureNType.conf");
		NMOType.AddBaseConfig("XBOX_SERIES", "{065D289C7FF8B20D}configs/ResourceTypes/XBOX_SERIES/TextureNType.conf");
		NMOType.AddBaseConfig("PS4", "{29DF4A6CBBABE916}configs/ResourceTypes/PS4/TextureNType.conf");
		NMOType.AddBaseConfig("PS5", "{DED3C8CA8494EA99}configs/ResourceTypes/PS5/TextureNType.conf");
		NMOType.AddBaseConfig("HEADLESS", "{AFD658E4D0EB5FBC}configs/ResourceTypes/HEADLESS/TextureNType.conf");

		ref TextureType NHOType = new TextureType(m_Types, "_nho");
		NHOType.CopyFrom(NMOType);

		ref TextureType NTCType = new TextureType(m_Types, "_ntc");
		NTCType.CopyFrom(NMOType);

		ref TextureType NTOType = new TextureType(m_Types, "_nto");
		NTOType.CopyFrom(NMOType);

		//--------------------------------------------------------------------
		ref TextureType VFXType = new TextureType(m_Types, "_vfx");
		VFXType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		VFXType.AddBaseConfig("PC", "{4EC6C20A754D338D}configs/ResourceTypes/PC/TextureEffect.conf");
		VFXType.AddBaseConfig("XBOX_ONE", "{CA41B00C72F6D179}configs/ResourceTypes/XBOX_ONE/TextureEffect.conf");
		VFXType.AddBaseConfig("XBOX_SERIES", "{5DF0BD2327CD114A}configs/ResourceTypes/XBOX_SERIES/TextureEffect.conf");
		VFXType.AddBaseConfig("PS4", "{0FAC34EBBDAC6F40}configs/ResourceTypes/PS4/TextureEffect.conf");
		VFXType.AddBaseConfig("PS5", "{0BB4B87850AF8CF9}configs/ResourceTypes/PS5/TextureEffect.conf");
		VFXType.AddBaseConfig("HEADLESS", "{3C96FCD2DDBE3074}configs/ResourceTypes/HEADLESS/TextureEffect.conf");

		//--------------------------------------------------------------------
		//various masks
		ref TextureType MASKQType = new TextureType(m_Types, "_mask");
		MASKQType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		MASKQType.AddBaseConfig("PC", "{2231C67ACC4BB0AC}configs/ResourceTypes/PC/TextureMask.conf");
		MASKQType.AddBaseConfig("XBOX_ONE", "{3B78A666261DD8B4}configs/ResourceTypes/XBOX_ONE/TextureMask.conf");
		MASKQType.AddBaseConfig("XBOX_SERIES", "{9A79E1F659B378E7}configs/ResourceTypes/XBOX_SERIES/TextureMask.conf");
		MASKQType.AddBaseConfig("PS4", "{5CB64ED6EEBDE368}configs/ResourceTypes/PS4/TextureMask.conf");
		MASKQType.AddBaseConfig("PS5", "{64796311C22F7B3C}configs/ResourceTypes/PS5/TextureMask.conf");
		MASKQType.AddBaseConfig("HEADLESS", "{0B025DFAF55852CD}configs/ResourceTypes/HEADLESS/TextureMask.conf");

		ref TextureType MASK1QType = new TextureType(m_Types, "_mask1");
		MASK1QType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedHQCompression);
		MASK1QType.AddBaseConfig("PC", "{51097E750D3FCE37}configs/ResourceTypes/PC/TextureMask1.conf");
		MASK1QType.AddBaseConfig("XBOX_ONE", "{870BE93DB3625C84}configs/ResourceTypes/XBOX_ONE/TextureMask1.conf");
		MASK1QType.AddBaseConfig("XBOX_SERIES", "{FE3C8C96E8D94604}configs/ResourceTypes/XBOX_SERIES/TextureMask1.conf");
		MASK1QType.AddBaseConfig("PS4", "{D1BEEE662C8A1D1F}configs/ResourceTypes/PS4/TextureMask1.conf");
		MASK1QType.AddBaseConfig("PS5", "{26B26CC013B51E90}configs/ResourceTypes/PS5/TextureMask1.conf");
		MASK1QType.AddBaseConfig("HEADLESS", "{57B7FCEE47CAABB5}configs/ResourceTypes/HEADLESS/TextureMask1.conf");

		ref TextureType MASK2QType = new TextureType(m_Types, "_mask2");
		MASK2QType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);
		MASK2QType.AddBaseConfig("PC", "{0FE15736B2B24038}configs/ResourceTypes/PC/TextureMask2.conf");
		MASK2QType.AddBaseConfig("XBOX_ONE", "{D9E3C07E0CEFD28B}configs/ResourceTypes/XBOX_ONE/TextureMask2.conf");
		MASK2QType.AddBaseConfig("XBOX_SERIES", "{A0D4A5D55754C80B}configs/ResourceTypes/XBOX_SERIES/TextureMask2.conf");
		MASK2QType.AddBaseConfig("PS4", "{8F56C72593079310}configs/ResourceTypes/PS4/TextureMask2.conf");
		MASK2QType.AddBaseConfig("PS5", "{785A4583AC38909F}configs/ResourceTypes/PS5/TextureMask2.conf");
		MASK2QType.AddBaseConfig("HEADLESS", "{095FD5ADF84725BA}configs/ResourceTypes/HEADLESS/TextureMask2.conf");


		//camo mask
		ref TextureType CRMType = new TextureType(m_Types, "_crm");
		CRMType.CopyFrom(MASKQType);

		//clutter mask
		ref TextureType CMASKType = new TextureType(m_Types, "_cmask");
		CMASKType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);
		CMASKType.Insert(GenerateMips, bool, false);
		CMASKType.AddBaseConfig("PC", "{6733F446131BB044}configs/ResourceTypes/PC/TextureCMask.conf");
		CMASKType.AddBaseConfig("XBOX_ONE", "{B131630EAD4622F8}configs/ResourceTypes/XBOX_ONE/TextureCMask.conf");
		CMASKType.AddBaseConfig("XBOX_SERIES", "{C80606A5F6FD3877}configs/ResourceTypes/XBOX_SERIES/TextureCMask.conf");
		CMASKType.AddBaseConfig("PS4", "{E784645532AE636D}configs/ResourceTypes/PS4/TextureCMask.conf");
		CMASKType.AddBaseConfig("PS5", "{1088E6F30D9160E3}configs/ResourceTypes/PS5/TextureCMask.conf");
		CMASKType.AddBaseConfig("HEADLESS", "{618D76DD59EED5C6}configs/ResourceTypes/HEADLESS/TextureCMask.conf");


		//--------------------------------------------------------------------
		//one channel
		ref TextureType AType = new TextureType(m_Types, "_a");
		AType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedHQCompression);
		AType.AddBaseConfig("PC", "{6B149ECE3A72EF45}configs/ResourceTypes/PC/TextureOneChannel.conf");
		AType.AddBaseConfig("XBOX_ONE", "{DC95471339A9BC2B}configs/ResourceTypes/XBOX_ONE/TextureOneChannel.conf");
		AType.AddBaseConfig("XBOX_SERIES", "{C2F053ADC70F3804}configs/ResourceTypes/XBOX_SERIES/TextureOneChannel.conf");
		AType.AddBaseConfig("PS4", "{19FBA5E0ACB1C98E}configs/ResourceTypes/PS4/TextureOneChannel.conf");
		AType.AddBaseConfig("PS5", "{EE57BA009CACD0D1}configs/ResourceTypes/PS5/TextureOneChannel.conf");
		AType.AddBaseConfig("HEADLESS", "{6866463573917E52}configs/ResourceTypes/HEADLESS/TextureOneChannel.conf");

		ref TextureType HType = new TextureType(m_Types, "_h");
		HType.CopyFrom(AType);

		ref TextureType OType = new TextureType(m_Types, "_o");
		OType.CopyFrom(AType);

		ref TextureType AOType = new TextureType(m_Types, "_ao");
		AOType.CopyFrom(AType);

		//--------------------------------------------------------------------
		ref TextureType layerType = new TextureType(m_Types, "_layer");
		layerType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.None);
		layerType.AddBaseConfig("PC", "{A59F335F96C4442F}configs/ResourceTypes/PC/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("XBOX_ONE", "{9843D16A29542D5A}configs/ResourceTypes/XBOX_ONE/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("XBOX_SERIES", "{2071F9D39E062268}configs/ResourceTypes/XBOX_SERIES/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("PS4", "{4C431036F57D4D58}configs/ResourceTypes/PS4/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("PS5", "{BA3AFE83BEFD6794}configs/ResourceTypes/PS5/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("HEADLESS", "{3B38AD285230D19F}configs/ResourceTypes/HEADLESS/TextureTerrainLayer.conf");

		ref TextureType supertextureType = new TextureType(m_Types, "_supertexture");
		supertextureType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		supertextureType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToSrgb);
		supertextureType.AddBaseConfig("PC", "{CBA1266690ABD336}configs/ResourceTypes/PC/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("XBOX_ONE", "{F67DC4532F3BBA43}configs/ResourceTypes/XBOX_ONE/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("XBOX_SERIES", "{4E4FECEA9869B571}configs/ResourceTypes/XBOX_SERIES/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("PS4", "{227D050FF312DA41}configs/ResourceTypes/PS4/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("PS5", "{D404EBBAB892F08D}configs/ResourceTypes/PS5/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("HEADLESS", "{5506B811545F4686}configs/ResourceTypes/HEADLESS/TextureTerrainSuper.conf");

		ref TextureType normaltextureType = new TextureType(m_Types, "_normal");
		normaltextureType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.DXTCompression);
		normaltextureType.AddBaseConfig("PC", "{835E083C88C3D9C3}configs/ResourceTypes/PC/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("XBOX_ONE", "{6C5EFFA712A52100}configs/ResourceTypes/XBOX_ONE/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("XBOX_SERIES", "{90ADE65A903F3042}configs/ResourceTypes/XBOX_SERIES/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("PS4", "{6F6E5A1432381FD2}configs/ResourceTypes/PS4/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("PS5", "{5CEA2418C9F889F8}configs/ResourceTypes/PS5/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("HEADLESS", "{EAB38BDFD096C0A6}configs/ResourceTypes/HEADLESS/TextureTerrainNormal.conf");

		// UI texture atlas
		ref TextureType UITextureAtlasType = new TextureType(m_Types, "_atlas");
		UITextureAtlasType.Insert(GenerateMips, bool, false);
		UITextureAtlasType.AddBaseConfig("PC", "{1D692833852EC72A}configs/ResourceTypes/PC/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("XBOX_ONE", "{96767353535A4256}configs/ResourceTypes/XBOX_ONE/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("XBOX_SERIES", "{8A3A5BA26FD3DE70}configs/ResourceTypes/XBOX_SERIES/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("PS4", "{962DE8F3FC2848A3}configs/ResourceTypes/PS4/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("PS5", "{BED5DFAD2A554849}configs/ResourceTypes/PS5/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("HEADLESS", "{15892FB1C3E30D2C}configs/ResourceTypes/HEADLESS/TextureUIAtlas.conf");

		// UI textures
		ref TextureType uiType = new TextureType(m_Types, "_ui");
		uiType.AddBaseConfig("PC", "{3549C665CAA9A8EA}configs/ResourceTypes/PC/TextureUI.conf");
		uiType.AddBaseConfig("XBOX_ONE", "{5E6D8E2B87D36B34}configs/ResourceTypes/XBOX_ONE/TextureUI.conf");
		uiType.AddBaseConfig("XBOX_SERIES", "{03194CD39583675A}configs/ResourceTypes/XBOX_SERIES/TextureUI.conf");
		uiType.AddBaseConfig("PS4", "{A752014F057BCE60}configs/ResourceTypes/PS4/TextureUI.conf");
		uiType.AddBaseConfig("PS5", "{8BEB72EF65A8FB2E}configs/ResourceTypes/PS5/TextureUI.conf");
		uiType.AddBaseConfig("HEADLESS", "{E604D15366D985DF}configs/ResourceTypes/HEADLESS/TextureUI.conf");

		// UI textures uncompressed
		ref TextureType uiTypeUncompressed = new TextureType(m_Types, "_uiuc");
		uiTypeUncompressed.AddBaseConfig("PC", "{D256EAFDC1226362}configs/ResourceTypes/PC/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("XBOX_ONE", "{712BC6681ADE741A}configs/ResourceTypes/XBOX_ONE/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("XBOX_SERIES", "{B613AECC9D415504}configs/ResourceTypes/XBOX_SERIES/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("PS4", "{9A726D592B629239}configs/ResourceTypes/PS4/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("PS5", "{30FCD9651534E6BB}configs/ResourceTypes/PS5/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("HEADLESS", "{A677FEF8099B272E}configs/ResourceTypes/HEADLESS/TextureUIUncompressed.conf");

		// Env. map (Generate env map from panorama, HDR compression enabled by default)
		ref TextureType environmentMapType = new TextureType(m_Types, "_env");
		environmentMapType.AddBaseConfig("PC", "{009B945B41B128FC}configs/ResourceTypes/PC/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("XBOX_ONE", "{841CE65D460ACA08}configs/ResourceTypes/XBOX_ONE/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("XBOX_SERIES", "{13ADEB7213310A3B}configs/ResourceTypes/XBOX_SERIES/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("PS4", "{41F162BA89507431}configs/ResourceTypes/PS4/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("PS5", "{45E9EE2964539788}configs/ResourceTypes/PS5/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("HEADLESS", "{72CBAA83E9422B05}configs/ResourceTypes/HEADLESS/TextureEnvMap.conf");

		//Lut texture
		ref TextureType lutType = new TextureType(m_Types, "_lut");
		lutType.AddBaseConfig("PC", "{FDA8724761A5E880}configs/ResourceTypes/PC/TextureLut.conf");
		lutType.AddBaseConfig("XBOX_ONE", "{9AE8215636729E7A}configs/ResourceTypes/XBOX_ONE/TextureLut.conf");
		lutType.AddBaseConfig("XBOX_SERIES", "{CBB7E88AA7DE5748}configs/ResourceTypes/XBOX_SERIES/TextureLut.conf");
		lutType.AddBaseConfig("PS4", "{F34906600DC4FE84}configs/ResourceTypes/PS4/TextureLut.conf");
		lutType.AddBaseConfig("PS5", "{5D15FD30EDF58E3B}configs/ResourceTypes/PS5/TextureLut.conf");
		lutType.AddBaseConfig("HEADLESS", "{3D6CF9FDEC095638}configs/ResourceTypes/HEADLESS/TextureLut.conf");

		//this is special type, it looks if file with same name but .fnt extension is exist next to .edds
		ref TextureType fontType = new FontTextureType(m_Types, "_fnt");
		fontType.AddBaseConfig("PC", "{F18523FA5BAAEA63}configs/ResourceTypes/PC/TextureFonts.conf");
		fontType.AddBaseConfig("XBOX_ONE", "{02B393E0229C74CB}configs/ResourceTypes/XBOX_ONE/TextureFonts.conf");
		fontType.AddBaseConfig("XBOX_SERIES", "{85ECC27BD918BA89}configs/ResourceTypes/XBOX_SERIES/TextureFonts.conf");
		fontType.AddBaseConfig("PS4", "{3C102C974458DF07}configs/ResourceTypes/PS4/TextureFonts.conf");
		fontType.AddBaseConfig("PS5", "{863E314F45203AC4}configs/ResourceTypes/PS5/TextureFonts.conf");
		fontType.AddBaseConfig("HEADLESS", "{D20F8633D63483FA}configs/ResourceTypes/HEADLESS/TextureFonts.conf");

		//this is special type which will be assigned when no one from normal types above matches
		ref TextureType unspecifiedType = new TextureType(m_Types, "");
		unspecifiedType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.DXTCompression);
		unspecifiedType.AddBaseConfig("PC", "{DC555BD399D92412}configs/ResourceTypes/PC/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("XBOX_ONE", "{8F13AE697AE60784}configs/ResourceTypes/XBOX_ONE/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("XBOX_SERIES", "{D28E01700D90F52C}configs/ResourceTypes/XBOX_SERIES/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("PS4", "{C6CD3D8752652D2A}configs/ResourceTypes/PS4/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("PS5", "{6248F71B9D7C1E93}configs/ResourceTypes/PS5/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("HEADLESS", "{699C82A6807668A7}configs/ResourceTypes/HEADLESS/TextureUnspecified.conf");
	}

	TextureType FindTextureType(string absFileName)
	{
		TextureType unspecifiedType = null;

		foreach (TextureType texType : m_Types)
		{
			if (texType.m_PostFix == "")
			{
				unspecifiedType = texType;
				continue;
			}

			if (texType.IsType(absFileName))
				return texType;
		}

		return unspecifiedType;
	}

	TextureCheckResult DoChecks(TextureIssueOp op, ResourceName resourceName, notnull MetaFile meta)
	{
		int combined = 0;
		TextureCheckResult checkRes;

		checkRes = CheckConfigurations(op, resourceName, meta);
		combined |= checkRes;
		if (checkRes == TextureCheckResult.UnfixedFatalIssue)
			return checkRes;

		TextureType matchingType = FindTextureType(resourceName);
		checkRes = CheckAncestors(op, resourceName, meta, matchingType);
		combined |= checkRes;
		if (checkRes == TextureCheckResult.UnfixedFatalIssue)
			return checkRes;

		checkRes = CheckUnnecessarySettingInPc(op, resourceName, meta);
		combined |= checkRes;
		if (checkRes == TextureCheckResult.UnfixedFatalIssue)
			return checkRes;

		checkRes = CheckSuspiciousNonPcSetting(op, resourceName, meta);
		combined |= checkRes;
		if (checkRes == TextureCheckResult.UnfixedFatalIssue)
			return checkRes;

		checkRes = CheckWrongPropertyValues(op, resourceName, meta, matchingType);
		combined |= checkRes;
		if (checkRes == TextureCheckResult.UnfixedFatalIssue)
			return checkRes;

		return combined;
	}

	TextureCheckResult CheckConfigurations(TextureIssueOp op, ResourceName resourceName, notnull MetaFile meta)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		if (!configurations)
		{
			if (op & TextureIssueOp.Report)
				PrintFormat("@\"%1\" : Meta-file is missing 'Configurations' property.", resourceName);

			return TextureCheckResult.UnfixedFatalIssue;
		}

		BaseContainer confPc = configurations.Get(0);
		string confClassPc = confPc.GetClassName();
		if (confClassPc == "EDDSResourceClass")
		{
			if (op & TextureIssueOp.Report)
				PrintFormat("@\"%1\" : Resource class is 'EDDSResourceClass'", resourceName);

			return TextureCheckResult.UnfixedFatalIssue;
		}

		int numVarsPc = confPc.GetNumVars();
		TextureCheckResult res = TextureCheckResult.NothingChanged;
		for (int iConf = 1, countConf = configurations.Count(); iConf < countConf; iConf++)
		{
			BaseContainer confDerived = configurations.Get(iConf);
			string confName = confDerived.GetName();
			string confClassDerived = confDerived.GetClassName();
			if (confClassPc != confClassDerived)
			{
				if (op & TextureIssueOp.Report)
				{
					PrintFormat(
						"@\"%1\" : Configuration '%2' has wrong type: expected: '%3', found: '%4'",
						resourceName, confName, confClassPc, confClassDerived
					);
				}
				res = TextureCheckResult.UnfixedFatalIssue;
			}

			int numVarsDerived = confDerived.GetNumVars();
			if (numVarsPc != numVarsDerived)
			{
				if (op & TextureIssueOp.Report)
				{
					PrintFormat(
						"@\"%1\" : Configuration '%1' has wrong num vars: expected: '%2', found: '%3'",
						resourceName, confName, numVarsPc, numVarsDerived
					);
				}
				res = TextureCheckResult.UnfixedFatalIssue;
			}
		}

		return res;
	}

	TextureCheckResult CheckAncestors(TextureIssueOp op, ResourceName resourceName, notnull MetaFile meta, notnull TextureType type)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");

		bool unfixedFatalIssue = false;
		bool somethingChanged = false;
		for (int iConf = 0, countConf = configurations.Count(); iConf < countConf; iConf++)
		{
			BaseContainer config = configurations.Get(iConf);
			string configName = config.GetName();
			ResourceName baseConfig = type.GetBaseConfig(configName);

			if (!baseConfig)
			{
				if (op & TextureIssueOp.Report)
				{
					PrintFormat("@\"%1\" : Ancestor config missing for configuration '%2'.",
						resourceName, configName
					);
				}

				unfixedFatalIssue = true;
				continue;
			}

			BaseContainer ancestor = config.GetAncestor();
			if (!ancestor || ancestor.GetResourceName() != baseConfig)
			{
				if (op & TextureIssueOp.Report)
				{
					PrintFormat("@\"%1\" : Incorrect ancestor on configuration '%2'",
						resourceName, configName
					);
				}
				if (op & TextureIssueOp.Fix)
				{
					config.SetAncestor(baseConfig);
					somethingChanged = true;
				}
				else
				{
					unfixedFatalIssue = true;
				}
			}
		}

		if (unfixedFatalIssue)
			return TextureCheckResult.UnfixedFatalIssue;
		else if (somethingChanged)
			return TextureCheckResult.SomethingChanged;
		else
			return TextureCheckResult.NothingChanged;
	}

	TextureCheckResult CheckUnnecessarySettingInPc(TextureIssueOp op, ResourceName resourceName, notnull MetaFile meta)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer confPc = configurations[0];
		BaseContainer confPcAncestor = confPc.GetAncestor();
		if (!confPcAncestor)
			return TextureCheckResult.NothingChanged;

		bool propertiesModified = false;
		for (int iVar = 0, countVar = confPc.GetNumVars(); iVar < countVar; iVar++)
		{
			string name = confPc.GetVarName(iVar);
			if (!confPc.IsVariableSetDirectly(name))
				continue;

			DataVarType varType = confPc.GetDataVarType(iVar);
			if (varType == DataVarType.SCALAR)
			{
				float propValPc, propValPcAncestor;
				if (!confPc.Get(name, propValPc))
					continue;
				if (!confPcAncestor.Get(name, propValPcAncestor))
					continue;

				if (propValPc == propValPcAncestor)
				{
					if (op & TextureIssueOp.Report)
					{
						PrintFormat(
							"@\"%1\" : Configuration 'PC' has directly set value for property '%2', even though same value is inherited from ancestor.",
							resourceName, name
						);
					}

					if (op & TextureIssueOp.Fix)
					{
						propertiesModified = true;
						confPc.ClearVariable(name);
					}
				}
			}
			else
			{
				int propValPc, propValPcAncestor;
				if (!confPc.Get(name, propValPc))
					continue;
				if (!confPcAncestor.Get(name, propValPcAncestor))
					continue;

				if (propValPc == propValPcAncestor)
				{
					if (op & TextureIssueOp.Report)
					{
						PrintFormat(
							"@\"%1\" : Configuration 'PC' has directly set value for property '%2', even though same value is inherited from ancestor.",
							resourceName, name
						);
					}

					if (op & TextureIssueOp.Fix)
					{
						propertiesModified = true;
						confPc.ClearVariable(name);
					}
				}
			}
		}

		if (propertiesModified)
			return TextureCheckResult.SomethingChanged;
		else
			return TextureCheckResult.NothingChanged;
	}

	TextureCheckResult CheckSuspiciousNonPcSetting(TextureIssueOp op, ResourceName resourceName, notnull MetaFile meta)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer confPc = configurations[0];
		bool propertiesModified = false;
		bool fatalIssue = false;
		for (int iConf = 1, countConf = configurations.Count(); iConf < countConf; iConf++)
		{
			BaseContainer confDerived = configurations.Get(iConf);
			string confName = confDerived.GetName();

			for (int iVar = 0, countVar = confPc.GetNumVars(); iVar < countVar; iVar++)
			{
				string namePc = confPc.GetVarName(iVar);
				string nameDerived = confDerived.GetVarName(iVar);
				if (namePc != nameDerived)
				{
					if (op & TextureIssueOp.Report)
					{
						PrintFormat(
							"@\"%1\" : Configuration '%2' has wrong var %3 name: expected: '%4', actual: '%5'",
							resourceName, confName, iVar, namePc, nameDerived
						);
					}

					fatalIssue = true;
					continue;
				}

				DataVarType varType = confPc.GetDataVarType(iVar);
				bool isSetDirectlyPc = confPc.IsVariableSetDirectly(namePc);
				bool isSetDirectlyDerived = confDerived.IsVariableSetDirectly(nameDerived);
				if (isSetDirectlyPc)
				{
					if (varType == DataVarType.SCALAR)
					{
						float propValPc, propValDerived;
						if (!confPc.Get(namePc, propValPc))
							continue;

						if (!confDerived.Get(nameDerived, propValDerived))
							continue;

						if (propValPc != propValDerived)
						{
							if (op & TextureIssueOp.Report)
							{
								PrintFormat(
									"@\"%1\" : Configuration '%2' has wrong value in property '%3': expected '%4', found '%5'",
									resourceName, confName, nameDerived, propValPc, propValDerived
								);
							}

							if (op & TextureIssueOp.Fix)
							{
								propertiesModified = true;
								confDerived.Set(nameDerived, propValPc);
							}
						}
					}
					else
					{
						int propValPc, propValDerived;
						if (!confPc.Get(namePc, propValPc))
							continue;

						if (!confDerived.Get(nameDerived, propValDerived))
							continue;

						if (propValPc != propValDerived)
						{
							if (op & TextureIssueOp.Report)
							{
								PrintFormat(
									"@\"%1\" : Configuration '%2' has wrong value in property '%3': expected '%4', found '%5'",
									resourceName, confName, nameDerived, propValPc, propValDerived
								);
							}

							if (op & TextureIssueOp.Fix)
							{
								propertiesModified = true;
								confDerived.Set(nameDerived, propValPc);
							}
						}
					}
				}
				else if (isSetDirectlyDerived)
				{
					if (varType == DataVarType.SCALAR)
					{
						float propValDerived;
						if (!confDerived.Get(nameDerived, propValDerived))
							continue;

						if (op & TextureIssueOp.Report)
						{
							PrintFormat(
								"@\"%1\" : Configuration '%2' has property '%3' set directly while 'PC' doesn't. value: '%4'",
								resourceName, confName, nameDerived, propValDerived
							);
						}

						if (op & TextureIssueOp.Fix)
						{
							propertiesModified = true;
							confDerived.ClearVariable(nameDerived);
						}
					}
					else
					{
						int propValDerived;
						if (!confDerived.Get(nameDerived, propValDerived))
							continue;

						if (op & TextureIssueOp.Report)
						{
							PrintFormat(
								"@\"%1\" : Configuration '%2' has property '%3' set directly while 'PC' doesn't. value: '%4'",
								resourceName, confName, nameDerived, propValDerived
							);
						}

						if (op & TextureIssueOp.Fix)
						{
							propertiesModified = true;
							confDerived.ClearVariable(nameDerived);
						}
					}
				}
			}
		}

		if (fatalIssue)
			return TextureCheckResult.UnfixedFatalIssue;
		else if (propertiesModified)
			return TextureCheckResult.SomethingChanged;
		else
			return TextureCheckResult.NothingChanged;
	}

	TextureCheckResult CheckWrongPropertyValues(TextureIssueOp op, ResourceName resourceName, notnull MetaFile meta, notnull TextureType type)
	{
		// This check is currently disabled because it is not clear what it should
		// be doing now that we have introduced configs for specific texture suffixes.
		return TextureCheckResult.NothingChanged;

		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer confPc = configurations[0];

		TextureCheckResult res = TextureCheckResult.NothingChanged;
		foreach (TextureTypeProperty prop : type.m_Properties)
		{
			int propVal;
			if (confPc.Get(prop.m_Name, propVal))
			{
				int propValCorrect = prop.m_Val;
				bool isIncorrect = propVal != propValCorrect;
				if (isIncorrect && prop.m_OtherVariants)
				{
					foreach (int otherVariant : prop.m_OtherVariants)
					{
						if (otherVariant == propVal)
						{
							isIncorrect = false;
							break;
						}
					}
				}

				if (isIncorrect)
				{
					if (op & TextureIssueOp.Report)
					{
						string valName, valNameCorrect;
						if (prop.m_ValType == bool)
						{
							if (propVal)        { valName        = "true"; } else { valName        = "false"; }
							if (propValCorrect) { valNameCorrect = "true"; } else { valNameCorrect = "false"; }
						}
						else
						{
							valName        = typename.EnumToString(prop.m_ValType, propVal);
							valNameCorrect = typename.EnumToString(prop.m_ValType, propValCorrect);
						}
						PrintFormat("@\"%1\" : Property '%2' has wrong value ('%3' instead of '%4').",
							resourceName, prop.m_Name, valName, valNameCorrect
						);
					}

					if (op & TextureIssueOp.Fix)
					{
						res = TextureCheckResult.SomethingChanged;
						confPc.Set(prop.m_Name, propValCorrect);
					}
				}
			}
			else
			{
				if (op & TextureIssueOp.Report)
				{
					PrintFormat("@\"%1\" : Property '%2' is not set", resourceName, prop.m_Name);
				}

				if (op & TextureIssueOp.Fix)
				{
					res = TextureCheckResult.SomethingChanged;
					confPc.Set(prop.m_Name, prop.m_Val);
				}
			}
		}

		return res;
	}
}

class FontTextureType: TextureType
{
	override bool IsType(string path)
	{
		string fntPath = FilePath.ReplaceExtension(path, "fnt");
		return FileIO.FileExists(fntPath);
	}
}

// //----------------------------------------------------------------------------------------------
// bool FixTextureMetaFile(MetaFile meta, string absFileName, TextureTypes textureTypes)
// {
// 	BaseContainerList configurations = meta.GetObjectArray("Configurations");
// 	if(!configurations)
// 		return false;

// 	bool anyChangeInMetaFile = false;

// 	for(int c = 0; c < configurations.Count(); c++)
// 	{
// 		TextureType typeToAssign = textureTypes.FindTextureType(absFileName);
// 		BaseContainer cfg = configurations.Get(c);
// 		string cfgName = cfg.GetName();
// 		ResourceName ancestor = typeToAssign.GetBaseConfig(cfgName);
// 		if (ancestor != "")
// 		{
// 			cfg.SetAncestor(ancestor);
// 			anyChangeInMetaFile = true;
// 		}
// 	}

// 	return anyChangeInMetaFile;
// }

[WorkbenchPluginAttribute("Texture Import", "Texture Import Helper", "", "", {"ResourceManager"},"",0xf574)]
class TextureImportPlugin: ResourceManagerPlugin
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;

	ref TextureTypes m_TextureTypes = new TextureTypes();

	//--------------------------------------------------------------------
	bool IsImage(string className)
	{
		return
			className == "PNGResourceClass" ||
			className == "DDSResourceClass" ||
			className == "TGAResourceClass" ||
			className == "TIFFResourceClass" ||
			className == "PNGResourceClass" ||
			className == "HDRResourceClass" ||
			className == "JPGResourceClass";
	}

	//--------------------------------------------------------------------
	override void OnRegisterResource(string absFileName, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled || !IsImage(conf.GetClassName()))
			return;

		TextureType type = m_TextureTypes.FindTextureType(absFileName);
		m_TextureTypes.CheckAncestors(TextureIssueOp.Fix, absFileName, metaFile, type);
		// FixTextureMetaFile(metaFile, absFileName, m_TextureTypes);
	}

	override void OnRenameResource(string absFileNameOld, string absFileNameNew, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled || !IsImage(conf.GetClassName()))
			return;

		TextureType type = m_TextureTypes.FindTextureType(absFileNameNew);
		m_TextureTypes.CheckAncestors(TextureIssueOp.Fix, absFileNameNew, metaFile, type);
		// FixTextureMetaFile(metaFile, absFileNameNew, m_TextureTypes);
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure Texture Import Plugin", "", this);
	}

	[ButtonAttribute("OK")]
	void OkButton() {}
}

[WorkbenchPluginAttribute("Batch texture processor", "Perform simple checks and fixes on many textures", "", "", {"ResourceManager"},"",0xf1c5)]
class BatchTextureProcessorPlugin: WorkbenchPlugin
{
	[Attribute("", UIWidgets.FileNamePicker, "Check only textures whose path starts with given filter string.", params:"folders")]
	string PathStartsWith;

	[Attribute("", UIWidgets.EditBox, "Check only textures whose path contains given filter string.")]
	string PathContains;

	[Attribute("", UIWidgets.EditBox, "Check only textures whose path ends with given filter string.")]
	string PathEndsWith;

	[Attribute("true", UIWidgets.CheckBox, "Report missing meta-file")]
	bool ReportMissingMetaFile;

	[Attribute(
		enumType: TextureIssueOp,
		uiwidget: UIWidgets.ComboBox,
		desc: "Check whether configurations are present and have correct classes."
	)]
	TextureIssueOp CheckConfigurations;

	[Attribute(
		enumType: TextureIssueOp,
		uiwidget: UIWidgets.ComboBox,
		desc: "Check whether configurations have correct ancestors."
	)]
	TextureIssueOp CheckAncestors;

	[Attribute(
		enumType: TextureIssueOp,
		uiwidget: UIWidgets.ComboBox,
		desc: "Check whether PC configuration is directly setting same value as inherited from ancestor"
	)]
	TextureIssueOp UnnecessarySettingInPc;

	[Attribute(
		enumType: TextureIssueOp,
		uiwidget: UIWidgets.ComboBox,
		desc: "Non-PC configuration property having different value than what is directly set in PC."
	)]
	TextureIssueOp CheckSuspiciousNonPcSetting;

	[Attribute(
		enumType: TextureIssueOp,
		uiwidget: UIWidgets.ComboBox,
		desc: "Check whether properties have values matching presets."
	)]
	TextureIssueOp CheckWrongPropertyValues;

	[Attribute("true", UIWidgets.CheckBox, "Reimport changed resources")]
	bool ReimportChanged;

	ref TextureTypes m_TextureTypes = new TextureTypes();
	ref array<string> m_Resources = new array<string>;

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	bool OK()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool TestAgainstFilter(string resource)
	{
		int resourceLength = resource.Length();

		int prefixLength = PathStartsWith.Length();
		if (resourceLength < prefixLength)
			return false;

		int suffixLength = PathEndsWith.Length();
		if (resourceLength < suffixLength)
			return false;

		int subLength = PathContains.Length();
		if (resourceLength < subLength)
			return false;

		resource.ToLower();
		if (!resource.StartsWith(PathStartsWith))
			return false;

		if (!resource.EndsWith(PathEndsWith))
			return false;

		if (!resource.Contains(PathContains))
			return false;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void Find(ResourceName resName, string filePath)
	{
		m_Resources.Insert(filePath);
	}

	//----------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Configure batch texture processor", "", this))
			return;

		PathStartsWith.ToLower();
		PathContains.ToLower();
		PathEndsWith.ToLower();

		// TODO: Search resources in specific directory? Must filter myself.

		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = {"edds"};
		ResourceDatabase.SearchResources(filter, Find);

		m_Resources.Sort();

		ResourceManager rb = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Processing...", rb);

		float count = m_Resources.Count();

		array<string> toRebuild = new array<string>;

		foreach (int resourceIdx, string resource : m_Resources)
		{
			progress.SetProgress(resourceIdx / count);

			if (!TestAgainstFilter(resource))
				continue;

			MetaFile meta = rb.GetMetaFile(resource);
			if (!meta)
			{
				if (ReportMissingMetaFile)
					PrintFormat("@\"%1\" : Meta-file is missing.", resource);

				continue;
			}

			TextureType matchingType;
			bool metaChanged = false;
			TextureCheckResult checkRes = TextureCheckResult.NothingChanged;

			if (checkRes != TextureCheckResult.UnfixedFatalIssue)
			{
				checkRes = m_TextureTypes.CheckConfigurations(CheckConfigurations, resource, meta);
				if (checkRes == TextureCheckResult.SomethingChanged)
					metaChanged = true;
			}

			if (checkRes != TextureCheckResult.UnfixedFatalIssue)
			{
				matchingType = m_TextureTypes.FindTextureType(resource);
				checkRes = m_TextureTypes.CheckAncestors(CheckAncestors, resource, meta, matchingType);
				if (checkRes == TextureCheckResult.SomethingChanged)
					metaChanged = true;
			}

			if (checkRes != TextureCheckResult.UnfixedFatalIssue)
			{
				checkRes = m_TextureTypes.CheckUnnecessarySettingInPc(UnnecessarySettingInPc, resource, meta);
				if (checkRes == TextureCheckResult.SomethingChanged)
					metaChanged = true;
			}

			if (checkRes != TextureCheckResult.UnfixedFatalIssue)
			{
				checkRes = m_TextureTypes.CheckSuspiciousNonPcSetting(CheckSuspiciousNonPcSetting, resource, meta);
				if (checkRes == TextureCheckResult.SomethingChanged)
					metaChanged = true;
			}

			if (checkRes != TextureCheckResult.UnfixedFatalIssue)
			{
				checkRes = m_TextureTypes.CheckWrongPropertyValues(CheckWrongPropertyValues, resource, meta, matchingType);
				if (checkRes == TextureCheckResult.SomethingChanged)
					metaChanged = true;
			}

			if (metaChanged)
			{
				meta.Save();
				toRebuild.Insert(resource);
			}
			meta.Release();
		}

		if (ReimportChanged && toRebuild.Count() > 0)
		{
			Print("Reimporting modified resources.");
			rb.RebuildResourceFiles(toRebuild, "PC");
		}

		m_Resources.Clear();
	}
}
