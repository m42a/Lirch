#ifndef REGISTRY_H_
#define REGISTRY_H_

#include <algorithm>
#include <functional>
#include <map>

class registry
{
public:
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
		//Fail if we get an out-of-bounds priority
		if (i>32766 || i<-32766)
			return false;
		//auto pos=registrations.emplace(i, s);
		auto pos=registrations.insert({i, s});
		return pos.second;
	}

	std::pair<int, std::string> get(int i) const
	{
		auto pos=registrations.lower_bound(i);
		if (pos==registrations.end())
			return {-32767,""};
		return *pos;
	}

	void removeall(std::string name)
	{
		decltype(registrations.begin()) i;
		while ((i=std::find_if(registrations.begin(), registrations.end(), [name](const std::pair<int, std::string> &p) {return p.second==name;}))!=registrations.end())
			registrations.erase(i);

	}

	void swap(registry &r) {registrations.swap(r.registrations);}

private:
	std::map<int, std::string, std::greater<int>> registrations;
};

#endif
