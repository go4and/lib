#pragma once

namespace marc {

bool untar(const boost::filesystem::wpath & source, const boost::filesystem::wpath & destFolder);
bool readFile(const boost::filesystem::wpath & input, std::vector<char> & out);

}
