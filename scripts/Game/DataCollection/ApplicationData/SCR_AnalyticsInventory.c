class SCR_AnalyticsInventory
{
	protected ref SCR_InventoryData m_Inventory = new SCR_InventoryData();
	protected ref SCR_AnalyticsDelayedSend m_DelayedSend = new SCR_AnalyticsDelayedSend();
	protected ref array<float> m_aOpenWeights = {};
	protected ref array<float> m_aCloseWeights = {};
	
	//------------------------------------------------------------------------------------------------
	//! User opens the inventory and it contains a specific weight. Save it for the analytics.
	//! \param[in] currentWeight
	void OpenInventory(float currentWeight)
	{
		m_DelayedSend.Start();
		
		if (!m_DelayedSend.IsAdditive())
		{
			m_Inventory = new SCR_InventoryData();
			m_aOpenWeights.Clear();
			m_aCloseWeights.Clear();
		}
		
		m_aOpenWeights.Insert(currentWeight);
	}

	//------------------------------------------------------------------------------------------------
	//! User closes the inventory and it contains a specific weight. Save it for the analytics.
	//! \param[in] currentWeight
	void CloseInventory(float currentWeight)
	{
		m_DelayedSend.Stop();

		if (m_DelayedSend.IsAdditive())
			m_Inventory.amt_inv_time_spent += m_DelayedSend.GetTimeSpent();
		else
			m_Inventory.amt_inv_time_spent = m_DelayedSend.GetTimeSpent();

		m_aCloseWeights.Insert(currentWeight);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update time spent in inventory and outside it. We need to wait until we send the inventoryClosed
	//! event to prevent multiple sends in rapid succession.
	//! \param[in] deltaSeconds
	void Update(float deltaSeconds)
	{
		m_DelayedSend.Update(deltaSeconds);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if we are ready to send our saved data to analytics.
	//! \return bool
	bool IsReadyToSend()
	{
		return m_DelayedSend.IsReadyToSend();
	}
	
	//------------------------------------------------------------------------------------------------
	//! We have sended the data, so we reset.
	void ResetReadyToSend()
	{
		m_DelayedSend.ResetReadyToSend();
	}
	
	//------------------------------------------------------------------------------------------------
	//! We have interacted with an item of particular slot type.
	//! \param[in] itemSlotType
	void InteractWithItem(SCR_EAnalyticalItemSlotType itemSlotType)
	{
		switch (itemSlotType)
		{
			case SCR_EAnalyticalItemSlotType.HORIZONTAL:
				m_Inventory.cnt_inv_interactions_horizontal += 1;
				break;

			case SCR_EAnalyticalItemSlotType.VERTICAL:
				m_Inventory.cnt_inv_interactions_vertical += 1;
				break;

			case SCR_EAnalyticalItemSlotType.CHARACTER_STORAGE:
				m_Inventory.cnt_inv_interactions_character_storage += 1;
				break;

			case SCR_EAnalyticalItemSlotType.VICINITY:
				m_Inventory.cnt_inv_interactions_vicinity += 1;
				break;
			
			case SCR_EAnalyticalItemSlotType.OTHER:
				// we don't track other types
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! We used healing (bandages) through the inventory. Save it for analytics.
	void UseHealing()
	{
		m_Inventory.cnt_inv_healing += 1;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! We used examination through the inventory. Save it for analytics.
	void UseExamination()
	{
		m_Inventory.cnt_inv_examination += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return currently saved data as a copy
	SCR_InventoryData GetClonedData()
	{
		SCR_InventoryData data = SCR_InventoryData.Cast(m_Inventory.Clone());
		data.array_inv_weight_on_open = ArrayToString(m_aOpenWeights);
		data.array_inv_weight_on_close = ArrayToString(m_aCloseWeights);
		return data;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return currently saved data as a copy
	protected string ArrayToString(array<float> arr)
	{
		string result = "[";
		bool firstElement = true;
		foreach (float val : arr)
		{
			if (firstElement)
				result += val.ToString(lenDec: 3);
			else
				result += "," + val.ToString(lenDec: 3);

			firstElement = false;
		}
		return result + "]";
	}
}