/**
 * @file Codebook.h
 * @brief Codebook components for validation and organization of maritime data
 *
 * This file defines codebook-related classes used throughout the VISTA SDK for
 * validating and organizing maritime vessel data according to standardized vocabularies.
 */

#pragma once

#include "CodebooksDto.h"

namespace dnv::vista::sdk
{
	class MetadataTag;
	enum class CodebookName;

	//-------------------------------------------------------------------
	// Validation Results
	//-------------------------------------------------------------------

	/**
	 * @brief Enumeration for validation results of position strings
	 */
	enum class PositionValidationResult
	{
		/** @brief Position is invalid */
		Invalid = 0,

		/** @brief Position components are in invalid order */
		InvalidOrder,

		/** @brief Position components have invalid grouping */
		InvalidGrouping,

		/** @brief Position is valid */
		Valid = 100,

		/** @brief Position is valid but custom */
		Custom = 101
	};

	/**
	 * @brief Utility class for working with position validation results
	 */
	class PositionValidationResults final
	{
	public:
		/**
		 * @brief Convert a string to a PositionValidationResult enum
		 * @param name The name of the validation result
		 * @return The corresponding PositionValidationResult
		 * @throws std::invalid_argument If the name is unknown
		 */
		static PositionValidationResult fromString( const std::string& name );
	};

	//-------------------------------------------------------------------
	// Standard Values Container
	//-------------------------------------------------------------------

	/**
	 * @brief Container for standard values of a codebook
	 */
	class CodebookStandardValues final
	{
	public:
		//=====================================================================
		// Types
		//=====================================================================

		/**
		 * @brief Iterator type for traversing standard values
		 */
		using iterator = std::unordered_set<std::string>::const_iterator;

		//=====================================================================
		// Construction / Destruction
		//=====================================================================

		/**
		 * @brief Construct with name and values
		 * @param name The codebook name
		 * @param standardValues The set of standard values
		 */
		explicit CodebookStandardValues( CodebookName name, const std::unordered_set<std::string>& standardValues );

		/** @brief Default constructor. */
		CodebookStandardValues() = default;

		/** @brief Copy constructor */
		CodebookStandardValues( const CodebookStandardValues& ) = default;

		/** @brief Move constructor */
		CodebookStandardValues( CodebookStandardValues&& ) noexcept = default;

		/** @brief Destructor */
		~CodebookStandardValues() = default;

		//=====================================================================
		// Special Member Functions
		//=====================================================================

		/** @brief Copy assignment operator */
		CodebookStandardValues& operator=( const CodebookStandardValues& ) = default;

		/** @brief Move assignment operator */
		CodebookStandardValues& operator=( CodebookStandardValues&& ) noexcept = default;

		//-------------------------------------------------------------------
		// Capacity
		//-------------------------------------------------------------------

		/**
		 * @brief Get the number of standard values
		 * @return The count of standard values
		 */
		[[nodiscard]] size_t count() const;

		//-------------------------------------------------------------------
		// Element Access
		//-------------------------------------------------------------------

		/**
		 * @brief Check if a value is contained in standard values
		 * @param tagValue The value to check
		 * @return True if the value is in standard values or is a numeric position
		 */
		bool contains( const std::string& tagValue ) const;

		//-------------------------------------------------------------------
		// Iterators
		//-------------------------------------------------------------------

		/**
		 * @brief Get iterator to the beginning
		 * @return Iterator to the first standard value
		 */
		[[nodiscard]] iterator begin() const;

		/**
		 * @brief Get iterator to the end
		 * @return Iterator past the last standard value
		 */
		[[nodiscard]] iterator end() const;

	private:
		//-------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------

		/** @brief The name of the codebook */
		CodebookName m_name;

		/** @brief The set of standard values */
		std::unordered_set<std::string> m_standardValues;
	};

	//-------------------------------------------------------------------
	// Groups Container
	//-------------------------------------------------------------------

	/**
	 * @brief Container for groups of a codebook
	 */
	class CodebookGroups final
	{
	public:
		//-------------------------------------------------------------------
		// Types
		//-------------------------------------------------------------------

		/**
		 * @brief Iterator type for traversing groups
		 */
		using Iterator = std::unordered_set<std::string>::const_iterator;

		//=====================================================================
		// Construction / Destruction
		//=====================================================================

		/**
		 * @brief Construct with groups
		 * @param groups The set of groups
		 */
		explicit CodebookGroups( const std::unordered_set<std::string>& groups );

