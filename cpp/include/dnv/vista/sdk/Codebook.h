/**
 * @file Codebook.h
 * @brief Defines classes for managing and validating data against VISTA codebooks.
 *
 * @details This file provides the core components for interacting with VISTA codebooks,
 * including enumerations for validation results, containers for standard values and groups,
 * and the main `Codebook` class for accessing and validating maritime data according
 * to standardized vocabularies.
 */

#pragma once

#include "CodebooksDto.h"
#include "CodebookName.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class MetadataTag;

	//=====================================================================
	// Heterogeneous lookup
	//=====================================================================

	struct StringHash
	{
		using is_transparent = void;

		[[nodiscard]] size_t operator()( std::string_view sv ) const noexcept;
		[[nodiscard]] size_t operator()( const std::string& s ) const noexcept;
	};

	struct StringEqual
	{
		using is_transparent = void;

		[[nodiscard]] bool operator()( const std::string& lhs, const std::string& rhs ) const noexcept;
		[[nodiscard]] bool operator()( const std::string& lhs, std::string_view rhs ) const noexcept;
		[[nodiscard]] bool operator()( std::string_view lhs, const std::string& rhs ) const noexcept;
		[[nodiscard]] bool operator()( std::string_view lhs, std::string_view rhs ) const noexcept;
	};

	//=====================================================================
	// PositionValidationResults class
	//=====================================================================

	/**
	 * @enum PositionValidationResult
	 * @brief Enumerates the possible outcomes of validating a position string against a codebook.
	 */
	enum class PositionValidationResult
	{
		Invalid = 0,
		InvalidOrder,
		InvalidGrouping,

		Valid = 100,

		Custom = 101
	};

	/**
	 * @class PositionValidationResults
	 * @brief Provides utility functions related to `PositionValidationResult`.
	 * @details This class currently offers conversion from string representations.
	 */
	class PositionValidationResults final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/** @brief Default constructor. */
		PositionValidationResults() = delete;

		/** @brief Copy constructor */
		PositionValidationResults( const PositionValidationResults& ) = delete;

		/** @brief Move constructor */
		PositionValidationResults( PositionValidationResults&& ) noexcept = delete;

		/** @brief Destructor */
		~PositionValidationResults() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		PositionValidationResults& operator=( const PositionValidationResults& ) = delete;

		/** @brief Move assignment operator */
		PositionValidationResults& operator=( PositionValidationResults&& ) noexcept = delete;

		//----------------------------------------------
		// Static utility methods
		//----------------------------------------------

		/**
		 * @brief Converts a string representation to a PositionValidationResult.
		 * @param[in] name The string name of the validation result (case-insensitive).
		 * @return The corresponding PositionValidationResult enum value.
		 * @throws std::invalid_argument If the name doesn't match any known validation result.
		 *
		 * @details Supported string values:
		 * - "Invalid" -> PositionValidationResult::Invalid
		 * - "InvalidOrder" -> PositionValidationResult::InvalidOrder
		 * - "InvalidGrouping" -> PositionValidationResult::InvalidGrouping
		 * - "Valid" -> PositionValidationResult::Valid
		 * - "Custom" -> PositionValidationResult::Custom
		 */
		[[nodiscard]] static PositionValidationResult fromString( std::string_view name );
	};

	//=====================================================================
	// CodebookStandardValues class
	//=====================================================================

	/**
	 * @brief Container for standard values of a codebook
	 */
	class CodebookStandardValues final
	{
	public:
		//----------------------------------------------
		// Public Types
		//----------------------------------------------

		/**
		 * @brief Iterator type for traversing standard values
		 */
		using Iterator = std::unordered_set<std::string>::const_iterator;

		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Construct with name and values
		 * @param name The codebook name
		 * @param standardValues The set of standard values
		 */
		explicit CodebookStandardValues( CodebookName name, std::unordered_set<std::string, StringHash, StringEqual>&& standardValues );

		/** @brief Default constructor. */
		CodebookStandardValues() = default;

		/** @brief Copy constructor */
		CodebookStandardValues( const CodebookStandardValues& ) = default;

		/** @brief Move constructor */
		CodebookStandardValues( CodebookStandardValues&& ) noexcept = default;

		/** @brief Destructor */
		~CodebookStandardValues() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		CodebookStandardValues& operator=( const CodebookStandardValues& ) = default;

		/** @brief Move assignment operator */
		CodebookStandardValues& operator=( CodebookStandardValues&& ) noexcept = default;

		//----------------------------------------------
		// Public methods
		//----------------------------------------------

		/**
		 * @brief Get the number of standard values
		 * @return The count of standard values
		 */
		[[nodiscard]] inline size_t count() const noexcept;

		/**
		 * @brief Check if a value is contained in standard values (string_view overload)
		 * @param tagValue The value to check
		 * @return True if the value is in standard values or is a numeric position
		 */
		bool contains( std::string_view tagValue ) const noexcept;

		//----------------------------------------------
		// Iteration
		//----------------------------------------------

		/**
		 * @brief Get iterator to the beginning
		 * @return Iterator to the first standard value
		 */
		[[nodiscard]] inline Iterator begin() const noexcept;

		/**
		 * @brief Get iterator to the end
		 * @return Iterator past the last standard value
		 */
		[[nodiscard]] inline Iterator end() const noexcept;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The name of the codebook */
		CodebookName m_name;

		/** @brief The set of standard values */
		std::unordered_set<std::string, StringHash, StringEqual> m_standardValues;
	};

	//=====================================================================
	// CodebookGroups class
	//=====================================================================

	/**
	 * @class CodebookGroups
	 * @brief A container managing the set of group names defined within a codebook.
	 * @details Provides efficient lookup (`contains`) and iteration over the group names.
	 */
	class CodebookGroups final
	{
	public:
		//----------------------------------------------
		// Public Types
		//----------------------------------------------

		/**
		 * @brief Iterator type for traversing groups
		 */
		using Iterator = std::unordered_set<std::string>::const_iterator;

		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Construct with groups
		 * @param groups The set of groups
		 */
		explicit CodebookGroups( std::unordered_set<std::string, StringHash, StringEqual>&& groups );

		/** @brief Default constructor. */
		CodebookGroups() = default;

		/** @brief Copy constructor */
		CodebookGroups( const CodebookGroups& ) = default;

		/** @brief Move constructor */
		CodebookGroups( CodebookGroups&& ) noexcept = default;

		/** @brief Destructor */
		~CodebookGroups() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		CodebookGroups& operator=( const CodebookGroups& ) = default;

		/** @brief Move assignment operator */
		CodebookGroups& operator=( CodebookGroups&& ) noexcept = default;

		//----------------------------------------------
		// Public methods
		//----------------------------------------------

		/**
		 * @brief Get the number of groups
		 * @return The count of groups
		 */
		[[nodiscard]] inline size_t count() const noexcept;

		/**
		 * @brief Check if a group is contained (string_view overload)
		 * @param group The group to check (as a `std::string_view`)
		 * @return True if the group exists
		 */
		inline bool contains( std::string_view group ) const noexcept;

		//----------------------------------------------
		// Iteration
		//----------------------------------------------

		/**
		 * @brief Get iterator to the beginning
		 * @return Iterator to the first group
		 */
		[[nodiscard]] inline Iterator begin() const noexcept;

		/**
		 * @brief Get iterator to the end
		 * @return Iterator past the last group
		 */
		[[nodiscard]] inline Iterator end() const noexcept;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The set of groups */
		std::unordered_set<std::string, StringHash, StringEqual> m_groups;
	};

	//=====================================================================
	// Codebook class
	//=====================================================================

	/**
	 * @class Codebook
	 * @brief Represents a complete VISTA codebook, containing standard values, groups, and validation logic.
	 * @details This class aggregates standard values and group information, providing methods
	 * to validate data, check for the existence of values/groups, and create associated `MetadataTag` objects.
	 * It is typically constructed from a `CodebookDto` object.
	 */
	class Codebook final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Construct from DTO
		 * @param dto The data transfer object
		 * @throws std::invalid_argument If the DTO has an unknown name
		 */
		explicit Codebook( const CodebookDto& dto );

		/** @brief Default constructor. */
		Codebook() = default;

		/** @brief Copy constructor */
		Codebook( const Codebook& ) = default;

		/** @brief Move constructor */
		Codebook( Codebook&& ) noexcept = default;

		/** @brief Destructor */
		~Codebook() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		Codebook& operator=( const Codebook& ) = default;

		/** @brief Move assignment operator */
		Codebook& operator=( Codebook&& ) noexcept = default;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the codebook name
		 * @return The codebook name
		 */
		[[nodiscard]] inline CodebookName name() const noexcept
		{
			return m_name;
		}

		/**
		 * @brief Get the groups
		 * @return Reference to the groups container
		 */
		[[nodiscard]] inline const CodebookGroups& groups() const noexcept;

		/**
		 * @brief Get the standard values
		 * @return Reference to the standard values container
		 */
		[[nodiscard]] inline const CodebookStandardValues& standardValues() const noexcept;

		/**
		 * @brief Get the raw data
		 * @return Map of groups to their values
		 */
		[[nodiscard]] inline const std::unordered_map<std::string, std::vector<std::string>>& rawData() const noexcept;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Check if a group exists
		 * @param group The group to check
		 * @return True if the group exists
		 */
		[[nodiscard]] inline bool hasGroup( std::string_view group ) const noexcept;

		/**
		 * @brief Check if a value is a standard value
		 * @param value The value to check
		 * @return True if the value is standard
		 */
		[[nodiscard]] inline bool hasStandardValue( std::string_view value ) const noexcept;

		//----------------------------------------------
		// Metadata tag creation
		//----------------------------------------------

		/**
		 * @brief Try to create a metadata tag
		 * @param valueView The tag value
		 * @return The metadata tag, or none if invalid
		 */
		[[nodiscard]] std::optional<MetadataTag> tryCreateTag( std::string_view valueView ) const;

		/**
		 * @brief Create a metadata tag
		 * @param value The tag value
		 * @return The metadata tag
		 * @throws std::invalid_argument If the value is invalid
		 */
		MetadataTag createTag( std::string_view value ) const;

		//----------------------------------------------
		// Position validation
		//----------------------------------------------

		/**
		 * @brief Validates a position string according to the rules defined for position codebooks.
		 * @details This method specifically applies validation logic relevant to position data
		 *          (e.g., ISO string format, hyphen separation, order, grouping).
		 *          It should only be called on `Codebook` instances representing position codebooks
		 *          (i.e., where `name()` returns `CodebookName::Position`).
		 * @param[in] position The position string to validate (as a `std::string_view`).
		 * @return A `PositionValidationResult` indicating the outcome of the validation.
		 * @warning Behavior is undefined if called on a non-position codebook.
		 */
		PositionValidationResult validatePosition( std::string_view position ) const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The name of this codebook */
		CodebookName m_name;

		/** @brief Mapping from values to their group names */
		std::unordered_map<std::string, std::string, StringHash, StringEqual> m_groupMap;

		/** @brief Container for standard values */
		CodebookStandardValues m_standardValues;

		/** @brief Container for groups */
		CodebookGroups m_groups;

		/** @brief Raw mapping of groups to their values */
		std::unordered_map<std::string, std::vector<std::string>> m_rawData;
	};
}

#include "Codebook.inl"
