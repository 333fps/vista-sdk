/**
 * @file LocalId.inl
 * @brief Inline implementations for performance-critical LocalId operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocalId class
	//=====================================================================

	//----------------------------------------------
	// Lookup operators
	//----------------------------------------------

	inline bool LocalId::equals( const LocalId& other ) const noexcept
	{
		return m_builder == other.m_builder;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline const LocalIdBuilder& LocalId::builder() const noexcept
	{
		return m_builder;
	}

	inline VisVersion LocalId::visVersion() const noexcept
	{
		return m_builder.visVersion().value();
	}

	inline const std::optional<GmodPath>& LocalId::primaryItem() const noexcept
	{
		return m_builder.primaryItem();
	}

	inline const std::optional<GmodPath>& LocalId::secondaryItem() const noexcept
	{
		return m_builder.secondaryItem();
	}

	inline std::vector<MetadataTag> LocalId::metadataTags() const noexcept
	{
		return m_builder.metadataTags();
	}

	inline size_t LocalId::hashCode() const noexcept
	{
		return m_builder.hashCode();
	}

	//----------------------------------------------
	// Metadata accessors
	//----------------------------------------------

	inline const std::optional<MetadataTag>& LocalId::quantity() const noexcept
	{
		return m_builder.quantity();
	}

	inline const std::optional<MetadataTag>& LocalId::content() const noexcept
	{
		return m_builder.content();
	}

	inline const std::optional<MetadataTag>& LocalId::calculation() const noexcept
	{
		return m_builder.calculation();
	}

	inline const std::optional<MetadataTag>& LocalId::state() const noexcept
	{
		return m_builder.state();
	}

	inline const std::optional<MetadataTag>& LocalId::command() const noexcept
	{
		return m_builder.command();
	}

	inline const std::optional<MetadataTag>& LocalId::type() const noexcept
	{
		return m_builder.type();
	}

	inline const std::optional<MetadataTag>& LocalId::position() const noexcept
	{
		return m_builder.position();
	}

	inline const std::optional<MetadataTag>& LocalId::detail() const noexcept
	{
		return m_builder.detail();
	}

	//----------------------------------------------
	// State inspection
	//----------------------------------------------

	inline bool LocalId::isVerboseMode() const noexcept
	{
		return m_builder.isVerboseMode();
	}

	inline bool LocalId::hasCustomTag() const noexcept
	{
		return m_builder.hasCustomTag();
	}

	//----------------------------------------------
	// String conversion
	//----------------------------------------------

	inline std::string LocalId::toString() const
	{
		return m_builder.toString();
	}

}
