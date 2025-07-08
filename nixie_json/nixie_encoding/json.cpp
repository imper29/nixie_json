/*license at bottom of file*/
#include "json.hpp"
#include "sstream"
#include "fstream"

struct nix::json_heap::json_utils {
	inline static json_node alloc_string(json_heap& h, json_string string) noexcept {
		size_t lengthl = strlen(string) + 1;
		if (lengthl > std::numeric_limits<uint32_t>::max())
			return 0;
		uint32_t length = uint32_t(lengthl);
		if (!h.reserve_chars(length + 1))
			return 0;
		if (!h.m_chars_count)
			h.m_chars_count = 1;
		memcpy(h.m_chars + h.m_chars_count, string, length);
		json_node start = h.m_chars_count;
		h.m_chars_count += length;
		return start;
	}
	inline static json_node alloc_node(json_heap& h, json_node object, json_node name) noexcept {
		if (!h.reserve_nodes(1))
			return 0;
		json_node node = ++h.m_values_count;
		h.m_values[node].parent = object;
		h.m_values[node].name = name;
		h.m_values[node].next = 0;
		json_node last = h.get_last(object);
		if (last)
			h.m_values[last].next = node;
		else
			h.m_values[object].object_or_array.first = node;
		h.m_values[object].object_or_array.last = node;
		return node;
	}
	inline static json_node alloc_node(json_heap& h, json_node array) noexcept {
		if (!h.reserve_nodes(1))
			return 0;
		json_node node = ++h.m_values_count;
		h.m_values[node].parent = array;
		h.m_values[node].name = 0;
		h.m_values[node].next = 0;
		json_node last = h.get_last(array);
		if (last)
			h.m_values[last].next = node;
		else
			h.m_values[array].object_or_array.first = node;
		h.m_values[array].object_or_array.last = node;
		return node;
	}
	inline static json_node alloc_node(json_heap& h) noexcept {
		if (!h.reserve_nodes(1))
			return 0;
		json_node node = ++h.m_values_count;
		h.m_values[node].parent = 0;
		h.m_values[node].name = 0;
		h.m_values[node].next = 0;
		return node;
	}

	inline static void dump_clear(json_heap& h) noexcept {
		h.m_dump_count = 0;
	}
	inline static bool dump_pad(json_heap& h, size_t d) {
		size_t d2 = d + d;
		if (!dump_grow(h, d2))
			return false;
		memset(h.m_dump + h.m_dump_count, ' ', d2);
		h.m_dump_count += d2;
		return true;
	}
	inline static bool dump_grow(json_heap& h, size_t l) noexcept {
		size_t capacity = h.m_dump_count + l;
		if (capacity > h.m_dump_capacity) {
			capacity = capacity + capacity;
			json_char* dump = (json_char*)malloc(capacity * sizeof(json_char));
			if (!dump)
				return false;
			memcpy(dump, h.m_dump, h.m_dump_count * sizeof(json_char));
			free(h.m_dump);
			h.m_dump = dump;
			h.m_dump_capacity = capacity;
		}
		return true;
	}
	inline static bool dump_char(json_heap& h, json_char c) noexcept {
		if (!dump_grow(h, 1))
			return false;
		h.m_dump[h.m_dump_count++] = c;
		return true;
	}
	inline static bool dump_number(json_heap& h, json_number n) noexcept {
		int r = snprintf(h.m_dump + h.m_dump_count, h.m_dump_capacity - h.m_dump_count, "%f", n);
		if (r >= (h.m_dump_capacity - h.m_dump_count)) {
			if (!dump_grow(h, r))
				return false;
			snprintf(h.m_dump + h.m_dump_count, h.m_dump_capacity - h.m_dump_count, "%f", n);
		}
		json_string s = h.m_dump + h.m_dump_count - 1;
		while (s[r] == '0') --r;
		if (s[r] == '.') --r;
		h.m_dump_count += r;
		return true;
	}
	inline static bool dump_string(json_heap& h, json_string s) noexcept {
		size_t l = strlen(s);
		if (!dump_grow(h, l))
			return false;
		memcpy(h.m_dump + h.m_dump_count, s, l);
		h.m_dump_count += l;
		return true;
	}
	inline static bool dump_string(json_heap& h, json_string& s, json_char e) noexcept {
		size_t l = 0;
		while (s[l] && s[l] != e) ++l;
		if (s[l] != e)
			return false;
		if (!dump_grow(h, l))
			return false;
		memcpy(h.m_dump + h.m_dump_count, s, l);
		h.m_dump_count += l;
		++s += l;
		return true;
	}

