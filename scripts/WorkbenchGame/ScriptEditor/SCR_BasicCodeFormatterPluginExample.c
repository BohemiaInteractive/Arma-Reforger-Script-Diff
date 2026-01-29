//
//	Basic Code Formatter formatting example file
//	highlight, press numpad "/" to uncomment, then format with Ctrl+Shift+K
//

//class abc:Managed						// must warn for non-prefixed class + must space around ':'
//{
//	[Attribute()];						// must remove the semicolon
//
//	// must not warn about these properly-named variables
//	protected static const ref array<ref array<int>> DEFAULT_QUICK_SLOTS = {};
//	protected array<string> m_aValue1;
//	protected bool m_bValue1;
//	protected float m_fValue1;
//	protected int m_iValue1;
//	protected map<string, string> m_mValue1;
//	protected vector m_vValue1;
//	protected Widget m_wValue1;
//	protected static bool s_bValue1;
//	protected const bool VALUE_1;
//
//	// must warn about these improperly-named variables
//	protected static const ref array<ref array<int>> m_aValue = {};
//	protected static const ref array<ref array<int>> s_aValue = {};
//	protected ref array<ref array<int>> M_AVALUE = {};
//	private array<string> m_Strings;
//	protected Color m_cValue;
//	protected float m_iValue2;
//	protected Color m_jValue;
//	protected Color m_kValue;
//	protected Color m_lValue;
//	protected Color m_mValue2;
//	protected Color m_nValue;
//	protected Color m_oValue;
//	protected Color m_pValue;
//	protected Color m_qValue;
//	protected Color m_rValue;
//	protected Color m_sValue2;
//	protected Color m_tValue;
//	protected Color m_uValue;
//	protected Color m_vValue2;
//	protected Color m_wValue2;
//	protected Color m_xValue;
//	protected Color m_yValue;
//	protected Color m_zValue;
//	protected int m_inumber;
//	protected ScriptInvokerVoid Event_OnSomething;
//	protected static string m_sValue3;
//
//										// must fix the separator (below)
//	//---
//	protected void Method()
//     {								// must replace 5 spaces by one tab
//		if(true ) return;				// must reformat the if and warn about one-liner
//										// must warn about two empty lines (below)
//
//
//										// must remove trailing empty space (below)
//	} 
//// must remove the final semicolon below
//};
//// must add a line return (below)