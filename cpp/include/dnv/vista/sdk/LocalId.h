/**
 * @file LocalId.h
 * @brief LocalId class implementation for the VIS system
 *
 * Defines the concrete LocalId class which implements the ILocalId interface.
 */

#pragma once

#include "ILocalId.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Forward declarations
	//-------------------------------------------------------------------------
	enum class VisVersion;
	class GmodPath;
	class LocalIdBuilder;
	class MetadataTag;
	class ParsingErrors;

	/**
	 * @brief Represents the parsing state for LocalId
	 *
	 * Used to track state during parsing and for error reporting.
	 */
	enum class LocalIdParsingState
	{
		/** Parsing the naming rule prefix */
		NamingRule = 0,
		/** Parsing the VIS version */
		VisVersion,
		/** Parsing the primary item path */
		PrimaryItem,
		/** Parsing the secondary item path */
		SecondaryItem,
		/** Parsing item description */
		ItemDescription,
		/** Parsing quantity metadata */
		MetaQuantity,
		/** Parsing content metadata */
		MetaContent,
		/** Parsing calculation metadata */
		MetaCalculation,
		/** Parsing state metadata */
		MetaState,
		/** Parsing command metadata */
		MetaCommand,
		/** Parsing type metadata */
		MetaType,
		/** Parsing position metadata */
		MetaPosition,
		/** Parsing detail metadata */
		MetaDetail,

		/** Empty state error */
		EmptyState = 100,
		/** Formatting error */
		Formatting = 101,
		/** Completeness error */
		Completeness = 102,

		/** Naming entity error */
		NamingEntity = 200,
		/** IMO number error */
		IMONumber = 201
	};

	/**
	 * @brief Local ID class for identifying items in the VIS system
	 *
	 * Concrete implementation of ILocalId that represents an immutable
	 * Local ID according to ISO 19848 standard.
	 */
	class LocalId final : public ILocalId<LocalId>
	{
	public:
		//-------------------------------------------------------------------------
		// Constants
		//-------------------------------------------------------------------------

		/** @brief Standard naming rule prefix for Local IDs */
		static const std::string namingRule;

		//-------------------------------------------------------------------------
		// Constructors and Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Constructor from builder
		 * @param builder The LocalIdBuilder to construct from
		 * @throws std::invalid_argument if builder is empty or invalid
		 */
		explicit LocalId( const LocalIdBuilder& builder );

		/** @brief Destructor */
		~LocalId() = default;

		/** @brief Copy constructor (deleted) */
		LocalId( const LocalId& ) = default;

		/** @brief Copy assignment operator (deleted) */
		LocalId& operator=( const LocalId& ) = default;

		/** @brief Move constructor */
		LocalId( LocalId&& other ) noexcept = default;

		/** @brief Move assignment operator */
		LocalId& operator=( LocalId&& other ) noexcept = default;

		//-------------------------------------------------------------------------
		// Builder Access
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the internal builder
		 * @return Reference to the LocalIdBuilder
		 */
		const LocalIdBuilder& builder() const;

		//-------------------------------------------------------------------------
		// ILocalId Interface Implementation
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS version
		 * @return The VIS version
		 * @throws std::runtime_error if VIS version is not set
		 */
		VisVersion visVersion() const override;

		/**
		 * @brief Check if verbose mode is enabled
		 * @return true if verbose mode is enabled
		 */
		bool isVerboseMode() const override;

		/**
		 * @brief Get the primary item
		 * @return Reference to the primary item (GmodPath)
		 * @throws std::runtime_error if primary item is not set
		 */
		const GmodPath& primaryItem() const override;

		/**
		 * @brief Get the secondary item
		 * @return Reference to the optional secondary item
		 */
		const std::optional<GmodPath>& secondaryItem() const override;

		/**
		 * @brief Check if the LocalId has any custom tags
		 * @return true if has custom tag
		 */
		bool hasCustomTag() const override;

		/**
		 * @brief Get metadata tags
		 * @return Reference to the vector of metadata tags
		 */
		const std::vector<MetadataTag>& metadataTags() const override;

		//-------------------------------------------------------------------------
		// Metadata Tag Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get quantity metadata tag
		 * @return The quantity tag, if present
		 */
		const std::optional<MetadataTag>& quantity() const;

		/**
		 * @brief Get content metadata tag
		 * @return The content tag, if present
		 */
		const std::optional<MetadataTag>& content() const;

		/**
		 * @brief Get calculation metadata tag
		 * @return The calculation tag, if present
		 */
		const std::optional<MetadataTag>& calculation() const;

		/**
		 * @brief Get state metadata tag
		 * @return The state tag, if present
		 */
		const std::optional<MetadataTag>& state() const;

		/**
		 * @brief Get command metadata tag
		 * @return The command tag, if present
		 */
		const std::optional<MetadataTag>& command() const;

		/**
		 * @brief Get type metadata tag
		 * @return The type tag, if present
		 */
		const std::optional<MetadataTag>& type() const;

		/**
		 * @brief Get position metadata tag
		 * @return The position tag, if present
		 */
		const std::optional<MetadataTag>& position() const;

		/**
		 * @brief Get detail metadata tag
		 * @return The detail tag, if present
		 */
		const std::optional<MetadataTag>& detail() const;

		//-------------------------------------------------------------------------
		// Object Overrides and Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert to string
		 * @return String representation
		 */
		std::string toString() const override;

		/**
		 * @brief Check if this LocalId equals another
		 * @param other The LocalId to compare with
		 * @return true if equal
		 */
		bool equals( const LocalId& other ) const override;

		/**
		 * @brief Equality operator
		 * @param other The LocalId to compare with
		 * @return true if equal
		 */
		bool operator==( const LocalId& other ) const noexcept;

		/**
		 * @brief Inequality operator
		 * @param other The LocalId to compare with
		 * @return true if not equal
		 */
		bool operator!=( const LocalId& other ) const noexcept;

		/**
		 * @brief Get hash code for use in containers
		 * @return Hash code of the LocalId
		 */
		size_t hashCode() const;

		//-------------------------------------------------------------------------
		// Static Factory Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a string to a LocalId
		 * @param localIdStr The string to parse
		 * @return The parsed LocalId
		 * @throws std::invalid_argument if parsing fails
		 */
		static LocalId parse( const std::string& localIdStr );

		/**
		 * @brief Try to parse a string to a LocalId
		 * @param localIdStr The string to parse
		 * @param errors Output parameter for parsing errors
		 * @param localId Output parameter for the parsed LocalId
		 * @return true if parsing succeeded
		 */
		static bool tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<LocalId>& localId );

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/**
		 * @brief The builder that contains all LocalId data
		 */
		std::shared_ptr<const LocalIdBuilder> m_builder;
	};

	/**
	 * @brief Builder for parsing errors related to LocalId
	 *
	 * Collects and formats error messages that occur during LocalId parsing.
	 */
	class LocalIdParsingErrorBuilder
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors and Destructor
		//-------------------------------------------------------------------------

		/** @brief Default constructor */
		LocalIdParsingErrorBuilder() = default;

		//-------------------------------------------------------------------------
		// Public Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Add an error with predefined message
		 * @param state The parsing state where the error occurred
		 * @return This builder for chaining
		 */
		LocalIdParsingErrorBuilder& addError( LocalIdParsingState state );

		/**
		 * @brief Add an error with custom message
		 * @param state The parsing state where the error occurred
		 * @param message The custom error message
		 * @return This builder for chaining
		 */
		LocalIdParsingErrorBuilder& addError( LocalIdParsingState state, const std::string& message );

		/**
		 * @brief Check if the builder has any errors
		 * @return true if errors exist
		 */
		bool hasError() const;

		/**
		 * @brief Create a ParsingErrors object from the collected errors
		 * @return A ParsingErrors object
		 */
		ParsingErrors build() const;

		/**
		 * @brief Create a new LocalIdParsingErrorBuilder
		 * @return A new instance of LocalIdParsingErrorBuilder
		 */
		static LocalIdParsingErrorBuilder create();

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief Collection of errors with their associated parsing states */
		std::vector<std::pair<LocalIdParsingState, std::string>> m_errors;

		/** @brief Predefined error messages for common parsing states */
		static const std::unordered_map<LocalIdParsingState, std::string> m_predefinedErrorMessages;
	};
}
