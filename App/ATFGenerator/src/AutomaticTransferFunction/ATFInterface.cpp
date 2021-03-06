/// ATFInterface.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ATFInterface.h"
#include "../Viewer.h"

#include <string>
#include <stdio.h>
#include <stdlib.h>

ATFInterface* ATFInterface::S_CURRENT = NULL;

ATFInterface::ATFInterface()
: m_menu(NULL)
, m_option_designer(NULL)
{
}

ATFInterface* ATFInterface::Instance()
{
  if (!S_CURRENT)
    S_CURRENT = new ATFInterface();

  return S_CURRENT;
}

void ATFInterface::Build()
{
  m_option_designer = IupItem("Alterar Fun��o", NULL);
  IupSetAttribute(m_option_designer, "ACTIVE", "YES");

  IupSetCallback(m_option_designer, "ACTION", (Icallback)ATFInterface::OpenDesignerDialog);

  m_menu = IupSubmenu("ATFG", IupMenu(m_option_designer, NULL));
}

int ATFInterface::OpenDesignerDialog(Ihandle* ih)
{
  //ATFInterface* current = ATFInterface::Instance();

  //Ihandle *dlg = IupDialog(IupHbox(bar, NULL));
  //IupSetAttribute(dlg, "TITLE", "Function Designer");

  //IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);
  return IUP_DEFAULT;
}

int ATFInterface::Value(Ihandle* ih)
{
  return IUP_DEFAULT;
}