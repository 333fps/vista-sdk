#pragma once

#include "GmodNode.h"

namespace dnv::vista::sdk
{
	class Gmod;
	class GmodParsePathResult;
	class GmodIndividualizableSet;
	class Locations;

	enum class VisVersion;

	/**
	 * @brief Record struct that represents a location set visitor
	 *
	 */
	struct LocationSetsVisitor final
	{
		size_t m_currentParentStart;

		LocationSetsVisitor() = default;

		/**
		 * @brief Visit a node in a GmodPath to identify individualizable sets
		 *
		 * @param node The current node being visited in the path
		 * @param i The index/depth of the current node in the path (0-based)
		 * @param parents Collection of parent nodes in the path, from root to leaf
		 * @param target The target node at the end of the path
		 * @return Optional tuple containing:
		 *         - size_t: Starting index of individualizable set
		 *         - size_t: Ending index of individualizable set
		 *         - Optional<Location>: Associated location if present
		 *         Returns std::nullopt if the current node doesn't form part of an individualizable set
		 */
		std::optional<std::tuple<size_t, size_t, std::optional<Location>>> visit(
			const GmodNode& node,
			size_t i,
			const std::vector<GmodNode>& parents,
			const GmodNode& target );
	};

	/**
	 * @brief Represents a path in the Generic Product Model (GMOD)
	 *
	 * A GmodPath consists of a sequence of parent nodes and a target node,
	 * forming a hierarchical path through the GMOD structure as defined in ISO 19848.
	 * Each node in the path may have an optional location.
	 */
	class GmodPath final
	{
	public:
		/**
		 * @brief Default constructor for use in containers only
		 * @warning Creates an invalid path, must be assigned before use
		 */
		GmodPath();

		/**
		 * @brief Construct a path from parent nodes and target node
		 *
		 * @param parents Vector of parent nodes in the path (from root to leaf)
		 * @param node The target node at the end of the path
		 * @param skipVerify If true, skip validation of parent-child relationships
		 * @throws std::invalid_argument If skipVerify is false and the path is invalid
		 */
		GmodPath( std::vector<GmodNode> parents, GmodNode node, bool skipVerify = true );

		/**
		 * @brief Equality operator
		 * @param other The path to compare with
		 * @return True if paths are equal
		 */
		bool operator==( const GmodPath& other ) const;

		/**
		 * @brief Inequality operator
		 * @param other The path to compare with
		 * @return True if paths are not equal
		 */
		bool operator!=( const GmodPath& other ) const;

		/**
		 * @brief Parent nodes in the path
		 * @return Const reference to the vector of parent nodes
		 */
		const std::vector<GmodNode>& parents() const;

		/**
		 * @brief Get the VIS version of this path
		 * @return The VIS version enum value
		 */
		VisVersion visVersion() const;

		/**
		 * @brief Get the target node at the end of the path
		 * @return Const reference to the target node
		 */
		const GmodNode& node() const;

		/**
		 * @brief Setter for the target node
		 * @param node New target node value
		 */
		void setNode( const GmodNode& node );

		/**
		 * @brief Get the total length of the path (parents + target node)
		 * @return The number of nodes in the path
		 */
		size_t length() const;

		/**
		 * @brief Check if the path's target node is mappable
		 * @return True if the target node can be mapped to other nodes
		 */
		bool isMappable() const;

		/**
		 * @brief Access a node at a specific depth in the path
		 *
		 * @param depth Zero-based depth index (0 is root, Length-1 is target node)
		 * @return Const reference to the node at specified depth
		 * @throws std::out_of_range If depth is out of valid range
		 */
		const GmodNode& operator[]( size_t depth ) const;

		/**
		 * @brief Access a node at a specific depth in the path (non-const version)
		 *
		 * @param depth Zero-based depth index (0 is root, Length-1 is target node)
		 * @return Reference to the node at specified depth
		 * @throws std::out_of_range If depth is out of valid range
		 */
		GmodNode& operator[]( size_t depth );

		/**
		 * @brief Create a copy of this path with all locations removed
		 * @return A new GmodPath with the same nodes but no locations
		 */
		GmodPath withoutLocations() const;

		/**
		 * @brief Convert the path to a string representation
		 *
		 * Format: code1/code2/.../targetCode (with optional location suffixes)
		 *
		 * @return String representation of the path
		 */
		std::string toString() const;

		/**
		 * @brief Write string representation to a string stream
		 *
		 * @param builder The string stream to write to
		 * @param separator The separator character to use between nodes (default: '/')
		 */
		void toString( std::stringstream& builder, char separator = '/' ) const;

