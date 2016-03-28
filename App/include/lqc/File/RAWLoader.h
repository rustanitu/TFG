#ifndef LQC_FILE_RAW_H
#define LQC_FILE_RAW_H

#include <lqc/lqcdefines.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace lqc
{
  class IRAWLoader
  {
  public:
    IRAWLoader (std::string fileName, size_t bytes_per_pixel, size_t num_voxels, size_t type_size);
    ~IRAWLoader ();

    void* GetData ();
    bool IsLoaded ();
  private:
    std::string m_filename;
    size_t m_bytesperpixel;
    size_t m_numvoxels;
    size_t m_typesize;
    unsigned char* m_data;
  };
}

#endif