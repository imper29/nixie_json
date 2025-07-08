/*license at bottom of file*/
#ifndef FILE_NIXIE_ENCODING_JSON
#define FILE_NIXIE_ENCODING_JSON
#include "cstdint"
namespace nix {
	/// <summary>
	/// Defines the json node types. Note that zero is reserved for invalid nodes.
	/// </summary>
	enum json_type : uint8_t {
		json_type_invalid = 0u,
		json_type_boolean = 1u,
		json_type_number = 2u,
		json_type_string = 3u,
		json_type_object = 4u,
		json_type_array = 5u,
		json_type_null = 6u,
	};
	/// <summary>
	/// Defines the json formats.
	/// </summary>
	enum json_format : uint8_t {
		/// <summary>
		/// Json text with pretty formatting.
		/// </summary>
		json_format_pretty = 0u,
		/// <summary>
		/// Json text without white spaces or new lines.
		/// </summary>
		json_format_dense = 1u,
	};
	/// <summary>
	/// Defines the type used for json booleans.
	/// </summary>
	using json_boolean = bool;
	/// <summary>
	/// Defines the type used for json numbers.
	/// </summary>
	using json_number = double;
	/// <summary>
	/// Defines the type used for json strings. Note that json strings are only valid until the json heap is modified or destroyed.
	/// </summary>
	using json_string = const char*;
	/// <summary>
	/// Defines the type used for json characters.
	/// </summary>
	using json_char = char;
	/// <summary>
	/// Defines the type used for json nodes. Note that zero is reserved for invalid nodes.
	/// </summary>
	using json_node = uint32_t;
	/// <summary>
	/// Represents storage for json objects.
	/// </summary>
	struct json_heap final {
		/// <summary>
		/// True if errors should cause the program to be aborted after the error is reported to the developer.
		/// False if you're a god like programmer who doesn't make mistakes.
		/// </summary>
		static constexpr bool validation = _DEBUG;

		/// <summary>
		/// Deconstructs the heap, all nodes, and all strings.
		/// </summary>
		~json_heap() noexcept;
		/// <summary>
		/// Constructs the heap.
		/// </summary>
		json_heap() noexcept;
		/// <summary>
		/// Constructs the heap.
		/// </summary>
		/// <param name="source">The heap to copy.</param>
		json_heap(const json_heap& source);
		/// <summary>
		/// Constructs the heap.
		/// </summary>
		/// <param name="source">The heap to move.</param>
		json_heap(json_heap&& source) noexcept;

		/// <summary>
		/// Assigns the heap.
		/// </summary>
		/// <param name="source">The heap to copy.</param>
		/// <returns>The new heap.</returns>
		json_heap& operator=(const json_heap& source);
		/// <summary>
		/// Assigns the heap.
		/// </summary>
		/// <param name="source">The heap to move.</param>
		/// <returns>The new heap.</returns>
		json_heap& operator=(json_heap&& source) noexcept;

		/// <summary>
		/// Loads the json.
		/// </summary>
		/// <param name="source">The json source.</param>
		/// <param name="format">The json format.</param>
		/// <returns>The json destination. Zero if something went wrong.</returns>
		json_node load_file(json_string source, json_format format = json_format_pretty) noexcept;
		/// <summary>
		/// Loads the json.
		/// </summary>
		/// <param name="source">The json source.</param>
		/// <param name="format">The json format.</param>
		/// <returns>The json destination. Zero if something went wrong.</returns>
		json_node load_string(json_string source, json_format format = json_format_pretty) noexcept;
		/// <summary>
		/// Saves the json.
		/// </summary>
		/// <param name="source">The json source.</param>
		/// <param name="format">The json format.</param>
		/// <returns>The json destination. Zero if something went wrong.</returns>
		json_string save_string(json_node source, json_format format = json_format_pretty) noexcept;

		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="object">The new node's parent.</param>
		/// <param name="name">The new node's name.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_boolean(json_node object, json_string name, json_boolean value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="array">The new node's parent.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_boolean(json_node array, json_boolean value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="value">The node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_boolean(json_boolean value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="object">The new node's parent.</param>
		/// <param name="name">The new node's name.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_number(json_node object, json_string name, json_number value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="array">The new node's parent.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_number(json_node array, json_number value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="value">The node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_number(json_number value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="object">The new node's parent.</param>
		/// <param name="name">The new node's name.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_string(json_node object, json_string name, json_string value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="array">The new node's parent.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_string(json_node array, json_string value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="value">The node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_string(json_string value) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="object">The new node's parent.</param>
		/// <param name="name">The new node's name.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_object(json_node object, json_string name) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="array">The new node's parent.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_object(json_node array) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="value">The node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_object() noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="object">The new node's parent.</param>
		/// <param name="name">The new node's name.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_array(json_node object, json_string name) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="array">The new node's parent.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_array(json_node array) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="value">The node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_array() noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="object">The new node's parent.</param>
		/// <param name="name">The new node's name.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_null(json_node object, json_string name) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="array">The new node's parent.</param>
		/// <param name="value">The new node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_null(json_node array) noexcept;
		/// <summary>
		/// Makes a node.
		/// </summary>
		/// <param name="value">The node's value.</param>
		/// <returns>The node that was made. Zero if something went wrong.</returns>
		json_node new_null() noexcept;