	inline static bool is_whitespace(json_char character) noexcept {
		return character == ' ' || character == '\t' || character == '\n';
	}
	inline static bool do_whitespace(json_string& source) noexcept {
		size_t s = 0;
		while (source[s] && is_whitespace(source[s])) ++s;
		source += s;
		return *source;
	}

	inline static nix::json_node load_pretty(json_heap& heap, json_string& source) noexcept {
		do_whitespace(source);
		//boolean
		if (strncmp(source, "true", 4) == 0) {
			source += 4;
			return heap.new_boolean(true);
		}
		if (strncmp(source, "false", 5) == 0) {
			source += 5;
			return heap.new_boolean(false);
		}
		//number
		char* endptr;
		json_number number = strtod(source, &endptr);
		if (source != endptr) {
			source = endptr;
			return heap.new_number(number);
		}
		//string
		if (*source == '"') {
			dump_clear(heap);
			if (!dump_string(heap, ++source, '"')) return 0;
			if (!dump_char(heap, '\0')) return 0;
			return heap.new_string(heap.m_dump);
		}
		//object
		if (*source == '{') {
			json_node node = heap.new_object();
			json_node last = 0;
			if (!do_whitespace(++source)) return 0;
			if (*source == '}') {
				++source;
				return node;
			}
			while (*source == '"') {
				dump_clear(heap);
				if (!do_whitespace(++source)) return 0;
				if (!dump_string(heap, source, '"')) return 0;
				if (!dump_char(heap, '\0')) return 0;
				if (*source++ != ':') return 0;
				json_node name = alloc_string(heap, heap.m_dump);
				json_node child = load_pretty(heap, source);
				if (!child) return 0;
				heap.m_values[child].name = name;
				heap.m_values[child].parent = node;
				heap.m_values[node].object_or_array.last = child;
				if (last)
					heap.m_values[last].next = child;
				else
					heap.m_values[node].object_or_array.first = child;
				last = child;
				if (!do_whitespace(source)) return 0;
				json_char character = *source++;
				if (character == '}') return node;
				if (character != ',') return 0;
				if (!do_whitespace(source)) return 0;
			}
		}
		//array
		if (*source == '[') {
			json_node node = heap.new_array();
			json_node last = 0;
			if (!do_whitespace(++source)) return 0;
			if (*source == ']') {
				++source;
				return node;
			}
			while (true) {
				json_node child = load_pretty(heap, source);
				if (!child) return 0;
				heap.m_values[child].parent = node;
				heap.m_values[node].object_or_array.last = child;
				if (last)
					heap.m_values[last].next = child;
				else
					heap.m_values[node].object_or_array.first = child;
				last = child;
				if (!do_whitespace(source)) return 0;
				json_char character = *source++;
				if (character == ']') return node;
				if (character != ',') return 0;
			}
		}
		//null
		if (strncmp(source, "null", 4) == 0) {
			source += 4;
			return heap.new_null();
		}
		return 0;
	}
	inline static nix::json_node load_dense(json_heap& heap, json_string& source) noexcept {
		//boolean
		if (strncmp(source, "true", 4) == 0) {
			source += 4;
			return heap.new_boolean(true);
		}
		if (strncmp(source, "false", 5) == 0) {
			source += 5;
			return heap.new_boolean(false);
		}
		//number
		char* endptr;
		json_number number = strtod(source, &endptr);
		if (source != endptr) {
			source = endptr;
			return heap.new_number(number);
		}
		//string
		if (*source == '"') {
			dump_clear(heap);
			if (!dump_string(heap, ++source, '"')) return 0;
			if (!dump_char(heap, '\0')) return 0;
			return heap.new_string(heap.m_dump);
		}
		//object
		if (*source == '{') {
			json_node node = heap.new_object();
			json_node last = 0;
			if (*++source == '}') {
				++source;
				return node;
			}
			while (*source == '"') {
				dump_clear(heap);
				if (!dump_string(heap, ++source, '"')) return 0;
				if (!dump_char(heap, '\0')) return 0;
				if (*source++ != ':') return 0;
				json_node name = alloc_string(heap, heap.m_dump);
				json_node child = load_dense(heap, source);
				if (!child) return 0;
				heap.m_values[child].name = name;
				heap.m_values[child].parent = node;
				heap.m_values[node].object_or_array.last = child;
				if (last)
					heap.m_values[last].next = child;
				else
					heap.m_values[node].object_or_array.first = child;
				last = child;
				json_char character = *source++;
				if (character == '}') return node;
				if (character != ',') return 0;
			}
		}
		//array
		if (*source == '[') {
			json_node node = heap.new_array();
			json_node last = 0;
			if (*++source == ']') {
				++source;
				return node;
			}
			while (true) {
				json_node child = load_dense(heap, source);
				if (!child) return 0;
				heap.m_values[child].parent = node;
				heap.m_values[node].object_or_array.last = child;
				if (last)
					heap.m_values[last].next = child;
				else
					heap.m_values[node].object_or_array.first = child;
				last = child;
				json_char character = *source++;
				if (character == ']') return node;
				if (character != ',') return 0;
			}
		}
		//null
		if (strncmp(source, "null", 4) == 0) {
			source += 4;
			return heap.new_null();
		}
		return 0;
	}
	inline static bool save_pretty(json_heap& heap, json_node source, size_t d = 0) noexcept {
		json_type type = heap.get_type(source);
		if (type == json_type_boolean) {
			if (heap.to_boolean(source))
				dump_string(heap, "true");
			else
				dump_string(heap, "false");
			return true;
		}
		if (type == json_type_number) {
			return dump_number(heap, heap.to_number(source));
		}
		if (type == json_type_string) {
			return dump_char(heap, '\"') && dump_string(heap, heap.to_string(source)) && dump_char(heap, '\"');
		}
		if (type == json_type_object) {
			if (!dump_char(heap, '{')) return false;
			json_node curr = heap.get_first(source);
			json_node last = heap.get_last(source);
			while (curr) {
				if (!dump_char(heap, '\n')) return false;
				if (!dump_pad(heap, d + 1)) return false;
				if (!dump_char(heap, '\"')) return false;
				if (!dump_string(heap, heap.get_name(curr))) return false;
				if (!dump_string(heap, "\": ")) return false;
				if (!save_pretty(heap, curr, d + 1)) return false;
				if (curr != last && !dump_char(heap, ',')) return false;
				curr = heap.get_next(curr);
			}
			if (last) {
				if (!dump_char(heap, '\n')) return false;
				if (!dump_pad(heap, d)) return false;
			}
			if (!dump_char(heap, '}')) return false;
			return true;
		}
		if (type == json_type_array) {
			if (!dump_char(heap, '[')) return false;
			json_node curr = heap.get_first(source);
			json_node last = heap.get_last(source);
			while (curr) {
				if (!dump_char(heap, '\n')) return false;
				if (!dump_pad(heap, d + 1)) return false;
				if (!save_pretty(heap, curr, d + 1)) return false;
				if (curr != last && !dump_char(heap, ',')) return false;
				curr = heap.get_next(curr);
			}
			if (last) {
				if (!dump_char(heap, '\n')) return false;
				if (!dump_pad(heap, d)) return false;
			}
			if (!dump_char(heap, ']')) return false;
			return true;
		}
		if (type == json_type_null) {
			return dump_string(heap, "null");
		}
		return false;
	}
	inline static bool save_dense(json_heap& heap, json_node source) noexcept {
		json_type type = heap.get_type(source);
		if (type == json_type_boolean) {
			if (heap.to_boolean(source))
				dump_string(heap, "true");
			else
				dump_string(heap, "false");
			return true;
		}
		if (type == json_type_number) {
			return dump_number(heap, heap.to_number(source));
		}
		if (type == json_type_string) {
			return dump_char(heap, '\"') && dump_string(heap, heap.to_string(source)) && dump_char(heap, '\"');
		}
		if (type == json_type_object) {
			if (!dump_char(heap, '{')) return false;
			json_node curr = heap.get_first(source);
			json_node last = heap.get_last(source);
			while (curr) {
				if (!dump_char(heap, '\"')) return false;
				if (!dump_string(heap, heap.get_name(curr))) return false;
				if (!dump_string(heap, "\":")) return false;
				if (!save_dense(heap, curr)) return false;
				if (curr != last && !dump_char(heap, ',')) return false;
				curr = heap.get_next(curr);
			}
			if (!dump_char(heap, '}')) return false;
			return true;
		}
		if (type == json_type_array) {
			if (!dump_char(heap, '[')) return false;
			json_node curr = heap.get_first(source);
			json_node last = heap.get_last(source);
			while (curr) {
				if (!save_dense(heap, curr)) return false;
				if (curr != last && !dump_char(heap, ',')) return false;
				curr = heap.get_next(curr);
			}
			if (!dump_char(heap, ']')) return false;
			return true;
		}
		if (type == json_type_null) {
			return dump_string(heap, "null");
		}
		return false;
	}
};

