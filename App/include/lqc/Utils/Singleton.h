#ifndef LQC_UTILS_SINGLETON_H
#define LQC_UTILS_SINGLETON_H
#include <stddef.h>  // defines NULL

namespace lqc
{
  /*template <class T>
  class Singleton
  {
  public:
    static T* Instance () {
      if (!m_pInstance) m_pInstance = new T;
      assert (m_pInstance != NULL);
      return m_pInstance;
    }
  protected:
    Singleton () {}
    ~Singleton () {}
  private:
    Singleton (Singleton const& t) { m_pInstance = t; }
    Singleton& operator=(Singleton const&) { m_pInstance = t; }
    static T* m_pInstance;
  };
  template <class T> T* Singleton<T>::m_pInstance = NULL;*/
}

#endif