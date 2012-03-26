#ifndef REGISTRY_H_
#define REGISTRY_H_

#include <algorithm>
#include <functional>
#include <map>

// So basically, we have a std::vector of std::pair<index,value> sorted by
// (x.first>y.first), which lets us use STL algorithms to do all of our work.

//The int type should have a strict total ordering (both numbers and strings
//have this)
class registry
{
public:
	//TODO: sort assigned values
	registry() = default;
	registry(const registry &) = default;
	registry(registry &&) = default;
	registry(std::initializer_list<std::pair<const int, std::string>> i) : registrations(i) {}
	registry &operator=(const registry &) = default;
	registry &operator=(registry &&) = default;

	bool empty() const {return registrations.empty();}
	size_t size() const {return registrations.size();}

	void reset() {clear();}
	void clear() {registrations.clear();}

	bool add(int i, const std::string &s)
	{
		//auto pos=registrations.emplace(i, s);
		auto pos=registrations.insert({i, s});
		return pos.second;
	}

	std::pair<int, std::string> get(int i)
	{
		auto pos=registrations.lower_bound(i);
		if (pos==registrations.end())
			return {-32767,""};
		return *pos;
	}

	void swap(registry &r) {registrations.swap(r.registrations);}

private:
	std::map<int, std::string, std::greater<int>> registrations;
};

#endif
