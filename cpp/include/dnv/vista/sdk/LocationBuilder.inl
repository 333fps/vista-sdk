/**
 * @file LocationBuilder.inl
 * @brief Inline implementations for performance-critical LocationBuilder operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocationBuilder class
	//=====================================================================

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline VisVersion LocationBuilder::visVersion() const noexcept
	{
		return m_visVersion;
	}

	inline const std::optional<int>& LocationBuilder::number() const noexcept
	{
		return m_number;
	}

	inline const std::optional<char>& LocationBuilder::side() const noexcept
	{
		return m_side;
	}

	inline const std::optional<char>& LocationBuilder::vertical() const noexcept
	{
		return m_vertical;
	}

	inline const std::optional<char>& LocationBuilder::transverse() const noexcept
	{
		return m_transverse;
	}

	inline const std::optional<char>& LocationBuilder::longitudinal() const noexcept
	{
		return m_longitudinal;
	}

}