		/**
		 * @brief Convert to a full path string representation including all nodes
		 * @return String representation of the full path
		 */
		std::string toFullPathString() const;

		/**
		 * @brief Write full path string representation to a string stream
		 * @param builder The string stream to write to
		 */
		void toFullPathString( std::stringstream& builder ) const;

		/**
		 * @brief Get a detailed string representation for debugging
		 * @return Detailed string representation with node information
		 */
		std::string toStringDump() const;

		/**
		 * @brief Write detailed debug representation to a string stream
		 * @param builder The string stream to write to
		 */
		void toStringDump( std::stringstream& builder ) const;

		/**
		 * @brief Equality comparison
		 *
		 * @param other The path to compare with
		 * @return True if paths are equal
		 */
		bool equals( const GmodPath& other ) const;

		/**
		 * @brief Calculate hash code for this path
		 * @return Hash code value
		 */
		size_t hashCode() const;

		/**
		 * @brief Get normal assignment name for a node at specific depth
		 *
		 * @param nodeDepth The depth of the node to get the name for
		 * @return The normal assignment name if present, or std::nullopt
		 */
		std::optional<std::string> normalAssignmentName( size_t nodeDepth ) const;

		/**
		 * @brief Get all common names in the path
		 * @return Vector of pairs with (depth, common_name)
		 */
		std::vector<std::pair<size_t, std::string>> commonNames() const;

		/**
		 * @brief Check if the path contains any individualizable nodes
		 * @return True if at least one node in the path is individualizable
		 */
		bool isIndividualizable() const;

		/**
		 * @brief Get all individualizable sets in this path
		 *
		 * An individualizable set is a group of nodes that can be assigned
		 * a unique instance identifier.
		 *
		 * @return Vector of individualizable sets
		 */
		std::vector<GmodIndividualizableSet> individualizableSets() const;

		/**
		 * @brief Validate a path structure
		 *
		 * @param parents Vector of parent nodes
		 * @param node Target node
		 * @return True if the path is valid
		 */
		static bool isValid( const std::vector<GmodNode>& parents, const GmodNode& node );

		/**
		 * @brief Validate a path structure and get the location of any invalid link
		 *
		 * @param parents Vector of parent nodes
		 * @param node Target node
		 * @param[out] missingLinkAt Index of the invalid parent-child link if any
		 * @return True if the path is valid
		 */
		static bool isValid( const std::vector<GmodNode>& parents, const GmodNode& node, int& missingLinkAt );

		/**
		 * @brief Parse a path string with a specific VIS version
		 *
		 * @param item The path string to parse
		 * @param visVersion The VIS version to use for parsing
		 * @return The parsed GmodPath
		 * @throws std::invalid_argument If parsing fails
		 */
		static GmodPath parse( const std::string& item, VisVersion visVersion );

		/**
		 * @brief Try to parse a path string with a specific VIS version
		 *
		 * @param item The path string to parse
		 * @param visVersion The VIS version to use for parsing
		 * @param[out] path Output parameter to store the parsed path if successful
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParse( const std::string& item, VisVersion visVersion, std::optional<GmodPath>& path );

		/**
		 * @brief Parse a path string using specific GMOD and Locations objects
		 *
		 * @param item The path string to parse
		 * @param gmod The GMOD object to use for resolving nodes
		 * @param locations The Locations object to use for resolving locations
		 * @return The parsed GmodPath
		 * @throws std::invalid_argument If parsing fails
		 */
		static GmodPath parse( const std::string& item, const Gmod& gmod, const Locations& locations );

		/**
		 * @brief Try to parse a path string using specific GMOD and Locations objects
		 *
		 * @param item The path string to parse
		 * @param gmod The GMOD object to use for resolving nodes
		 * @param locations The Locations object to use for resolving locations
		 * @param[out] path Output parameter to store the parsed path if successful
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParse( const std::string& item, const Gmod& gmod, const Locations& locations, GmodPath& path );

		/**
		 * @brief Parse a full path string with a specific VIS version
		 *
		 * @param pathStr The full path string to parse
		 * @param visVersion The VIS version to use for parsing
		 * @return The parsed GmodPath
		 * @throws std::invalid_argument If parsing fails
		 */
		static GmodPath parseFullPath( const std::string& pathStr, VisVersion visVersion );

