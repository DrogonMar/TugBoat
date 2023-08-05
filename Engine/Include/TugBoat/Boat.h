#pragma once
#include <string>
#include <unordered_map>

#define BOAT(type)                                                       \
public:                                                                    \
    static void Register(const std::string& name, TugBoat::Ref<type> boat) \
    {                                                                      \
        s_Boats.emplace(name, boat);                                   \
    }                                                                      \
    static void Unregister(const std::string& name)                        \
    {                                                                      \
        if (s_Boats.find(name) != s_Boats.end())                       \
            s_Boats.erase(name);                                         \
    }                                                                      \
    static TugBoat::Ref<type> Get(const std::string& name)                   \
    {                                                                      \
        if (s_Boats.empty())                                             \
            return nullptr;                                                \
        return s_Boats[name];                                            \
    }                                                                      \
    static TugBoat::Ref<type> Get()                                          \
    {                                                                      \
        if (s_Boats.empty())                                             \
            return nullptr;                                                \
        return s_Boats.begin()->second;                                  \
    }                                                                    \
                                                                         \
   static void ClearBoats(){                                             \
   		s_Boats.clear();                                                 \
   }                                                                      \
                                                                           \
private:                                                                   \
    static std::unordered_map<std::string, TugBoat::Ref<type>> s_Boats;

#define BOAT_IMPL(type) std::unordered_map<std::string, TugBoat::Ref<type>> type::s_Boats;