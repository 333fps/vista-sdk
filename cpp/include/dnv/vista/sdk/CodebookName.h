/**
 * @file CodebookName.h
 * @brief Enumeration and utility functions for ISO 19848 codebook naming
 *
 * This file defines the standard codebook types as specified in the ISO 19848 standard
 * for vessel information structure, along with utility functions to convert between
 * enum values and their string prefix representations.
 *
 * @see ISO 19848:2018 - Ships and marine technology — Standard data for shipboard
 *      machinery and equipment
 */

#pragma once

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Codebook Type Definitions
	//-------------------------------------------------------------------

	/**
	 * @brief Enumeration of codebook types used in the VIS system
	 *
	 * Each value represents a different category of data that can be stored
	 * in the vessel information structure according to ISO 19848.
	 */
	enum class CodebookName
	{
		/** @brief Represents physical quantities and units (prefix: qty) */
		Quantity = 1,

		/** @brief Represents content types (prefix: cnt) */
		Content = 2,

		/** @brief Represents calculation methods (prefix: calc) */
		Calculation = 3,

		/** @brief Represents states or conditions (prefix: state) */
		State = 4,

		/** @brief Represents commands or actions (prefix: cmd) */
		Command = 5,

		/** @brief Represents type classifications (prefix: type) */
		Type = 6,

		/** @brief Represents functional services (prefix: funct.svc) */
		FunctionalServices = 7,

		/** @brief Represents maintenance categories (prefix: maint.cat) */
		MaintenanceCategory = 8,

		/** @brief Represents types of activities (prefix: act.type) */
		ActivityType = 9,

		/** @brief Represents positions or locations (prefix: pos) */
		Position = 10,

		/** @brief Represents detailed information (prefix: detail) */
		Detail = 11
	};

	//-------------------------------------------------------------------
	// Utility Functions
	//-------------------------------------------------------------------

	/**
	 * @brief Utility class for working with CodebookName values
	 *
	 * Provides conversion methods between CodebookName enum values
	 * and their string prefix representations as defined in the ISO 19848 standard.
	 */
	class CodebookNames final
	{
	public:
		/**
		 * @brief Convert a string prefix to a CodebookName
		 * @param prefix The string prefix to convert (e.g., "pos", "qty", "calc")
		 * @return The corresponding CodebookName value
		 * @throws std::invalid_argument If the prefix is empty or unknown
		 */
		static CodebookName fromPrefix( const std::string_view prefix );

		/**
		 * @brief Convert a CodebookName to its string prefix
		 * @param name The CodebookName to convert
		 * @return The corresponding string prefix (e.g., "pos" for Position)
		 * @throws std::invalid_argument If the name is unknown
		 */
		static std::string toPrefix( CodebookName name );
	};
}