nix::json_heap::~json_heap() noexcept {
	free(m_values + 1);
	free(m_chars);
	free(m_dump);
}
nix::json_heap::json_heap() noexcept
	: m_values_capacity(0), m_values_count(0), m_values(((json_value*)(0)) - 1)
	, m_chars_capacity(0), m_chars_count(0), m_chars(0)
	, m_dump_capacity(0), m_dump_count(0), m_dump(0) {

}
nix::json_heap::json_heap(const json_heap& source) : json_heap() {
	//assign self and copy source.
	reserve_chars(source.m_chars_count);
	reserve_nodes(source.m_values_count);
	memcpy(m_chars, source.m_chars, source.m_chars_count * sizeof(json_char));
	memcpy(m_values + 1, source.m_values + 1, source.m_values_count * sizeof(json_value));
	m_chars_count = source.m_chars_count;
	m_values_count = source.m_values_count;
}
nix::json_heap::json_heap(json_heap&& source) noexcept {
	//assign self and clear source.
	m_values_capacity = source.m_values_capacity;
	m_values_count = source.m_values_count;
	m_values = source.m_values;
	source.m_values = ((json_value*)(0)) - 1;
	m_chars_capacity = source.m_chars_capacity;
	m_chars_count = source.m_chars_count;
	m_chars = source.m_chars;
	source.m_chars = 0;
	m_dump_capacity = source.m_dump_capacity;
	m_dump_count = source.m_dump_count;
	m_dump = source.m_dump;
	source.m_dump = 0;
}
nix::json_heap& nix::json_heap::operator=(const json_heap& source) {
	if (this == &source)
		return *this;
	//delete self.
	free(m_values + 1);
	free(m_chars);
	free(m_dump);
	clear();
	//assign self and copy source.
	reserve_chars(source.m_chars_count);
	reserve_nodes(source.m_values_count);
	memcpy(m_chars, source.m_chars, source.m_chars_count * sizeof(json_char));
	memcpy(m_values + 1, source.m_values + 1, source.m_values_count * sizeof(json_value));
	return *this;
}
nix::json_heap& nix::json_heap::operator=(json_heap&& source) noexcept {
	if (this == &source)
		return *this;
	//delete self.
	free(m_values + 1);
	free(m_chars);
	free(m_dump);
	//assign self and clear source.
	m_values_capacity = source.m_values_capacity;
	m_values_count = source.m_values_count;
	m_values = source.m_values;
	source.m_values = ((json_value*)(0)) - 1;
	m_chars_capacity = source.m_chars_capacity;
	m_chars_count = source.m_chars_count;
	m_chars = source.m_chars;
	source.m_chars = 0;
	m_dump_capacity = source.m_dump_capacity;
	m_dump_count = source.m_dump_count;
	m_dump = source.m_dump;
	source.m_dump = 0;
	return *this;
}

