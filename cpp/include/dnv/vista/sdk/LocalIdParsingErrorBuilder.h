/**
 * @file LocalIdParsingErrorBuilder.h
 * @brief Defines the LocalIdParsingErrorBuilder class for collecting LocalId parsing errors.
 * @details This class provides a mechanism to accumulate errors encountered during the
 *          parsing of a Local ID string, associating them with specific parsing stages.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class ParsingErrors;

	//=====================================================================
	// Enumerations
	//=====================================================================

	/**
	 * @brief Represents the specific stage or aspect of LocalId parsing.
	 * @details Used internally by the parser to track progress and externally
	 *          within ParsingErrors to categorize issues found during LocalId parsing.
	 *          Values 0-99 represent sequential parsing stages, 100-199 represent
	 *          structural errors, and 200+ represent validation errors.
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

	//=====================================================================
	// LocalIdParsingErrorBuilder class
	//=====================================================================

	/**
	 * @class LocalIdParsingErrorBuilder
	 * @brief A builder class for accumulating errors encountered during LocalId parsing.
	 *
	 * @details This class provides methods to add errors associated with specific parsing states
	 *          (defined by `LocalIdParsingState`) and finally builds a `ParsingErrors` object
	 *          containing the collected issues, formatted for user presentation. It is used
	 *          internally by the `LocalIdBuilder` parsing logic.
	 */
	class LocalIdParsingErrorBuilder final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/** @brief Default constructor. */
		LocalIdParsingErrorBuilder();

		/** @brief Copy constructor */
		LocalIdParsingErrorBuilder( const LocalIdParsingErrorBuilder& ) = default;

		/** @brief Move constructor */
		LocalIdParsingErrorBuilder( LocalIdParsingErrorBuilder&& ) noexcept = default;

		/** @brief Destructor */
		~LocalIdParsingErrorBuilder() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocalIdParsingErrorBuilder& operator=( const LocalIdParsingErrorBuilder& ) = default;

		/** @brief Move assignment operator */
		LocalIdParsingErrorBuilder& operator=( LocalIdParsingErrorBuilder&& ) noexcept = default;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if any errors have been added to the builder.
		 * @return `true` if at least one error has been added, `false` otherwise.
		 */
		[[nodiscard]] inline bool hasError() const;

		//----------------------------------------------
		// Static factory method
		//----------------------------------------------

		/**
		 * @brief Creates a new, empty LocalIdParsingErrorBuilder instance.
		 * @details Provides a clear entry point for creating a builder.
		 * @return A new instance of `LocalIdParsingErrorBuilder`.
		 */
		[[nodiscard]] static LocalIdParsingErrorBuilder create();

		//----------------------------------------------
		// ParsingErrors construction
		//----------------------------------------------

		/**
		 * @brief Constructs a `ParsingErrors` object from the errors collected by this builder.
		 * @details Formats the collected state/message pairs into a final `ParsingErrors` object.
		 * @return A `ParsingErrors` object containing the formatted error messages.
		 *         Returns an empty `ParsingErrors` object if `hasError()` is false.
		 */
		[[nodiscard]] ParsingErrors build() const;

		//----------------------------------------------
		// Error addition
		//----------------------------------------------

		/**
		 * @brief Adds an error associated with a specific parsing state, using a predefined message.
		 * @details Retrieves a standard error message based on the `state` and adds it to the list.
		 * @param[in] state The `LocalIdParsingState` where the error occurred.
		 * @return A reference to this builder instance for method chaining.
		 */
		LocalIdParsingErrorBuilder& addError( LocalIdParsingState state );

		/**
		 * @brief Adds an error associated with a specific parsing state, with optional custom message.
		 * @details If message has no value, falls back to predefined message for the state.
		 * @param[in] state The `LocalIdParsingState` where the error occurred.
		 * @param[in] message Optional custom error message. If nullopt, uses predefined message.
		 * @return A reference to this builder instance for method chaining.
		 */
		LocalIdParsingErrorBuilder& addError( LocalIdParsingState state, const std::optional<std::string>& message );

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/**
		 * @brief Internal storage for the collected parsing errors.
		 * @details Each pair contains the `LocalIdParsingState` where the error occurred
		 *          and the associated error message string.
		 */
		std::vector<std::pair<LocalIdParsingState, std::string>> m_errors;
	};
}

#include "LocalIdParsingErrorBuilder.inl"
