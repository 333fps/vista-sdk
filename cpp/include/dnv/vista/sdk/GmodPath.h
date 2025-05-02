/**
 * @file GmodPath.h
 * @brief Declarations for GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 */

#pragma once

#include "GmodNode.h"
#include "Locations.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Forward Declarations
	//-------------------------------------------------------------------------
	class Gmod;
	class Locations;
	enum class VisVersion;
	class GmodPath;
	class GmodIndividualizableSet;
	class GmodParsePathResult;

	//-------------------------------------------------------------------------
	// Helper Classes (Used internally by GmodPath or its operations)
	//-------------------------------------------------------------------------

	/**
	 * @brief Represents a single node component parsed from a path string.
	 *
	 * Contains the node's code and optional location before it's resolved to a GmodNode.
	 */
	struct PathNode final
	{
		/** @brief Code identifying the node. */
		std::string code{};

		/** @brief Optional location associated with the node. */
		std::optional<Location> location{};

		/**
		 * @brief Construct a new PathNode.
		 *
		 * @param nodeCode The node code (default: empty string).
		 * @param nodeLocation The optional location (default: no location).
		 */
		PathNode( const std::string& nodeCode = "", const std::optional<Location>& nodeLocation = std::nullopt );
	};

	/**
	 * @brief Helper struct for identifying individualizable node sets within a GmodPath during traversal.
	 */
	struct LocationSetsVisitor final
	{
		/** @brief Starting index of the current potential individualizable parent sequence. Initialized in constructor. */
		size_t m_currentParentStart;

		/**
		 * @brief Default constructor. Initializes internal state.
		 */
		LocationSetsVisitor();

		/**
		 * @brief Visits a node during path traversal to identify potential individualizable sets.
		 *
		 * @param node The current GmodNode being visited in the path.
		 * @param i The index/depth of the current node in the path (0-based).
		 * @param parents Collection of parent nodes in the path, from root to leaf.
		 * @param target The target GmodNode at the end of the path.
		 * @return Optional tuple containing:
		 *         - size_t: Starting index (inclusive) of the identified individualizable set.
		 *         - size_t: Ending index (inclusive) of the identified individualizable set.
		 *         - std::optional<Location>: The common location associated with the set, if any.
		 *         Returns std::nullopt if the current node doesn't complete an individualizable set.
		 */
		std::optional<std::tuple<size_t, size_t, std::optional<Location>>> visit(
			const GmodNode& node,
			size_t i,
			const std::vector<GmodNode>& parents,
			const GmodNode& target );
	};

	//-------------------------------------------------------------------------
	// Main Path Class DEFINITION
	//-------------------------------------------------------------------------

	/**
	 * @brief Represents a hierarchical path in the Generic Product Model (GMOD).
	 *
	 * A GmodPath consists of a sequence of parent GmodNodes and a target GmodNode,
	 * forming a path through the GMOD structure as defined in ISO 19848.
	 * Each node in the path may have an optional Location.
	 * This class is designed to be immutable after construction (move-only semantics).
	 */
	class GmodPath final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors and Special Member Functions
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor. Creates an empty, invalid path.
		 * @warning Intended primarily for use in containers or as a placeholder before assignment.
		 *          The resulting path is not valid until properly initialized or assigned via move.
		 */
		GmodPath();

		/**
		 * @brief Construct a path from a vector of parent nodes and a target node.
		 *
		 * @param parents Vector of parent nodes, ordered from root towards the target (moved).
		 * @param node The target node at the end of the path (moved).
		 * @param visVersion The VIS version associated with this path.
		 * @param skipVerify If true (default), skips validation of parent-child relationships.
		 *                   If false, validates the path structure upon construction.
		 * @throws std::invalid_argument If skipVerify is false and the provided path structure is invalid.
		 */
		GmodPath( const std::vector<GmodNode>& parents, GmodNode node, VisVersion visVersion, bool skipVerify = true );

		/** @brief Default copy constructor (if appropriate). */
		GmodPath( const GmodPath& ) = delete;

		/** @brief Default copy assignment operator (if appropriate). */
		GmodPath& operator=( const GmodPath& ) = delete;

		/** @brief Default move constructor. */
		GmodPath( GmodPath&& other ) noexcept = default;

		/** @brief Default move assignment operator. */
		GmodPath& operator=( GmodPath&& other ) noexcept = default;

		/** @brief Default destructor. */
		~GmodPath() = default;

		//-------------------------------------------------------------------------
		// Core Properties & Operations
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the total number of nodes in the path (parents + target node).
		 * @return The length of the path. Returns 0 for a default-constructed (empty) path.
		 */
		[[nodiscard]] size_t length() const noexcept;

		/**
		 * @brief Check if the target node of this path is designated as mappable in the GMOD.
		 * @return True if the target node is mappable, false otherwise. Returns false for empty path.
		 */
		[[nodiscard]] bool isMappable() const noexcept;

		/**
		 * @brief Creates a new GmodPath instance identical to this one but with all locations removed from all nodes.
		 * @return A new GmodPath instance without any locations. Returns an empty path if called on an empty path.
		 */
		[[nodiscard]] GmodPath withoutLocations() const;

		/**
		 * @brief Performs deep equality comparison with another GmodPath.
		 *
		 * Compares VIS version, all parent nodes (including locations), and the target node (including location).
		 *
		 * @param other The GmodPath to compare against.
		 * @return True if both paths represent the exact same sequence of nodes and locations, false otherwise.
		 */
		[[nodiscard]] bool equals( const GmodPath& other ) const;

		/**
		 * @brief Calculate a hash code for this GmodPath.
		 *
		 * Suitable for use in hash-based containers like std::unordered_map.
		 * The hash considers the VIS version, all nodes, and their locations.
		 *
		 * @return A size_t hash code value. Returns 0 for an empty path.
		 */
		[[nodiscard]] size_t hashCode() const noexcept;

		/**
		 * @brief Check if the path contains any nodes marked as individualizable in the GMOD.
		 * @return True if at least one node in the path (parents or target) is individualizable, false otherwise or for empty path.
		 */
		[[nodiscard]] bool isIndividualizable() const noexcept;

		/**
		 * @brief Identifies and returns all sets of consecutive individualizable nodes within this path.
		 *
		 * An individualizable set represents a segment of the path where instance identifiers
		 * can be applied, potentially sharing a common location.
		 *
		 * @return A vector of GmodIndividualizableSet objects representing the identified sets.
		 *         Returns an empty vector if the path is not individualizable or is empty.
		 * @warning This method returns sets that operate on a mutable reference to this path.
		 *          Use the `build()` method on the returned sets to get the modified path.
		 */
		[[nodiscard]] std::vector<GmodIndividualizableSet> individualizableSets();

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS (Vessel Information Structure) version associated with this path.
		 * @return The VisVersion enum value. Returns VisVersion::Unknown for an empty path.
		 */
		[[nodiscard]] VisVersion visVersion() const noexcept;

		/**
		 * @brief Get a const reference to the target node (the last node) of the path.
		 * @return Const reference to the target GmodNode.
		 * @throws std::logic_error If called on a default-constructed (empty) path.
		 */
		[[nodiscard]] const GmodNode& node() const;

		/**
		 * @brief Get a const reference to the vector containing the parent nodes of the path.
		 *
		 * The vector is ordered from the root node towards the target node.
		 *
		 * @return Const reference to the vector of parent GmodNodes. Returns an empty vector for a path of length 1 or 0.
		 */
		[[nodiscard]] const std::vector<GmodNode>& parents() const noexcept;

		/**
		 * @brief Get the normal assignment name for the node at a specific depth, if defined in the GMOD.
		 *
		 * @param nodeDepth The zero-based depth index of the node (0 for the root parent, length()-1 for the target node).
		 * @return An std::optional<std::string> containing the normal assignment name if found, otherwise std::nullopt.
		 * @throws std::out_of_range If nodeDepth is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] std::optional<std::string> normalAssignmentName( size_t nodeDepth ) const;

		/**
		 * @brief Get all common names defined for the nodes in this path, along with their depths.
		 * @return A vector of pairs, where each pair contains the zero-based depth and the common name string. Returns empty vector for empty path.
		 */
		[[nodiscard]] std::vector<std::pair<size_t, std::string>> commonNames() const;

		//-------------------------------------------------------------------------
		// Mutators (Use with caution, primarily for internal construction/modification)
		//-------------------------------------------------------------------------

		/**
		 * @brief Sets the target node of the path.
		 * @warning This method directly modifies the path's state. It's intended for internal use
		 *          during construction or specific modification scenarios (like in GmodIndividualizableSet).
		 *          It does not re-validate the path structure. Throws if called on an empty path.
		 * @param newNode The new target GmodNode (moved).
		 * @throws std::logic_error If called on an empty path.
		 */
		void setNode( GmodNode newNode );

		//-------------------------------------------------------------------------
		// String Conversions
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert the path to its standard string representation (e.g., "CODE1/CODE2.LOC/CODE3").
		 *
		 * Uses '/' as the default separator and includes location suffixes where present.
		 *
		 * @return The string representation of the path. Returns an empty string for an empty path.
		 */
		[[nodiscard]] std::string toString() const;

		/**
		 * @brief Append the string representation of the path to a string stream.
		 *
		 * @param builder The std::stringstream to append to.
		 * @param separator The character to use as a separator between node codes (default: '/').
		 */
		void toString( std::stringstream& builder, char separator = '/' ) const;

		/**
		 * @brief Convert the path to its full string representation, including all parent nodes explicitly.
		 *
		 * Format: "RootCode/ParentCode/.../TargetCode" (locations included).
		 *
		 * @return The full path string representation. Returns empty string for empty path.
		 */
		[[nodiscard]] std::string toFullPathString() const;

		/**
		 * @brief Append the full path string representation to a string stream.
		 * @param builder The std::stringstream to append to.
		 */
		void toFullPathString( std::stringstream& builder ) const;

		/**
		 * @brief Generate a detailed string representation suitable for debugging purposes.
		 *
		 * Includes node codes, names, locations, and potentially other details.
		 *
		 * @return A detailed debug string. Returns "[Empty GmodPath]" for empty path.
		 */
		[[nodiscard]] std::string toStringDump() const;

		/**
		 * @brief Append the detailed debug string representation to a string stream.
		 * @param builder The std::stringstream to append to.
		 */
		void toStringDump( std::stringstream& builder ) const;

		//-------------------------------------------------------------------------
		// Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Access a node at a specific depth within the path (const version).
		 *
		 * @param depth Zero-based depth index (0 is the first parent/root, length()-1 is the target node).
		 * @return Const reference to the GmodNode at the specified depth.
		 * @throws std::out_of_range If the depth index is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		const GmodNode& operator[]( size_t depth ) const;

		/**
		 * @brief Access a node at a specific depth within the path (non-const version).
		 * @warning Provides mutable access. Intended for internal modification scenarios (like GmodIndividualizableSet).
		 *          Modifying nodes directly can invalidate path consistency if not done carefully.
		 * @param depth Zero-based depth index (0 is the first parent/root, length()-1 is the target node).
		 * @return Reference to the GmodNode at the specified depth.
		 * @throws std::out_of_range If the depth index is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		GmodNode& operator[]( size_t depth );

		/**
		 * @brief Equality operator. Equivalent to calling `equals(other)`.
		 * @param other The GmodPath to compare with.
		 * @return True if the paths are equal, false otherwise.
		 */
		bool operator==( const GmodPath& other ) const;

		/**
		 * @brief Inequality operator. Equivalent to `!(*this == other)`.
		 * @param other The GmodPath to compare with.
		 * @return True if the paths are not equal, false otherwise.
		 */
		bool operator!=( const GmodPath& other ) const;

		//-------------------------------------------------------------------------
		// Static Methods - Validation
		//-------------------------------------------------------------------------

		/**
		 * @brief Statically validate the structural integrity of a potential path.
		 *
		 * Checks if the parent-child relationships between the provided nodes are valid according to GMOD rules.
		 *
		 * @param parents Vector of potential parent nodes, ordered root to leaf.
		 * @param node The potential target node.
		 * @return True if the sequence forms a valid path structure, false otherwise.
		 */
		[[nodiscard]] static bool isValid(
			const std::vector<GmodNode>& parents,
			const GmodNode& node );

		/**
		 * @brief Statically validate the structural integrity and identify the point of failure.
		 *
		 * Checks parent-child relationships. If invalid, provides the index of the first invalid link.
		 *
		 * @param parents Vector of potential parent nodes, ordered root to leaf.
		 * @param node The potential target node.
		 * @param[out] missingLinkAt Output parameter set to the index `i` where `parents[i]` is not a valid parent of `parents[i+1]`
		 *                         (or `node` if `i` is the last parent index). Set to -1 if the path is valid.
		 * @return True if the sequence forms a valid path structure, false otherwise.
		 */
		[[nodiscard]] static bool isValid(
			const std::vector<GmodNode>& parents,
			const GmodNode& node,
			size_t& missingLinkAt );

		//-------------------------------------------------------------------------
		// Static Methods - Parsing (using default GMOD/Locations for VisVersion)
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a path string using the default GMOD and Locations for the specified VIS version.
		 *
		 * @param item The path string to parse (e.g., "CODE1/CODE2.LOC").
		 * @param visVersion The VIS version context to use for looking up GMOD/Locations data.
		 * @return The parsed GmodPath object.
		 * @throws std::invalid_argument If parsing fails (e.g., invalid format, unknown codes, invalid locations).
		 * @throws std::runtime_error If the required GMOD/Locations data for the visVersion is not loaded.
		 */
		[[nodiscard]] static GmodPath parse(
			const std::string& item,
			VisVersion visVersion );

		/**
		 * @brief Try to parse a path string using the default GMOD and Locations for the specified VIS version.
		 *
		 * @param item The path string to parse.
		 * @param visVersion The VIS version context.
		 * @param[out] path Output parameter where the parsed GmodPath will be stored if successful.
		 *                  The optional will remain empty if parsing fails.
		 * @return True if parsing succeeded, false otherwise. Does not throw on parsing errors but may throw if GMOD/Locations data is missing.
		 */
		[[nodiscard]] static bool tryParse(
			const std::string& item,
			VisVersion visVersion,
			GmodPath& path );

		/**
		 * @brief Parse a full path string using the default GMOD and Locations for the specified VIS version.
		 *
		 * A full path string explicitly includes all nodes from the root.
		 *
		 * @param pathStr The full path string to parse.
		 * @param visVersion The VIS version context.
		 * @return The parsed GmodPath object.
		 * @throws std::invalid_argument If parsing fails.
		 * @throws std::runtime_error If GMOD/Locations data is missing.
		 */
		[[nodiscard]] static GmodPath parseFullPath(
			const std::string& pathStr,
			VisVersion visVersion );

		/**
		 * @brief Try to parse a full path string using the default GMOD and Locations for the specified VIS version.
		 *
		 * @param pathStr The full path string to parse.
		 * @param visVersion The VIS version context.
		 * @param[out] path Output parameter for the parsed GmodPath if successful.
		 * @return True if parsing succeeded, false otherwise.
		 */
		[[nodiscard]] static bool tryParseFullPath(
			const std::string& pathStr,
			VisVersion visVersion,
			GmodPath& path );

		/**
		 * @brief Try to parse a full path string (provided as string_view) using the default GMOD/Locations.
		 *
		 * @param pathStr The full path string_view to parse.
		 * @param visVersion The VIS version context.
		 * @param[out] path Output parameter for the parsed GmodPath if successful.
		 * @return True if parsing succeeded, false otherwise.
		 */
		[[nodiscard]] static bool tryParseFullPath(
			std::string_view pathStr,
			VisVersion visVersion,
			GmodPath& path );

		//-------------------------------------------------------------------------
		// Static Methods - Parsing (using provided GMOD/Locations)
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a path string using explicitly provided GMOD and Locations objects.
		 *
		 * @param item The path string to parse.
		 * @param gmod The Gmod object containing the model structure.
		 * @param locations The Locations object for resolving location codes.
		 * @return The parsed GmodPath object.
		 * @throws std::invalid_argument If parsing fails.
		 */
		[[nodiscard]] static GmodPath parse(
			const std::string& item,
			const Gmod& gmod,
			const Locations& locations );

		/**
		 * @brief Try to parse a path string using explicitly provided GMOD and Locations objects.
		 *
		 * @param item The path string to parse.
		 * @param gmod The Gmod object.
		 * @param locations The Locations object.
		 * @param[out] path Output parameter for the parsed GmodPath if successful.
		 * @return True if parsing succeeded, false otherwise.
		 */
		[[nodiscard]] static bool tryParse(
			const std::string& item,
			const Gmod& gmod,
			const Locations& locations,
			GmodPath& path );

		/**
		 * @brief Try to parse a full path string (provided as string_view) using explicit GMOD/Locations.
		 *
		 * @param pathStr The full path string_view to parse.
		 * @param gmod The Gmod object.
		 * @param locations The Locations object.
		 * @param[out] path Output parameter for the parsed GmodPath if successful.
		 * @return True if parsing succeeded, false otherwise.
		 */
		[[nodiscard]] static bool tryParseFullPath(
			std::string_view pathStr,
			const Gmod& gmod,
			const Locations& locations,
			GmodPath& path );

		//-------------------------------------------------------------------------
		// Enumerator for Path Traversal
		//-------------------------------------------------------------------------

		/**
		 * @brief Provides forward iteration over the nodes of a GmodPath, yielding depth and node reference.
		 *
		 * Allows iterating through the path from parent nodes to the target node.
		 * Supports use in range-based for loops via its begin() and end() methods returning an Iterator.
		 */
		class Enumerator final
		{
		public:
			/**
			 * @brief Construct a new Enumerator for a given GmodPath.
			 *
			 * @param path The GmodPath to enumerate over.
			 * @param fromDepth Optional zero-based depth index to start enumeration from.
			 *                  If std::nullopt (default), starts from the beginning (depth 0).
			 * @throws std::out_of_range If fromDepth is specified and is invalid for the path.
			 * @throws std::logic_error If constructed for an empty path.
			 */
			explicit Enumerator( const GmodPath& path, std::optional<size_t> fromDepth = std::nullopt );

			/** @brief Deleted copy constructor. */
			Enumerator( const Enumerator& ) = delete;

			/**
			 * @brief Gets the current item in the enumeration.
			 * @return A std::pair containing the current zero-based depth and a const reference_wrapper to the GmodNode.
			 * @throws std::runtime_error If called when the enumerator is not positioned on a valid element (e.g., before first next() or after end).
			 */
			[[nodiscard]] std::pair<size_t, std::reference_wrapper<const GmodNode>> current() const;

			/**
			 * @brief Advances the enumerator to the next node in the path.
			 * @return True if the enumerator was successfully advanced to a valid node,
			 *         false if the end of the path was reached.
			 */
			bool next();

			/**
			 * @brief Resets the enumerator to its initial state (before the first element or at the specified fromDepth).
			 */
			void reset();

			/** @brief Deleted copy assignment operator. */
			Enumerator& operator=( const Enumerator& ) = delete;

			Enumerator( Enumerator&& ) = delete;

			Enumerator& operator=( Enumerator&& ) = delete;

			//-------------------------------------------------------------------------
			// Iterator Implementation (for range-based for loops)
			//-------------------------------------------------------------------------

			/**
			 * @brief Input iterator adapter for the Enumerator.
			 *
			 * Enables the use of GmodPath::Enumerator with range-based for loops and standard algorithms.
			 */
			class Iterator final
			{
			public:
				using iterator_category = std::input_iterator_tag;
				using value_type = std::pair<size_t, std::reference_wrapper<const GmodNode>>;
				using difference_type = std::ptrdiff_t;
				using pointer = const value_type*;
				using reference = const value_type&;

				/**
				 * @brief Default constructor (creates an end iterator).
				 */
				Iterator() = default;

				/**
				 * @brief Dereference operator. Gets the current element.
				 * @return Const reference to the current depth-node pair.
				 */
				[[nodiscard]] reference operator*() const;

				/**
				 * @brief Pre-increment operator. Advances the iterator.
				 * @return Reference to this iterator after advancing.
				 */
				Iterator& operator++();

				/**
				 * @brief Post-increment operator. Advances the iterator.
				 * @return A copy of the iterator before advancing.
				 */
				Iterator operator++( int );

				/**
				 * @brief Equality comparison.
				 * @param other The iterator to compare with.
				 * @return True if both iterators point to the same element or are both end iterators.
				 */
				bool operator==( const Iterator& other ) const;

				/**
				 * @brief Inequality comparison.
				 * @param other The iterator to compare with.
				 * @return True if the iterators point to different elements.
				 */
				bool operator!=( const Iterator& other ) const;

			private:
				friend class Enumerator;
				Iterator( Enumerator* enumerator, bool isEnd );

				/** @brief Pointer to the underlying enumerator (can be nullptr for end iterator). */
				Enumerator* m_enumerator{ nullptr };

				/** @brief Flag indicating if this is the end iterator. */
				bool m_isEnd{ true };

				/** @brief Cached current value for dereferencing. */
				mutable std::optional<value_type> m_cachedValue;

				/** @brief Helper to update the cached value. */
				void updateCache() const;
			};

			/**
			 * @brief Get an iterator pointing to the beginning of the sequence.
			 * @return An Iterator positioned at the first node (or end() if empty or starting past the end).
			 */
			Iterator begin();

			/**
			 * @brief Get an iterator pointing past the end of the sequence.
			 * @return An end Iterator.
			 */
			Iterator end();

		private:
			/** @brief Const reference to the path being enumerated. */
			const GmodPath& m_path;

			/** @brief Current internal index (relative to start index, -1 means before start). */
			int m_currentIndex;

			/** @brief The maximum index (equal to path length). */
			const size_t m_endIndex;

			/** @brief The initial index to start enumeration from. */
			const size_t m_startIndex;
		};

		//-------------------------------------------------------------------------
		// Enumerator Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get an enumerator to traverse the full path from the beginning (depth 0).
		 * @return An Enumerator instance for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] Enumerator fullPath() const;

		/**
		 * @brief Get an enumerator to traverse the path starting from a specific depth.
		 * @param fromDepth The zero-based depth index to start enumeration from.
		 * @return An Enumerator instance for this path, starting at the specified depth.
		 * @throws std::out_of_range If fromDepth is invalid for this path.
		 * @throws std::logic_error If called on an empty path.
		 */
		[[nodiscard]] Enumerator fullPathFrom( size_t fromDepth ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Static Helper Methods (Implementation Details)
		//-------------------------------------------------------------------------

		/**
		 * @brief Internal implementation for parsing a standard path string.
		 *
		 * @param item The path string to parse.
		 * @param gmod The GMOD object to use for resolving nodes.
		 * @param locations The Locations object to use for resolving locations.
		 * @return A GmodParsePathResult (either Ok or Err). Ownership of the result is transferred.
		 */
		[[nodiscard]] static GmodParsePathResult parseInternal(
			const std::string& item,
			const Gmod& gmod,
			const Locations& locations );

		/**
		 * @brief Internal implementation for parsing a full path string.
		 *
		 * @param span The full path string to parse, provided as a string_view.
		 * @param gmod The GMOD object to use for resolving nodes.
		 * @param locations The Locations object to use for resolving locations.
		 * @return A GmodParsePathResult (either Ok or Err). Ownership of the result is transferred.
		 */
		[[nodiscard]] static GmodParsePathResult parseFullPathInternal(
			std::string_view span,
			const Gmod& gmod,
			const Locations& locations );

		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief The VIS version associated with this path, determined during parsing or construction. */
		VisVersion m_visVersion;

		/** @brief The target node at the end of the path. Only valid if length > 0. */
		GmodNode m_node;

		/** @brief Vector storing the parent nodes, ordered from root towards the target. Empty if path length <= 1. */
		std::vector<GmodNode> m_parents;

		/** @brief Internal flag indicating if the path is empty (default constructed). */
		bool m_isEmpty{ true };
	};

	//-------------------------------------------------------------------------
	// GmodIndividualizableSet Declaration (Depends on GmodPath)
	//-------------------------------------------------------------------------

	/**
	 * @brief Represents a set of consecutive individualizable nodes within a GmodPath.
	 *
	 * Allows applying a common location to these nodes and retrieving the modified path.
	 * This class operates on a mutable reference to the original GmodPath.
	 */
	class GmodIndividualizableSet final
	{
	public:
		/**
		 * @brief Construct a new individualizable set for a segment of a GmodPath.
		 *
		 * @param nodeIndices Vector of zero-based indices identifying the consecutive nodes in the set within the path.
		 * @param path A mutable reference to the GmodPath containing these nodes.
		 * @throws std::invalid_argument If the provided indices are invalid, non-consecutive,
		 *                               or if the corresponding nodes are not individualizable or have inconsistent locations.
		 * @throws std::logic_error If the referenced path is empty.
		 */
		GmodIndividualizableSet( const std::vector<size_t>& nodeIndices, GmodPath& path );

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Gets const references to the GmodNodes included in this set.
		 * @return A vector of std::reference_wrapper<const GmodNode> pointing to the nodes.
		 * @throws std::runtime_error If the internal path pointer is null (e.g., after build() was called).
		 */
		[[nodiscard]] std::vector<std::reference_wrapper<const GmodNode>> nodes() const;

		/**
		 * @brief Get the zero-based indices of the nodes belonging to this set within the original path.
		 * @return Const reference to the vector of node indices.
		 */
		[[nodiscard]] const std::vector<size_t>& nodeIndices() const noexcept;

		/**
		 * @brief Get the common location currently associated with this set (if any).
		 *
		 * Initially reflects the location of the first node in the set upon construction.
		 *
		 * @return The common location as std::optional<Location>. Returns std::nullopt if no location is set.
		 * @throws std::runtime_error If the internal path pointer is null (e.g., after build() was called).
		 */
		[[nodiscard]] std::optional<Location> location() const;

		//-------------------------------------------------------------------------
		// Mutators and Operations
		//-------------------------------------------------------------------------

		/**
		 * @brief Sets or removes the location for all nodes within this individualizable set.
		 *
		 * Modifies the underlying GmodPath referenced during construction.
		 *
		 * @param newLocation The location to apply, or std::nullopt to remove the location from these nodes.
		 * @throws std::runtime_error If the internal path pointer is null (e.g., after build() was called).
		 */
		void setLocation( const std::optional<Location>& newLocation );

		/**
		 * @brief Finalizes modifications and returns the modified GmodPath via move semantics.
		 *
		 * After calling build(), this set becomes invalid and should not be used further.
		 * The original GmodPath object referenced during construction is moved from and becomes invalid.
		 *
		 * @return The modified GmodPath object (moved).
		 * @throws std::runtime_error If the internal path pointer is null (e.g., after build() was called).
		 */
		[[nodiscard]] GmodPath build();

		/**
		 * @brief Get a string representation of the individualizable set (for debugging).
		 * @return A string describing the set, including node indices and current location.
		 */
		[[nodiscard]] std::string toString() const;

	private:
		/** @brief Zero-based indices of the nodes in the set within the m_path. */
		std::vector<size_t> m_nodeIndices;

		/** @brief Pointer to the GmodPath being modified. Becomes null after build(). */
		GmodPath* m_path;
	};

	//-------------------------------------------------------------------------
	// Result Class for Parsing Operations (Declaration & Definition)
	//-------------------------------------------------------------------------

	/**
	 * @brief Represents the result of parsing a GmodPath
	 *
	 * Abstract base class for success and error results from path parsing operations.
	 */
	class GmodParsePathResult
	{
	protected:
		/** @brief Protected constructor to prevent direct instantiation */
		GmodParsePathResult() = default;

	public:
		/** @brief Virtual destructor */
		virtual ~GmodParsePathResult() = default;

		/** @brief Delete copy constructor - results shouldn't be copied */
		GmodParsePathResult( const GmodParsePathResult& ) = delete;

		/** @brief Enable move constructor for return-by-value */
		GmodParsePathResult( GmodParsePathResult&& ) = default;

		/** @brief Delete copy assignment operator - results shouldn't be assigned */
		GmodParsePathResult& operator=( const GmodParsePathResult& ) = delete;

		/** @brief Enable move assignment */
		GmodParsePathResult& operator=( GmodParsePathResult&& ) = default;

		/** @brief Success result type */
		class Ok;

		/** @brief Error result type */
		class Err;
	};

	/**
	 * @brief Successful path parsing result
	 */
	class GmodParsePathResult::Ok final : public GmodParsePathResult
	{
	public:
		/** @brief The successfully parsed path */
		GmodPath path;

		/**
		 * @brief Construct a successful result
		 * @param path The parsed path
		 */
		explicit Ok( const GmodPath& path ) = delete;

		/**
		 * @brief Construct a successful result by moving
		 * @param path The parsed path to move
		 */
		explicit Ok( GmodPath&& path );

		/**
		 * @brief Delete copy constructor - results shouldn't be copied
		 */
		Ok( const Ok& ) = delete;

		/**
		 * @brief Delete copy assignment - results shouldn't be assigned
		 */
		Ok& operator=( const Ok& ) = delete;

		/**
		 * @brief Move constructor
		 * @param other The object to move from
		 */
		Ok( Ok&& other ) noexcept;
	};

	/**
	 * @brief Failed path parsing result
	 */
	class GmodParsePathResult::Err final : public GmodParsePathResult
	{
	public:
		/** @brief Error message describing the parsing failure */
		std::string error;

		/**
		 * @brief Construct an error result
		 * @param error The error message
		 */
		explicit Err( const std::string& error );

		/**
		 * @brief Delete copy constructor - results shouldn't be copied
		 */
		Err( const Err& ) = delete;

		/**
		 * @brief Delete copy assignment - results shouldn't be assigned
		 */
		Err& operator=( const Err& ) = delete;

		/**
		 * @brief Move constructor
		 * @param other The object to move from
		 */
		Err( Err&& other ) noexcept;
	};

	/**
	 * @brief Represents the context and state during the GmodPath parsing process.
	 */
	class ParseContext final
	{
	public:
		/**
		 * @brief Construct a new parse context.
		 * @param initialParts Queue of path node components to process.
		 */
		explicit ParseContext( std::queue<PathNode> initialParts );

		ParseContext( const ParseContext& ) = delete;
		ParseContext& operator=( const ParseContext& ) = delete;

		ParseContext( ParseContext&& ) noexcept = default;
		ParseContext& operator=( ParseContext&& ) noexcept = default;

		/** @brief Queue of remaining path node components to process. */
		std::queue<PathNode> parts;

		/** @brief The current node component being searched for. */
		PathNode toFind;

		/** @brief Mapping of node codes encountered so far to their resolved locations. */
		std::unordered_map<std::string, Location> locations;

		std::optional<GmodPath> path;
	};
}
