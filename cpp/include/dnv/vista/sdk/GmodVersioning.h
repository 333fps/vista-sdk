/**
 * @file GmodVersioning.h
 * @brief Defines the GmodVersioning class for converting GMOD entities between VIS versions.
 * @details This file contains the declaration of the GmodVersioning class, which is responsible
 *          for applying versioning rules to Generic Product Model (GMOD) nodes, paths, and
 *          LocalIds to transform them from one Vessel Information Structure (VIS) version
 *          to another. It uses GMOD versioning data (typically from JSON files) to perform
 *          these conversions.
 */

#pragma once

#include "GmodVersioningDto.h"
#include "GmodPath.h"
#include "LocalId.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class GmodNode;
	class LocalIdBuilder;
	enum class VisVersion;

	//=====================================================================
	// GmodVersioning class
	//=====================================================================

	/**
	 * @brief Class responsible for converting GMOD objects to a higher version
	 */
	class GmodVersioning final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructor
		 *
		 * @param dto Dictionary of GmodVersioningDto objects indexed by version string
		 */
		explicit GmodVersioning( const std::unordered_map<std::string, GmodVersioningDto>& dto );

		/** @brief Default constructor. */
		GmodVersioning() = delete;

		/** @brief Copy constructor */
		GmodVersioning( const GmodVersioning& ) = default;

		/** @brief Move constructor */
		GmodVersioning( GmodVersioning&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioning() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodVersioning& operator=( const GmodVersioning& ) = delete;

		/** @brief Move assignment operator */
		GmodVersioning& operator=( GmodVersioning&& ) noexcept = delete;

		//----------------------------------------------
		// Conversion
		//----------------------------------------------

		/**
		 * @brief Convert a GmodNode from one version to a higher version
		 * @throws std::invalid_argument If targetVersion is not higher than sourceVersion
		 */
		[[nodiscard]] std::optional<GmodNode> convertNode(
			VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const;

		/**
		 * @brief Converts a GmodPath from a source VIS version to a target VIS version.
		 * @param sourceVersion The VIS version of the sourcePath.
		 * @param sourcePath The GmodPath to convert (passed by const reference).
		 * @param targetVersion The target VIS version.
		 * @return An optional containing the converted GmodPath if successful, otherwise std::nullopt.
		 * @throws std::invalid_argument if source or target versions are invalid or source >= target.
		 */
		[[nodiscard]] std::optional<GmodPath> convertPath(
			VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) const;

		/**
		 * @brief Convert a LocalIdBuilder from one version to a higher version
		 */
		std::optional<LocalIdBuilder> convertLocalId( const LocalIdBuilder& sourceLocalId, VisVersion targetVersion ) const;

		/**
		 * @brief Convert a LocalId from one version to a higher version
		 */
		std::optional<LocalId> convertLocalId( const LocalId& sourceLocalId, VisVersion targetVersion ) const;

	private:
		//----------------------------------------------
		// Private Types
		//----------------------------------------------

		/**
		 * @brief Enumeration of conversion operation types
		 */
		enum class ConversionType
		{
			ChangeCode = 0,
			Merge,
			Move,
			AssignmentChange = 20,
			AssignmentDelete = 21
		};

		/**
		 * @brief Structure holding information about a node conversion
		 */
		struct GmodNodeConversion final
		{
			std::unordered_set<ConversionType> operations;
			std::string source;
			std::optional<std::string> target;
			std::optional<std::string> oldAssignment;
			std::optional<std::string> newAssignment;
			std::optional<bool> deleteAssignment;
		};

		//----------------------------------------------
		// GmodVersioningNode class
		//----------------------------------------------

		/**
		 * @brief Class representing versioning information for a specific VIS version
		 */
		class GmodVersioningNode final
		{
		public:
			//----------------------------------------------
			// Construction / destruction
			//----------------------------------------------
			/**
			 * @brief Constructor for GmodVersioningNode
			 */
			GmodVersioningNode( VisVersion visVersion,
				const std::unordered_map<std::string, GmodNodeConversionDto>& dto );

			/** @brief Default constructor. */
			GmodVersioningNode() = default;

			/** @brief Copy constructor */
			GmodVersioningNode( const GmodVersioningNode& ) = default;

			/** @brief Move constructor */
			GmodVersioningNode( GmodVersioningNode&& ) noexcept = default;

			/** @brief Destructor */
			~GmodVersioningNode() = default;

			//----------------------------------------------
			// Assignment operators
			//----------------------------------------------

			/** @brief Copy assignment operator */
			GmodVersioningNode& operator=( const GmodVersioningNode& ) = default;

			/** @brief Move assignment operator */
			GmodVersioningNode& operator=( GmodVersioningNode&& ) noexcept = delete;

			//----------------------------------------------
			// Accessors
			//----------------------------------------------

			/**
			 * @brief Get the VIS version of this versioning node
			 */
			VisVersion visVersion() const;

			/**
			 * @brief Try to get code changes for a specific node code
			 */
			bool tryGetCodeChanges( const std::string& code, GmodNodeConversion& nodeChanges ) const;

		private:
			//----------------------------------------------
			// Private member variables
			//----------------------------------------------
			VisVersion m_visVersion;
			std::unordered_map<std::string, GmodNodeConversion> m_versioningNodeChanges;
		};

		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		std::unordered_map<VisVersion, GmodVersioningNode> m_versioningsMap;

		//----------------------------------------------
		// Private helper methods
		//----------------------------------------------

		/**
		 * @brief Internal implementation for converting a node between adjacent versions
		 */
		[[nodiscard]] std::optional<GmodNode> convertNodeInternal(
			VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const;

		/**
		 * @brief Try to get a versioning node for a specific VIS version
		 */
		[[nodiscard]] bool tryGetVersioningNode( VisVersion visVersion, GmodVersioningNode& versioningNode ) const;
		[[nodiscard]] const GmodVersioning::GmodVersioningNode* tryGetVersioningNode( VisVersion visVersion ) const noexcept;
		//----------------------------------------------
		// Private validation methods
		//----------------------------------------------

		/**
		 * @brief Validate source and target versions
		 */
		void validateSourceAndTargetVersions( VisVersion sourceVersion, VisVersion targetVersion ) const;

		/**
		 * @brief Validate source and target version pair
		 */
		void validateSourceAndTargetVersionPair( VisVersion sourceVersion, VisVersion targetVersion ) const;

		//----------------------------------------------
		// Private static utility methods
		//----------------------------------------------

		/**
		 * @brief Parse a conversion type from a string
		 */
		static ConversionType parseConversionType( const std::string& type );
	};
}
