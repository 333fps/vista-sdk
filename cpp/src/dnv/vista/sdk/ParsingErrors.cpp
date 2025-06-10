/**
 * @file ParsingErrors.cpp
 * @brief Implementation of ParsingErrors class
 */

#include "pch.h"

#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// ParsingErrors class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	ParsingErrors::ParsingErrors( const std::vector<ErrorEntry>& errors )
		: m_errors{ errors }
	{
	}

	ParsingErrors::ParsingErrors( std::vector<ErrorEntry>&& errors ) noexcept
		: m_errors{ std::move( errors ) }
	{
	}

	ParsingErrors::ParsingErrors()
		: m_errors{}
	{
	}

	ParsingErrors::ParsingErrors( ParsingErrors&& errors ) noexcept
		: m_errors{ std::move( errors.m_errors ) }
	{
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	bool ParsingErrors::operator==( const ParsingErrors& other ) const noexcept
	{
		if ( this == &other )
		{
			return true;
		}

		if ( m_errors.size() != other.m_errors.size() )
		{
			return false;
		}

		return m_errors == other.m_errors;
	}

	bool ParsingErrors::operator!=( const ParsingErrors& other ) const noexcept
	{
		return !( *this == other );
	}

	bool ParsingErrors::equals( const ParsingErrors& other ) const noexcept
	{
		return *this == other;
	}

	//----------------------------------------------
	// Public static members
	//----------------------------------------------

	const ParsingErrors& ParsingErrors::empty()
	{
		static const ParsingErrors instance{};

		return instance;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	size_t ParsingErrors::count() const noexcept
	{
		return m_errors.size();
	}

	size_t ParsingErrors::hashCode() const noexcept
	{
		size_t hash = 0;
		std::hash<std::string> stringHasher;

		for ( const auto& error : m_errors )
		{
			size_t typeHash = stringHasher( error.type );
			size_t messageHash = stringHasher( error.message );

			hash ^= typeHash + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
			hash ^= messageHash + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}

		return hash;
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	bool ParsingErrors::hasErrors() const noexcept
	{
		return !m_errors.empty();
	}

	bool ParsingErrors::hasErrorType( std::string_view type ) const noexcept
	{
		return std::any_of( m_errors.begin(), m_errors.end(),
			[type]( const ErrorEntry& error ) { return error.type == type; } );
	}

	//----------------------------------------------
	// String conversion methods
	//----------------------------------------------

	std::string ParsingErrors::toString() const
	{
		if ( m_errors.empty() )
		{
			return "Success";
		}

		constexpr std::string_view header = "Parsing errors:\n";

		/* Pre-calculate exact capacity */
		size_t capacity = header.size();
		for ( const auto& error : m_errors )
		{
			capacity += 1 + error.type.size() + 3 + error.message.size() + 1;
			/*          ↑                       ↑                          ↑  */
			/*        '\t'                    " - "                      '\n' */
		}

		std::string result;
		result.reserve( capacity );
		result = header;

		for ( const auto& error : m_errors )
		{
			result += '\t';
			result += error.type;
			result += " - ";
			result += error.message;
			result += '\n';
		}

		return result;
	}

	//----------------------------------------------
	// Enumeration
	//----------------------------------------------

	ParsingErrors::Enumerator ParsingErrors::enumerator() const
	{
		return Enumerator( &m_errors );
	}

	//----------------------------------------------
	// ParsingErrors::Enumerator class
	//----------------------------------------------

	//----------------------------
	// Construction / destruction
	//----------------------------

	ParsingErrors::Enumerator::Enumerator( const std::vector<ErrorEntry>* data )
		: m_data{ data },
		  m_index{ 0 }
	{
	}

	//----------------------------
	// Enumeration methods
	//----------------------------

	bool ParsingErrors::Enumerator::next() noexcept
	{
		if ( m_index < m_data->size() )
		{
			++m_index;

			return true;
		}

		return false;
	}

	const ParsingErrors::ErrorEntry& ParsingErrors::Enumerator::current() const
	{
		if ( m_index == 0 || m_index > m_data->size() )
		{
			throw std::out_of_range( "Enumerator not positioned on valid element" );
		}

		return ( *m_data )[m_index - 1];
	}

	void ParsingErrors::Enumerator::reset() noexcept
	{
		m_index = 0;
	}

	//----------------------------------------------
	// ParsingErrors::ErrorEntry struct
	//----------------------------------------------

	ParsingErrors::ErrorEntry::ErrorEntry( std::string_view type, std::string_view message )
		: type{ type }, message{ message }
	{
	}

	ParsingErrors::ErrorEntry::ErrorEntry( std::string&& type, std::string&& message )
		: type{ std::move( type ) },
		  message{ std::move( message ) }
	{
	}

	bool ParsingErrors::ErrorEntry::operator==( const ErrorEntry& other ) const noexcept
	{
		return type == other.type && message == other.message;
	}

	bool ParsingErrors::ErrorEntry::operator!=( const ErrorEntry& other ) const noexcept
	{
		return !( *this == other );
	}
}