nix::json_node nix::json_heap::load_file(json_string source, json_format format) noexcept {
	std::ifstream stream(source);
	if (!stream.good())
		return 0;
	std::streampos end = stream.seekg(0, std::ios::end).tellg();
	stream.seekg(0, std::ios::beg);
	json_char* str = new json_char[end];
	stream.read(str, end);
	nix::json_node node = load_string(str, format);
	delete[] str;
	return node;
}
nix::json_node nix::json_heap::load_string(json_string source, json_format format) noexcept {
	if (format == json_format_pretty)
		return json_utils::load_pretty(*this, source);
	if (format == json_format_dense)
		return json_utils::load_dense(*this, source);
	return 0;
}
nix::json_string nix::json_heap::save_string(json_node source, json_format format) noexcept {
	json_utils::dump_clear(*this);
	if (format == json_format_pretty) {
		if (!json_utils::save_pretty(*this, source))
			return 0;
		if (!json_utils::dump_char(*this, '\0'))
			return 0;
		return m_dump;
	}
	if (format == json_format_dense) {
		if (!json_utils::save_dense(*this, source))
			return 0;
		if (!json_utils::dump_char(*this, '\0'))
			return 0;
		return m_dump;
	}
	return 0;
}

nix::json_node nix::json_heap::new_boolean(json_node object, json_string name, json_boolean value) noexcept {
	json_node node = json_utils::alloc_node(*this, object, json_utils::alloc_string(*this, name));
	if (!node) return 0;
	m_values[node].type = json_type_boolean;
	m_values[node].boolean.value = value;
	return node;
}
nix::json_node nix::json_heap::new_boolean(json_node array, json_boolean value) noexcept {
	json_node node = json_utils::alloc_node(*this, array);
	if (!node) return 0;
	m_values[node].type = json_type_boolean;
	m_values[node].boolean.value = value;
	return node;
}
nix::json_node nix::json_heap::new_boolean(json_boolean value) noexcept {
	json_node node = json_utils::alloc_node(*this);
	if (!node) return 0;
	m_values[node].type = json_type_boolean;
	m_values[node].boolean.value = value;
	return node;
}
nix::json_node nix::json_heap::new_number(json_node object, json_string name, json_number value) noexcept {
	json_node node = json_utils::alloc_node(*this, object, json_utils::alloc_string(*this, name));
	if (!node) return 0;
	m_values[node].type = json_type_number;
	m_values[node].number.value = value;
	return node;
}
nix::json_node nix::json_heap::new_number(json_node array, json_number value) noexcept {
	json_node node = json_utils::alloc_node(*this, array);
	if (!node) return 0;
	m_values[node].type = json_type_number;
	m_values[node].number.value = value;
	return node;
}
nix::json_node nix::json_heap::new_number(json_number value) noexcept {
	json_node node = json_utils::alloc_node(*this);
	if (!node) return 0;
	m_values[node].type = json_type_number;
	m_values[node].number.value = value;
	return node;
}
nix::json_node nix::json_heap::new_string(json_node object, json_string name, json_string value) noexcept {
	json_node node = json_utils::alloc_node(*this, object, json_utils::alloc_string(*this, name));
	if (!node) return 0;
	m_values[node].type = json_type_string;
	m_values[node].string.value = json_utils::alloc_string(*this, value);
	return node;
}
nix::json_node nix::json_heap::new_string(json_node array, json_string value) noexcept {
	json_node node = json_utils::alloc_node(*this, array);
	if (!node) return 0;
	m_values[node].type = json_type_string;
	m_values[node].string.value = json_utils::alloc_string(*this, value);
	return node;
}
nix::json_node nix::json_heap::new_string(json_string value) noexcept {
	json_node node = json_utils::alloc_node(*this);
	if (!node) return 0;
	m_values[node].type = json_type_string;
	m_values[node].string.value = json_utils::alloc_string(*this, value);
	return node;
}
nix::json_node nix::json_heap::new_object(json_node object, json_string name) noexcept {
	json_node node = json_utils::alloc_node(*this, object, json_utils::alloc_string(*this, name));
	if (!node) return 0;
	m_values[node].type = json_type_object;
	m_values[node].object_or_array = {};
	return node;
}
nix::json_node nix::json_heap::new_object(json_node array) noexcept {
	json_node node = json_utils::alloc_node(*this, array);
	if (!node) return 0;
	m_values[node].type = json_type_object;
	m_values[node].object_or_array = {};
	return node;
}
nix::json_node nix::json_heap::new_object() noexcept {
	json_node node = json_utils::alloc_node(*this);
	if (!node) return 0;
	m_values[node].type = json_type_object;
	m_values[node].object_or_array = {};
	return node;
}
nix::json_node nix::json_heap::new_array(json_node object, json_string name) noexcept {
	json_node node = json_utils::alloc_node(*this, object, json_utils::alloc_string(*this, name));
	if (!node) return 0;
	m_values[node].type = json_type_array;
	m_values[node].object_or_array = {};
	return node;
}
nix::json_node nix::json_heap::new_array(json_node array) noexcept {
	json_node node = json_utils::alloc_node(*this, array);
	if (!node) return 0;
	m_values[node].type = json_type_array;
	m_values[node].object_or_array = {};
	return node;
}
nix::json_node nix::json_heap::new_array() noexcept {
	json_node node = json_utils::alloc_node(*this);
	if (!node) return 0;
	m_values[node].type = json_type_array;
	m_values[node].object_or_array = {};
	return node;
}
nix::json_node nix::json_heap::new_null(json_node object, json_string name) noexcept {
	json_node node = json_utils::alloc_node(*this, object, json_utils::alloc_string(*this, name));
	if (!node) return 0;
	m_values[node].type = json_type_null;
	return node;
}
nix::json_node nix::json_heap::new_null(json_node array) noexcept {
	json_node node = json_utils::alloc_node(*this, array);
	if (!node) return 0;
	m_values[node].type = json_type_null;
	return node;
}
nix::json_node nix::json_heap::new_null() noexcept {
	json_node node = json_utils::alloc_node(*this);
	if (!node) return 0;
	m_values[node].type = json_type_null;
	return node;
}

