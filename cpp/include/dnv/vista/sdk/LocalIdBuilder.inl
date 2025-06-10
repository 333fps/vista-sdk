/**
 * @file LocalIdBuilder.inl
 * @brief Inline implementations for performance-critical LocalIdBuilder operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocalIdBuilder class
	//=====================================================================

	//----------------------------------------------
	// Assignment operators
	//----------------------------------------------

	inline LocalIdBuilder& LocalIdBuilder::operator=( LocalIdBuilder&& other ) noexcept
	{
		if ( this != &other )
		{
			m_visVersion = std::move( other.m_visVersion );
			m_verboseMode = other.m_verboseMode;
			m_items = std::move( other.m_items );
			m_quantity = std::move( other.m_quantity );
			m_content = std::move( other.m_content );
			m_calculation = std::move( other.m_calculation );
			m_state = std::move( other.m_state );
			m_command = std::move( other.m_command );
			m_type = std::move( other.m_type );
			m_position = std::move( other.m_position );
			m_detail = std::move( other.m_detail );
		}
		return *this;
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	inline bool LocalIdBuilder::operator==( const LocalIdBuilder& other ) const
	{
		return equals( other );
	}

	inline bool LocalIdBuilder::operator!=( const LocalIdBuilder& other ) const
	{
		return !equals( other );
	}

	inline bool LocalIdBuilder::equals( const LocalIdBuilder& other ) const
	{
		if ( m_visVersion != other.m_visVersion )
		{
			throw std::invalid_argument( "Cant compare local IDs from different VIS versions" );
		}

		return m_items.primaryItem() == other.m_items.primaryItem() &&
			   m_items.secondaryItem() == other.m_items.secondaryItem() &&
			   m_quantity == other.m_quantity &&
			   m_calculation == other.m_calculation &&
			   m_content == other.m_content &&
			   m_position == other.m_position &&
			   m_state == other.m_state &&
			   m_command == other.m_command &&
			   m_type == other.m_type &&
			   m_detail == other.m_detail;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline std::optional<VisVersion> LocalIdBuilder::visVersion() const noexcept
	{
		return m_visVersion;
	}

	inline const std::optional<GmodPath>& LocalIdBuilder::primaryItem() const noexcept
	{
		return m_items.primaryItem();
	}

	inline const std::optional<GmodPath>& LocalIdBuilder::secondaryItem() const noexcept
	{
		return m_items.secondaryItem();
	}

	inline std::vector<MetadataTag> LocalIdBuilder::metadataTags() const noexcept
	{
		std::vector<MetadataTag> tags;
		tags.reserve( 8 );

		if ( m_quantity.has_value() )
		{
			tags.push_back( *m_quantity );
		}
		if ( m_calculation.has_value() )
		{
			tags.push_back( *m_calculation );
		}
		if ( m_content.has_value() )
		{
			tags.push_back( *m_content );
		}
		if ( m_position.has_value() )
		{
			tags.push_back( *m_position );
		}
		if ( m_state.has_value() )
		{
			tags.push_back( *m_state );
		}
		if ( m_command.has_value() )
		{
			tags.push_back( *m_command );
		}
		if ( m_type.has_value() )
		{
			tags.push_back( *m_type );
		}
		if ( m_detail.has_value() )
		{
			tags.push_back( *m_detail );
		}

		return tags;
	}

	inline size_t LocalIdBuilder::hashCode() const noexcept
	{
		size_t hash = 0;

		if ( m_items.primaryItem().has_value() )
		{
			hash ^= m_items.primaryItem()->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_items.secondaryItem().has_value() )
		{
			hash ^= m_items.secondaryItem()->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_quantity.has_value() )
		{
			hash ^= m_quantity->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_calculation.has_value() )
		{
			hash ^= m_calculation->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_content.has_value() )
		{
			hash ^= m_content->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_position.has_value() )
		{
			hash ^= m_position->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_state.has_value() )
		{
			hash ^= m_state->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}

		if ( m_command.has_value() )
		{
			hash ^= m_command->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_type.has_value() )
		{
			hash ^= m_type->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}
		if ( m_detail.has_value() )
		{
			hash ^= m_detail->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}

		return hash;
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	inline bool LocalIdBuilder::isValid() const noexcept
	{
		if ( !m_visVersion.has_value() )
		{
			return false;
		}

		if ( !m_items.primaryItem().has_value() )
		{
			return false;
		}

		if ( !m_quantity.has_value() &&
			 !m_calculation.has_value() &&
			 !m_content.has_value() &&
			 !m_position.has_value() &&
			 !m_state.has_value() &&
			 !m_command.has_value() &&
			 !m_type.has_value() &&
			 !m_detail.has_value() )
		{
			return false;
		}

		return true;
	}

	inline bool LocalIdBuilder::isEmpty() const noexcept
	{
		bool itemsEmpty = !m_items.primaryItem().has_value() &&
						  !m_items.secondaryItem().has_value();

		bool metadataEmpty = !m_quantity.has_value() &&
							 !m_content.has_value() &&
							 !m_calculation.has_value() &&
							 !m_state.has_value() &&
							 !m_command.has_value() &&
							 !m_type.has_value() &&
							 !m_position.has_value() &&
							 !m_detail.has_value();

		return itemsEmpty && metadataEmpty;
	}

	inline bool LocalIdBuilder::isVerboseMode() const noexcept
	{
		return m_verboseMode;
	}

	//----------------------------------------------
	// Metadata inspection methods
	//----------------------------------------------

	inline bool LocalIdBuilder::hasCustomTag() const noexcept
	{
		return ( m_quantity.has_value() && m_quantity->isCustom() ) ||
			   ( m_content.has_value() && m_content->isCustom() ) ||
			   ( m_calculation.has_value() && m_calculation->isCustom() ) ||
			   ( m_state.has_value() && m_state->isCustom() ) ||
			   ( m_command.has_value() && m_command->isCustom() ) ||
			   ( m_type.has_value() && m_type->isCustom() ) ||
			   ( m_position.has_value() && m_position->isCustom() ) ||
			   ( m_detail.has_value() && m_detail->isCustom() );
	}

	inline bool LocalIdBuilder::isEmptyMetadata() const noexcept
	{
		return !m_quantity.has_value() &&
			   !m_content.has_value() &&
			   !m_calculation.has_value() &&
			   !m_state.has_value() &&
			   !m_command.has_value() &&
			   !m_type.has_value() &&
			   !m_position.has_value() &&
			   !m_detail.has_value();
	}

	inline const LocalIdItems& LocalIdBuilder::items() const noexcept
	{
		return m_items;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::quantity() const noexcept
	{
		return m_quantity;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::content() const noexcept
	{
		return m_content;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::calculation() const noexcept
	{
		return m_calculation;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::state() const noexcept
	{
		return m_state;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::command() const noexcept
	{
		return m_command;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::type() const noexcept
	{
		return m_type;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::position() const noexcept
	{
		return m_position;
	}

	inline const std::optional<MetadataTag>& LocalIdBuilder::detail() const noexcept
	{
		return m_detail;
	}
}
