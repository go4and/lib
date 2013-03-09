#pragma once

namespace mptree {

class node;

void parse_xml(node & root, char * data, const char * root_name);
void write_xml(std::ostream & out, const node & root, const char * root_name);

}
