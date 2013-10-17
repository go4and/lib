/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace mptree {

class node;

void parse_xml(node & root, char * data, const char * root_name);
void write_xml(std::ostream & out, const node & root, const char * root_name);

}