		/**
		 * @brief Try to parse a full path string with a specific VIS version
		 *
		 * @param pathStr The full path string to parse
		 * @param visVersion The VIS version to use for parsing
		 * @param[out] path Output parameter to store the parsed path if successful
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParseFullPath( const std::string& pathStr, VisVersion visVersion, GmodPath& path );

		/**
		 * @brief Try to parse a full path string with a specific VIS version
		 *
		 * @param pathStr The full path string to parse as string_view
		 * @param visVersion The VIS version to use for parsing
		 * @param[out] path Output parameter to store the parsed path if successful
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParseFullPath( std::string_view pathStr, VisVersion visVersion, GmodPath& path );

		/**
		 * @brief Try to parse a full path string using specific GMOD and Locations objects
		 *
		 * @param pathStr The full path string to parse as string_view
		 * @param gmod The GMOD object to use for resolving nodes
		 * @param locations The Locations object to use for resolving locations
		 * @param[out] path Output parameter to store the parsed path if successful
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParseFullPath( std::string_view pathStr, const Gmod& gmod, const Locations& locations, GmodPath& path );

		/**
		 * @brief Enumerator for traversing the GmodPath
		 *
		 * Provides sequential access to the nodes in the path with their depths.
		 */
		class Enumerator final
		{
		public:
			/**
			 * @brief Construct a new Enumerator
			 *
			 * @param path The path to enumerate
			 * @param fromDepth Optional starting depth (default is beginning of path)
			 */
			Enumerator( const GmodPath& path, std::optional<size_t> fromDepth = std::nullopt );

			/**
			 * @brief Copy assignment operator that ensures proper path compatibility
			 *
			 * @param other The enumerator to copy from
			 * @return Reference to this object
			 * @throws std::invalid_argument If paths are different
			 */
			Enumerator& operator=( const Enumerator& other );

			/**
			 * @brief Current item in the enumeration
			 * @return Pair of (depth, node_reference)
			 */
			std::pair<size_t, std::reference_wrapper<const GmodNode>> current() const;

			/**
			 * @brief Move to the next node in the path
			 * @return True if moved to a valid node, false if reached the end
			 */
			bool next();

			/**
			 * @brief Reset the enumerator
			 */
			void reset();

			/**
			 * @brief Iterator adapter for the Enumerator
			 *
			 * Enables use with range-based for loops and standard algorithms.
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
				 * @brief Construct a new Iterator
				 *
				 * @param enumerator The enumerator to iterate
				 * @param end True if this is the end iterator
				 */
				Iterator( Enumerator& enumerator, bool end );

				/**
				 * @brief Copy constructor
				 *
				 * @param other The iterator to copy from
				 */
				Iterator( const Iterator& other );

				/**
				 * @brief Pre-increment operator
				 * @return Reference to this iterator after advancing
				 */
				Iterator&
				operator++();

				/**
				 * @brief Post-increment operator
				 * @return Iterator before advancing
				 */
				Iterator operator++( int );

				/**
				 * @brief Equality comparison
				 * @param other Iterator to compare with
				 * @return True if iterators are equal
				 */
				bool operator==( const Iterator& other ) const;

				/**
				 * @brief Inequality comparison
				 * @param other Iterator to compare with
				 * @return True if iterators are not equal
				 */
				bool operator!=( const Iterator& other ) const;

				/**
				 * @brief Dereference operator
				 * @return The current depth-node pair
				 */
				reference operator*() const;

				/**
				 * @brief Copy assignment operator
				 *
				 * @param other The iterator to copy from
				 * @return Reference to this iterator
				 * @throws std::invalid_argument If iterators don't reference the same enumerator
				 */
				Iterator& operator=( const Iterator& other );

			private:
				/** @brief Reference to the underlying enumerator */
				Enumerator& m_enumerator;

				/** @brief Flag indicating if this is the end iterator */
				bool m_end;

				/** @brief Cached current value */
				mutable std::optional<value_type> m_current;

				/**
				 * @brief Cached value for deferred evaluation during iteration
				 *
				 * This member caches the current iterator value to optimize multiple
				 * consecutive dereferences without redundant lookups. Being mutable allows
				 * updating the cache even in const methods like operator*().
				 *
				 * The cached value consists of:
				 * - size_t: The depth/position in the path
				 * - reference_wrapper: Non-owning reference to the GmodNode at that depth
				 */
				mutable std::optional<std::pair<size_t, std::reference_wrapper<const GmodNode>>> m_cachedValue;
			};

			/**
			 * @brief Get iterator to the beginning
			 * @return Iterator positioned at the first node
			 */
			Iterator begin();

			/**
			 * @brief Get iterator to the end
			 * @return Iterator positioned after the last node
			 */
			Iterator end();

		private:
			/** @brief Reference to the path being enumerated */
			const GmodPath& m_path;

			/** @brief Current position index in the path */
			size_t m_current;