nix::json_node nix::json_heap::get_child(json_node node, const json_char* name) const noexcept {
	node = get_first(node);
	while (node) {
		if (strcmp(name, get_name(node)) == 0)
			return node;
		node = get_next(node);
	}
	return 0;
}
nix::json_node nix::json_heap::get_first(json_node node) const noexcept {
	if (is_object(node) || is_array(node))
		return m_values[node].object_or_array.first;
	return 0;
}
nix::json_node nix::json_heap::get_next(json_node child) const noexcept {
	if (is_invalid(child))
		return 0;
	return m_values[child].next;
}
nix::json_node nix::json_heap::get_last(json_node node) const noexcept {
	if (is_object(node) || is_array(node))
		return m_values[node].object_or_array.last;
	return 0;
}
nix::json_type nix::json_heap::get_type(json_node node) const noexcept {
	if (is_invalid(node))
		return json_type_invalid;
	return m_values[node].type;
}
nix::json_string nix::json_heap::get_name(json_node node) const noexcept {
	if (is_invalid(node))
		return "invalid";
	json_node name = m_values[node].name;
	return name ? (m_chars + name) : "nameless";
}

nix::json_boolean nix::json_heap::to_boolean(json_node node, json_boolean fallback) const noexcept {
	if (is_boolean(node))
		return m_values[node].boolean.value;
	return fallback;
}
nix::json_boolean nix::json_heap::to_boolean(json_node node) const {
	if (!is_boolean(node)) {
		throw std::exception("[json error] expected boolean.");
	}
	return m_values[node].boolean.value;
}
nix::json_number nix::json_heap::to_number(json_node node, json_number fallback) const noexcept {
	if (is_number(node))
		return m_values[node].number.value;
	return fallback;
}
nix::json_number nix::json_heap::to_number(json_node node) const {
	if (!is_number(node)) {
		throw std::exception("[json error] expected number.\n");
	}
	return m_values[node].number.value;
}
nix::json_string nix::json_heap::to_string(json_node node, json_string fallback) const noexcept {
	if (is_string(node))
		return m_chars + m_values[node].string.value;
	return fallback;
}
nix::json_string nix::json_heap::to_string(json_node node) const {
	if (!is_string(node)) {
		throw std::exception("[json error] expected string.");
	}
	return m_chars + m_values[node].string.value;
}