		/** @brief Default constructor. */
		CodebookGroups() = default;

		/** @brief Copy constructor */
		CodebookGroups( const CodebookGroups& ) = default;

		/** @brief Move constructor */
		CodebookGroups( CodebookGroups&& ) noexcept = default;

		/** @brief Destructor */
		~CodebookGroups() = default;

		//=====================================================================
		// Special Member Functions
		//=====================================================================

		/** @brief Copy assignment operator */
		CodebookGroups& operator=( const CodebookGroups& ) = default;

		/** @brief Move assignment operator */
		CodebookGroups& operator=( CodebookGroups&& ) noexcept = default;

		//-------------------------------------------------------------------
		// Capacity
		//-------------------------------------------------------------------

		/**
		 * @brief Get the number of groups
		 * @return The count of groups
		 */
		[[nodiscard]] size_t count() const;

		//-------------------------------------------------------------------
		// Element Access
		//-------------------------------------------------------------------

		/**
		 * @brief Check if a group is contained
		 * @param group The group to check
		 * @return True if the group exists
		 */
		bool contains( const std::string& group ) const;

		//-------------------------------------------------------------------
		// Iterators
		//-------------------------------------------------------------------

		/**
		 * @brief Get iterator to the beginning
		 * @return Iterator to the first group
		 */
		[[nodiscard]] Iterator begin() const;

		/**
		 * @brief Get iterator to the end
		 * @return Iterator past the last group
		 */
		[[nodiscard]] Iterator end() const;

	private:
		//-------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------

		/** @brief The set of groups */
		std::unordered_set<std::string> m_groups;
	};

	//-------------------------------------------------------------------
	// Main Codebook Class
	//-------------------------------------------------------------------

	/**
	 * @brief A codebook containing standard values and their groups
	 */
	class Codebook final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		Codebook() = default;

		/**
		 * @brief Construct from DTO
		 * @param dto The data transfer object
		 * @throws std::invalid_argument If the DTO has an unknown name
		 */
		Codebook( const CodebookDto& dto );

		//-------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the codebook name
		 * @return The codebook name
		 */
		[[nodiscard]] CodebookName name() const;

		/**
		 * @brief Get the groups
		 * @return Reference to the groups container
		 */
		[[nodiscard]] const CodebookGroups& groups() const;

		/**
		 * @brief Get the standard values
		 * @return Reference to the standard values container
		 */
		[[nodiscard]] const CodebookStandardValues& standardValues() const;

		/**
		 * @brief Get the raw data
		 * @return Map of groups to their values
		 */
		[[nodiscard]] const std::unordered_map<std::string, std::vector<std::string>>& rawData() const;

		//-------------------------------------------------------------------
		// Queries
		//-------------------------------------------------------------------

		/**
		 * @brief Check if a group exists
		 * @param group The group to check
		 * @return True if the group exists
		 */
		[[nodiscard]] bool hasGroup( const std::string& group ) const;

		/**
		 * @brief Check if a value is a standard value
		 * @param value The value to check
		 * @return True if the value is standard
		 */
		[[nodiscard]] bool hasStandardValue( const std::string& value ) const;

		//-------------------------------------------------------------------
		// Operations
		//-------------------------------------------------------------------

		/**
		 * @brief Try to create a metadata tag
		 * @param value The tag value
		 * @return The metadata tag, or none if invalid
		 */
		[[nodiscard]] std::optional<MetadataTag> tryCreateTag( const std::string_view valueView ) const;

		/**
		 * @brief Create a metadata tag
		 * @param value The tag value
		 * @return The metadata tag
		 * @throws std::invalid_argument If the value is invalid
		 */
		MetadataTag createTag( const std::string& value ) const;

		/**
		 * @brief Validate a position string
		 * @param position The position to validate
		 * @return The validation result
		 */
		PositionValidationResult validatePosition( const std::string& position ) const;

	private:
		//-------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------

		/** @brief The name of this codebook */
		CodebookName m_name;

		/** @brief Mapping from values to their group names */
		std::unordered_map<std::string, std::string> m_groupMap;

		/** @brief Container for standard values */
		CodebookStandardValues m_standardValues;

		/** @brief Container for groups */
		CodebookGroups m_groups;

		/** @brief Raw mapping of groups to their values */
		std::unordered_map<std::string, std::vector<std::string>> m_rawData;
	};
}
