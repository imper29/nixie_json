#include "nixie_encoding/json.hpp"
#include "iostream"

int main(int argc, const char* argv[]) {
	//the json heap manages all memory internally. just don't try to access json_nodes created from other json_heaps.
	nix::json_heap json;

	//load and print main json file.
	nix::json_node root = json.load_file("examples/main.json");
	printf("[main.json]\n%s\n", json.save_string(root));

	//load and print argv json files.
	for (int i = 0; i < argc; ++i) {
		nix::json_node curr = json.load_file(argv[i]);
		printf("\n[%s]\n%s\n", argv[i], json.save_string(curr));
	}

	//access to json elements.
	printf("\n[access json elements]\n");
	if (nix::json_node player = json.get_child(root, "player")) {
		if (nix::json_node name = json.get_child(player, "name")) {
			printf("player name = %s\n", json.to_string(name));
		}
		if (nix::json_node items = json.get_child(player, "items")) {
			nix::json_node current = json.get_first(items);//get first element in object or array.
			int index = 0;
			while (current) {
				printf("player item %i = %s\n", index++, json.to_string(current, "fallback value because of invalid input"));
				current = json.get_next(current);//get next element in object or array.
			}
		}
	}

	//access to json elements that are missing is no problem as long as you provide a fallback value!
	printf("\n[access invalid json elements]\n");
	printf("invalid age = %f\n", json.to_number(json.get_child(json.get_child(root, "invalid"), "age"), 42));

	//generate json elements.
	printf("\n[generate json elements]\n");
	nix::json_node gen = json.new_object();
	nix::json_node child = json.new_object(gen, "child");
	json.new_string(child, "name", "steve");
	nix::json_node grandchild = json.new_object(child, "grandchild");
	json.new_string(grandchild, "name", "mike");
	printf("%s\n", json.save_string(gen));

	//json heaps can be copied; the nodes remain valid in both copies.
	nix::json_heap copy = json;
	//json heaps can be movied; the nodes remain valid in the moved heap.
	nix::json_heap move = std::move(json);

	//you should clear long-lived json heaps because nodes and are only deallocated when the heap is destroyed or cleared.
	//just remember not to use strings or nodes from the heap that were generated prior to being cleared.
	//heaps are automatically cleared when they are destroyed. you don't NEED to call this function.
	move.clear();

	//showing that the nodes are still valid in the copy heap despite the original heap being cleared.
	printf("\n[access json elements from copy]\n");
	if (nix::json_node player = copy.get_child(root, "player")) {
		if (nix::json_node name = copy.get_child(player, "name")) {
			printf("player name = %s\n", copy.to_string(name));
		}
		if (nix::json_node items = copy.get_child(player, "items")) {
			nix::json_node current = copy.get_first(items);//get first element in object or array.
			int index = 0;
			while (current) {
				printf("player item %i = %s\n", index++, copy.to_string(current, "fallback value because of invalid input"));
				current = copy.get_next(current);//get next element in object or array.
			}
		}
	}


	return 0;
}