			/** @brief Current depth value */
			size_t m_depth;

			/** @brief Initial depth to start from */
			std::optional<size_t> m_fromDepth;
		};

		/**
		 * @brief Get an enumerator for the full path
		 * @return Enumerator starting at the beginning of the path
		 */
		Enumerator fullPath() const;

		/**
		 * @brief Get an enumerator starting at the specified depth
		 * @param fromDepth The depth to start at
		 * @return Enumerator starting at the specified depth
		 */
		Enumerator fullPathFrom( size_t fromDepth ) const;

	private:
		/**
		 * @brief Internal method to parse a path string
		 *
		 * @param item The path string to parse
		 * @param gmod The GMOD object to use for resolving nodes
		 * @param locations The Locations object to use for resolving locations
		 * @return A GmodParsePathResult containing either a path or an error
		 */
		static GmodParsePathResult parseInternal( const std::string& item, const Gmod& gmod, const Locations& locations );

		/**
		 * @brief Internal method to parse a full path string
		 *
		 * @param span The full path string to parse as string_view
		 * @param gmod The GMOD object to use for resolving nodes
		 * @param locations The Locations object to use for resolving locations
		 * @return A GmodParsePathResult containing either a path or an error
		 */
		static GmodParsePathResult parseFullPathInternal( std::string_view span, const Gmod& gmod, const Locations& locations );

	private:
		/** @brief VIS version associated with this path */
		VisVersion m_visVersion;

		/** @brief Target node at the end of the path */
		GmodNode m_node;

		/** @brief Vector of parent nodes in the path (from root to target) */
		std::vector<GmodNode> m_parents;
	};

	/**
	 * @brief Represents a single node in the path
	 *
	 * Contains the node's code and optional location.
	 */
	struct PathNode final
	{
		/** @brief Code identifying the node */
		std::string code;

		/** @brief Optional location associated with the node */
		std::optional<Location> location;

		/**
		 * @brief Construct a new PathNode
		 *
		 * @param code The node code (default: empty string)
		 * @param location The optional location (default: no location)
		 */
		PathNode( const std::string& code = "", const std::optional<Location>& location = std::nullopt );
	};

	/**
	 * @brief Represents the context for parsing a GmodPath
	 *
	 * Contains the state needed during path parsing operations.
	 */
	class ParseContext final
	{
	public:
		/** @brief Queue of remaining path nodes to process */
		std::queue<PathNode> parts;

		/** @brief Current node to find */
		PathNode toFind;

		/** @brief Mapping of node codes to their locations */
		std::unordered_map<std::string, Location> locations;

		/** @brief The resulting path if parsing succeeds */
		std::optional<GmodPath> path;

		/**
		 * @brief Construct a new parse context
		 * @param parts Queue of path nodes to process
		 */
		ParseContext( std::queue<PathNode> parts );
	};

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
		explicit Ok( const GmodPath& path );

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
	 * @brief Represents a set of individualizable nodes in a GmodPath
	 *
	 * An individualizable set is a group of nodes that can be assigned
	 * a unique instance identifier and share the same location.
	 */
	class GmodIndividualizableSet final
	{
	public:
		/**
		 * @brief Construct a new individualizable set
		 *
		 * @param nodes Vector of node indices in the path
		 * @param path Reference to the path containing the nodes
		 * @throws std::invalid_argument If constraints are not satisfied
		 */
		GmodIndividualizableSet( const std::vector<size_t>& nodes, GmodPath& path );

		/**
		 * @brief Get the actual nodes in this set
		 * @return Vector of nodes
		 */
		std::vector<GmodNode> nodes() const;

		/**
		 * @brief Get the indices of nodes in this set
		 * @return Const reference to the vector of indices
		 */
		const std::vector<size_t>& nodeIndices() const;

		/**
		 * @brief Get the location associated with this set
		 * @return The location if present, or std::nullopt
		 */
		std::optional<Location> location() const;

		/**
		 * @brief Set the location for all nodes in this set
		 * @param location The location to set, or std::nullopt to remove location
		 */
		void setLocation( const std::optional<Location>& location );

		/**
		 * @brief Build and return the modified path
		 * @return The path with any modifications applied
		 * @throws std::runtime_error If the set was already built
		 */
		GmodPath build();

		/**
		 * @brief Convert to string representation
		 * @return String representation of the individualizable set
		 */
		std::string toString() const;

	private:
		/** @brief Indices of nodes in the individualizable set */
		std::vector<size_t> m_nodes;

		/** @brief Pointer to the path containing the nodes */
		GmodPath* m_path;
	};
}
