#pragma once

#include "ILocalId.h"
#include "LocalIdBuilder.h"

namespace dnv::vista::sdk
{
	enum class VisVersion;
	class GmodPath;
	class MetadataTag;
	class ParsingErrors;

	/**
	 * @brief Local ID class for identifying items in VIS
	 */
	class LocalId : public ILocalId<LocalId>
	{
	public:
		//-------------------------------------------------------------------------
		// Constants
		//-------------------------------------------------------------------------

		/**
		 * @brief Naming rule constant
		 */
		static const std::string NamingRule;

		//-------------------------------------------------------------------------
		// Constructors
		//-------------------------------------------------------------------------

		/**
		 * @brief Constructor from builder
		 * @param builder The LocalIdBuilder to construct from
		 * @throws std::invalid_argument if builder is empty or invalid
		 */
		explicit LocalId( const LocalIdBuilder& builder );

		/**
		 * @brief Copy constructor (deleted)
		 * @details LocalId objects shouldn't be copied since base interface deletes this operation
		 */
		LocalId( const LocalId& ) = delete;

		/**
		 * @brief Copy assignment operator (deleted)
		 * @details LocalId objects shouldn't be copied since base interface deletes this operation
		 */
		LocalId& operator=( const LocalId& ) = delete;

		/**
		 * @brief Move constructor
		 * @details Allows LocalId objects to be moved efficiently
		 */
		LocalId( LocalId&& other ) noexcept;

		/**
		 * @brief Move assignment operator
		 * @details Allows LocalId objects to be move-assigned efficiently
		 */
		LocalId& operator=( LocalId&& other ) noexcept;

		//-------------------------------------------------------------------------
		// ILocalId Interface Implementation
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS version
		 * @return The VIS version
		 */
		virtual VisVersion visVersion() const override;

		/**
		 * @brief Check if verbose mode is enabled
		 * @return true if verbose mode is enabled
		 */
		virtual bool isVerboseMode() const override;

		/**
		 * @brief Get the primary item
		 * @return The primary item (GmodPath)
		 */
		virtual const GmodPath& primaryItem() const override;

		/**
		 * @brief Get the secondary item
		 * @return The secondary item, if present
		 */
		virtual std::optional<GmodPath> secondaryItem() const override;

		/**
		 * @brief Check if the LocalId has any custom tags
		 * @return true if has custom tag
		 */
		virtual bool hasCustomTag() const override;

		/**
		 * @brief Get metadata tags
		 * @return Vector of metadata tags
		 */
		virtual const std::vector<MetadataTag> metadataTags() const override;

		/**
		 * @brief Convert to string
		 * @return String representation
		 */
		virtual std::string toString() const override;

		/**
		 * @brief Check if this LocalId equals another
		 * @param other The LocalId to compare with
		 * @return true if equal
		 */
		virtual bool equals( const LocalId& other ) const override;

		//-------------------------------------------------------------------------
		// Metadata Tag Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get quantity metadata tag
		 * @return The quantity tag, if present
		 */
		std::optional<MetadataTag> quantity() const;

		/**
		 * @brief Get content metadata tag
		 * @return The content tag, if present
		 */
		std::optional<MetadataTag> content() const;

		/**
		 * @brief Get calculation metadata tag
		 * @return The calculation tag, if present
		 */
		std::optional<MetadataTag> calculation() const;

		/**
		 * @brief Get state metadata tag
		 * @return The state tag, if present
		 */
		std::optional<MetadataTag> state() const;

		/**
		 * @brief Get command metadata tag
		 * @return The command tag, if present
		 */
		std::optional<MetadataTag> command() const;

		/**
		 * @brief Get type metadata tag
		 * @return The type tag, if present
		 */
		std::optional<MetadataTag> type() const;

		/**
		 * @brief Get position metadata tag
		 * @return The position tag, if present
		 */
		std::optional<MetadataTag> position() const;

		/**
		 * @brief Get detail metadata tag
		 * @return The detail tag, if present
		 */
		std::optional<MetadataTag> detail() const;

		//-------------------------------------------------------------------------
		// Builder Access
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the builder
		 * @return The LocalIdBuilder
		 */
		const LocalIdBuilder& builder() const;

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

		//-------------------------------------------------------------------------
		// Operators and Hash Code
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality operator
		 * @param other The LocalId to compare with
		 * @return true if equal
		 */
		bool operator==( const LocalId& other ) const;

		/**
		 * @brief Inequality operator
		 * @param other The LocalId to compare with
		 * @return true if not equal
		 */
		bool operator!=( const LocalId& other ) const;

		/**
		 * @brief Get hash code
		 * @return Hash code of the LocalId
		 */
		size_t hashCode() const;

	private:
		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------
		LocalIdBuilder m_builder;
	};

	/**
	 * @brief Represents the parsing state for LocalId.
	 */
	enum class LocalIdParsingState
	{
		NamingRule = 0,
		VisVersion,
		PrimaryItem,
		SecondaryItem,
		ItemDescription,
		MetaQuantity,
		MetaContent,
		MetaCalculation,
		MetaState,
		MetaCommand,
		MetaType,
		MetaPosition,
		MetaDetail,

		EmptyState = 100,
		Formatting = 101,
		Completeness = 102,

		NamingEntity = 200,
		IMONumber = 201
	};

	/**
	 * @brief Builder for parsing errors related to LocalId.
	 */
	class LocalIdParsingErrorBuilder
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors
		//-------------------------------------------------------------------------
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
		// Member Variables
		//-------------------------------------------------------------------------
		std::vector<std::pair<LocalIdParsingState, std::string>> m_errors;

		/**
		 * @brief Predefined error messages for common parsing states
		 * Made static since these messages are shared across all instances
		 */
		static const inline std::unordered_map<LocalIdParsingState, std::string> m_predefinedErrorMessages = {
			{ LocalIdParsingState::NamingRule, "Missing or invalid naming rule" },
			{ LocalIdParsingState::VisVersion, "Missing or invalid vis version" },
			{ LocalIdParsingState::PrimaryItem, "Invalid or missing Primary item. Local IDs require at least a primary item and 1 metadata tag." },
			{ LocalIdParsingState::SecondaryItem, "Invalid secondary item" },
			{ LocalIdParsingState::ItemDescription, "Missing or invalid /meta prefix" },
			{ LocalIdParsingState::MetaQuantity, "Invalid metadata tag: Quantity" },
			{ LocalIdParsingState::MetaContent, "Invalid metadata tag: Content" },
			{ LocalIdParsingState::MetaCommand, "Invalid metadata tag: Command" },
			{ LocalIdParsingState::MetaPosition, "Invalid metadata tag: Position" },
			{ LocalIdParsingState::MetaCalculation, "Invalid metadata tag: Calculation" },
			{ LocalIdParsingState::MetaState, "Invalid metadata tag: State" },
			{ LocalIdParsingState::MetaType, "Invalid metadata tag: Type" },
			{ LocalIdParsingState::MetaDetail, "Invalid metadata tag: Detail" },
			{ LocalIdParsingState::EmptyState, "Missing primary path or metadata" } };
	};
}
