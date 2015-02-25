#include <ssengine/uri.h>

#include <regex>
#include <sstream>

//(?:(?<protocol>http(?:s?)|ftp)(?:\:\/\/)) 
// (?:(?<usrpwd>\w+\:\w+)(?:\@))? 
// (?<domain>[^/\r\n\:]+)? (?<port>\:\d+)? 
// (?<path>(?:\/.*)*\/)? 
// (?<filename>.*?\.(?<ext>\w{2,4}))? 
// (?<qrystr>\??(?:\w+\=[^\#]+)(?:\&?\w+\=\w+)*)* (?<bkmrk>\#.*)?

static std::regex reg(
	"(?:(\\w+)\\:\\/\\/"			//schema
	"(?:"
	"(?:(\\w+)(?:\\:(\\w+))?\\@)?" 	//user & password
	"([\\w\\.]+)(?:\\:(\\d+))?)?"	//host & port
	"(?=\\/))?"
	"([^\\?\\#]+)"					//path
	"(?:\\?([^\\#]+))?"				//search
	"(?:\\#+(.+))?");				//tag

/*static*/ ss_uri ss_uri::parse(ss_core_context* C, const std::string& uri){
	std::cmatch match;
	if (!std::regex_match(uri.c_str(), uri.c_str() + uri.length(), match, reg)){
		return ss_uri(C);
	}

	ss_uri ret(C);
	ret.schema = match[1].str();
	ret.user = match[2].str();
	ret.password = match[3].str();
	ret.host = match[4].str();
	ret.port = atoi(match[5].str().c_str());
	ret.path = match[6];
	ret.search = match[7];
	ret.tag = match[8];

	return ret;
}

#if WIN32
#include <Shlwapi.h>
#include <Wininet.h>

#pragma comment(lib, "Shlwapi.lib")

ss_uri ss_uri::from_file(ss_core_context* C,  const char* path){
	wchar_t* wPath = char2wchar_t(path);

	wchar_t wOutPath[MAX_PATH + 1];

	wchar_t sUrl[INTERNET_MAX_URL_LENGTH + 1];
	DWORD sz = INTERNET_MAX_URL_LENGTH + 1;

	if (!PathIsNetworkPathW(wPath) && PathGetDriveNumberW(wPath) < 0){
		GetCurrentDirectoryW(MAX_PATH + 1, wOutPath);
		PathAppendW(wOutPath, wPath);
		UrlCreateFromPathW(wOutPath, sUrl, &sz, NULL);
	}
	else {
		UrlCreateFromPathW(wPath, sUrl, &sz, NULL);
	}

	delete[] wPath;
	return ss_uri::parse(C, wstring2string(sUrl));
}
#else
/*static*/ ss_uri ss_uri::from_file(ss_core_context* C, const char* path){
	ss_uri ret;

	bool isAbsolute = (path[0] == '/');

	return ret;
}
#endif

ss_uri ss_uri::join(const ss_uri& other) const{

	if (!other.schema.empty()){
		// absolute path
		return other;
	}

	ss_uri ret = (*this);

	ret.search = other.search;
	ret.tag = other.tag;

	if (!other.path.empty() && other.path[0] == '/'){
		ret.path = other.path;
	}
	else {
		ret.path = ret.path + "/" + other.path;
	}
	return ret;
}

void ss_uri::normalize(){
	std::vector<std::string> parts;

	size_t begin = 0;

	bool isAbsolute = (!schema.empty()) || (path.length() > 0 && path[0] == '/');

	for (;;){
		size_t pos = path.find('/', begin);

		std::string part = path.substr(begin, pos - begin);
		if (part == "" || part == "."){
		}
		else if (part == ".."){
			if (parts.size() > 0){
				parts.pop_back();
			}
			else if (isAbsolute){
				// Ignore leading `..` if it's a absolute path.
			}
			else {
				parts.push_back(part);
			}
		}
		else {
			parts.push_back(part);
		}

		if (pos == std::string::npos){
			break;
		}
		begin = pos + 1;
	}

	std::string ret = isAbsolute ? "/" : "";
	auto it = parts.begin();
	ret.append(*it);
	for (++it; it != parts.end(); ++it){
		ret.append("/");
		ret.append(*it);
	}

	if (ret.length() == 0){
		this->path = ".";
	}
	else {
		this->path = ret;
	}
}

ss_uri ss_uri::base_dir() const{
	ss_uri ret = (*this);
	ret.normalize();

	size_t pos = ret.path.rfind("/");
	if (pos == std::string::npos){
		return (*this);
	}

	if (pos == 0){
		ret.path == "/";
	}
	else {
		ret.path = ret.path.substr(0, pos);
	}

	return ret;
}

std::string ss_uri::str() const{
	std::ostringstream out;

	if (!schema.empty()){
		out << schema << "://";
		if (!host.empty()){
			if (!user.empty()){
				out << user;
				if (!password.empty()){
					out << ":" << password;
				}
				out << "@";
			}
			out << host;
			if (port > 0){
				out << ":" << port;
			}
		}
	}
	out << path;

	if (!search.empty()){
		out << "?" << search;
	}
	if (!tag.empty()){
		out << "#" << tag;
	}

	return out.str();
}