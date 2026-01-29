/*!
Classes for handling limits of configurable widget entries.
*/

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_WidgetEditFormat 
{	
	// String for storing message of current problem 
	// Useful when format has multiple state
	protected string m_sFormatMessage;
	
	protected EditBoxFilterComponent m_EditBoxFilter;
	
	//------------------------------------------------------------------------------------------------
	//! Method ready for overr to verifying that format is correct
	bool IsFormatValid(string str)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFormatMessage()
	{
		return m_sFormatMessage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEditBoxFilter(EditBoxFilterComponent filter)
	{
		m_EditBoxFilter = filter;
	}
}

//------------------------------------------------------------------------------------------------
//! Is string of count
[BaseContainerProps(configRoot: true)]
class SCR_WidgetEditFormatRange : SCR_WidgetEditFormat 
{
	[Attribute("0")]
	protected int m_iMinRange;
	
	[Attribute("0")]
	protected int m_iMaxRange;
	
	//------------------------------------------------------------------------------------------------
	override bool IsFormatValid(string str)
	{
		return BellowMin(str) && OverMax(str);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool BellowMin(string str)
	{
		return str.Length() < m_iMinRange;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected bool OverMax(string str)
	{
		return str.Length() >= m_iMaxRange;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMinRange()
	{
		return m_iMinRange;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaxRange()
	{
		return m_iMaxRange;
	}
}

//------------------------------------------------------------------------------------------------
//! Is string of count
[BaseContainerProps(configRoot: true)]
class SCR_WidgetEditFormatNumberRange : SCR_WidgetEditFormat 
{
	protected const string HINT_RANGE = "#AR-ServerHosting_ValidRangeHint";
	
	[Attribute("0")]
	protected float m_fMinRange;
	
	[Attribute("0")]
	protected float m_fMaxRange;
	
	//------------------------------------------------------------------------------------------------
	override bool IsFormatValid(string str)
	{
		if (str.IsEmpty())
			return false;
		
		float number = str.ToFloat();
		
		// Save check 
		if (number == 0 && str != "0")
			return false;
		
		m_sFormatMessage = WidgetManager.Translate(HINT_RANGE, m_fMinRange.ToString(), m_fMaxRange.ToString());
		
		return (m_fMinRange <= number) && (number <= m_fMaxRange);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxRange(float max)
	{
		m_fMaxRange = max;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxRange()
	{
		return m_fMaxRange;
	}
	
}

//------------------------------------------------------------------------------------------------
//! Format for ip
[BaseContainerProps(configRoot: true)]
class SCR_WidgetEditFormatIP : SCR_WidgetEditFormat
{
	static const string LAN_VALUE = "local";
	protected static int ADDRESS_VALUES_COUNT = 4;
	
	protected const string HINT_RANGE = "#AR-ServerHosting_ValidRangeHint";
	protected const string HINT_PRIVATE = "#AR-ServerHosting_PrivateIPHint";
	
	protected bool m_bIsValidated;
	protected bool m_bIsPrivate; 
	
	//------------------------------------------------------------------------------------------------
	override bool IsFormatValid(string str)
	{
		m_bIsValidated = false;
		m_sFormatMessage = WidgetManager.Translate(HINT_RANGE, "0.0.0.0", "255.255.255.255");
		
		if (str == string.Empty)
			return false;
		
		// local string check 
		if (str == LAN_VALUE)
			return true;
		
		// IP format check 
		array<string> values = new array<string>();
		str.Split(".", values, true);
		int valueCount = values.Count();
		
		// Count dots 
		int dots = 0; 
		for (int i = 0, len = str.Length(); i < len; i++)
		{
			if (str[i] == ".")
				dots++;
		}
		
		// 0.1.2.3 = 4 values
		if (valueCount != 4 || dots != 3)
			return false;
		
		int zeroElements = 0;
		
		// check if values are ok
		for (int i = 0; i < 4; i++)
		{
			// Iterate through all tokens parsed between '.'
			// if all of the characters are digit (ascii 48-57)
			// it then checks if each value is within 0-255 range
			// if anything fails, returns false
			int strLen = values[i].Length();
			for (int j = 0; j < strLen; j++)
			{
				// Check if it's a valid digit
				string char = values[i].Get(j);
				int asciiValue = char.ToAscii();
				if (asciiValue >= 48 && asciiValue <= 57)
					continue;
				else
					return false;
			}
			
			int value = values[i].ToInt();
			if (value == 0)
				zeroElements++;
			
			if (value >= 0 && value <= 255)
				continue;
			else
				return false;
		}
		
		/*
		m_bIsPrivate = IsPrivate(str);
		if (m_bIsPrivate)
			m_sFormatMessage = HINT_PRIVATE;
		*/
		
		m_bIsValidated = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPrivate(string address)
	{
		int decAddress = AddressToDecimal(address, false);
		
		// Class A
		if (IsAddressInRange(address, "10.0.0.0", "10.255.255.255", false))
			return true;
		
		// CLass B 
		if (IsAddressInRange(address, "172.16.0.0", "172.31.255.255", false))
			return true;
		
		// Class C
		if (IsAddressInRange(address, "192.168.0.0", "192.168.255.255", false))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if address is in given range
	static bool IsAddressInRange(string address, string min, string max, bool validCheck = true)
	{
		int decAddress = AddressToDecimal(address, validCheck);
		int decMin = AddressToDecimal(min, validCheck);
		int decMax = AddressToDecimal(max, validCheck);
		
		return decMin <= decAddress && decAddress <= decMax;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Retrun IP address formant number to int
	static int AddressToDecimal(string address, bool validCheck = true)
	{
		if (validCheck && !IsFormatValid(address))
			return -1;
		
		int value = 0;
		
		array<string> values = new array<string>();
		address.Split(".", values, true);
		
		// check if values are ok
		for (int i = ADDRESS_VALUES_COUNT - 1; i >= 0; i--)
		{
			int decimal = Math.Pow(256, ADDRESS_VALUES_COUNT - i - 1);
			decimal *= values[i].ToInt();
			
			value += decimal;
		}
		
		return value;
	} 
	
	//------------------------------------------------------------------------------------------------
	bool IsValidated()
	{
		return m_bIsValidated;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPrivate()
	{
		return m_bIsPrivate;
	}
}

//------------------------------------------------------------------------------------------------
//! Format for name with specific messages and variable limitations 
[BaseContainerProps(configRoot: true)]
class SCR_WidgetEditFormatName : SCR_WidgetEditFormatRange
{
	protected const string HINT_SHORT = "#AR-ServerHosting_ShortTextWarning";
	protected const string HINT_LONG = "";
	protected const string HINT_WRONG_INPUT = "";
	
	//------------------------------------------------------------------------------------------------
	override bool IsFormatValid(string str)
	{
		if (!m_EditBoxFilter)
			return true;
		
		// Invalid input
		if (!m_EditBoxFilter.IsLastInputValid())
		{
			m_sFormatMessage = HINT_WRONG_INPUT;
			return false;
		}
		
		// Min
		if (BellowMin(str))
		{
			m_sFormatMessage =  WidgetManager.Translate(HINT_SHORT, m_iMinRange);
			return false;
		}	
		
		// Max
		if (OverMax(str))
		{
			m_sFormatMessage = HINT_LONG;
			return false;
		}
		
		m_sFormatMessage = "";
		return true;
	}
}
