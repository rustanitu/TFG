#ifndef LQC_FILE_PVM_H
#define LQC_FILE_PVM_H

#include <lqc/lqcdefines.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <fstream>

namespace lqc
{
  class PVMLoader
  {
  public:
    PVMLoader (std::string fileName)
    {
      m_pvm_types[0] = "PVM";
      m_pvm_types[1] = "PVM2";
      m_pvm_types[2] = "PVM3";
      m_pvm_types[3] = "DDS v3d";
      m_pvm_types[4] = "DDS v3e";

      std::string line;
      std::ifstream myfile (fileName + ".pvm");
      if (myfile.is_open ())
      {
        getline (myfile, line);
        for (int i = 0; i < PVM_TYPES; i++)
        {
          if (m_pvm_types[i].compare (line) == 0)
          {
            m_pvm_type = i;
            break;
          }
        }
        std::cout << "PVM type: " << m_pvm_types[m_pvm_type] << std::endl;

        getline (myfile, line);
        std::cout << "line2: " << line << std::endl;

        getline (myfile, line);
        std::cout << "line3: " << line << std::endl;

        myfile.close ();
      }
      else
      {
        std::cout << "Unable to open file";
        return;
      }
    }

    ~PVMLoader ()
    {

    }

    void* GetData ()
    {
      return NULL;
    }

    bool IsLoaded ()
    {
    }

  private:
    std::string m_filename;
    int m_pvm_type;
    const int PVM_TYPES = 5;
    std::string m_pvm_types[5];
  };
}

#endif
