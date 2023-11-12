#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>

#include <vector>
#include <functional>

namespace TugBoat {
    template <typename... Args>
    class TB_API Event {
        typedef std::function<void(Args...)> EventCallback;
        
    public:
        inline TB_API void Register(const EventCallback& callback){
            m_Callbacks.emplace_back(callback);
        }

        template <typename T>
        inline void Register(void (T::*memberFunction)(Args...), T* object) {
            Register([=](Args... args) {
                (object->*memberFunction)(args...);
            });
        }
        
        TB_API inline void Remove(const EventCallback& callback){
            m_Callbacks.erase(std::find(m_Callbacks.begin(), m_Callbacks.end(), callback));
        }
        
        TB_API inline void Invoke(Args... args){
            for(auto& callback : m_Callbacks){
                callback(args...);
            }
        }
    private:
        LOG("Event");
        std::vector<EventCallback> m_Callbacks;
    };
}