		/// <summary>
		/// Returns the node's child with the specified name.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <param name="name">The child name.</param>
		/// <returns>The node's child with the specified name. Zero if the child could not be found or if something went wrong.</returns>
		json_node get_child(json_node node, const json_char* name) const noexcept;
		/// <summary>
		/// Returns the node's first child.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The node's first child. Zero if the child could not be found or if something went wrong.</returns>
		json_node get_first(json_node node) const noexcept;
		/// <summary>
		/// Returns the node's next child.
		/// </summary>
		/// <param name="child">The node's child.</param>
		/// <returns>The node's next child. Zero if the child could not be found or if something went wrong.</returns>
		json_node get_next(json_node child) const noexcept;
		/// <summary>
		/// Returns the node's last child.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The node's last child. Zero if the child could not be found or if something went wrong.</returns>
		json_node get_last(json_node node) const noexcept;
		/// <summary>
		/// Returns the node's type.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The node's type.</returns>
		json_type get_type(json_node node) const noexcept;
		/// <summary>
		/// Returns the node's name.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The node's name. The word 'invalid' if the node is invalid. The word 'nameless' if the node is nameless.</returns>
		json_string get_name(json_node node) const noexcept;

		/// <summary>
		/// Returns the value of the node.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <param name="fallback">The fallback value.</param>
		/// <returns>The value of the node. The fallback value if something went wrong.</returns>
		json_boolean to_boolean(json_node node, json_boolean fallback) const noexcept;
		/// <summary>
		/// Returns the value of the node.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The value of the node.</returns>
		json_boolean to_boolean(json_node node) const;
		/// <summary>
		/// Returns the value of the node.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <param name="fallback">The fallback value.</param>
		/// <returns>The value of the node. The fallback value if something went wrong.</returns>
		json_number to_number(json_node node, json_number fallback) const noexcept;
		/// <summary>
		/// Returns the value of the node.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The value of the node.</returns>
		json_number to_number(json_node node) const;
		/// <summary>
		/// Returns the value of the node.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <param name="fallback">The fallback value.</param>
		/// <returns>The value of the node. The fallback value if something went wrong.</returns>
		json_string to_string(json_node node, json_string fallback) const noexcept;
		/// <summary>
		/// Returns the value of the node.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>The value of the node.</returns>
		json_string to_string(json_node node) const;

		/// <summary>
		/// Returns true if the node is invalid.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is invalid.</returns>
		bool is_invalid(json_node node) const noexcept;
		/// <summary>
		/// Returns true if the node is a boolean.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is a boolean.</returns>
		bool is_boolean(json_node node) const noexcept;
		/// <summary>
		/// Returns true if the node is a number.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is a number.</returns>
		bool is_number(json_node node) const noexcept;
		/// <summary>
		/// Returns true if the node is a string.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is a string.</returns>
		bool is_string(json_node node) const noexcept;
		/// <summary>
		/// Returns true if the node is an object.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is an object.</returns>
		bool is_object(json_node node) const noexcept;
		/// <summary>
		/// Returns true if the node is an array.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is an array.</returns>
		bool is_array(json_node node) const noexcept;
		/// <summary>
		/// Returns true if the node is null.
		/// </summary>
		/// <param name="node">The node.</param>
		/// <returns>True if the node is null.</returns>
		bool is_null(json_node node) const noexcept;

		/// <summary>
		/// Clears all nodes and strings.
		/// </summary>
		void clear() noexcept;
		/// <summary>
		/// Preallocates memory for chars to reduce allocation overhead when creating json strings. Does nothing if the requested memory is already reserved. Requests enough memory to double the memory usage.
		/// </summary>
		/// <returns>True if successfull. False if something went wrong.</returns>
		bool reserve_chars() noexcept;
		/// <summary>
		/// Preallocates memory for chars to reduce allocation overhead when creating json strings. Does nothing if the requested memory is already reserved.
		/// </summary>
		/// <param name="capacity">The char capacity.</param>
		/// <returns>True if successfull. False if something went wrong.</returns>
		bool reserve_chars(uint32_t capacity) noexcept;
		/// <summary>
		/// Preallocates memory for nodes to reduce allocation overhead when creating json nodes. Does nothing if the requested memory is already reserved. Requests enough memory to double the memory usage.
		/// </summary>
		/// <returns>True if successfull. False if something went wrong.</returns>
		bool reserve_nodes() noexcept;
		/// <summary>
		/// Preallocates memory for nodes to reduce allocation overhead when creating json nodes. Does nothing if the requested memory is already reserved.
		/// </summary>
		/// <param name="capacity">The node capacity.</param>
		/// <returns>True if successfull. False if something went wrong.</returns>
		bool reserve_nodes(uint32_t capacity) noexcept;

	private:
		struct json_utils;
		struct json_value {
			union {
				struct { json_boolean value; } boolean;
				struct { json_number value; } number;
				struct { json_node value; } string;
				struct { json_node first, last; } object_or_array;
			};
			json_node parent;
			json_node name;
			json_node next;
			json_type type;
		};

		uint32_t m_values_capacity;
		uint32_t m_values_count;
		json_value* m_values;
		uint32_t m_chars_capacity;
		uint32_t m_chars_count;
		json_char* m_chars;
		size_t m_dump_capacity;
		size_t m_dump_count;
		json_char* m_dump;
	};
}
#endif
/*
MIT License

Copyright (c) 2025 Marten Dittmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