bool nix::json_heap::is_invalid(json_node node) const noexcept {
	return (node == 0) || (node > m_values_count);
}
bool nix::json_heap::is_boolean(json_node node) const noexcept {
	if (is_invalid(node))
		return false;
	return m_values[node].type == json_type_boolean;
}
bool nix::json_heap::is_number(json_node node) const noexcept {
	if (is_invalid(node))
		return false;
	return m_values[node].type == json_type_number;
}
bool nix::json_heap::is_string(json_node node) const noexcept {
	if (is_invalid(node))
		return false;
	return m_values[node].type == json_type_string;
}
bool nix::json_heap::is_object(json_node node) const noexcept {
	if (is_invalid(node))
		return false;
	return m_values[node].type == json_type_object;
}
bool nix::json_heap::is_array(json_node node) const noexcept {
	if (is_invalid(node))
		return false;
	return m_values[node].type == json_type_array;
}
bool nix::json_heap::is_null(json_node node) const noexcept {
	if (is_invalid(node))
		return false;
	return m_values[node].type == json_type_null;
}

void nix::json_heap::clear() noexcept {
	m_values_count = 0u;
	m_chars_count = 0u;
}
bool nix::json_heap::reserve_chars() noexcept {
	return reserve_chars((m_chars_count + 2u) * 2u);
}
bool nix::json_heap::reserve_chars(uint32_t capacity) noexcept {
	uint32_t ncapacity = m_chars_count + capacity;
	if (ncapacity > m_chars_capacity) {
		json_char* nchars = (json_char*)malloc(ncapacity * sizeof(json_char));
		if (!nchars)
			return false;
		memcpy(nchars, m_chars, m_chars_count * sizeof(json_char));
		free(m_chars);
		m_chars = nchars;
		m_chars_capacity = ncapacity;
	}
	return true;
}
bool nix::json_heap::reserve_nodes() noexcept {
	return reserve_nodes((m_values_count + 2u) * 2u);
}
bool nix::json_heap::reserve_nodes(uint32_t capacity) noexcept {
	uint32_t ncapacity = m_values_count + capacity;
	if (ncapacity > m_values_capacity) {
		json_value* nvalues = (json_value*)malloc(ncapacity * sizeof(json_value));
		if (!nvalues)
			return false;
		memcpy(nvalues, m_values + 1, m_values_count * sizeof(json_value));
		free(m_values + 1);
		m_values = nvalues - 1;
		m_values_capacity = ncapacity;
	}
	return true;
}
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
