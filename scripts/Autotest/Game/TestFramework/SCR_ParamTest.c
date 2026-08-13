//! This is a wrapper of the Test attribute class that allows us 
//! to distunguish between normal tests and parameterized ones using reflection
class SCR_ParamTest : Test {}

//! This is the base paramData class. Inherit from this class to make custom param data containers.
//! Each container holds ONE set of parameters
//! The framework will create a test case instance for each container provided by SCR_AutotestSuiteBase::CreateParams method
class SCR_AutotestParamData 
{
	protected int m_iIdx;
	
	//------------------------------------------------------------------------------------------------
	[Friend(SCR_AutotestSuiteBase)]
	protected SCR_AutotestParamData WithIdx(int idx)
	{
		m_iIdx = idx;
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get identifier of the parameter that will be appended to the name of test case in report files.
	string GetIdentifier()
	{
		return m_iIdx.ToString();
	}
}
