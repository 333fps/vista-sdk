/**
 * @file LocalIdItems.inl
 * @brief Inline implementations for performance-critical LocalIdItems operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocalIdItems class
	//=====================================================================

	//----------------------------------------------
	// Comparison operators
	//----------------------------------------------

	inline bool LocalIdItems::operator==( const LocalIdItems& other ) const noexcept
	{
		return m_primaryItem == other.m_primaryItem && m_secondaryItem == other.m_secondaryItem;
	}

	inline bool LocalIdItems::operator!=( const LocalIdItems& other ) const noexcept
	{
		return !( *this == other );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline const std::optional<GmodPath>& LocalIdItems::primaryItem() const noexcept
	{
		return m_primaryItem;
	}

	inline const std::optional<GmodPath>& LocalIdItems::secondaryItem() const noexcept
	{
		return m_secondaryItem;
	}

	inline bool LocalIdItems::isEmpty() const noexcept
	{
		return !m_primaryItem.has_value() && !m_secondaryItem.has_value();
	}
}
