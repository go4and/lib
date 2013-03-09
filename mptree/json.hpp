#pragma once

namespace mptree {

class node;

void parse_json(node & root, const char * data, size_t len);
void write_json(std::ostream & out, const node & root);

}
