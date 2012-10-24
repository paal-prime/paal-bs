//Grzegorz Prusak
#ifndef format_H
#define format_H

#include <vector>
#include <sstream>

namespace _format
{
	typedef std::vector<std::stringstream> VSS;
	inline void push(VSS &vss, size_t i){}
	template<typename T, typename ...Args>
		inline void push(VSS &vss, size_t i, const T &v, Args ...args)
	{ vss[i] << v; push(vss, i+1, args...); }

	template<typename ...Args>
		inline std::string format(std::string format_str, Args ...args)
	{
		VSS vss(sizeof...(Args)); push(vss, 0, args...);
		std::stringstream ss; int i = 0;
		for(char c : format_str) if(c!='%') ss << c;
			else ss << vss.at(i++).str();
		return ss.str();
	}
}

using _format::format;

#endif
