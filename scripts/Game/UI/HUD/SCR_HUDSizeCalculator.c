enum SCR_EHUDManagerResizeType
{
	HEIGHT,
	WIDTH
};

class SCR_HUDSizeCalculator
{
	//------------------------------------------------------------------------------------------------
	/*!
	Resizes the provided slots based on the provided Group Height and Group Width.
	\param slots array of slots to Resize.
	\param groupHeight Height of the Group to utilize for resizing.
	\param groupWidth Width of the Group to utilize for resizing.
	*/
	void ResizeGroup(notnull array<SCR_HUDSlotUIComponent> slots, int groupHeight, int groupWidth)
	{
		array<SCR_HUDSlotUIComponent> remainingSlots = {};
		foreach (SCR_HUDSlotUIComponent slot : slots)
		{
			// Resetting the Height and Width of the Slot before attempting to resize.
			slot.SetHeight(0, false);
			slot.SetWidth(0, false);

			// If the Slot isn't visible or doesn't have any content, the Resizing algorithm shouldn't take it into calculation.
			if (slot.GetRootWidget().IsVisible() && slot.GetContentWidget() && slot.GetContentWidget().IsVisible())
				remainingSlots.Insert(slot);
		}

		// Sorting the remainingSlots array by priority using a bubble sort algorithm.
		SortSlotsByPriority(remainingSlots);

		// Looping until all slots have been assigned a height and a width.
		while (remainingSlots.Count() > 0)
		{
			// Getting the first Slot from the sorted array (highest priority)
			SCR_HUDSlotUIComponent currentSlot = remainingSlots[0];

			if (!currentSlot.m_aHeightSteps.IsEmpty())
				Resize(slots, remainingSlots, groupHeight, SCR_EHUDManagerResizeType.HEIGHT);
			if (!currentSlot.m_aWidthSteps.IsEmpty())
				Resize(slots, remainingSlots, groupWidth, SCR_EHUDManagerResizeType.WIDTH);

			// Removing the first Slot from the array after being done with the calculation.
			remainingSlots.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	private void Resize(notnull array<SCR_HUDSlotUIComponent> slots, notnull array<SCR_HUDSlotUIComponent> remainingSlots, int totalSize, SCR_EHUDManagerResizeType resizeType)
	{
		int remainingGroupSize;
		int maxAvailableSizeSum;
		array<int> sizeSteps = null;
		

		// Get the first Slot from the sorted array (highest priority)
		SCR_HUDSlotUIComponent currentSlot = remainingSlots[0];

		if (resizeType == SCR_EHUDManagerResizeType.HEIGHT)
		{
			// Calculating the remaining Group Height by subtracting the Total Assigned Height from the Group Height
			remainingGroupSize = totalSize - GetTotalAssignedHeight(slots);
			// Calculating the sum of the Maximum Available Heights of the remaining Slots.
			maxAvailableSizeSum = GetMaxAvailableHeightSizeSum(remainingSlots);
			sizeSteps = currentSlot.m_aHeightSteps;
		}
		else
		{
			// Calculating the remaining Group Width by subtracting the Total Assigned Width from the Group Width
			remainingGroupSize = totalSize - GetTotalAssignedWidth(slots);
			// Calculating the sum of the Maximum Available Widths of the remaining Slots.
			maxAvailableSizeSum = GetMaxAvailableWidthSizeSum(remainingSlots);
			sizeSteps = currentSlot.m_aWidthSteps;
		}

		// If the sum of all Maximum Available Sizes of remaining Slots is greater than the Remaining Group Size
		// This means that, if all Slots uses their Max Size than we will go over the Group Size Limit.
		if (maxAvailableSizeSum > remainingGroupSize)
		{
			// Iterating through each Size Step of a slot, from high to small. (This is possible becasue Size Step Arrays are sorted.)
			for (int i = sizeSteps.Count() - 1; i >= 0; i--)
			{
				int stepToUse = sizeSteps[i];

				// Calculating the difference between the current step (stepToUse) and the Highest Step.
				int differenceBetweenHighest = sizeSteps[sizeSteps.Count() - 1] - sizeSteps[i];
				// Subtracting the previously calculated difference from the sum of all the Highest Size Step of all the remaining Slots.
				int modifiedMaxAvailableSizeSum = maxAvailableSizeSum - differenceBetweenHighest;

				// Checking if all the slots can fit into the Remaining Size in the case of using the currently iterated Height Step instead of the Highest.
				if (modifiedMaxAvailableSizeSum <= remainingGroupSize)
				{
					if (resizeType == SCR_EHUDManagerResizeType.HEIGHT)
						currentSlot.SetHeight(stepToUse);
					else
						currentSlot.SetWidth(stepToUse);

					return;
				}
			}
			// If we failed to find a suitable Size Step so far, algorithm falls back to the smallest Size Step possible.
			if (resizeType == SCR_EHUDManagerResizeType.HEIGHT)
				currentSlot.SetHeight(sizeSteps[0]);
			else
				currentSlot.SetWidth(sizeSteps[0]);
		}
		else
		{
			/*
				Assigning the new Size to the minimum between Remaining Group Size or Max Size Step of the Slot.
				We don't want the Slot to be larger than it's Maximum Size Step and we also don't want it to occupy more space than the Remaining Size.
			*/
			if (resizeType == SCR_EHUDManagerResizeType.HEIGHT)
			{
				int distributedSize = Math.Min(remainingGroupSize, GetMaxAvailableHeight(currentSlot));
				currentSlot.SetHeight(distributedSize);
			}
			else
			{
				int distributedSize = Math.Min(remainingGroupSize, GetMaxAvailableWidth(currentSlot));
				currentSlot.SetWidth(distributedSize);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculate the Total Assigned Height of the Slots in the Group
	*/
	private int GetTotalAssignedHeight(notnull array<SCR_HUDSlotUIComponent> slots)
	{
		int totalHeight;
		foreach (SCR_HUDSlotUIComponent slot : slots)
		{
			if (slot.GetRootWidget().IsVisible())
			{
				totalHeight += slot.GetHeight();
			}
		}

		return totalHeight;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculate the Total Assigned Width of the Slots in the Group
	*/
	private int GetTotalAssignedWidth(notnull array<SCR_HUDSlotUIComponent> slots)
	{
		int totalWidth;
		foreach (SCR_HUDSlotUIComponent slot : slots)
		{
			if (slot.GetRootWidget().IsVisible())
			{
				totalWidth += slot.GetWidth();
			}
		}

		return totalWidth;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Sort the Slots array by Priority using the bubble sort algorithm
	*/
	private void SortSlotsByPriority(notnull array<SCR_HUDSlotUIComponent> slots)
	{
		for (int i = 0; i < slots.Count() - 1; i++)
		{
			for (int j = 0; j < slots.Count() - 1 - i; j++)
			{
				if (slots[j].GetPriority() > slots[j + 1].GetPriority())
				{
					SCR_HUDSlotUIComponent temp = slots[j];
					slots[j] = slots[j + 1];
					slots[j + 1] = temp;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculate the sum of the Maximum Available Heights of the Remaining Slots
	*/
	private int GetMaxAvailableHeightSizeSum(notnull array<SCR_HUDSlotUIComponent> remainingSlots)
	{
		int sum;
		foreach (SCR_HUDSlotUIComponent slot : remainingSlots)
		{
			if (slot.GetRootWidget().IsVisible())
				sum += GetMaxAvailableHeight(slot);
		}

		return sum;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculate the sum of the Maximum Available Widths of the Remaining Slots
	*/
	private int GetMaxAvailableWidthSizeSum(notnull array<SCR_HUDSlotUIComponent> remainingSlots)
	{
		int sum;
		foreach (SCR_HUDSlotUIComponent slot : remainingSlots)
		{
			if (slot.GetRootWidget().IsVisible())
				sum += GetMaxAvailableWidth(slot);
		}

		return sum;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the Highest Height Step of a Slot.
	*/
	private int GetMaxAvailableHeight(notnull SCR_HUDSlotUIComponent slot)
	{
		// If the Slot isn't visible, it shouldn't be part of the calculation.
		if (!slot.GetRootWidget().IsVisible())
			return 0;
		// If the Slot is marked as Size To Content, it's size will always be the same as it's Content.
		else if (slot.m_bSizeToContent)
			return GetContentHeight(slot);

		array<int> heightSteps = slot.m_aHeightSteps;
		// As the Size Steps are sorted on ascending order, the Last element is the Heighest one.
		return heightSteps[heightSteps.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the Highest Width Step of a Slot.
	*/
	private int GetMaxAvailableWidth(notnull SCR_HUDSlotUIComponent slot)
	{
		// If the Slot isn't visible, it shouldn't be part of the calculation.
		if (!slot.GetRootWidget().IsVisible())
			return 0;
		// If the Slot is marked as Size To Content, it's size will always be the same as it's Content.
		else if (slot.m_bSizeToContent)
			return GetContentWidth(slot);

		array<int> widthSteps = slot.m_aWidthSteps;
		// As the Size Steps are sorted on ascending order, the Last element is the Heighest one.
		return widthSteps[widthSteps.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the Smallest Height Step of a Slot.
	*/
	private int GetMinAvailableHeight(notnull SCR_HUDSlotUIComponent slot)
	{
		// If the Slot isn't visible, it shouldn't be part of the calculation.
		if (!slot.GetRootWidget().IsVisible())
			return 0;
		// If the Slot is marked as Size To Content, it's size will always be the same as it's Content.
		else if (slot.m_bSizeToContent)
			return GetContentHeight(slot);

		// As the Size Steps are sorted on ascending order, the first element is the smallest one.
		return slot.m_aHeightSteps[0];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the Smallest Width Step of a Slot.
	*/
	private int GetMinAvailableWidth(notnull SCR_HUDSlotUIComponent slot)
	{
		// If the Slot isn't visible, it shouldn't be part of the calculation.
		if (!slot.GetRootWidget().IsVisible())
			return 0;
		// If the Slot is marked as Size To Content, it's size will always be the same as it's Content.
		else if (slot.m_bSizeToContent)
			return GetContentWidth(slot);

		// As the Size Steps are sorted on ascending order, the first element is the smallest one.
		return slot.m_aWidthSteps[0];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the Height of the Content this Slot owns.
	*/
	private int GetContentHeight(notnull SCR_HUDSlotUIComponent slot)
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
		{
			array<int> heightSteps = slot.m_aHeightSteps;
			if (!heightSteps || heightSteps.IsEmpty())
				return 0;

			return heightSteps[heightSteps.Count() - 1];
		}
#endif
		Widget child = slot.GetContentWidget();
		if (!child || !slot.GetRootWidget().IsVisible())
			return 0;

		float childWidth, childHeight;
		child.GetScreenSize(childWidth, childHeight);
		childHeight = GetGame().GetWorkspace().DPIUnscale(childHeight);

		return childHeight;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the Width of the Content this Slot owns.
	*/
	private int GetContentWidth(notnull SCR_HUDSlotUIComponent slot)
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode() && slot.m_bSizeToContent)
		{
			array<int> widthSteps = slot.m_aWidthSteps;
			if (!widthSteps || widthSteps.IsEmpty())
				return 0;

			return widthSteps[widthSteps.Count() - 1];
		}
#endif
		Widget child = slot.GetRootWidget().GetChildren();
		if (!child || !slot.GetRootWidget().IsVisible())
			return 0;

		float childWidth, childHeight;
		child.GetScreenSize(childWidth, childHeight);
		childWidth = GetGame().GetWorkspace().DPIUnscale(childWidth);

		return childWidth;
	}
};
