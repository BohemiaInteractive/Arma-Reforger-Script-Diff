[BaseContainerProps(configRoot: true), SCR_BaseContainerMilitarySymbol()]
class SCR_MilitarySymbol
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIdentity))]
	protected EMilitarySymbolIdentity m_Identity;
	
	[Attribute("1", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	protected EMilitarySymbolDimension m_Dimension;
	
	[Attribute("0", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	protected EMilitarySymbolIcon m_Icons;
	
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolAmplifier))]
	protected EMilitarySymbolAmplifier m_Amplifier;
	
	/*!
	Get symbol identity.
	\return Identity
	*/
	EMilitarySymbolIdentity GetIdentity()
	{
		return m_Identity;
	}
	/*!
	Set symbol identity.
	\param identity Identity
	*/
	void SetIdentity(EMilitarySymbolIdentity identity)
	{
		m_Identity = identity;
	}
	/*!
	Get symbol dimension.
	\return Dimension
	*/
	EMilitarySymbolDimension GetDimension()
	{
		return m_Dimension;
	}
	/*!
	Set symbol dimension.
	\param dimension Dimension
	*/
	void SetDimension(EMilitarySymbolDimension dimension)
	{
		m_Dimension = dimension;
	}
	/*!
	Get symbol icons.
	\return Icon flags
	*/
	EMilitarySymbolIcon GetIcons()
	{
		return m_Icons;
	}
	/*!
	Set symbol icons.
	\param icons Icons
	*/
	void SetIcons(EMilitarySymbolIcon icons)
	{
		m_Icons = icons;
	}
	/*!
	Check if the symbol has given icon.
	\return True when it has the icon
	*/
	bool HasIcon(EMilitarySymbolIcon icon)
	{
		return SCR_Enum.HasFlag(m_Icons, icon);
	}
	/*!
	Get symbol amplifier.
	\return Amplifier
	*/
	EMilitarySymbolAmplifier GetAmplifier()
	{
		return m_Amplifier;
	}
	/*!
	Set symbol amplifier.
	\param amplifier Amplifier
	*/
	void SetAmplifier(EMilitarySymbolAmplifier amplifier)
	{
		m_Amplifier = amplifier;
	}
	
	/*!
	Set all symbol values.
	\param identity Identity
	\param dimension Dimension
	\param icons Icons
	\param amplifier Amplifier
	*/
	void Set(EMilitarySymbolIdentity identity, EMilitarySymbolDimension dimension, EMilitarySymbolIcon icons, EMilitarySymbolAmplifier amplifier)
	{
		m_Identity = identity;
		m_Dimension = dimension;
		m_Icons = icons;
		m_Amplifier = amplifier;
	}
	
	/*!
	Copy values from another symbol.
	\param from Source symbol
	*/
	void CopyFrom(SCR_MilitarySymbol from)
	{
		m_Identity = from.m_Identity;
		m_Dimension = from.m_Dimension;
		m_Icons = from.m_Icons;
		m_Amplifier = from.m_Amplifier;
	}
	/*!
	Check if this symbol is equal to another one.
	\param symbol Compared symbol
	\return True when values are the same
	*/
	bool IsEqual(SCR_MilitarySymbol symbol)
	{
		return m_Identity == symbol.m_Identity
			&& m_Dimension == symbol.m_Dimension
			&& m_Icons == symbol.m_Icons
			&& m_Amplifier == symbol.m_Amplifier;
	}
	
	/*!
	Log the symbol values to console.
	\param logLevel Log level
	*/
	void Log(LogLevel logLevel = LogLevel.DEBUG)
	{
		string identityName = typename.EnumToString(EMilitarySymbolIdentity, m_Identity);
		string dimensionName = typename.EnumToString(EMilitarySymbolDimension, m_Dimension);
		string amplifierName = typename.EnumToString(EMilitarySymbolAmplifier, m_Amplifier);
		string iconNames = SCR_Enum.FlagsToString(EMilitarySymbolIcon, m_Icons);
		
		Print(string.Format("%1 | %2 | %3 | %4", identityName, dimensionName, iconNames, amplifierName), logLevel);
	}
	
	//--- Codec
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 16);
	}
	static bool Decode(ScriptBitSerializer packet, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 16);
	}
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx hint) 
	{
		return lhs.CompareSnapshots(rhs, 16);
	}
	static bool PropCompare(SCR_MilitarySymbol prop, SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		return snapshot.CompareInt(prop.m_Identity)
			&& snapshot.CompareInt(prop.m_Dimension)
			&& snapshot.CompareInt(prop.m_Icons)
			&& snapshot.CompareInt(prop.m_Amplifier);
	}
	static bool Extract(SCR_MilitarySymbol prop, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeInt(prop.m_Identity);
		snapshot.SerializeInt(prop.m_Dimension);
		snapshot.SerializeInt(prop.m_Icons);
		snapshot.SerializeInt(prop.m_Amplifier);
		return true;
	}
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx hint, SCR_MilitarySymbol prop) 
	{
		snapshot.SerializeInt(prop.m_Identity);
		snapshot.SerializeInt(prop.m_Dimension);
		snapshot.SerializeInt(prop.m_Icons);
		snapshot.SerializeInt(prop.m_Amplifier);
		return true;
	}
	
	void OnRplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_Identity);
		writer.WriteInt(m_Dimension);
		writer.WriteInt(m_Icons);
		writer.WriteInt(m_Amplifier);
	}
	void OnRplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_Identity);
		reader.ReadInt(m_Dimension);
		reader.ReadInt(m_Icons);
		reader.ReadInt(m_Amplifier);
	}
};
class SCR_BaseContainerMilitarySymbol: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		EMilitarySymbolIdentity identity;
		source.Get("m_Identity", identity);
		
		EMilitarySymbolDimension dimension;
		source.Get("m_Dimension", dimension);
		
		EMilitarySymbolIcon icons;
		source.Get("m_Icons", icons);
		
		EMilitarySymbolAmplifier amplifier;	
		source.Get("m_Amplifier", amplifier);
		
		string identityName = typename.EnumToString(EMilitarySymbolIdentity, identity);
		string dimensionName = typename.EnumToString(EMilitarySymbolDimension, dimension);
		string amplifierName = typename.EnumToString(EMilitarySymbolAmplifier, amplifier);
		string iconNames = SCR_Enum.FlagsToString(EMilitarySymbolIcon, icons);
		
		title = string.Format("%1 | %2 | %3 | %4", identityName, dimensionName, iconNames, amplifierName);
		
		return true;
	}
};