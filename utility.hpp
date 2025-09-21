// miscellaneous bits and bobs used for other parts
// type_<T,...> sor SFINAE
// stringpool

#pragma once
#ifndef SHEEN_UTILITY
#define SHEEN_UTILITY

#include <string>
#include <unordered_set>
#include <mutex>
#include <vector>
#include <initializer_list>

#include <type_traits>
//#include <iostream>

namespace sheen{

    template <typename T,typename... Enable> using type_ = T;

    
    struct stringpool {
    public:
        // intern a string. If it's already in the pool, return existing pointer.
        const std::string* intern(const std::string& str);
        // intern multiple strings, return vector of string pointers
        std::vector<const std::string*> intern(std::initializer_list<const std::string> stringlist);
        // intern multiple strings from std compatible container, return vector of string pointers
        template<typename C>
        auto intern(const C& strings) -> type_<
                std::vector<const std::string*>,
                decltype(std::declval<C>().begin()),
                decltype(std::declval<C>().end()),
                decltype(std::declval<C>().size())>;
        // unintern(remove) a string
        void stringpool::unintern(const std::string& str);
        // unintern(remove) multiple strings from std compatible container
        template<typename C>
        auto unintern(const C& strings) -> type_<
                    void,
                    decltype(std::declval<C>().begin()),
                    decltype(std::declval<C>().end())>;
        // clear entire string pool
        void clear();
        
    protected:
        std::unordered_set<std::string> set_;
        std::mutex mutex_; // thread safety
    };


    const std::string* stringpool::intern(const std::string& str) {
        std::lock_guard<std::mutex> lock(mutex_);
        // C++ 17 structed bindings
        auto [it, inserted] = set_.emplace(str);
        return &(*it);
    }
    
    std::vector<const std::string*> stringpool::intern(std::initializer_list<const std::string> stringlist){
        std::vector<const std::string*> ret_vec(stringlist.size());
        std::lock_guard<std::mutex> lock(mutex_);
        auto i = 0;
        for(const std::string str:stringlist){
            auto [it, inserted] = set_.emplace(str);
            ret_vec[i] = &(*it);
            ++i;
        }
        return ret_vec;
    }
    
    template<typename C>
    auto stringpool::intern(const C& strings) -> type_<
                std::vector<const std::string*>,
                decltype(std::declval<C>().begin()),
                decltype(std::declval<C>().end()),
                decltype(std::declval<C>().size())>
    {
        std::vector<const std::string*> ret_vec(strings.size());
        std::lock_guard<std::mutex> lock(mutex_);
        auto i = 0;
        for(const std::string str:strings){
            auto [it, inserted] = set_.emplace(str);
            ret_vec[i] = &(*it);
            ++i;
        }
        return ret_vec;
    }


    void stringpool::unintern(const std::string& str) {
        std::lock_guard<std::mutex> lock(mutex_);
        set_.erase(str);
    }

    // intern multiple strings, return vector of string pointers
    template<typename C>
    auto stringpool::unintern(const C& strings) -> type_<
                void,
                decltype(std::declval<C>().begin()),
                decltype(std::declval<C>().end())>
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(const std::string str:strings){
            set_.erase(str);
        }
    }

    void stringpool::clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        set_.clear();
    }


};
#endif