#pragma once
#include <string>
#include <unordered_map>

#define BOAT(type)                                                       \
public:                                                                    \
    static void Register(const std::string& name, type * boat) \
    {                                                                      \
        s_Boats.emplace(name, boat);                                   \
    }                                                                      \
    static void Unregister(const std::string& name)                        \
    {                                                                      \
        if (s_Boats.find(name) != s_Boats.end())                       \
            s_Boats.erase(name);                                         \
    }                                                                      \
    static type* Get(const std::string& name)                   \
    {                                                                      \
        if (s_Boats.empty())                                             \
            return nullptr;                                                \
        return s_Boats[name];                                            \
    }                                                                      \
    static type* Get()                                          \
    {                                                                      \
        if (s_Boats.empty())                                             \
            return nullptr;  												\
		if (s_PreferredBoat) return s_PreferredBoat;                       \
        return s_Boats.begin()->second;                                   \
    }                                                                    \
                                                                         \
   static void ClearBoats(){                                             \
        s_PreferredBoat = nullptr;                                                         \
        for(auto& boat : s_Boats){                                       \
        	delete boat.second; \
		}                                                                 \
   		s_Boats.clear();                                                 \
   }                                                                      \
   static bool SetPreferredBoat(const std::string& name){                \
        if (s_Boats.find(name) == s_Boats.end())                       \
            return false;                                                      \
   		s_PreferredBoat = s_Boats[name];                                    \
        return true;                                                    \
   }                                                                       \
    static std::vector<std::string> GetBoatNames()                         \
    {                                                                      \
        std::vector<std::string> names;                                   \
        for (auto& boat : s_Boats)                                        \
            names.push_back(boat.first);                                  \
        return names;                                                      \
    }                                                                      \
private:                                                                   \
    static std::unordered_map<std::string, type*> s_Boats;  \
	static type* s_PreferredBoat;


#define BOAT_IMPL(type) \
std::unordered_map<std::string, type*> type::s_Boats; \
type* type::s_PreferredBoat;
