#ifndef ATF_INTERFACE_H
#define ATF_INTERFACE_H

#include <iup.h>
#include <iupgl.h>

class ATFInterface
{
public:
  ATFInterface();

  void Build();
  static ATFInterface* Instance();
  
  static int OpenDesignerDialog(Ihandle* ih);
  static int Value(Ihandle* ih);

  Ihandle* GetMenu()
  {
    return m_menu;
  }

  Ihandle* GetDesignerOption()
  {
    return m_option_designer;
  }


private:
  static ATFInterface* S_CURRENT;
  Ihandle* m_menu;
  Ihandle* m_option_designer;
};

#endif