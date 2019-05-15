//
// Created by test2 on 19-5-14.
//

#ifndef MUDUO_SINGLETON_H
#define MUDUO_SINGLETON_H
#include <mutex>

template<typename T>
class Singleton
{
public:
    static T& Instance() {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance();
        }
        return *(Singleton::s_instance);
    }

    static T* GetInstance() {
        std::mutex objMutex;
        if(Singleton::s_instance==0) {
            objMutex.lock();
            if (Singleton::s_instance == 0) {
                Singleton::s_instance = CreateInstance();
            }
            objMutex.unlock();
        }
        return Singleton::s_instance;
    }

    static void Destroy() {
        if(Singleton::s_instance!=0) {
            DestroyInstance(Singleton::s_instance);
            Singleton::s_instance=0;
        }
    }

protected:
    Singleton()	{
        Singleton::s_instance = static_cast<T*>(this);
    }

    ~Singleton() {
        Singleton::s_instance = 0;
    }



private:
    static T* CreateInstance(){
        return new T();
    }

    static void DestroyInstance(T* p) {
        delete p;
    }


private:
    explicit Singleton(Singleton const&) {}
    Singleton&operator = (Singleton const&) {return *this;}

private:
    static T* s_instance;
};

template <typename T>
T* Singleton<T>::s_instance = NULL;

#endif //MUDUO_SINGLETON_H
