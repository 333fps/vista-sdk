#include "pch.h"

#include "dnv/vista/sdk/MetadataTag.h"

namespace dnv::vista::sdk
{
	MetadataTag::MetadataTag( CodebookName name, const std::string& value, bool isCustom )
		: m_name{ name }, m_value{ value }, m_custom{ isCustom }
	{
		SPDLOG_INFO( "Created MetadataTag: name={}, value='{}', custom={}",
			static_cast<int>( name ), value, isCustom );
	}

	CodebookName MetadataTag::name() const
	{
		return m_name;
	}

	const std::string& MetadataTag::value() const
	{
		return m_value;
	}

	bool MetadataTag::isCustom() const
	{
		return m_custom;
	}

	char MetadataTag::prefix() const
	{
		return m_custom ? '~' : '-';
	}

	MetadataTag::operator std::string() const
	{
		return m_value;
	}

	bool MetadataTag::operator==( const MetadataTag& other ) const
	{
		if ( m_name != other.m_name )
		{
			SPDLOG_ERROR( "Can't compare tags with different names: {} vs {}",
				static_cast<int>( m_name ), static_cast<int>( other.m_name ) );
			throw std::invalid_argument( "Can't compare tags with different names: " +
										 toString() + " vs " + other.toString() );
		}

		return m_value == other.m_value;
	}

	bool MetadataTag::operator!=( const MetadataTag& other ) const
	{
		return !( *this == other );
	}

	size_t MetadataTag::hashCode() const
	{
		return std::hash<std::string>{}( m_value );
	}

	std::string MetadataTag::toString() const
	{
		return m_value;
	}

	void MetadataTag::toString( std::ostringstream& builder, char separator ) const
	{
		SPDLOG_INFO( "Formatting tag: name={}, value='{}', custom={}, separator='{}'",
			static_cast<int>( m_name ), m_value, m_custom, separator );

		std::string prefix;

		switch ( m_name )
		{
			case CodebookName::Position:
				prefix = "pos";
				break;
			case CodebookName::Quantity:
				prefix = "qty";
				break;
			case CodebookName::Calculation:
				prefix = "calc";
				break;
			case CodebookName::State:
				prefix = "state";
				break;
			case CodebookName::Content:
				prefix = "cnt";
				break;
			case CodebookName::Command:
				prefix = "cmd";
				break;
			case CodebookName::Type:
				prefix = "type";
				break;
			case CodebookName::FunctionalServices:
				prefix = "funct.svc";
				break;
			case CodebookName::MaintenanceCategory:
				prefix = "maint.cat";
				break;
			case CodebookName::ActivityType:
				prefix = "act.type";
				break;
			case CodebookName::Detail:
				prefix = "detail";
				break;
			default:
			{
				SPDLOG_ERROR( "Unknown metadata tag: {}", static_cast<int>( m_name ) );
				throw std::invalid_argument( "Unknown metadata tag: " +
											 std::to_string( static_cast<int>( m_name ) ) );
			}
		}

		builder << prefix;
		builder << ( isCustom() ? '~' : '-' );
		builder << m_value;
		builder << separator;
	}
}
