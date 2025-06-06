/**
 * @file LocationParsingErrorBuilder.h
 * @brief Defines the LocationParsingErrorBuilder class for collecting Location parsing errors.
 * @details This class provides a mechanism to accumulate errors encountered during the
 *          parsing of a Location string, associating them with specific validation results.
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
	 * @brief Represents the result of Location validation.
	 * @details Used internally by the parser to track validation results and externally
	 *          within ParsingErrors to categorize issues found during Location parsing.
	 */
	enum class LocationValidationResult
	{
		Invalid = 0,
		InvalidCode,
		InvalidOrder,
		NullOrWhiteSpace,
		Valid
	};

	//=====================================================================
	// LocationParsingErrorBuilder class
	//=====================================================================

	/**
	 * @class LocationParsingErrorBuilder
	 * @brief A builder class for accumulating errors encountered during Location parsing.
	 *
	 * @details This class provides methods to add errors associated with specific validation results
	 *          (defined by `LocationValidationResult`) and finally builds a `ParsingErrors` object
	 *          containing the collected issues, formatted for user presentation. It is used
	 *          internally by the `Locations` parsing logic.
	 */
	class LocationParsingErrorBuilder final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/** @brief Default constructor */
		LocationParsingErrorBuilder();

		/** @brief Copy constructor */
		LocationParsingErrorBuilder( const LocationParsingErrorBuilder& ) = default;

		/** @brief Move constructor */
		LocationParsingErrorBuilder( LocationParsingErrorBuilder&& ) noexcept = default;

		/** @brief Destructor */
		~LocationParsingErrorBuilder() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocationParsingErrorBuilder& operator=( const LocationParsingErrorBuilder& ) = default;

		/** @brief Move assignment operator */
		LocationParsingErrorBuilder& operator=( LocationParsingErrorBuilder&& ) noexcept = default;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if any errors have been added to the builder.
		 * @return `true` if at least one error has been added, `false` otherwise.
		 */
		[[nodiscard]] bool hasError() const;

		//----------------------------------------------
		// Static factory method
		//----------------------------------------------

		/**
		 * @brief Creates a new, empty LocationParsingErrorBuilder instance.
		 * @details Provides a clear entry point for creating a builder.
		 * @return A new instance of `LocationParsingErrorBuilder`.
		 */
		[[nodiscard]] static LocationParsingErrorBuilder create();

		//----------------------------------------------
		// ParsingErrors construction
		//----------------------------------------------

		/**
		 * @brief Constructs a `ParsingErrors` object from the errors collected by this builder.
		 * @details Formats the collected validation result/message pairs into a final `ParsingErrors` object.
		 * @return A `ParsingErrors` object containing the formatted error messages.
		 *         Returns `ParsingErrors::Empty` if `hasError()` is false.
		 */
		[[nodiscard]] ParsingErrors build() const;

		//----------------------------------------------
		// Error addition
		//----------------------------------------------

		/**
		 * @brief Adds an error with a specific validation result and message.
		 * @param[in] validationResult The `LocationValidationResult` indicating the type of error.
		 * @param[in] message The error message describing the issue.
		 * @return A reference to this builder instance for method chaining.
		 */
		LocationParsingErrorBuilder& addError( LocationValidationResult validationResult, const std::optional<std::string>& message );

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/**
		 * @brief Internal storage for the collected parsing errors.
		 * @details Each pair contains the `LocationValidationResult` and the associated error message string.
		 */
		std::vector<std::pair<LocationValidationResult, std::string>> m_errors;
	};
}
